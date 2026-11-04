// Horse-relative riding controls: on ground mounts, replaces the camera direction vectors in
// ClientInputActorComponent's per-frame update with the mount's, so move input is resolved against the mount's facing.
// Also widens and ramps in forward steering angles, and removes backward input, which leaves S with a steering input as
// a pivot. On the horse, S also walks it backward from a near stop. All of this stops while the taming minigame runs,
// because it needs the game's own input directions.
//
// F5        toggle the mod
// Shift+F5  flip the forward direction

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <string>
#include <unordered_map>
#include <vector>

#include "safetyhook.hpp"

namespace {

constexpr const char* kVersion = "0.0.1";

// A code signature with ?? for bytes that vary, and the target's distance from its first byte, which must be literal.
// tools/check_lookups.py runs the same lookups on saved exes, so change both together.
struct Signature {
    const char* text;
    int distance;
};

// Inside ClientInputActorComponent's update, right after it copies the camera vectors to +0x154 and +0x160.
constexpr Signature kHookSiteSignature{
    "4C 8D B7 60 01 00 00 C5 F0 14 C0 C4 C1 7B 11 06 C4 C1 7A 11 5E 08 C5 FB 11 A7 54 01 00 00 89 8F 5C 01 00 00 "
    "49 8B 40 20 48 8B 88 30 12 00 00 48 8B 99 90 00 00 00",
    36};
// In the same update, right before it reads the raw stick from +0x48.
constexpr Signature kStickSiteSignature{
    "FF 50 48 48 8B 08 48 89 4F 48 48 8D 77 50 48 8B 47 48 48 89 06 C5 FA 10 0E C5 FA 10 46 04", 10};
// The chart's key condition check, which reads the key's hash from its condition. The signature starts 14 bytes in,
// because another mod hooks the function's first bytes at runtime.
constexpr Signature kKeyCheckSignature{
    "48 8B 48 68 48 8B 81 B0 01 00 00 80 B8 79 01 00 00 00 75 ?? 48 8B 05 ?? ?? ?? ?? 48 8B 88 D8 00 00 00 "
    "48 8B 41 20 83 78 28 00",
    -14};
constexpr uint8_t kKeyCheckBytes[] = {0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0x02, 0x8B, 0x50, 0x08};
// mov rax,[rip+x]; mov rcx,[rax+0x30], which loads the WorldSystem global far more often than any other global.
constexpr const char* kWorldReferenceSignature = "48 8B 05 ?? ?? ?? ?? 48 8B 48 30";
constexpr int kWorldMinRatio = 3;
// Class names in RTTI for the vtables.
constexpr const char* kClassActorManager = ".?AVClientActorManager@pa@@";
constexpr const char* kClassVehicleComponent = ".?AVClientVehicleActorComponent@pa@@";
constexpr const char* kClassRodeoUi = ".?AVUIGamePlayControlRootMiniGameRodeo@uiCommonScript@pa@@";
// The slot of UIGamePlayControlRootMiniGameRodeo's vtable that the game calls every frame while the minigame is open and
// at no other time.
constexpr size_t kRodeoUiFrameSlot = 47;
constexpr ULONGLONG kRodeoTimeoutMs = 250;

constexpr uint32_t kInput_Actor = 0x08;
constexpr uint32_t kInput_Stick = 0x38;
constexpr uint32_t kInput_RawStick = 0x48;
constexpr uint32_t kInput_MoveAngle = 0x88;
constexpr uint32_t kInput_MoveAngleRelative = 0x8C;
constexpr uint32_t kInput_CameraForward = 0x154;
constexpr uint32_t kInput_CameraRight = 0x160;

// Land mounts' packages. Water mounts, the dragon and machines are left out.
constexpr const char* kGroundMountPackages[] = {
    "CD_R0001_Horse", "CD_M0002_Camel", "CD_M0002_Reindeer", "CD_M0002_Elephant",  "CD_M0002_BattleBoar",
    "CD_M0002_Bear",  "CD_M0002_Wolf",  "CD_M0002_Wolf_Elite", "CD_M0002_Wolf_Elite_Big", "CD_M0002_Wolf_Boss",
    "CD_R0034_Iguana", "CD_R0035_CarmaBirdsaurus", "CD_M0003_CucuBird",
};

// Forward steering multiplies W+A's 45 degree stick angle by these, blending between the two speeds in m/s.
constexpr float kTightSteerFactor = 80.0f / 45.0f;
constexpr float kWideSteerFactor = 55.0f / 45.0f;
constexpr float kTightSteerSpeed = 2.5f;
constexpr float kWideSteerSpeed = 5.0f;
constexpr float kMaxSteerAngle = 85.0f / 57.29578f;
constexpr float kSteerRampMs = 300.0f;
// Added to the horse's position per second during the backward walk. It moves at about twice this, roughly 1 m/s.
constexpr float kBackwardMoveRate = 0.5f;
// Reads as forward input for this long, so that the horse leaves idle for a state that uses the walking blend.
constexpr int kBackwardPulseMs = 150;
// The backward walk starts once the horse is below this speed in m/s, which is near the end of a stop.
constexpr float kBackwardMaxStartSpeed = 0.2f;
// A full steering input leaves the walking blend for a forward walk. This size stays in the blend and curves the path.
constexpr float kBackwardSteer = 0.56f;
// Low 16 bits of the horse's chart state pointer during the walk, which stay the same between sessions.
constexpr uint16_t kStateWalkStart = 0xA37D;
constexpr uint16_t kStateWalk = 0xA73F;
constexpr uint16_t kStateWalkSteer = 0xA56E;
// Minimum time between pulses that return the horse to the walk state, in case a pulse doesn't get it there.
constexpr ULONGLONG kBackwardRecoveryGapMs = 1000;
constexpr const char* kHorsePackage = "CD_R0001_Horse";

constexpr float kMissing = -9999.0f;
constexpr float kRadToDeg = 57.29578f;

struct Vec3 {
    float x, y, z;
};

struct Ride {
    uintptr_t rider = 0;
    uintptr_t mount = 0;
    uint32_t mountKey = 0;
    bool mounted = false;
    bool haveForward = false;
    bool forwardFromMount = false;
    Vec3 axisZ{};
};

// Latest values from one input component, for logging.
struct InputSample {
    std::atomic<uintptr_t> actor{0};
    std::atomic<float> stickX{0}, stickY{0}, angle{0}, relative{0}, cameraYaw{kMissing}, targetYaw{kMissing};
    std::atomic<bool> applied{false};
};

FILE* g_log = nullptr;
FILE* g_csv = nullptr;
uintptr_t g_base = 0;
// Offsets from g_base, set by RunLookups before the hooks are installed.
uintptr_t g_worldSystemSlot = 0;
uintptr_t g_vtActorManager = 0;
uintptr_t g_vtVehicleComponent = 0;
uintptr_t g_vtRodeoUi = 0;
uintptr_t g_hookSite = 0;
uintptr_t g_stickSite = 0;
uintptr_t g_keyCheck = 0;
std::atomic<bool> g_enabled{true};
std::atomic<bool> g_flip{false};
InputSample g_samples[2];
SafetyHookMid g_hook;
SafetyHookMid g_stickHook;
std::atomic<uintptr_t> g_groundMount{0};
std::atomic<float> g_mountSpeed{0};
std::atomic<int> g_steerSide{0};
std::atomic<ULONGLONG> g_steerStart{0};
volatile uint64_t* g_rodeoCalls = nullptr;
std::atomic<uintptr_t> g_horseMount{0};
std::atomic<ULONGLONG> g_backwardPulseEnd{0};
std::atomic<bool> g_backwardWanted{false};
std::atomic<bool> g_backwardWalking{false};

void Log(const char* fmt, ...) {
    if (!g_log) return;
    va_list args;
    va_start(args, fmt);
    SYSTEMTIME t;
    GetLocalTime(&t);
    fprintf(g_log, "[%02d:%02d:%02d.%03d] ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    vfprintf(g_log, fmt, args);
    fputc('\n', g_log);
    fflush(g_log);
    va_end(args);
}

bool SafeRead(uintptr_t addr, void* out, size_t size) {
    __try {
        memcpy(out, reinterpret_cast<const void*>(addr), size);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool SafeWrite(uintptr_t addr, const void* in, size_t size) {
    __try {
        memcpy(reinterpret_cast<void*>(addr), in, size);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

template <typename T>
bool Read(uintptr_t addr, T& out) {
    return SafeRead(addr, &out, sizeof(T));
}

uintptr_t ReadPtr(uintptr_t addr) {
    uintptr_t p = 0;
    if (!Read(addr, p) || p < 0x10000 || p >= 0x7FFFFFFFFFFF) return 0;
    return p;
}

bool Finite(const Vec3& v) { return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z); }

float YawDeg(float x, float z) { return std::atan2(x, z) * kRadToDeg; }

bool HasVtable(uintptr_t obj, uintptr_t rva) {
    uintptr_t vt = 0;
    return Read(obj, vt) && vt == g_base + rva;
}

// Return the actor for `key` from the ClientActorManager hash table, or 0.
uintptr_t FindActor(uintptr_t manager, uint32_t key) {
    const uintptr_t table = ReadPtr(manager + 0x08);
    if (!table) return 0;
    uint32_t buckets = 0;
    if (!Read(table + 0x88, buckets) || buckets == 0 || buckets > (1u << 24)) return 0;
    const uintptr_t bucketBase = ReadPtr(table + 0x98);
    const uintptr_t nodes = ReadPtr(table + 0xA0);
    if (!bucketBase || !nodes) return 0;
    const uintptr_t bucket = bucketBase + static_cast<uintptr_t>(key % buckets) * 0x100;
    uint32_t count = 0;
    if (!Read(bucket, count)) return 0;
    for (uint32_t i = 0; i < count && i < 31; ++i) {
        uint32_t k = 0, idx = 0;
        if (!Read(bucket + 8 + i * 8, k) || k != key) continue;
        if (!Read(bucket + 0xC + i * 8, idx)) return 0;
        const uintptr_t node = ReadPtr(nodes + static_cast<uintptr_t>(idx) * 8);
        return node ? ReadPtr(node + 0x08) : 0;
    }
    return 0;
}

// Return an actor's physics object, or 0.
uintptr_t PhysicsObject(uintptr_t actor) {
    const uintptr_t components = ReadPtr(actor + 0x68);
    const uintptr_t control = components ? ReadPtr(components + 0x40) : 0;
    const uintptr_t inner = control ? ReadPtr(control + 0x140) : 0;
    return inner ? ReadPtr(inner + 0x2C0) : 0;
}

// Read the Z axis and position of an actor's physics object.
bool ReadPhysics(uintptr_t actor, Vec3& axisZ, Vec3* position = nullptr) {
    const uintptr_t physics = PhysicsObject(actor);
    if (!physics || !Read(physics + 0x80, axisZ) || !Finite(axisZ)) return false;
    if (position && (!Read(physics + 0x90, *position) || !Finite(*position))) return false;
    return true;
}

// Return an actor's yaw from the TransformSync rotation the game itself uses in the update, or kMissing.
float ReadSyncYaw(uintptr_t actor) {
    const uintptr_t components = ReadPtr(actor + 0x68);
    const uintptr_t sync = components ? ReadPtr(components + 0x1A0) : 0;
    float q[4];
    if (!sync || !SafeRead(sync + 0xFC, q, sizeof q)) return kMissing;
    const float n = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
    if (!std::isfinite(n) || std::fabs(n - 1.0f) > 0.02f) return kMissing;
    return YawDeg(2 * (q[0] * q[2] + q[3] * q[1]), 1 - 2 * (q[0] * q[0] + q[1] * q[1]));
}

// Read an actor's current chart state pointer and that state's value, leaving value at kMissing if the state has none.
// Return false if the chain to the state can't be read.
bool ReadChartState(uintptr_t actor, uintptr_t& state, float& value) {
    state = 0;
    value = kMissing;
    const uintptr_t components = ReadPtr(actor + 0x68);
    const uintptr_t control = components ? ReadPtr(components + 0x40) : 0;
    const uintptr_t chartInfo = control ? ReadPtr(control + 0x30) : 0;
    uint8_t index = 0;
    if (!chartInfo || !Read(chartInfo + 0x1B, index)) return false;
    const uintptr_t chart = ReadPtr(control + static_cast<uintptr_t>(index) * 8 + 0x88);
    const uintptr_t runtime = chart ? ReadPtr(chart + 0x18) : 0;
    state = runtime ? ReadPtr(runtime + 0x28) : 0;
    if (!state) return false;
    uint8_t valueKind = 0;
    uint16_t valueOffset = 0;
    const uintptr_t values = ReadPtr(runtime + 0x38);
    if (values && Read(state + 0x7A, valueKind) && valueKind >= 1 && valueKind <= 32 && Read(state + 0x78, valueOffset)) {
        float v = 0;
        if (Read(values + valueOffset + 0x18, v) && std::isfinite(v)) value = v;
    }
    return true;
}

// Read the X and Z of an actor's velocity from its TransformSync object.
bool ReadVelocity(uintptr_t actor, float& x, float& z) {
    const uintptr_t components = ReadPtr(actor + 0x68);
    const uintptr_t sync = components ? ReadPtr(components + 0x1A0) : 0;
    return sync && Read(sync + 0x188, x) && Read(sync + 0x190, z) && std::isfinite(x) && std::isfinite(z);
}

// Resolve the controlled actor, its mount through the vehicle component link, and the mount's facing.
Ride ResolveRide() {
    Ride ride;
    const uintptr_t world = ReadPtr(g_base + g_worldSystemSlot);
    const uintptr_t manager = world ? ReadPtr(world + 0x30) : 0;
    if (!manager || !HasVtable(manager, g_vtActorManager)) return ride;
    ride.rider = ReadPtr(manager + 0x50);
    if (!ride.rider) return ride;
    const uintptr_t components = ReadPtr(ride.rider + 0x68);
    const uintptr_t vehicle = components ? ReadPtr(components + 0x48) : 0;
    if (!vehicle || !HasVtable(vehicle, g_vtVehicleComponent)) return ride;
    const uintptr_t link = ReadPtr(vehicle + 0x18);
    int8_t seat = -1;
    if (!link || !Read(link + 0x10, ride.mountKey) || ride.mountKey == 0) return ride;
    if (!Read(link + 0x14, seat) || seat == -1) return ride;
    ride.mounted = true;
    ride.mount = FindActor(manager, ride.mountKey);
    if (ride.mount && ReadPhysics(ride.mount, ride.axisZ)) {
        ride.haveForward = true;
        ride.forwardFromMount = true;
    } else if (ReadPhysics(ride.rider, ride.axisZ)) {
        ride.haveForward = true;
    }
    return ride;
}

// Return the name of a mount's action package, which is the element name in characteractionpackagedescription.xml.
std::string ReadPackageName(uintptr_t mount) {
    const uintptr_t components = ReadPtr(mount + 0x68);
    const uintptr_t control = components ? ReadPtr(components + 0x40) : 0;
    const uintptr_t owner = control ? ReadPtr(control + 0x28) : 0;
    const uintptr_t package = owner ? ReadPtr(owner + 0x18) : 0;
    char name[64] = {};
    if (!package || !SafeRead(package + 0x48, name, sizeof name - 1)) return {};
    return std::string(name, strnlen(name, sizeof name - 1));
}

bool IsGroundMountPackage(const std::string& name) {
    for (const char* allowed : kGroundMountPackages) {
        if (name == allowed) return true;
    }
    return false;
}

// Return whether the taming minigame's UI was updated within kRodeoTimeoutMs.
bool RodeoActive() {
    static std::atomic<uint64_t> lastCalls{0};
    static std::atomic<ULONGLONG> lastChange{0};
    if (!g_rodeoCalls) return false;
    const uint64_t calls = *g_rodeoCalls;
    const ULONGLONG now = GetTickCount64();
    if (calls != lastCalls.exchange(calls)) lastChange = now;
    return now - lastChange.load() < kRodeoTimeoutMs;
}

bool RidingGroundMount(const Ride& ride) {
    return g_enabled.load() && ride.mounted && ride.mount != 0 && ride.mount == g_groundMount.load() && !RodeoActive();
}

// Return how far steering has ramped in, from 0 to 1, restarting when stickX starts steering or changes direction.
float SteerRamp(float stickX) {
    const int side = stickX > 0.01f ? 1 : stickX < -0.01f ? -1 : 0;
    const ULONGLONG now = GetTickCount64();
    if (side != g_steerSide.exchange(side)) g_steerStart = now;
    return std::min(static_cast<float>(now - g_steerStart.load()) / kSteerRampMs, 1.0f);
}

// Steer a forward stick vector further from straight ahead, by more at low speed, scaling the angle by ramp and keeping
// its magnitude.
void WidenSteering(float& x, float& y, float speed, float ramp) {
    const float t = std::clamp((speed - kTightSteerSpeed) / (kWideSteerSpeed - kTightSteerSpeed), 0.0f, 1.0f);
    const float factor = kTightSteerFactor + (kWideSteerFactor - kTightSteerFactor) * t;
    const float magnitude = std::hypot(x, y);
    const float angle = std::min(std::atan2(std::fabs(x), y) * factor, kMaxSteerAngle) * ramp;
    x = std::copysign(magnitude * std::sin(angle), x);
    y = magnitude * std::cos(angle);
}

// Replace the component's camera forward and right vectors with ones aligned to the mount, keeping their handedness.
bool ApplyMountDirection(uintptr_t input, const Ride& ride, InputSample& sample) {
    Vec3 forward, right;
    if (!Read(input + kInput_CameraForward, forward) || !Read(input + kInput_CameraRight, right)) return false;
    if (!Finite(forward) || !Finite(right)) return false;
    sample.cameraYaw = YawDeg(forward.x, forward.z);

    // The game's move angle orients the actor's +Z against the resolved direction, so forward is minus the Z axis.
    const float sign = g_flip.load() ? 1.0f : -1.0f;
    const float len = std::hypot(ride.axisZ.x, ride.axisZ.z);
    if (len < 0.001f) return false;
    const float tx = sign * ride.axisZ.x / len, tz = sign * ride.axisZ.z / len;

    static float handedness = 1.0f;
    const float cross = forward.x * right.z - forward.z * right.x;
    if (std::fabs(cross) > 0.01f) handedness = cross > 0 ? 1.0f : -1.0f;
    const float rightLen = std::hypot(right.x, right.z);

    const Vec3 newForward{tx, 0.0f, tz};
    const Vec3 newRight{-tz * handedness * rightLen, right.y, tx * handedness * rightLen};
    sample.targetYaw = YawDeg(tx, tz);
    return SafeWrite(input + kInput_CameraForward, &newForward, sizeof newForward) &&
           SafeWrite(input + kInput_CameraRight, &newRight, sizeof newRight);
}

// Move the horse backward along its facing while g_backwardWanted is set, starting once it's below kBackwardMaxStartSpeed.
// The move goes through its physics position, so the game still applies ground height and collisions.
void UpdateBackwardWalk(const Ride& ride) {
    static bool active = false;
    static LARGE_INTEGER last{};

    const uintptr_t physics = PhysicsObject(ride.mount);
    Vec3 axisZ{}, position{};
    if (!g_backwardWanted.load() || !physics || !Read(physics + 0x80, axisZ) || !Read(physics + 0x90, position)) {
        active = false;
        g_backwardWalking = false;
        return;
    }
    LARGE_INTEGER now, frequency;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&frequency);
    if (!active) {
        if (g_mountSpeed.load() >= kBackwardMaxStartSpeed) return;
        active = true;
        g_backwardWalking = true;
        last = now;
        return;
    }
    const float dt = std::min(static_cast<float>(now.QuadPart - last.QuadPart) / frequency.QuadPart, 0.1f);
    last = now;
    const float len = std::hypot(axisZ.x, axisZ.z);
    if (len < 0.001f) return;
    position.x += axisZ.x / len * kBackwardMoveRate * dt;
    position.z += axisZ.z / len * kBackwardMoveRate * dt;
    SafeWrite(physics + 0x90, &position, sizeof position);
}

void OnInputUpdate(SafetyHookContext& ctx) {
    const uintptr_t input = ctx.rdi;
    uintptr_t actor = 0;
    Read(input + kInput_Actor, actor);
    const Ride ride = ResolveRide();

    InputSample& sample = g_samples[actor != 0 && actor == ride.rider ? 0 : 1];
    sample.actor = actor;
    float stick[2] = {}, angle = 0, relative = 0;
    SafeRead(input + kInput_Stick, stick, sizeof stick);
    Read(input + kInput_MoveAngle, angle);
    Read(input + kInput_MoveAngleRelative, relative);
    sample.stickX = stick[0];
    sample.stickY = stick[1];
    sample.angle = angle * kRadToDeg;
    sample.relative = relative * kRadToDeg;

    bool applied = false;
    if (RidingGroundMount(ride) && ride.haveForward) applied = ApplyMountDirection(input, ride, sample);
    sample.applied = applied;
    if (RidingGroundMount(ride) && actor == ride.mount && ride.mount == g_horseMount.load()) UpdateBackwardWalk(ride);
}

// Return whether backward input should read as forward for now: for kBackwardPulseMs once the horse is below
// kBackwardMaxStartSpeed, once per press.
bool UpdateBackwardPulse(bool wanted) {
    static std::atomic<bool> pulsed{false};
    g_backwardWanted = wanted;
    const ULONGLONG now = GetTickCount64();
    if (!wanted) {
        pulsed = false;
        g_backwardPulseEnd = 0;
        return false;
    }
    if (g_mountSpeed.load() < kBackwardMaxStartSpeed && !pulsed.exchange(true)) {
        g_backwardPulseEnd = now + kBackwardPulseMs;
    }
    return now < g_backwardPulseEnd.load();
}

// Start another backward pulse if the mount's chart state has left the walk states while the walk is running, so that
// the walking animation returns. Wait for steering to end, because steering with a pulse is a forward turn.
void UpdateBackwardRecovery(const Ride& ride, bool steering) {
    static bool inWalk = false;
    static ULONGLONG nextAllowed = 0;
    if (!g_backwardWanted.load() || !g_backwardWalking.load()) {
        inWalk = false;
        return;
    }
    uintptr_t state = 0;
    float value = 0;
    if (!ReadChartState(ride.mount, state, value)) return;
    const auto id = static_cast<uint16_t>(state);
    if (id == kStateWalk || id == kStateWalkSteer) {
        inWalk = true;
        return;
    }
    const ULONGLONG now = GetTickCount64();
    if (!inWalk || id == kStateWalkStart || steering || now < nextAllowed) return;
    nextAllowed = now + kBackwardRecoveryGapMs;
    g_backwardPulseEnd = now + kBackwardPulseMs;
    Log("backward recovery pulse from state %04X at speed %.2f", id, g_mountSpeed.load());
}

// Adjust the raw stick before the update reads it: clear backward input, leaving the backward walk to the position write,
// and widen forward steering. On the horse, S alone starts the backward walk, and A or D during it steers.
void OnStickRead(SafetyHookContext& ctx) {
    const Ride ride = ResolveRide();
    if (!RidingGroundMount(ride)) {
        g_backwardWanted = false;
        g_backwardWalking = false;
        return;
    }
    float velX = 0, velZ = 0;
    if (ReadVelocity(ride.mount, velX, velZ)) g_mountSpeed = std::hypot(velX, velZ);
    float stick[2];
    if (!SafeRead(ctx.rdi + kInput_RawStick, stick, sizeof stick)) return;
    const float ramp = SteerRamp(stick[0]);
    const bool backward = stick[1] < 0.0f;
    const bool steering = std::fabs(stick[0]) > 0.01f;
    const bool reverseSteering = backward && steering && g_backwardWalking.load();
    const bool isHorse = ride.mount == g_horseMount.load();
    const bool pulse = UpdateBackwardPulse(isHorse && backward && (!steering || reverseSteering));
    UpdateBackwardRecovery(ride, steering);
    if (backward) {
        stick[1] = pulse ? 1.0f : 0.0f;
        if (reverseSteering) stick[0] = std::copysign(kBackwardSteer, stick[0]);
    } else if (stick[1] > 0.0f && std::fabs(stick[0]) > 0.01f) {
        WidenSteering(stick[0], stick[1], g_mountSpeed.load(), ramp);
    } else {
        return;
    }
    SafeWrite(ctx.rdi + kInput_RawStick, stick, sizeof stick);
}

// Replace the rodeo UI's per-frame vtable slot with a stub that counts the call and jumps to the original, leaving every
// register except the flags untouched, so that RodeoActive can tell when the minigame is open.
void InstallRodeoSwitch() {
    const auto vtable = reinterpret_cast<uintptr_t*>(g_base + g_vtRodeoUi);
    auto* stub = static_cast<uint8_t*>(VirtualAlloc(nullptr, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!stub) {
        Log("rodeo switch: stub allocation failed");
        return;
    }
    // lock inc qword ptr [rip+8]; jmp qword ptr [rip+10]; the counter is at +16 and the original address at +24.
    const uint8_t code[] = {0xF0, 0x48, 0xFF, 0x05, 8, 0, 0, 0, 0xFF, 0x25, 10, 0, 0, 0};
    memcpy(stub, code, sizeof code);
    memcpy(stub + 24, &vtable[kRodeoUiFrameSlot], sizeof(uintptr_t));
    FlushInstructionCache(GetCurrentProcess(), stub, 32);
    DWORD oldProtect = 0;
    if (!VirtualProtect(&vtable[kRodeoUiFrameSlot], sizeof(uintptr_t), PAGE_READWRITE, &oldProtect)) {
        Log("rodeo switch: vtable protection failed");
        return;
    }
    g_rodeoCalls = reinterpret_cast<volatile uint64_t*>(stub + 16);
    vtable[kRodeoUiFrameSlot] = reinterpret_cast<uintptr_t>(stub);
    VirtualProtect(&vtable[kRodeoUiFrameSlot], sizeof(uintptr_t), oldProtect, &oldProtect);
    Log("rodeo switch installed");
}

// Log the key check's first bytes if they differ from the file's, which means something patched it at runtime.
void LogKeyCheckBytes() {
    const auto* live = reinterpret_cast<const uint8_t*>(g_base + g_keyCheck);
    if (memcmp(live, kKeyCheckBytes, sizeof kKeyCheckBytes) == 0) {
        Log("key check bytes match the file");
        return;
    }
    char text[3 * 32 + 1] = {};
    for (int i = 0; i < 32; ++i) snprintf(text + i * 3, 4, "%02X ", live[i]);
    Log("key check bytes differ: %s", text);
    if (live[0] == 0xE9) {
        int32_t offset = 0;
        memcpy(&offset, live + 1, sizeof offset);
        const uintptr_t target = g_base + g_keyCheck + 5 + offset;
        HMODULE owner = nullptr;
        wchar_t path[MAX_PATH] = L"unknown";
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCWSTR>(target), &owner)) {
            GetModuleFileNameW(owner, path, MAX_PATH);
        }
        Log("key check jumps to %p in %ls", reinterpret_cast<void*>(target), path);
    }
}

bool KeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

bool GameFocused() {
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid == GetCurrentProcessId();
}

void WriteTelemetry(const Ride& ride) {
    static Vec3 lastPos{};
    static ULONGLONG lastTick = 0, lastFlush = 0;
    static size_t rows = 0;
    if (!g_csv || rows > 500000) return;

    Vec3 riderAxis{}, riderPos{}, mountAxis{};
    const bool haveRider = ride.rider && ReadPhysics(ride.rider, riderAxis, &riderPos);
    const bool haveMount = ride.mount && ReadPhysics(ride.mount, mountAxis);

    const ULONGLONG now = GetTickCount64();
    float speed = kMissing, travelYaw = kMissing;
    if (haveRider && lastTick != 0 && now > lastTick) {
        const float dt = static_cast<float>(now - lastTick) / 1000.0f;
        const float dx = riderPos.x - lastPos.x, dz = riderPos.z - lastPos.z;
        speed = std::hypot(dx, dz) / dt;
        if (speed > 0.1f) travelYaw = YawDeg(dx, dz);
    }
    if (haveRider) {
        lastPos = riderPos;
        lastTick = now;
    }

    fprintf(g_csv, "%llu,%d,%d,%d,%d,%d,%d,%d,%08X,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", now, KeyDown('W') ? 1 : 0,
            KeyDown('A') ? 1 : 0, KeyDown('S') ? 1 : 0, KeyDown('D') ? 1 : 0, g_enabled.load() ? 1 : 0,
            g_flip.load() ? 1 : 0, ride.mounted ? 1 : 0, ride.mountKey, ride.forwardFromMount ? 1 : 0,
            haveRider ? YawDeg(riderAxis.x, riderAxis.z) : kMissing, haveMount ? YawDeg(mountAxis.x, mountAxis.z) : kMissing,
            ride.rider ? ReadSyncYaw(ride.rider) : kMissing, ride.mount ? ReadSyncYaw(ride.mount) : kMissing, speed,
            travelYaw);
    for (const InputSample& s : g_samples) {
        fprintf(g_csv, ",%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", s.applied.load() ? 1 : 0, s.stickX.load(), s.stickY.load(),
                s.angle.load(), s.relative.load(), s.cameraYaw.load(), s.targetYaw.load());
    }
    uintptr_t chartState = 0;
    float chartValue = kMissing, velX = kMissing, velZ = kMissing;
    if (ride.mount) ReadChartState(ride.mount, chartState, chartValue);
    const bool haveVelocity = ride.mount && ReadVelocity(ride.mount, velX, velZ);
    fprintf(g_csv, ",%llX,%.3f,%.3f,%.3f,%.3f,%.2f", static_cast<unsigned long long>(chartState), chartValue,
            haveVelocity ? velX : kMissing, haveVelocity ? velZ : kMissing,
            haveVelocity ? std::hypot(velX, velZ) : kMissing, haveRider ? riderPos.y : kMissing);
    fputc('\n', g_csv);
    ++rows;
    if (now - lastFlush > 1000) {
        fflush(g_csv);
        lastFlush = now;
    }
}

// Return whether this process is the game, because the ASI loader also loads this module into crashpad_handler.exe.
bool IsGameProcess() {
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    const wchar_t* slash = wcsrchr(path, L'\\');
    return _wcsicmp(slash ? slash + 1 : path, L"CrimsonDesert.exe") == 0;
}

std::wstring ModuleDir(HMODULE module) {
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(module, path, MAX_PATH);
    std::wstring dir(path);
    const size_t slash = dir.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"." : dir.substr(0, slash);
}

const IMAGE_NT_HEADERS* GameHeaders() {
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(g_base);
    return reinterpret_cast<const IMAGE_NT_HEADERS*>(g_base + dos->e_lfanew);
}

// Which of the game's sections to scan. Section names change between builds, so these go by the section's flags.
enum class SectionKind { Code, Data, ReadOnlyData };

bool SectionIs(const IMAGE_SECTION_HEADER& section, SectionKind kind) {
    const bool exec = (section.Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
    const bool write = (section.Characteristics & IMAGE_SCN_MEM_WRITE) != 0;
    switch (kind) {
        case SectionKind::Code: return exec;
        case SectionKind::Data: return !exec;
        case SectionKind::ReadOnlyData: return !exec && !write;
    }
    return false;
}

struct Range {
    const uint8_t* first = nullptr;
    const uint8_t* last = nullptr;
};

// Return the readable parts of the game's sections of the given kind, joining adjacent regions so that a match can span
// them.
std::vector<Range> ReadableRanges(SectionKind kind) {
    constexpr DWORD kReadable = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
                                PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
    std::vector<Range> ranges;
    const IMAGE_NT_HEADERS* nt = GameHeaders();
    const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section) {
        if (!SectionIs(*section, kind)) continue;
        const auto* p = reinterpret_cast<const uint8_t*>(g_base + section->VirtualAddress);
        const uint8_t* const end = p + section->Misc.VirtualSize;
        while (p < end) {
            MEMORY_BASIC_INFORMATION info;
            if (!VirtualQuery(p, &info, sizeof info)) break;
            const uint8_t* const regionEnd =
                std::min(static_cast<const uint8_t*>(info.BaseAddress) + info.RegionSize, end);
            if (info.State == MEM_COMMIT && (info.Protect & kReadable) && !(info.Protect & PAGE_GUARD)) {
                if (!ranges.empty() && ranges.back().last == p) {
                    ranges.back().last = regionEnd;
                } else {
                    ranges.push_back({p, regionEnd});
                }
            }
            p = regionEnd;
        }
    }
    return ranges;
}

// Bytes to search for. literal[i] is 0 where bytes[i] can be anything.
struct Pattern {
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> literal;
};

Pattern ParseSignature(const char* text) {
    Pattern pattern;
    for (const char* p = text; *p;) {
        if (*p == ' ') {
            ++p;
            continue;
        }
        const bool wildcard = *p == '?';
        pattern.bytes.push_back(wildcard ? 0 : static_cast<uint8_t>(strtoul(std::string(p, 2).c_str(), nullptr, 16)));
        pattern.literal.push_back(wildcard ? 0 : 1);
        p += 2;
    }
    return pattern;
}

Pattern LiteralPattern(const void* data, size_t size) {
    const auto* bytes = static_cast<const uint8_t*>(data);
    return {std::vector<uint8_t>(bytes, bytes + size), std::vector<uint8_t>(size, 1)};
}

// Append the address of every match in range to hits, and return false if reading faults.
bool ScanRange(const Range& range, const Pattern& pattern, std::vector<uintptr_t>& hits) {
    const size_t size = pattern.bytes.size();
    if (static_cast<size_t>(range.last - range.first) < size) return true;
    const uint8_t* const last = range.last - size;
    const uint8_t first = pattern.bytes[0];
    __try {
        for (const uint8_t* p = range.first; p <= last; ++p) {
            p = static_cast<const uint8_t*>(memchr(p, first, static_cast<size_t>(last - p) + 1));
            if (!p) break;
            size_t i = 1;
            while (i < size && (!pattern.literal[i] || p[i] == pattern.bytes[i])) ++i;
            if (i == size) hits.push_back(reinterpret_cast<uintptr_t>(p));
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

// Return the offset from g_base of every match in the game's sections of the given kind, or nothing if reading faults.
std::vector<uintptr_t> FindAll(const Pattern& pattern, SectionKind kind) {
    std::vector<uintptr_t> hits;
    for (const Range& range : ReadableRanges(kind)) {
        if (!ScanRange(range, pattern, hits)) {
            Log("lookup scan: read fault in %p to %p", range.first, range.last);
            return {};
        }
    }
    for (uintptr_t& hit : hits) hit -= g_base;
    return hits;
}

// Return the offsets of the vtables for the primary base of the class with this RTTI name.
std::vector<uintptr_t> FindVtables(const char* className) {
    std::vector<uintptr_t> vtables;
    const std::vector<uintptr_t> names = FindAll(LiteralPattern(className, strlen(className) + 1), SectionKind::Data);
    if (names.empty()) return vtables;
    const auto descriptor = static_cast<uint32_t>(names[0] - 0x10);
    for (const uintptr_t field : FindAll(LiteralPattern(&descriptor, sizeof descriptor), SectionKind::ReadOnlyData)) {
        // A CompleteObjectLocator has signature 1 at +0, offset 0 at +4 for the primary base, the type descriptor at +12
        // and its own offset at +20. A vtable's entry -1 points to it.
        const uintptr_t locator = field - 12;
        uint32_t signature = 0, offset = 1, self = 0;
        if (!Read(g_base + locator, signature) || !Read(g_base + locator + 4, offset) ||
            !Read(g_base + locator + 20, self) || signature != 1 || offset != 0 || self != locator) {
            continue;
        }
        const uintptr_t pointer = g_base + locator;
        for (const uintptr_t hit : FindAll(LiteralPattern(&pointer, sizeof pointer), SectionKind::ReadOnlyData)) {
            vtables.push_back(hit + sizeof(uintptr_t));
        }
    }
    return vtables;
}

// Set out to the vtable of the class with this RTTI name and log the result. Return whether it has exactly one.
bool LookupVtable(const char* name, const char* className, uintptr_t& out) {
    const std::vector<uintptr_t> vtables = FindVtables(className);
    if (vtables.size() != 1) {
        Log("lookup %s FAILED: %zu vtables for %s", name, vtables.size(), className);
        return false;
    }
    out = vtables[0];
    Log("lookup %s: exe+0x%llX", name, static_cast<unsigned long long>(out));
    return true;
}

// Set out to the signature's target and log the result. Return whether the signature has exactly one match.
bool LookupSignature(const char* name, const Signature& signature, uintptr_t& out) {
    const std::vector<uintptr_t> hits = FindAll(ParseSignature(signature.text), SectionKind::Code);
    if (hits.size() != 1) {
        Log("lookup %s FAILED: %zu signature matches", name, hits.size());
        return false;
    }
    out = hits[0] + signature.distance;
    Log("lookup %s: exe+0x%llX", name, static_cast<unsigned long long>(out));
    return true;
}

// Set g_worldSystemSlot to the global that kWorldReferenceSignature loads most, and log the result. Return false unless
// it has at least kWorldMinRatio times the references of the next.
bool LookupWorldSystemSlot() {
    std::unordered_map<uintptr_t, int> counts;
    for (const uintptr_t hit : FindAll(ParseSignature(kWorldReferenceSignature), SectionKind::Code)) {
        int32_t displacement = 0;
        if (Read(g_base + hit + 3, displacement)) ++counts[hit + 7 + displacement];
    }
    uintptr_t slot = 0;
    int top = 0, next = 0;
    for (const auto& [address, count] : counts) {
        if (count > top) {
            next = top;
            top = count;
            slot = address;
        } else if (count > next) {
            next = count;
        }
    }
    if (top == 0 || top < kWorldMinRatio * next) {
        Log("lookup worldSystemSlot FAILED: %d references, next %d", top, next);
        return false;
    }
    g_worldSystemSlot = slot;
    Log("lookup worldSystemSlot: exe+0x%llX, %d references, next %d", static_cast<unsigned long long>(slot), top, next);
    return true;
}

// Run every lookup and log each result. Return whether the mod can run, which needs all of them except the key check,
// because that one is only diagnostics.
bool RunLookups() {
    const ULONGLONG start = GetTickCount64();
    bool ok = LookupVtable("vtActorManager", kClassActorManager, g_vtActorManager);
    ok &= LookupVtable("vtVehicleComponent", kClassVehicleComponent, g_vtVehicleComponent);
    ok &= LookupVtable("vtRodeoUi", kClassRodeoUi, g_vtRodeoUi);
    ok &= LookupWorldSystemSlot();
    ok &= LookupSignature("hookSite", kHookSiteSignature, g_hookSite);
    ok &= LookupSignature("stickSite", kStickSiteSignature, g_stickSite);
    LookupSignature("keyCheck", kKeyCheckSignature, g_keyCheck);
    Log("lookups took %llu ms", GetTickCount64() - start);
    return ok;
}

bool InstallHook() {
    Log("HorseSteer %s loaded: base %p, PE timestamp 0x%08X", kVersion, reinterpret_cast<void*>(g_base),
        GameHeaders()->FileHeader.TimeDateStamp);
    if (!RunLookups()) {
        Log("a required lookup failed: hooks not installed, mod idle");
        return false;
    }
    auto hook = safetyhook::MidHook::create(reinterpret_cast<void*>(g_base + g_hookSite), OnInputUpdate);
    auto stickHook = safetyhook::MidHook::create(reinterpret_cast<void*>(g_base + g_stickSite), OnStickRead);
    if (!hook || !stickHook) {
        Log("hooks failed to install");
        return false;
    }
    g_hook = std::move(*hook);
    g_stickHook = std::move(*stickHook);
    Log("hooks installed at exe+0x%llX and exe+0x%llX", static_cast<unsigned long long>(g_hookSite),
        static_cast<unsigned long long>(g_stickSite));
    InstallRodeoSwitch();
    if (g_keyCheck) LogKeyCheckBytes();
    return true;
}

DWORD WINAPI ModThread(void* param) {
    const std::wstring dir = ModuleDir(static_cast<HMODULE>(param));
    g_log = _wfsopen((dir + L"\\HorseSteer.log").c_str(), L"w", _SH_DENYNO);
    g_csv = _wfsopen((dir + L"\\HorseSteer.csv").c_str(), L"w", _SH_DENYNO);
    if (g_csv) {
        fputs("tick_ms,w,a,s,d,enabled,flip,mounted,mount_key,forward_from_mount,rider_phys_yaw,mount_phys_yaw,"
              "rider_sync_yaw,mount_sync_yaw,speed,travel_yaw,"
              "i0_applied,i0_stick_x,i0_stick_y,i0_angle,i0_relative,i0_camera_yaw,i0_target_yaw,"
              "i1_applied,i1_stick_x,i1_stick_y,i1_angle,i1_relative,i1_camera_yaw,i1_target_yaw,"
              "mount_state,mount_value,mount_vel_x,mount_vel_z,mount_vel,rider_y\n", g_csv);
    }
    g_base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));

    // Other mods report crashes when hooking during the first seconds of launch.
    Sleep(20000);
    if (!InstallHook()) return 0;

    bool f5Was = false, wasMounted = false, wasFromMount = false, wasRodeo = false;
    uintptr_t seen[2] = {};
    uintptr_t classified = 0;
    bool reportedUnreadable = false;
    ULONGLONG lastTelemetry = 0;
    for (;;) {
        Sleep(10);
        const bool f5 = GameFocused() && KeyDown(VK_F5);
        if (f5 && !f5Was) {
            if (KeyDown(VK_SHIFT)) {
                g_flip = !g_flip.load();
                Log("flip %s", g_flip.load() ? "on" : "off");
                Beep(600, 120);
            } else {
                g_enabled = !g_enabled.load();
                Log("mod %s", g_enabled.load() ? "enabled" : "disabled");
                Beep(g_enabled.load() ? 1200 : 400, 120);
            }
        }
        f5Was = f5;

        const ULONGLONG now = GetTickCount64();
        if (now - lastTelemetry < 100) continue;
        lastTelemetry = now;
        const bool rodeo = RodeoActive();
        if (rodeo != wasRodeo) {
            Log("rodeo %s", rodeo ? "started: mod paused" : "ended");
            wasRodeo = rodeo;
        }
        const Ride ride = ResolveRide();
        if (ride.mounted != wasMounted || ride.forwardFromMount != wasFromMount) {
            Log("mounted=%d key=0x%08X mount_actor=%p facing from %s", ride.mounted ? 1 : 0, ride.mountKey,
                reinterpret_cast<void*>(ride.mount), ride.forwardFromMount ? "mount" : "rider");
            wasMounted = ride.mounted;
            wasFromMount = ride.forwardFromMount;
        }
        if (ride.mount != classified) {
            g_groundMount = 0;
            g_horseMount = 0;
            classified = 0;
            if (ride.mount) {
                const std::string package = ReadPackageName(ride.mount);
                if (!package.empty()) {
                    classified = ride.mount;
                    reportedUnreadable = false;
                    const bool ground = IsGroundMountPackage(package);
                    if (ground) g_groundMount = ride.mount;
                    if (package == kHorsePackage) g_horseMount = ride.mount;
                    Log("mount package: %s (%s)", package.c_str(), ground ? "controls on" : "not a ground mount, mod inactive");
                } else if (!reportedUnreadable) {
                    reportedUnreadable = true;
                    Log("mount package unreadable: mod inactive until it can be read");
                }
            }
        }
        for (int i = 0; i < 2; ++i) {
            const uintptr_t actor = g_samples[i].actor.load();
            if (actor != seen[i]) {
                seen[i] = actor;
                Log("input slot %d actor=%p (rider=%p mount=%p)", i, reinterpret_cast<void*>(actor),
                    reinterpret_cast<void*>(ride.rider), reinterpret_cast<void*>(ride.mount));
            }
        }
        if (ride.mounted) WriteTelemetry(ride);
    }
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        if (!IsGameProcess()) return TRUE;
        if (HANDLE thread = CreateThread(nullptr, 0, ModThread, module, 0, nullptr)) CloseHandle(thread);
    }
    return TRUE;
}
