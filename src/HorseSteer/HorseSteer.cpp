// Horse-relative riding controls: on ground mounts, replaces the camera direction vectors in
// ClientInputActorComponent's per-frame update with the mount's, so move input is resolved against the mount's facing.
// Also widens and ramps in forward steering angles, and removes backward input, which leaves S with a steering input as
// a pivot. S also walks the mount backward from a near stop, or on mounts with a built-in reverse, acts as the game's
// brake key. Tapping S with W held slows the mount by a gait. All of this stops while the taming minigame runs, because
// it needs the game's own input directions.
//
// F5, or the key set by Toggle under [Keys] in ProperHorseControls.ini, toggles the mod.

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
#include <cwctype>
#include <string>
#include <unordered_map>
#include <vector>

#include "safetyhook.hpp"

namespace {

constexpr const char* kVersion = "1.1.0";

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
    "48 8B 41 20 83 78 28 00 77 04 33 C9 EB 08 48 8B 40 20 48 8B 48 10 48 8B 01 FF 50 40",
    -14};
constexpr uint8_t kKeyCheckBytes[] = {0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0x02, 0x8B, 0x50, 0x08};
// A call to the function that returns the chart condition handler table, in the code that checks a condition type is
// below 0x286 and indexes the table by type - 0x83.
constexpr Signature kConditionTableCallSignature{
    "E8 ?? ?? ?? ?? 81 FB 86 02 00 00 73 ?? 8D 8B 7D FF FF FF 48 8D 1C CD 00 00 00 00 48 83 3C 03 00", 0};
constexpr size_t kKeyConditionSlot = 0xEA - 0x83;
constexpr uint32_t kKeyBrakeHash = 0x377B9DB8;
// The input object's vtable slot that takes a key hash and returns whether the key is held.
constexpr size_t kKeyQuerySlot = 8;
// Chart condition handlers return 0 when the condition holds.
constexpr uint8_t kConditionHolds = 0;
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

// A land mount's package, its backward walk speed in m/s, which LoadSettings can replace from ProperHorseControls.ini,
// and whether S reverses it with the game's brake key instead of the backward walk.
struct GroundMount {
    const char* package;
    float backwardSpeed;
    bool brakeReverse = false;
};

// Water mounts, the dragon and machines are left out. The elephant's, the wagon's and other wolf packages' speeds are
// untuned.
GroundMount g_groundMounts[] = {
    {"CD_R0001_Horse", 1.0f},          {"CD_M0002_Camel", 1.9f},
    {"CD_M0002_Reindeer", 1.2f},       {"CD_M0002_Elephant", 1.0f},
    {"CD_M0002_BattleBoar", 1.6f},     {"CD_M0002_Bear", 1.8f},
    {"CD_M0002_Wolf", 1.8f},           {"CD_M0002_Wolf_Elite", 1.7f},
    {"CD_M0002_Wolf_Elite_Big", 1.8f}, {"CD_M0002_Wolf_Boss", 1.8f},
    {"CD_R0034_Iguana", 1.2f},         {"CD_R0035_CarmaBirdsaurus", 1.6f},
    {"CD_M0003_CucuBird", 1.0f},       {"CD_R0012_Wagon_0001", 1.0f, true},
};

// Forward steering multiplies W+A's 45 degree stick angle by these, blending between the two speeds in m/s.
constexpr float kTightSteerFactor = 80.0f / 45.0f;
constexpr float kWideSteerFactor = 55.0f / 45.0f;
constexpr float kTightSteerSpeed = 2.5f;
constexpr float kWideSteerSpeed = 5.0f;
constexpr float kMaxSteerAngle = 85.0f / 57.29578f;
constexpr float kSteerRampMs = 300.0f;
// A raw stick below this Y counts as backward input. Above it, a negative Y is cleared so that it steers like A or D.
constexpr float kBackwardStickY = -0.5f;
// Converts a speed in m/s to the rate added to the position, because every mount tested moves at about twice its rate.
constexpr float kBackwardRatePerSpeed = 0.5f;
constexpr float kBackwardSpeedMin = 0.2f;
constexpr float kBackwardSpeedMax = 6.0f;
// Reads as forward input for this long, so that the mount leaves idle for a state that uses the walking blend.
constexpr int kBackwardPulseMs = 150;
// The backward walk starts once the mount is below this speed in m/s, which is near the end of a stop.
constexpr float kBackwardMaxStartSpeed = 0.2f;
// A full steering input leaves the walking blend for a forward walk. This size stays in the blend and curves the path.
constexpr float kBackwardSteer = 0.56f;
// UpdateBackwardRecovery only pulses for a mount above this speed in m/s, which leaves out one held still by a wall. A
// mount with no chart value pulses once it has had none for this long, repeating at this gap.
constexpr float kBackwardLostMinSpeed = 0.3f;
constexpr ULONGLONG kBackwardLostDelayMs = 200;
constexpr ULONGLONG kBackwardLostGapMs = 500;
// Low 16 bits of the horse's chart state pointer during the walk, which stay the same between sessions.
constexpr uint16_t kStateWalkStart = 0xA37D;
constexpr uint16_t kStateWalk = 0xA73F;
constexpr uint16_t kStateWalkSteer = 0xA56E;
// Minimum time between pulses that return the horse to the walk state, in case a pulse doesn't get it there.
constexpr ULONGLONG kBackwardRecoveryGapMs = 1000;
// UpdateBrakeGap only starts at or above this speed in m/s, ends after this long at the latest, and treats a sideways
// stick above this size as steering, which leaves room for a gamepad pulled back slightly off center.
constexpr float kBrakeGapMinSpeed = 1.0f;
constexpr ULONGLONG kBrakeGapMaxMs = 3000;
constexpr float kBrakeGapSteerX = 0.3f;
// UpdateBrakeGap ends below this speed in m/s, so that forward input works again once the mount has stopped.
constexpr float kBrakeGapStopSpeed = 0.3f;
// A gap on g_brakeMount that starts above this chart value can end early, below this speed in m/s.
constexpr float kBrakeMountLowValue = 4.0f;
constexpr float kBrakeMountEndSpeed = 5.0f;

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
std::wstring g_logPath;
bool g_logging = false;
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
uintptr_t g_conditionTableCall = 0;
std::atomic<bool> g_enabled{true};
// The virtual-key code that toggles the mod, set by LoadSettings before ModThread's key loop starts.
int g_toggleKey = VK_F5;
InputSample g_samples[2];
SafetyHookMid g_hook;
SafetyHookMid g_stickHook;
std::atomic<uintptr_t> g_groundMount{0};
std::atomic<float> g_mountSpeed{0};
std::atomic<int> g_steerSide{0};
std::atomic<ULONGLONG> g_steerStart{0};
volatile uint64_t* g_rodeoCalls = nullptr;
std::atomic<float> g_backwardRate{kBackwardRatePerSpeed};
// Whether A and D curve the backward walk and the brake reverse the opposite way.
std::atomic<bool> g_backwardSteerInvert{true};
std::atomic<ULONGLONG> g_backwardPulseEnd{0};
std::atomic<bool> g_backwardWanted{false};
std::atomic<bool> g_backwardWalking{false};
// The ground mount that S reverses with the brake key, or 0, and whether S is braking it.
std::atomic<uintptr_t> g_brakeMount{0};
std::atomic<bool> g_braking{false};
// Whether a stick gap is running.
std::atomic<bool> g_brakeGap{false};
// The mount input component's raw stick before OnStickRead edits it, for logging.
std::atomic<float> g_rawStickX{0}, g_rawStickY{0};

using ConditionHandler = uint8_t (*)(uintptr_t, uintptr_t*, void*, void*, uint8_t, void*, void*);
ConditionHandler g_keyCondition = nullptr;

const IMAGE_NT_HEADERS* GameHeaders();
bool KeyDown(int vk);
bool GameFocused();

// Write one timestamped line to g_log, if it's open.
void WriteLogLine(const char* fmt, va_list args) {
    if (!g_log) return;
    SYSTEMTIME t;
    GetLocalTime(&t);
    fprintf(g_log, "[%02d:%02d:%02d.%03d] ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    vfprintf(g_log, fmt, args);
    fputc('\n', g_log);
    fflush(g_log);
}

// Create g_log at g_logPath if it isn't open yet, starting with the mod's version and the game's PE timestamp.
void OpenLog() {
    if (g_log) return;
    g_log = _wfsopen(g_logPath.c_str(), L"w", _SH_DENYNO);
    if (!g_log) return;
    fprintf(g_log, "ProperHorseControls %s, base %p, PE timestamp 0x%08X\n", kVersion, reinterpret_cast<void*>(g_base),
            GameHeaders()->FileHeader.TimeDateStamp);
    fflush(g_log);
}

// Write a tracing line, only when Logging is on in the ini.
void Log(const char* fmt, ...) {
    if (!g_logging) return;
    va_list args;
    va_start(args, fmt);
    WriteLogLine(fmt, args);
    va_end(args);
}

// Write a line about something that stops the mod or part of it working, whatever Logging is set to. Only call this from
// ModThread, because it can create the log.
void LogProblem(const char* fmt, ...) {
    OpenLog();
    va_list args;
    va_start(args, fmt);
    WriteLogLine(fmt, args);
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

// Return the printable text at addr, up to 127 characters.
std::string ReadText(uintptr_t addr) {
    std::string text;
    for (int i = 0; i < 127; ++i) {
        char c = 0;
        if (!Read(addr + i, c) || c < 0x20 || c > 0x7E) break;
        text.push_back(c);
    }
    return text;
}

// Return the entry in g_groundMounts for a package name in any case, or nullptr.
const GroundMount* FindGroundMount(const std::string& name) {
    for (const GroundMount& mount : g_groundMounts) {
        if (_stricmp(name.c_str(), mount.package) == 0) return &mount;
    }
    return nullptr;
}

// Return a mount's action package name, which is the element name in characteractionpackagedescription.xml, in lowercase
// at +0x18. The name at +0x48 is only there on some mounts, so it's used if +0x18 isn't a ground mount.
std::string ReadPackageName(uintptr_t mount) {
    const uintptr_t components = ReadPtr(mount + 0x68);
    const uintptr_t control = components ? ReadPtr(components + 0x40) : 0;
    const uintptr_t owner = control ? ReadPtr(control + 0x28) : 0;
    const uintptr_t package = owner ? ReadPtr(owner + 0x18) : 0;
    if (!package) return {};
    const std::string lower = ReadText(package + 0x18);
    if (FindGroundMount(lower)) return lower;
    const std::string upper = ReadText(package + 0x48);
    return FindGroundMount(upper) ? upper : lower;
}

// Return a mount's appearance file path, which separates mounts that share a package, or nothing if it can't be read.
std::string ReadAppearance(uintptr_t mount) {
    const uintptr_t components = ReadPtr(mount + 0x68);
    const uintptr_t control = components ? ReadPtr(components + 0x40) : 0;
    const uintptr_t first = control ? ReadPtr(control + 0x40) : 0;
    const uintptr_t second = first ? ReadPtr(first + 0x38) : 0;
    return second ? ReadText(second + 0x18) : std::string();
}

// Key names accepted by Toggle under [Keys], besides F1 to F24, A to Z, 0 to 9 and Numpad0 to Numpad9.
constexpr struct {
    const wchar_t* name;
    int vk;
} kNamedKeys[] = {
    {L"Insert", VK_INSERT}, {L"Delete", VK_DELETE}, {L"Home", VK_HOME},           {L"End", VK_END},
    {L"PageUp", VK_PRIOR},  {L"PageDown", VK_NEXT}, {L"Pause", VK_PAUSE},         {L"ScrollLock", VK_SCROLL},
};

// Return the virtual-key code for a key name in any case, or 0 if it isn't an accepted name. W, A, S and D return 0,
// because they ride the mount.
int ParseKeyName(const std::wstring& name) {
    const auto digits = [](const wchar_t* p) {
        if (!*p) return false;
        for (; *p; ++p) {
            if (!iswdigit(*p)) return false;
        }
        return true;
    };
    if (name.size() == 1) {
        const wchar_t c = towupper(name[0]);
        if (c == L'W' || c == L'A' || c == L'S' || c == L'D') return 0;
        return (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9') ? c : 0;
    }
    if (towupper(name[0]) == L'F' && name.size() <= 3 && digits(name.c_str() + 1)) {
        const int n = _wtoi(name.c_str() + 1);
        return n >= 1 && n <= 24 ? VK_F1 + n - 1 : 0;
    }
    if (name.size() == 7 && _wcsnicmp(name.c_str(), L"Numpad", 6) == 0 && digits(name.c_str() + 6)) {
        return VK_NUMPAD0 + (name[6] - L'0');
    }
    for (const auto& key : kNamedKeys) {
        if (_wcsicmp(name.c_str(), key.name) == 0) return key.vk;
    }
    return 0;
}

// Replace the backward speeds, reverse steering invert, toggle key and slowdown assist with any values in the ini at
// path, write each setting it lacks with its default, so that an ini from an earlier version gains new settings, and log
// what's in use.
void LoadSettings(const std::wstring& path) {
    wchar_t text[32];
    const auto readText = [&](const wchar_t* section, const wchar_t* key) {
        return GetPrivateProfileStringW(section, key, L"", text, 32, path.c_str()) != 0;
    };
    const auto read = [&](const wchar_t* section, const wchar_t* key, float& value, float low, float high) {
        if (!readText(section, key)) return false;
        value = std::clamp(wcstof(text, nullptr), low, high);
        return true;
    };
    int added = 0;
    bool writeFailed = false;
    const auto addMissing = [&](const wchar_t* section, const wchar_t* key, const wchar_t* value) {
        if (WritePrivateProfileStringW(section, key, value, path.c_str())) {
            ++added;
        } else if (!writeFailed) {
            writeFailed = true;
            LogProblem("ini: couldn't add %ls=%ls under [%ls], error %lu", key, value, section, GetLastError());
        }
    };
    int found = 0;
    for (GroundMount& mount : g_groundMounts) {
        const std::string name(mount.package);
        const std::wstring key(name.begin(), name.end());
        if (read(L"BackwardSpeed", key.c_str(), mount.backwardSpeed, kBackwardSpeedMin, kBackwardSpeedMax)) {
            ++found;
        } else {
            wchar_t value[16];
            swprintf(value, 16, L"%.1f", mount.backwardSpeed);
            addMissing(L"BackwardSpeed", key.c_str(), value);
        }
    }
    if (readText(L"Reverse", L"InvertSteer")) {
        g_backwardSteerInvert = wcstol(text, nullptr, 10) != 0;
        ++found;
    } else {
        addMissing(L"Reverse", L"InvertSteer", g_backwardSteerInvert.load() ? L"1" : L"0");
    }
    std::wstring keyName = L"F5";
    if (readText(L"Keys", L"Toggle")) {
        ++found;
        if (const int vk = ParseKeyName(text)) {
            g_toggleKey = vk;
            keyName = text;
        } else {
            LogProblem("ini: Toggle=%ls isn't an accepted key, so F5 toggles the mod", text);
        }
    } else {
        addMissing(L"Keys", L"Toggle", L"F5");
    }
    if (!readText(L"Debug", L"Logging")) addMissing(L"Debug", L"Logging", L"0");
    Log("ini: %d values read, %d added, invert %d, toggle key %ls", found, added, g_backwardSteerInvert.load() ? 1 : 0,
        keyName.c_str());
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

// Return the milliseconds stickX has steered to one side, restarting when it starts steering or changes direction.
float SteerHeldMs(float stickX) {
    const int side = stickX > 0.01f ? 1 : stickX < -0.01f ? -1 : 0;
    const ULONGLONG now = GetTickCount64();
    if (side != g_steerSide.exchange(side)) g_steerStart = now;
    return static_cast<float>(now - g_steerStart.load());
}

// Return how far steering has ramped in over durationMs, from 0 to 1.
float SteerRamp(float heldMs, float durationMs) {
    return durationMs > 0.0f ? std::min(heldMs / durationMs, 1.0f) : 1.0f;
}

// Return whether a raw stick reads like keys: each axis at 0 or full, or both axes equal and large, as on a diagonal.
bool DigitalStick(float x, float y) {
    const float ax = std::fabs(x), ay = std::fabs(y);
    const auto keyAxis = [](float v) { return v < 0.001f || v > 0.999f; };
    return (keyAxis(ax) && keyAxis(ay)) || (std::fabs(ax - ay) < 0.001f && ax > 0.7f);
}

// Log the raw stick each time steering input changes between digital and analog.
void LogStickKind(float x, float y, bool digital) {
    static std::atomic<int> last{-1};
    if (last.exchange(digital ? 1 : 0) != (digital ? 1 : 0)) {
        Log("stick input %s: %.3f, %.3f", digital ? "digital" : "analog", x, y);
    }
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
    const float sign = -1.0f;
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

// Move the mount backward along its facing while g_backwardWanted is set, starting once it's below kBackwardMaxStartSpeed.
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
    const float rate = g_backwardRate.load();
    position.x += axisZ.x / len * rate * dt;
    position.z += axisZ.z / len * rate * dt;
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
    if (RidingGroundMount(ride) && actor == ride.mount) {
        UpdateBackwardWalk(ride);
    }
}

// Return whether backward input should read as forward for now: for kBackwardPulseMs once the mount is below
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

// Start another backward pulse if the mount has left its walk while the walk is running, so that the walking animation
// returns. On any mount, a chart state with no value means a stop or idle. The horse's kStateWalk checks also catch states
// that have a value. Wait for steering to end, because steering with a pulse is a forward turn.
void UpdateBackwardRecovery(const Ride& ride, bool steering) {
    static bool inWalk = false;
    static ULONGLONG nextAllowed = 0, lostSince = 0, nextLostPulse = 0;
    if (!g_backwardWanted.load() || !g_backwardWalking.load()) {
        inWalk = false;
        lostSince = 0;
        return;
    }
    uintptr_t state = 0;
    float value = 0;
    if (!ReadChartState(ride.mount, state, value)) return;
    const auto id = static_cast<uint16_t>(state);
    const ULONGLONG now = GetTickCount64();
    if (value == kMissing && !steering && g_mountSpeed.load() > kBackwardLostMinSpeed) {
        if (lostSince == 0) lostSince = now;
        if (now - lostSince >= kBackwardLostDelayMs && now >= nextLostPulse) {
            nextLostPulse = now + kBackwardLostGapMs;
            g_backwardPulseEnd = now + kBackwardPulseMs;
            Log("backward lost pulse from state %04X tick=%llu speed=%.2f", id, now, g_mountSpeed.load());
        }
        return;
    }
    lostSince = 0;
    if (id == kStateWalk || id == kStateWalkSteer) {
        inWalk = true;
        return;
    }
    if (!inWalk || id == kStateWalkStart || steering || now < nextAllowed ||
        g_mountSpeed.load() <= kBackwardLostMinSpeed) {
        return;
    }
    nextAllowed = now + kBackwardRecoveryGapMs;
    g_backwardPulseEnd = now + kBackwardPulseMs;
    Log("backward recovery pulse from state %04X at speed %.2f", id, g_mountSpeed.load());
}

// Set g_braking from backward input on a g_brakeMount, and log when it changes.
void UpdateBraking(bool braking) {
    if (g_braking.exchange(braking) != braking) Log("brake reverse %s", braking ? "on" : "off");
}

// Return whether the game's brake key is held, asking the same input object as the chart key condition, so that
// rebinding and gamepads count. Return false if g_keyCheck wasn't found or the object isn't available.
bool BrakeKeyHeld() {
    if (!g_keyCheck) return false;
    const uintptr_t world = ReadPtr(g_base + g_worldSystemSlot);
    const uintptr_t system = world ? ReadPtr(world + 0xD8) : 0;
    const uintptr_t list = system ? ReadPtr(system + 0x20) : 0;
    uint32_t count = 0;
    if (!list || !Read(list + 0x28, count) || count == 0) return false;
    const uintptr_t first = ReadPtr(list + 0x20);
    const uintptr_t input = first ? ReadPtr(first + 0x10) : 0;
    const uintptr_t vtable = input ? ReadPtr(input) : 0;
    const uintptr_t query = vtable ? ReadPtr(vtable + kKeyQuerySlot * sizeof(uintptr_t)) : 0;
    if (!query) return false;
    __try {
        return reinterpret_cast<bool (*)(uintptr_t, uint32_t)>(query)(input, kKeyBrakeHash);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// Return whether forward input should read as released, so that the chart slows the mount. This starts when S is pressed
// with W held, the brake key is pressed with forward true, or backward becomes true, while the mount is at
// kBrakeGapMinSpeed or above in a chart state with a value
// above 0. It ends on the second consecutive change into a chart state whose value is 0 or missing, below
// kBrakeGapStopSpeed, on steering, or after kBrakeGapMaxMs. A gap on g_brakeMount that started above kBrakeMountLowValue also ends on the first such
// change once speed is below kBrakeMountEndSpeed, so that a tap at cruise doesn't run on to the time cap.
bool UpdateBrakeGap(const Ride& ride, bool steering, bool backward, bool forward) {
    static bool sWas = false, backwardWas = false, brakeWas = false;
    static uintptr_t lastState = 0;
    static int slowingStates = 0;
    static ULONGLONG start = 0;
    static float startValue = 0.0f;

    const bool s = GameFocused() && KeyDown('S');
    const bool brake = BrakeKeyHeld();
    const bool brakePressed = brake && !brakeWas;
    const bool sPressed = s && !sWas && KeyDown('W');
    const bool backwardPressed = backward && !backwardWas;
    const bool pressed = sPressed || backwardPressed || (brakePressed && forward);
    brakeWas = brake;
    sWas = s;
    backwardWas = backward;
    uintptr_t state = 0;
    float value = kMissing;
    const bool haveState = ReadChartState(ride.mount, state, value);
    const ULONGLONG now = GetTickCount64();
    const float speed = g_mountSpeed.load();
    if (brakePressed) {
        Log("brake key press tick=%llu forward=%d steering=%d speed=%.2f value=%.3f", now, forward ? 1 : 0,
            steering ? 1 : 0, speed, value);
    }
    if (!g_brakeGap.load()) {
        if (!pressed || steering || speed < kBrakeGapMinSpeed || !haveState || value <= 0.0f ||
            g_backwardWalking.load()) {
            return false;
        }
        g_brakeGap = true;
        startValue = value;
        lastState = state;
        slowingStates = 0;
        start = now;
        Log("brake gap start tick=%llu speed=%.2f value=%.3f state=%04X shift=%d source=%s", now, speed, value,
            static_cast<uint16_t>(state), KeyDown(VK_SHIFT) ? 1 : 0,
            sPressed ? "S" : backwardPressed ? "stick" : "brake key");
        return true;
    }
    if (haveState && state != lastState) {
        lastState = state;
        slowingStates = value == 0.0f || value == kMissing ? slowingStates + 1 : 0;
    }
    const bool brakeMountDropped = ride.mount == g_brakeMount.load() && startValue > kBrakeMountLowValue &&
                                   slowingStates >= 1 && speed < kBrakeMountEndSpeed;
    const char* reason = slowingStates >= 2           ? "next state"
                         : brakeMountDropped          ? "end speed"
                         : speed < kBrakeGapStopSpeed ? "stopped"
                         : steering                   ? "steering"
                                                      : nullptr;
    if (!reason && now - start >= kBrakeGapMaxMs) reason = "time cap";
    if (!reason) return true;
    g_brakeGap = false;
    Log("brake gap end tick=%llu after=%llums speed=%.2f reason=%s", now, now - start, speed, reason);
    return false;
}

// Adjust the raw stick before the update reads it: clear backward input, leaving the backward walk to the position write,
// and widen forward steering. S alone starts the backward walk, and A or D during it steers. On g_brakeMount, S brakes
// instead, which the chart takes as reverse, and A or D pass through as they do with the brake key. In both reverses,
// g_backwardSteerInvert swaps A and D. Forward steering ramps in only for digital input, as an analog stick already moves
// gradually.
void OnStickRead(SafetyHookContext& ctx) {
    const Ride ride = ResolveRide();
    if (!RidingGroundMount(ride)) {
        g_backwardWanted = false;
        g_backwardWalking = false;
        UpdateBraking(false);
        g_brakeGap = false;
        return;
    }
    float velX = 0, velZ = 0;
    if (ReadVelocity(ride.mount, velX, velZ)) g_mountSpeed = std::hypot(velX, velZ);
    float stick[2];
    if (!SafeRead(ctx.rdi + kInput_RawStick, stick, sizeof stick)) return;
    uintptr_t inputActor = 0;
    if (Read(ctx.rdi + kInput_Actor, inputActor) && inputActor == ride.mount) {
        g_rawStickX = stick[0];
        g_rawStickY = stick[1];
    }
    const float heldMs = SteerHeldMs(stick[0]);
    const bool backward = stick[1] < kBackwardStickY;
    const bool steering = std::fabs(stick[0]) > 0.01f;
    const bool digital = DigitalStick(stick[0], stick[1]);
    if (steering) LogStickKind(stick[0], stick[1], digital);
    const bool brakeMount = ride.mount == g_brakeMount.load();
    UpdateBraking(brakeMount && backward);
    if (brakeMount && backward) {
        UpdateBackwardPulse(false);
        if (g_backwardSteerInvert.load()) stick[0] = -stick[0];
        stick[1] = 0.0f;
        SafeWrite(ctx.rdi + kInput_RawStick, stick, sizeof stick);
        return;
    }
    if (UpdateBrakeGap(ride, std::fabs(stick[0]) > kBrakeGapSteerX, backward, stick[1] > 0.0f)) {
        UpdateBackwardPulse(false);
        stick[0] = stick[1] = 0.0f;
        SafeWrite(ctx.rdi + kInput_RawStick, stick, sizeof stick);
        return;
    }
    const bool reverseSteering = backward && steering && g_backwardWalking.load();
    const bool pulse = UpdateBackwardPulse(backward && (!steering || reverseSteering));
    UpdateBackwardRecovery(ride, steering);
    if (backward) {
        if (reverseSteering) {
            // Scale by the stick's sideways push relative to its backward push, which is 1 for keys and diagonals.
            const float share = std::fabs(stick[0]) / std::max(std::fabs(stick[0]), std::fabs(stick[1]));
            stick[0] = std::copysign(kBackwardSteer * share, stick[0]);
            if (g_backwardSteerInvert.load()) stick[0] = -stick[0];
        }
        stick[1] = pulse ? 1.0f : 0.0f;
    } else if (stick[1] < 0.0f) {
        stick[1] = 0.0f;
    } else if (stick[1] > 0.0f && steering) {
        const float ramp = digital ? SteerRamp(heldMs, kSteerRampMs) : 1.0f;
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
        LogProblem("rodeo switch: stub allocation failed, so the mod won't pause for the taming minigame");
        return;
    }
    // lock inc qword ptr [rip+8]; jmp qword ptr [rip+10]; the counter is at +16 and the original address at +24.
    const uint8_t code[] = {0xF0, 0x48, 0xFF, 0x05, 8, 0, 0, 0, 0xFF, 0x25, 10, 0, 0, 0};
    memcpy(stub, code, sizeof code);
    memcpy(stub + 24, &vtable[kRodeoUiFrameSlot], sizeof(uintptr_t));
    FlushInstructionCache(GetCurrentProcess(), stub, 32);
    DWORD oldProtect = 0;
    if (!VirtualProtect(&vtable[kRodeoUiFrameSlot], sizeof(uintptr_t), PAGE_READWRITE, &oldProtect)) {
        LogProblem("rodeo switch: vtable protection failed, so the mod won't pause for the taming minigame");
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

// Answer g_brakeMount's key_brake condition as held while g_braking is set, and otherwise return the game's answer.
uint8_t OnKeyCondition(uintptr_t context, uintptr_t* condition, void* a3, void* a4, uint8_t a5, void* a6, void* a7) {
    const uint8_t result = g_keyCondition(context, condition, a3, a4, a5, a6, a7);
    if (!g_braking.load()) return result;
    uintptr_t conditionData = 0;
    uint32_t hash = 0;
    if (!Read(reinterpret_cast<uintptr_t>(condition), conditionData) || !Read(conditionData + 8, hash) ||
        hash != kKeyBrakeHash) {
        return result;
    }
    uintptr_t actor = 0;
    Read(context + 8, actor);
    // Log the first few actors that read key_brake while braking, to show which chart takes the brake.
    static std::atomic<uintptr_t> logged[4];
    static std::atomic<int> loggedCount{0};
    if (g_logging && loggedCount.load() < 4 &&
        std::none_of(logged, logged + 4, [actor](const auto& seen) { return seen.load() == actor; })) {
        logged[loggedCount.fetch_add(1) & 3] = actor;
        Log("key_brake read for actor %p, brake mount %p, game answers %d", reinterpret_cast<void*>(actor),
            reinterpret_cast<void*>(g_brakeMount.load()), result);
    }
    return actor != 0 && actor == g_brakeMount.load() ? kConditionHolds : result;
}

// Return the key condition's slot in the chart condition handler table, or null if it doesn't hold g_keyCheck.
uintptr_t* FindKeyConditionSlot() {
    __try {
        int32_t offset = 0;
        memcpy(&offset, reinterpret_cast<const void*>(g_base + g_conditionTableCall + 1), sizeof offset);
        const auto getTable = reinterpret_cast<uintptr_t* (*)()>(g_base + g_conditionTableCall + 5 + offset);
        uintptr_t* table = getTable();
        return table && table[kKeyConditionSlot] == g_base + g_keyCheck ? &table[kKeyConditionSlot] : nullptr;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

// Replace the key condition's handler table slot with OnKeyCondition, which calls the original, so that another mod's
// hook on the handler itself still runs. Without it, S uses the backward walk on every mount.
void InstallBrakeKey() {
    uintptr_t* slot = g_keyCheck && g_conditionTableCall ? FindKeyConditionSlot() : nullptr;
    if (!slot) {
        LogProblem("brake key: condition table not found, so S uses the backward walk on every mount");
        return;
    }
    DWORD oldProtect = 0;
    if (!VirtualProtect(slot, sizeof(uintptr_t), PAGE_READWRITE, &oldProtect)) {
        LogProblem("brake key: table protection failed, so S uses the backward walk on every mount");
        return;
    }
    g_keyCondition = reinterpret_cast<ConditionHandler>(*slot);
    *slot = reinterpret_cast<uintptr_t>(&OnKeyCondition);
    VirtualProtect(slot, sizeof(uintptr_t), oldProtect, &oldProtect);
    Log("brake key installed at table slot %p", static_cast<void*>(slot));
}

bool KeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

bool GameFocused() {
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid == GetCurrentProcessId();
}

// Log the ground mount's chart state when it changes, because the CSV's 10 rows a second can miss short states.
void LogStateChange() {
    static uint16_t last = 0;
    const uintptr_t mount = g_groundMount.load();
    uintptr_t state = 0;
    float value = 0;
    if (!mount || !ReadChartState(mount, state, value)) {
        last = 0;
        return;
    }
    const auto id = static_cast<uint16_t>(state);
    if (id == last) return;
    last = id;
    float x = 0, z = 0;
    const bool haveVelocity = ReadVelocity(mount, x, z);
    Log("state %04X tick=%llu speed=%.2f", id, GetTickCount64(), haveVelocity ? std::hypot(x, z) : kMissing);
}

void WriteTelemetry(const Ride& ride, const std::string& package) {
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

    fprintf(g_csv, "%llu,%d,%d,%d,%d,%d,%d,%08X,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", now, KeyDown('W') ? 1 : 0,
            KeyDown('A') ? 1 : 0, KeyDown('S') ? 1 : 0, KeyDown('D') ? 1 : 0, g_enabled.load() ? 1 : 0,
            ride.mounted ? 1 : 0, ride.mountKey, ride.forwardFromMount ? 1 : 0,
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
    fprintf(g_csv, ",%s,%.2f,%.3f,%.3f,%d,%d\n", package.c_str(), g_backwardRate.load(), g_rawStickX.load(),
            g_rawStickY.load(), KeyDown(VK_SHIFT) ? 1 : 0, g_brakeGap.load() ? 1 : 0);
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

constexpr DWORD kReadable = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
                            PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

// Return the readable parts of the game's sections of the given kind, joining adjacent regions so that a match can span
// them.
std::vector<Range> ReadableRanges(SectionKind kind) {
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
            LogProblem("lookup scan: read fault in %p to %p", range.first, range.last);
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
        LogProblem("lookup %s FAILED: %zu vtables for %s", name, vtables.size(), className);
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
        LogProblem("lookup %s FAILED: %zu signature matches", name, hits.size());
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
        LogProblem("lookup worldSystemSlot FAILED: %d references, next %d", top, next);
        return false;
    }
    g_worldSystemSlot = slot;
    Log("lookup worldSystemSlot: exe+0x%llX, %d references, next %d", static_cast<unsigned long long>(slot), top, next);
    return true;
}

// Run every lookup and log each result. Return whether the mod can run. The key check and condition table are left out
// of that, because without them only the brake reverse and the brake key's slowdown are lost.
bool RunLookups() {
    const ULONGLONG start = GetTickCount64();
    bool ok = LookupVtable("vtActorManager", kClassActorManager, g_vtActorManager);
    ok &= LookupVtable("vtVehicleComponent", kClassVehicleComponent, g_vtVehicleComponent);
    ok &= LookupVtable("vtRodeoUi", kClassRodeoUi, g_vtRodeoUi);
    ok &= LookupWorldSystemSlot();
    ok &= LookupSignature("hookSite", kHookSiteSignature, g_hookSite);
    ok &= LookupSignature("stickSite", kStickSiteSignature, g_stickSite);
    LookupSignature("keyCheck", kKeyCheckSignature, g_keyCheck);
    LookupSignature("conditionTableCall", kConditionTableCallSignature, g_conditionTableCall);
    Log("lookups took %llu ms", GetTickCount64() - start);
    return ok;
}

bool InstallHook() {
    if (!RunLookups()) {
        LogProblem("a required lookup failed: hooks not installed, mod idle");
        return false;
    }
    auto hook = safetyhook::MidHook::create(reinterpret_cast<void*>(g_base + g_hookSite), OnInputUpdate);
    auto stickHook = safetyhook::MidHook::create(reinterpret_cast<void*>(g_base + g_stickSite), OnStickRead);
    if (!hook || !stickHook) {
        LogProblem("hooks failed to install: mod idle");
        return false;
    }
    g_hook = std::move(*hook);
    g_stickHook = std::move(*stickHook);
    Log("hooks installed at exe+0x%llX and exe+0x%llX", static_cast<unsigned long long>(g_hookSite),
        static_cast<unsigned long long>(g_stickSite));
    InstallRodeoSwitch();
    if (g_keyCheck && g_logging) LogKeyCheckBytes();
    InstallBrakeKey();
    return true;
}

DWORD WINAPI ModThread(void* param) {
    const std::wstring dir = ModuleDir(static_cast<HMODULE>(param));
    g_base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    const std::wstring settings = dir + L"\\ProperHorseControls.ini";
    const std::wstring csvPath = dir + L"\\ProperHorseControls.csv";
    g_logPath = dir + L"\\ProperHorseControls.log";
    g_logging = GetPrivateProfileIntW(L"Debug", L"Logging", 0, settings.c_str()) != 0;
    if (g_logging) {
        OpenLog();
        g_csv = _wfsopen(csvPath.c_str(), L"w", _SH_DENYNO);
    } else {
        // Remove files from an earlier run, so that a log existing means this run had a problem.
        DeleteFileW(g_logPath.c_str());
        DeleteFileW(csvPath.c_str());
    }
    if (g_csv) {
        fputs("tick_ms,w,a,s,d,enabled,mounted,mount_key,forward_from_mount,rider_phys_yaw,mount_phys_yaw,"
              "rider_sync_yaw,mount_sync_yaw,speed,travel_yaw,"
              "i0_applied,i0_stick_x,i0_stick_y,i0_angle,i0_relative,i0_camera_yaw,i0_target_yaw,"
              "i1_applied,i1_stick_x,i1_stick_y,i1_angle,i1_relative,i1_camera_yaw,i1_target_yaw,"
              "mount_state,mount_value,mount_vel_x,mount_vel_z,mount_vel,rider_y,mount_package,backward_rate,"
              "raw_stick_x,raw_stick_y,shift,brake_gap\n",
              g_csv);
    }

    LoadSettings(settings);

    // Other mods report crashes when hooking during the first seconds of launch.
    Sleep(20000);
    if (!InstallHook()) return 0;

    bool toggleWas = false, wasMounted = false, wasFromMount = false, wasRodeo = false;
    uintptr_t seen[2] = {};
    uintptr_t currentMount = 0;
    bool classified = false, reportedUnknown = false;
    ULONGLONG lastTelemetry = 0;
    std::string package;
    for (;;) {
        Sleep(10);
        const bool toggle = GameFocused() && KeyDown(g_toggleKey);
        if (toggle && !toggleWas) {
            g_enabled = !g_enabled.load();
            Log("mod %s", g_enabled.load() ? "enabled" : "disabled");
        }
        toggleWas = toggle;
        if (g_logging) LogStateChange();

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
        if (ride.mount != currentMount) {
            currentMount = ride.mount;
            g_groundMount = 0;
            g_brakeMount = 0;
            classified = reportedUnknown = false;
            package.clear();
            if (currentMount) Log("mount appearance: %s", ReadAppearance(currentMount).c_str());
        }
        if (currentMount && !classified) {
            package = ReadPackageName(currentMount);
            if (const GroundMount* ground = FindGroundMount(package)) {
                classified = true;
                g_groundMount = currentMount;
                g_backwardRate = ground->backwardSpeed * kBackwardRatePerSpeed;
                const bool brake = ground->brakeReverse && g_keyCondition;
                if (brake) g_brakeMount = currentMount;
                Log("mount package: %s key=0x%08X tick=%llu (controls on, backward speed %.2f m/s, reverse by %s)",
                    package.c_str(), ride.mountKey, now, ground->backwardSpeed, brake ? "brake key" : "backward walk");
            } else if (!reportedUnknown) {
                reportedUnknown = true;
                Log("mount package not recognised: '%s' key=0x%08X tick=%llu (mod inactive)", package.c_str(),
                    ride.mountKey, now);
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
        if (ride.mounted) WriteTelemetry(ride, package);
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
