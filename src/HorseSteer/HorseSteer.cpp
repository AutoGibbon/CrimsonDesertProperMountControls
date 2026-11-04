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

#include "safetyhook.hpp"

namespace {

constexpr const char* kVersion = "0.0.1";

// Steam build 26423880.
constexpr uint32_t kPeTimestamp = 0x6AEA07A6;
constexpr uintptr_t kWorldSystemSlot = 0x71BDEF0;
constexpr uintptr_t kChunkOrigin = 0x71AA150;
constexpr uintptr_t kVtActorManager = 0x5944B38;
constexpr uintptr_t kVtVehicleComponent = 0x5944B90;
// Inside ClientInputActorComponent's update (exe+0xB6D470), right after it copies the camera vectors.
constexpr uintptr_t kHookSite = 0xB6D526;
constexpr uint8_t kHookSiteBytes[] = {0x49, 0x8B, 0x40, 0x20, 0x48, 0x8B, 0x88, 0x30, 0x12, 0x00, 0x00};
// In the same update, right before it reads the raw stick from +0x48.
constexpr uintptr_t kStickSite = 0xB6D626;
constexpr uint8_t kStickSiteBytes[] = {0x48, 0x8D, 0x77, 0x50, 0x48, 0x8B, 0x47, 0x48};
// The chart's key condition check, which reads the key's hash from its condition.
constexpr uintptr_t kKeyCheck = 0x3B0750;
constexpr uint8_t kKeyCheckBytes[] = {0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0x02, 0x8B, 0x50, 0x08};
// UIGamePlayControlRootMiniGameRodeo's vtable, its first entry, which confirms the address, and the slot the game calls
// every frame while the minigame is open and at no other time.
constexpr uintptr_t kVtRodeoUi = 0x5A75218;
constexpr uintptr_t kRodeoUiSlot0 = 0x11F09D0;
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
// The backward walk starts once the horse is below this speed in m/s, which is about its position noise at rest.
constexpr float kBackwardMaxStartSpeed = 0.2f;
// A full steering input leaves the walking blend for a forward walk. This size stays in the blend and curves the path.
constexpr float kBackwardSteer = 0.56f;
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

// Resolve the controlled actor, its mount through the vehicle component link, and the mount's facing.
Ride ResolveRide() {
    Ride ride;
    const uintptr_t world = ReadPtr(g_base + kWorldSystemSlot);
    const uintptr_t manager = world ? ReadPtr(world + 0x30) : 0;
    if (!manager || !HasVtable(manager, kVtActorManager)) return ride;
    ride.rider = ReadPtr(manager + 0x50);
    if (!ride.rider) return ride;
    const uintptr_t components = ReadPtr(ride.rider + 0x68);
    const uintptr_t vehicle = components ? ReadPtr(components + 0x48) : 0;
    if (!vehicle || !HasVtable(vehicle, kVtVehicleComponent)) return ride;
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

// Adjust the raw stick before the update reads it: clear backward input, leaving the backward walk to the position write,
// and widen forward steering. On the horse, S alone starts the backward walk, and A or D during it steers.
void OnStickRead(SafetyHookContext& ctx) {
    const Ride ride = ResolveRide();
    if (!RidingGroundMount(ride)) {
        g_backwardWanted = false;
        g_backwardWalking = false;
        return;
    }
    float stick[2];
    if (!SafeRead(ctx.rdi + kInput_RawStick, stick, sizeof stick)) return;
    const float ramp = SteerRamp(stick[0]);
    const bool backward = stick[1] < 0.0f;
    const bool steering = std::fabs(stick[0]) > 0.01f;
    const bool reverseSteering = backward && steering && g_backwardWalking.load();
    const bool isHorse = ride.mount == g_horseMount.load();
    const bool pulse = UpdateBackwardPulse(isHorse && backward && (!steering || reverseSteering));
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
    const auto vtable = reinterpret_cast<uintptr_t*>(g_base + kVtRodeoUi);
    if (vtable[0] != g_base + kRodeoUiSlot0) {
        Log("rodeo switch: vtable differs, not installed");
        return;
    }
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
    const auto* live = reinterpret_cast<const uint8_t*>(g_base + kKeyCheck);
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
        const uintptr_t target = g_base + kKeyCheck + 5 + offset;
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

    Vec3 origin{}, riderAxis{}, riderPos{}, mountAxis{};
    Read(g_base + kChunkOrigin, origin);
    const bool haveRider = ride.rider && ReadPhysics(ride.rider, riderAxis, &riderPos);
    const bool haveMount = ride.mount && ReadPhysics(ride.mount, mountAxis);

    const ULONGLONG now = GetTickCount64();
    float speed = kMissing, travelYaw = kMissing;
    const Vec3 world{riderPos.x + origin.x, riderPos.y + origin.y, riderPos.z + origin.z};
    if (haveRider && lastTick != 0 && now > lastTick) {
        const float dt = static_cast<float>(now - lastTick) / 1000.0f;
        const float dx = world.x - lastPos.x, dz = world.z - lastPos.z;
        speed = std::hypot(dx, dz) / dt;
        g_mountSpeed = speed;
        if (speed > 0.1f) travelYaw = YawDeg(dx, dz);
    }
    if (haveRider) {
        lastPos = world;
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

bool InstallHook() {
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(g_base);
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(g_base + dos->e_lfanew);
    Log("HorseSteer %s loaded: base %p, PE timestamp 0x%08X", kVersion, reinterpret_cast<void*>(g_base),
        nt->FileHeader.TimeDateStamp);
    if (nt->FileHeader.TimeDateStamp != kPeTimestamp) {
        Log("unsupported game build: hook not installed");
        return false;
    }
    if (memcmp(reinterpret_cast<const void*>(g_base + kHookSite), kHookSiteBytes, sizeof kHookSiteBytes) != 0 ||
        memcmp(reinterpret_cast<const void*>(g_base + kStickSite), kStickSiteBytes, sizeof kStickSiteBytes) != 0) {
        Log("hook site bytes differ, possibly another mod's hook: hooks not installed");
        return false;
    }
    auto hook =safetyhook::MidHook::create(reinterpret_cast<void*>(g_base + kHookSite), OnInputUpdate);
    auto stickHook = safetyhook::MidHook::create(reinterpret_cast<void*>(g_base + kStickSite), OnStickRead);
    if (!hook || !stickHook) {
        Log("hooks failed to install");
        return false;
    }
    g_hook = std::move(*hook);
    g_stickHook = std::move(*stickHook);
    Log("hooks installed at exe+0x%llX and exe+0x%llX", static_cast<unsigned long long>(kHookSite),
        static_cast<unsigned long long>(kStickSite));
    InstallRodeoSwitch();
    LogKeyCheckBytes();
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
              "i1_applied,i1_stick_x,i1_stick_y,i1_angle,i1_relative,i1_camera_yaw,i1_target_yaw\n", g_csv);
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
