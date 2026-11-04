# Crashes, freezes and DMM problems

Build 1.0.0.3479.

## Freeze after a won minigame

The horse ignores input after a won minigame until a remount. It happens with both input hooks installed,
even when they write nothing and without the rodeo stub, and doesn't happen with the hooks left out or `HorseSteer` disabled. Which
hook causes it, and how, is unknown. No branch in the exe lands inside either hook's patched bytes. Logs are in
`scratch/probe-runs/` (`f5-off-win`, `no-stub-win`, `no-hooks-win`).

## Crashes and hangs

- **Stage 5 test chart crash (`scratch/probe-runs/stage5-backward/`):** the game crashed during loading on the only run with
  `HorseSteer Backward Walk Test` on, with the same `HorseSteer` build that loaded in part 1. It read a null vtable on the main thread
  at `exe+0x15D3E2A`, calling through `[rdi+0x160]`, which held the static object `exe+0x6E87008`. No mod is on the logged stack.
  In stage 5b the same 7 edits loaded and played without a crash as a JSON byte patch, and the built chart differed from the original
  by only those bytes, so the edits aren't the cause. Whether the loose file caused the crash or it was intermittent is unknown.
  DMM's support bundle, extracted to `support/` there, shows the chart in its manifest, and it's the only `actionchart/bin__` file in
  that manifest, so no other enabled mod shows that DMM can deliver a binary chart. `DmmRuntime.log` stops at 23:04:49, 25 s before the
  crash, and doesn't list the chart, so it doesn't show whether the game read it. The dump doesn't include the object at `rdi`.
  In stage 5b the chart was served 33 s after the same textures that ended the crash run's log, and the crash came 25 s after them, so
  the game may not have read the chart before crashing. CDMW's research notes report edited charts in DMM group folders loading and
  working on this exe with DMM 1.0, which also counts against the loose file.
- **Settings crash (14:47):** `user_engine_option_save.xml` was written at 14:47:24, the Nvidia driver logged GPU engine
  resets (`nvlddmkm` event 153) at 14:47:26 and 14:47:30, and the game then stopped with `DXGI_ERROR_DEVICE_HUNG`. The horse
  had been idle for over 2 minutes. HorseSteer's hook is CPU-only, so it's an unlikely cause, but it was active.
- **Camera settings:** with follow and lookahead on from launch, the game starts and runs normally, so the GPU reset is assumed
  not to be caused by the mod.
- **GPU crash while alt-tabbing (stage 5h, `scratch/probe-runs/stage5h/`):** a device removed error (`887A0006`) inside D3D12, with `renodx` and the Nvidia driver on the
  stack and no mod code. DMM reported a GPU hang or driver loss. The tester then removed OptiScaler, and no crash dump has been written since, through stage 6.
- **F10 freeze:** in HorseProbe run 1, the game froze near the end of the write watch, during Esc and Alt+Tab. The cause is unknown, and a probe
  cause is not ruled out.
- **Probe freeze (13:57):** Windows logged it as an application hang (`AppHangB1`), not a crash.

## DMM

- **Stale manifest:** a stale `bin64\dmm\manifest.tsv` pointing at a moved mods folder stops saves loading, with a missing
  `decorationparam_cclite_female.xml` error. A full mount rebuilds it.
- **Logs:** disabling an ASI in DMM deletes its logs.
