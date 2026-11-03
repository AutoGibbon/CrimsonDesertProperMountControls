// Read-only diagnostic for riding: logs camera, rider and horse headings, finds the WASD move vector in memory, and
// traces the code that writes it. Never writes game memory.
//
// F8  guided scan for the move vector. F10 hardware write watch on the next 4 scan results for 5 seconds.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>

#include <atomic>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

namespace {

// Published for build 1.0.0.3479 by CrimsonDesertCoop; every chain is checked at runtime.
constexpr uint32_t kPeTimestamp = 0x6AE6E3E8;
constexpr uintptr_t kWorldSystemSlot = 0x71B6C50;
constexpr uintptr_t kChunkOrigin = 0x71A2EB0;
constexpr uintptr_t kCameraSlot = 0x70D2F68;
constexpr float kCameraBlockSentinel = 6360000.0f;
constexpr uintptr_t kVtActorManager = 0x593ECF0;
constexpr uint32_t kHorseKey = 0xB0100003;

constexpr float kMissing = -9999.0f;
constexpr float kRadToDeg = 57.29578f;

struct Vec3 {
    float x, y, z;
};

struct ActorPose {
    bool valid = false;
    Vec3 pos{kMissing, kMissing, kMissing};
    float physYaw = kMissing;
    float syncYaw = kMissing;
};

FILE* g_log = nullptr;
FILE* g_csv = nullptr;
uintptr_t g_base = 0;
bool g_offsetsValid = false;
std::string g_lastFailure;

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

void LogFailureOnce(const char* why) {
    if (g_lastFailure == why) return;
    g_lastFailure = why;
    Log("telemetry: %s", why);
}

bool SafeRead(uintptr_t addr, void* out, size_t size) {
    __try {
        memcpy(out, reinterpret_cast<const void*>(addr), size);
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

// Read an actor's heading from both its physics basis and its TransformSync quaternion.
ActorPose ReadPose(uintptr_t actor, const Vec3& origin) {
    ActorPose pose;
    const uintptr_t components = ReadPtr(actor + 0x68);
    if (!components) return pose;

    if (const uintptr_t control = ReadPtr(components + 0x40)) {
        const uintptr_t inner = ReadPtr(control + 0x140);
        const uintptr_t physics = inner ? ReadPtr(inner + 0x2C0) : 0;
        Vec3 axisZ, local;
        if (physics && Read(physics + 0x80, axisZ) && Read(physics + 0x90, local) && Finite(axisZ) && Finite(local)) {
            pose.physYaw = YawDeg(axisZ.x, axisZ.z);
            pose.pos = {local.x + origin.x, local.y + origin.y, local.z + origin.z};
            pose.valid = true;
        }
    }
    if (const uintptr_t sync = ReadPtr(components + 0x1A0)) {
        float q[4];
        if (SafeRead(sync + 0xA4, q, sizeof q)) {
            const float n = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
            if (std::isfinite(n) && std::fabs(n - 1.0f) < 0.02f) {
                pose.syncYaw = YawDeg(2 * (q[0] * q[2] + q[3] * q[1]), 1 - 2 * (q[0] * q[0] + q[1] * q[1]));
                pose.valid = true;
            }
        }
    }
    return pose;
}

bool ReadCameraForward(Vec3& forward) {
    const uintptr_t camera = ReadPtr(g_base + kCameraSlot);
    if (!camera) {
        LogFailureOnce("camera slot empty");
        return false;
    }
    const uintptr_t block = ReadPtr(camera + 0x428);
    float sentinel = 0.0f;
    if (!block || !Read(block + 0xAC0, sentinel) || sentinel != kCameraBlockSentinel) {
        LogFailureOnce("camera block sentinel mismatch");
        return false;
    }
    if (!Read(block + 0x90, forward) || !Finite(forward)) {
        LogFailureOnce("camera forward unreadable");
        return false;
    }
    return true;
}

bool KeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

bool GameFocused() {
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid == GetCurrentProcessId();
}

void WriteTelemetry() {
    static ULONGLONG lastFlush = 0;
    static Vec3 lastPos{};
    static ULONGLONG lastTick = 0;
    static size_t rows = 0;
    if (!g_csv || rows > 500000) return;

    const uintptr_t world = ReadPtr(g_base + kWorldSystemSlot);
    const uintptr_t manager = world ? ReadPtr(world + 0x30) : 0;
    if (!manager || !HasVtable(manager, kVtActorManager)) {
        LogFailureOnce("actor manager missing (not in world?)");
        return;
    }
    Vec3 origin{};
    if (!Read(g_base + kChunkOrigin, origin) || !Finite(origin)) {
        LogFailureOnce("chunk origin unreadable");
        return;
    }
    const uintptr_t playerActor = ReadPtr(manager + 0x50);
    const uintptr_t horseActor = FindActor(manager, kHorseKey);
    const ActorPose player = playerActor ? ReadPose(playerActor, origin) : ActorPose{};
    const ActorPose horse = horseActor ? ReadPose(horseActor, origin) : ActorPose{};
    Vec3 cam{kMissing, kMissing, kMissing};
    const float camYaw = ReadCameraForward(cam) ? YawDeg(cam.x, cam.z) : kMissing;

    float distance = kMissing;
    if (player.valid && horse.valid && player.pos.x != kMissing && horse.pos.x != kMissing) {
        const float dx = player.pos.x - horse.pos.x, dy = player.pos.y - horse.pos.y, dz = player.pos.z - horse.pos.z;
        distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    const ULONGLONG now = GetTickCount64();
    float speed = kMissing, travelYaw = kMissing;
    if (horse.pos.x != kMissing && lastTick != 0 && now > lastTick) {
        const float dt = static_cast<float>(now - lastTick) / 1000.0f;
        const float dx = horse.pos.x - lastPos.x, dz = horse.pos.z - lastPos.z;
        speed = std::sqrt(dx * dx + dz * dz) / dt;
        if (speed > 0.1f) travelYaw = YawDeg(dx, dz);
    }
    if (horse.pos.x != kMissing) {
        lastPos = horse.pos;
        lastTick = now;
    }

    fprintf(g_csv, "%llu,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f\n", now,
            KeyDown('W') ? 1 : 0, KeyDown('A') ? 1 : 0, KeyDown('S') ? 1 : 0, KeyDown('D') ? 1 : 0, camYaw,
            player.physYaw, player.syncYaw, horse.physYaw, horse.syncYaw, horse.pos.x, horse.pos.y, horse.pos.z,
            distance, speed, travelYaw);
    ++rows;
    if (now - lastFlush > 1000) {
        fflush(g_csv);
        lastFlush = now;
    }
    g_lastFailure.clear();
}

struct Candidate {
    uintptr_t addr;
    float wx, wy, dx, dy;
};

std::vector<Candidate> g_candidates;
size_t g_nextWatch = 0;

constexpr size_t kChunkSize = 64 * 1024 * 1024;
constexpr size_t kPerThreadCap = 4 * 1024 * 1024;
constexpr float kAxisTolerance = 0.05f;
constexpr float kMatchTolerance = 0.15f;

struct Chunk {
    uintptr_t base;
    size_t size;
};

// Append addresses in [base, base+size) that hold an axis-aligned unit 2D float vector; returns false on a fault.
bool ScanChunk(uintptr_t base, size_t size, uintptr_t* out, size_t cap, size_t* count) {
    __try {
        for (size_t off = 0; off + 8 <= size; off += 4) {
            const float* p = reinterpret_cast<const float*>(base + off);
            const float ax = std::fabs(p[0]), ay = std::fabs(p[1]);
            const bool match = (ax < kAxisTolerance && std::fabs(ay - 1.0f) < kAxisTolerance) ||
                               (ay < kAxisTolerance && std::fabs(ax - 1.0f) < kAxisTolerance);
            if (match && *count < cap) out[(*count)++] = base + off;
        }
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

std::vector<Chunk> CollectChunks() {
    std::vector<Chunk> chunks;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    uintptr_t addr = reinterpret_cast<uintptr_t>(si.lpMinimumApplicationAddress);
    const uintptr_t end = reinterpret_cast<uintptr_t>(si.lpMaximumApplicationAddress);
    MEMORY_BASIC_INFORMATION mbi;
    while (addr < end && VirtualQuery(reinterpret_cast<void*>(addr), &mbi, sizeof mbi) == sizeof mbi) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        // Write-combined pages hold GPU uploads and read very slowly.
        if (mbi.State == MEM_COMMIT && mbi.Protect == PAGE_READWRITE && mbi.Type == MEM_PRIVATE) {
            for (size_t off = 0; off < mbi.RegionSize; off += kChunkSize) {
                const size_t left = mbi.RegionSize - off;
                chunks.push_back({base + off, left > kChunkSize ? kChunkSize + 4 : left});
            }
        }
        addr = base + mbi.RegionSize;
    }
    return chunks;
}

void FirstPass() {
    const std::vector<Chunk> chunks = CollectChunks();
    size_t totalBytes = 0;
    for (const Chunk& c : chunks) totalBytes += c.size;
    constexpr unsigned kThreads = 4;
    std::vector<std::vector<uintptr_t>> results(kThreads, std::vector<uintptr_t>(kPerThreadCap));
    size_t counts[kThreads] = {};
    std::atomic<size_t> next{0};
    std::vector<std::thread> workers;
    const ULONGLONG start = GetTickCount64();
    for (unsigned t = 0; t < kThreads; ++t) {
        workers.emplace_back([&, t] {
            for (size_t i = next.fetch_add(1); i < chunks.size(); i = next.fetch_add(1)) {
                ScanChunk(chunks[i].base, chunks[i].size, results[t].data(), kPerThreadCap, &counts[t]);
            }
        });
    }
    for (std::thread& w : workers) w.join();

    g_candidates.clear();
    for (unsigned t = 0; t < kThreads; ++t) {
        if (counts[t] == kPerThreadCap) Log("scan: thread %u hit its result cap; results may be incomplete", t);
        for (size_t i = 0; i < counts[t]; ++i) {
            float v[2];
            if (SafeRead(results[t][i], v, sizeof v)) g_candidates.push_back({results[t][i], v[0], v[1], 0, 0});
        }
    }
    Log("scan W: %.1f GB in %llu ms, %zu candidates", static_cast<double>(totalBytes) / (1024.0 * 1024.0 * 1024.0),
        GetTickCount64() - start, g_candidates.size());
}

enum class Pass { D, S, A, Neutral };

void FilterPass(Pass pass) {
    size_t kept = 0;
    for (const Candidate& original : g_candidates) {
        Candidate c = original;
        float v[2];
        if (!SafeRead(c.addr, v, sizeof v)) continue;
        bool keep = false;
        switch (pass) {
            case Pass::D:
                keep = std::fabs(std::hypot(v[0], v[1]) - 1.0f) < kMatchTolerance &&
                       std::fabs(v[0] * c.wx + v[1] * c.wy) < kMatchTolerance;
                c.dx = v[0];
                c.dy = v[1];
                break;
            case Pass::S:
                keep = std::hypot(v[0] + c.wx, v[1] + c.wy) < kMatchTolerance;
                break;
            case Pass::A:
                keep = std::hypot(v[0] + c.dx, v[1] + c.dy) < kMatchTolerance;
                break;
            case Pass::Neutral:
                keep = std::hypot(v[0], v[1]) < kMatchTolerance;
                break;
        }
        if (keep) g_candidates[kept++] = c;
    }
    g_candidates.resize(kept);
}

// Guided scan: F8, then hold W, D, S and A in turn until each beep, then release everything until the final beeps.
class ScanGuide {
public:
    bool Active() const { return step_ != 0; }

    void Start() {
        step_ = 1;
        heldSince_ = 0;
        waitForRelease_ = false;
        g_nextWatch = 0;
        Log("scan: started; hold W, D, S, A in turn until each beep, then release");
        Beep(1000, 150);
    }

    void Update() {
        static constexpr int kKeys[4] = {'W', 'D', 'S', 'A'};
        const bool any = KeyDown('W') || KeyDown('A') || KeyDown('S') || KeyDown('D');
        if (waitForRelease_) {
            if (!any) {
                waitForRelease_ = false;
                heldSince_ = 0;
            }
            return;
        }
        bool ready;
        if (step_ <= 4) {
            int held = 0;
            for (int k : kKeys) held += KeyDown(k) ? 1 : 0;
            ready = held == 1 && KeyDown(kKeys[step_ - 1]);
        } else {
            ready = !any;
        }
        if (!ready) {
            heldSince_ = 0;
            return;
        }
        const ULONGLONG now = GetTickCount64();
        if (heldSince_ == 0) heldSince_ = now;
        if (now - heldSince_ < (step_ <= 4 ? 800u : 1500u)) return;

        RunStep();
        if (step_ == 0) return;
        waitForRelease_ = step_ <= 5;
        heldSince_ = 0;
    }

private:
    void RunStep() {
        switch (step_) {
            case 1:
                FirstPass();
                break;
            case 2:
                FilterPass(Pass::D);
                Log("scan D: %zu candidates", g_candidates.size());
                break;
            case 3:
                FilterPass(Pass::S);
                Log("scan S: %zu candidates", g_candidates.size());
                break;
            case 4:
                FilterPass(Pass::A);
                Log("scan A: %zu candidates", g_candidates.size());
                break;
            default:
                FilterPass(Pass::Neutral);
                Log("scan neutral: %zu candidates", g_candidates.size());
                for (size_t i = 0; i < g_candidates.size() && i < 256; ++i) {
                    const Candidate& c = g_candidates[i];
                    Log("  result %zu: %p W=(%.2f,%.2f) D=(%.2f,%.2f)", i, reinterpret_cast<void*>(c.addr), c.wx, c.wy,
                        c.dx, c.dy);
                }
                Beep(1000, 100);
                Beep(1500, 100);
                Beep(2000, 200);
                step_ = 0;
                return;
        }
        Beep(1500, 150);
        ++step_;
    }

    int step_ = 0;
    ULONGLONG heldSince_ = 0;
    bool waitForRelease_ = false;
};

struct WriterCount {
    std::atomic<uintptr_t> rip{0};
    std::atomic<uint32_t> hits{0};
    std::atomic<uint32_t> slots{0};
};

constexpr size_t kWriters = 64;
WriterCount g_writers[kWriters];
std::atomic<bool> g_watchEverArmed{false};
ULONGLONG g_watchUntil = 0;
void* g_vectoredHandler = nullptr;

LONG CALLBACK WatchHandler(EXCEPTION_POINTERS* info) {
    if (info->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP || !g_watchEverArmed.load()) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    const DWORD64 dr6 = info->ContextRecord->Dr6;
    if ((dr6 & 0xF) == 0) return EXCEPTION_CONTINUE_SEARCH;
    const uintptr_t rip = reinterpret_cast<uintptr_t>(info->ExceptionRecord->ExceptionAddress);
    for (WriterCount& w : g_writers) {
        uintptr_t expected = 0;
        if (w.rip.load() == rip || w.rip.compare_exchange_strong(expected, rip) || expected == rip) {
            w.hits.fetch_add(1);
            w.slots.fetch_or(static_cast<uint32_t>(dr6 & 0xF));
            break;
        }
    }
    info->ContextRecord->Dr6 = 0;
    return EXCEPTION_CONTINUE_EXECUTION;
}

// Set debug registers on every other thread of this process; `count` of 0 clears them. New threads are not covered.
void ApplyDebugRegisters(const uintptr_t* addrs, size_t count) {
    const DWORD self = GetCurrentThreadId(), pid = GetCurrentProcessId();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;
    THREADENTRY32 entry{};
    entry.dwSize = sizeof entry;
    DWORD64 dr7 = 0;
    for (size_t i = 0; i < count; ++i) {
        // Local enable, break on write, 4-byte length.
        dr7 |= (1ull << (i * 2)) | (1ull << (16 + i * 4)) | (3ull << (18 + i * 4));
    }
    int applied = 0, failed = 0;
    for (BOOL ok = Thread32First(snapshot, &entry); ok; ok = Thread32Next(snapshot, &entry)) {
        if (entry.th32OwnerProcessID != pid || entry.th32ThreadID == self) continue;
        HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT |
                                       THREAD_QUERY_INFORMATION, FALSE, entry.th32ThreadID);
        if (!thread) {
            ++failed;
            continue;
        }
        if (SuspendThread(thread) != static_cast<DWORD>(-1)) {
            alignas(16) CONTEXT ctx{};
            ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
            if (GetThreadContext(thread, &ctx)) {
                ctx.Dr0 = count > 0 ? addrs[0] : 0;
                ctx.Dr1 = count > 1 ? addrs[1] : 0;
                ctx.Dr2 = count > 2 ? addrs[2] : 0;
                ctx.Dr3 = count > 3 ? addrs[3] : 0;
                ctx.Dr7 = dr7;
                if (SetThreadContext(thread, &ctx)) ++applied;
                else ++failed;
            } else {
                ++failed;
            }
            ResumeThread(thread);
        } else {
            ++failed;
        }
        CloseHandle(thread);
    }
    CloseHandle(snapshot);
    Log("watch: debug registers %s on %d threads, %d failed", count ? "armed" : "cleared", applied, failed);
}

void ArmWatch() {
    if (g_watchUntil != 0) return;
    if (g_candidates.empty()) {
        Log("watch: no scan results; press F8 first");
        return;
    }
    if (g_nextWatch >= g_candidates.size()) g_nextWatch = 0;
    uintptr_t addrs[4];
    size_t count = 0;
    while (count < 4 && g_nextWatch < g_candidates.size()) addrs[count++] = g_candidates[g_nextWatch++].addr;
    for (WriterCount& w : g_writers) {
        w.rip = 0;
        w.hits = 0;
        w.slots = 0;
    }
    if (!g_vectoredHandler) g_vectoredHandler = AddVectoredExceptionHandler(1, WatchHandler);
    g_watchEverArmed = true;
    for (size_t i = 0; i < count; ++i) Log("watch: slot %zu = %p", i, reinterpret_cast<void*>(addrs[i]));
    ApplyDebugRegisters(addrs, count);
    g_watchUntil = GetTickCount64() + 5000;
    Beep(800, 150);
}

void FinishWatchIfDue() {
    if (g_watchUntil == 0 || GetTickCount64() < g_watchUntil) return;
    ApplyDebugRegisters(nullptr, 0);
    g_watchUntil = 0;
    for (const WriterCount& w : g_writers) {
        const uintptr_t rip = w.rip.load();
        if (rip == 0) continue;
        Log("watch: writer %p (exe+0x%llX) hits=%u slots=0x%X", reinterpret_cast<void*>(rip),
            static_cast<unsigned long long>(rip - g_base), w.hits.load(), w.slots.load());
    }
    Log("watch: finished");
    Beep(800, 80);
    Beep(800, 80);
}

std::wstring ModuleDir(HMODULE module) {
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(module, path, MAX_PATH);
    std::wstring dir(path);
    const size_t slash = dir.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"." : dir.substr(0, slash);
}

DWORD WINAPI ProbeThread(void* param) {
    const std::wstring dir = ModuleDir(static_cast<HMODULE>(param));
    g_log = _wfsopen((dir + L"\\HorseProbe.log").c_str(), L"w", _SH_DENYNO);
    g_csv = _wfsopen((dir + L"\\HorseProbe.csv").c_str(), L"w", _SH_DENYNO);
    if (g_csv) {
        fputs("tick_ms,w,a,s,d,cam_yaw,player_phys_yaw,player_sync_yaw,horse_phys_yaw,horse_sync_yaw,"
              "horse_x,horse_y,horse_z,rider_horse_dist,speed,travel_yaw\n", g_csv);
    }

    g_base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(g_base);
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(g_base + dos->e_lfanew);
    g_offsetsValid = nt->FileHeader.TimeDateStamp == kPeTimestamp;
    Log("HorseProbe loaded: base %p, PE timestamp 0x%08X", reinterpret_cast<void*>(g_base),
        nt->FileHeader.TimeDateStamp);
    if (!g_offsetsValid) Log("unsupported build: telemetry disabled; F8 and F10 still work");

    // Other mods report crashes when acting during the first seconds of launch.
    Sleep(20000);
    Log("ready: F8 scan, F10 write watch");

    ScanGuide guide;
    bool f8Was = false, f10Was = false;
    ULONGLONG lastTelemetry = 0;
    for (;;) {
        Sleep(10);
        const bool focused = GameFocused();
        const bool f8 = focused && KeyDown(VK_F8), f10 = focused && KeyDown(VK_F10);
        if (f8 && !f8Was) guide.Start();
        if (f10 && !f10Was) ArmWatch();
        f8Was = f8;
        f10Was = f10;

        if (guide.Active() && focused) guide.Update();
        FinishWatchIfDue();

        const ULONGLONG now = GetTickCount64();
        if (g_offsetsValid && now - lastTelemetry >= 100) {
            lastTelemetry = now;
            WriteTelemetry();
        }
    }
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        if (HANDLE thread = CreateThread(nullptr, 0, ProbeThread, module, 0, nullptr)) CloseHandle(thread);
    }
    return TRUE;
}
