# Crashes, freezes and DMM problems

Build 1.0.0.3479.

## Freeze after a won minigame

The horse ignores input after a won minigame until a remount. In every logged run, it froze when no W, A, S or D key was pressed between the last release of W and the end of the minigame's UI, and moved when one was. The hooks were first blamed because `no-hooks-win` moved and the runs with hooks froze, but that run also had keys down in the window, so the hooks aren't shown to be the cause, and the freeze may happen without the mod. Six runs is few, and other keys aren't logged. No branch in the exe lands inside either hook's patched bytes.

| Run in `scratch/probe-runs/` | Hooks | UI ended, seconds after mounting | Keys in the window | Horse after the UI |
| --- | --- | --- | --- | --- |
| `stage10`, first tame | Yes | 15.3 | A, S, D and W from 11.4 to 14.0 | Moved |
| `no-hooks-win` | No | Not logged, 13.7 to 16.9 in the other runs | W from 13.8, D until 20.0 | Moved |
| `stage10`, second tame | Yes | 16.9 | None: W released at 12.8, next W at 17.5 | Frozen |
| `stage8` | Yes | 13.7 | None: W released at 10.2, next W at 14.7 | Frozen |
| `f5-off-win` | Yes, mod off with F5 | 16.1 | None: W released at 13.9, next W at 17.2 | Frozen |
| `no-stub-win` | Yes, no rodeo stub | Not logged | None: W released at 12.8, next W at 17.0 | Frozen |

[Stage 10b](../horsesteer-tests/stage-10b.md) tamed one horse with no hooks and it moved, but the tester wasn't sure the keys were released in time, and that build logs no keys, so it doesn't settle whether the freeze happens without hooks.

In `stage10`, both tames reached state `672A9`. The first left it for `62592` as the UI closed, and the second stayed in it until the dismount 11.5 s later, with keys reaching the stick values. S still moves the horse backward in that state, because the position writes don't depend on it.

One earlier freeze has been logged with chart state, on Steam build 26423880 (`scratch/probe-runs/stage8/`). The mount's chart reached a state seen nowhere else in that run (`state` ending `372A9`, no `value`, velocity 0) about when the
minigame ended, and stayed there for about 6 s until the dismount, with W held for about 3.5 s of that, and A and D for part of it. The keys did reach the stick values in both input slots, so the chart isn't leaving the state despite the input. The remount
put it in `3A73F`, and normal states followed.

## Crashes and hangs

- **Test chart crash (stage 5, `scratch/probe-runs/stage5-backward/`, with DMM's support bundle in `support/`):** the game crashed during
  loading on the only run with the chart as a loose file in a DMM group folder: a null vtable on the main thread at `exe+0x15D3E2A`, calling
  through `[rdi+0x160]`, with no mod on the stack. The same 7 edits loaded as a JSON byte patch in stage 5b, so the edits aren't the cause.
  Timing suggests the game may not have read the chart before crashing, and CDMW's notes report loose charts working on this exe with DMM 1.0,
  so the loose file isn't proven to be the cause either.
- **GPU reset after changing settings (14:47):** two `nvlddmkm` event 153 resets within 6 s of `user_engine_option_save.xml` being written,
  then `DXGI_ERROR_DEVICE_HUNG`, with the horse idle. Assumed not to be the mod: its hook is CPU-only, and the game runs normally with
  camera follow and lookahead on from launch.
- **GPU crash while alt-tabbing (stage 5h, `scratch/probe-runs/stage5h/`):** device removed (`887A0006`) inside D3D12, with `renodx` and the
  Nvidia driver on the stack and no mod code. The tester then removed OptiScaler, and no crash dump has been written since, through stage 6.
- **Stage 22 crash (`scratch/probe-runs/stage22-crash/`):** an access violation while a `std::vector` grew inside `OnStickRead`, on the second value the first build of `UpdateBrakeGap` tried to add. Windows reports it in `KERNELBASE!RaiseException` because the vector's cleanup rethrows, and the real stack is in the dump at `%LOCALAPPDATA%\CrashDumps`. The likely cause, not proven, is the hook running on more than one thread. Keep containers and other allocating state out of the hooks, and have `ModThread` write anything they read.
- **Freeze before stage 24 (`scratch/probe-runs/stage24-crash/`):** an `AppHangB1` about 45 s after the hooks were installed, with no dump. The log has no `mounted=` line, so `BrakeKeyHeld` hadn't run. The cause is unknown, and it didn't recur in stages 24 and 25.
- **Hook threads:** `OnStickRead` runs on more than one game thread, as one `brake gap end` in about 266 logged taps was written twice on one line with the same tick (stage 29). `UpdateBrakeGap`'s statics are plain values, so a clash can only miscount a state change or key press. `BrakeKeyHeld` calls the game's key query from those threads, and whether that query is safe to call concurrently isn't known. It's wrapped in `__try` and ran through stages 24 to 29 with no crash. If players report hangs while riding, look there first.
- **HorseProbe freezes:** the game froze near the end of the F10 write watch in run 1, during Esc and Alt+Tab, with a probe cause not ruled out.
  A freeze at 13:57 was logged by Windows as an application hang (`AppHangB1`), not a crash.

## DMM

- **Stale manifest:** a stale `bin64\dmm\manifest.tsv` pointing at a moved mods folder stops saves loading, with a missing
  `decorationparam_cclite_female.xml` error. A full mount rebuilds it.
- **Logs:** disabling an ASI in DMM deletes its logs.
