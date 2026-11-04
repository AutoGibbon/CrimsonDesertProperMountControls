# Crashes, freezes and DMM problems

Build 1.0.0.3479.

## Freeze after a won minigame

The horse ignores input after a won minigame until a remount. It happens with both input hooks installed,
even when they write nothing and without the rodeo stub, and doesn't happen with the hooks left out or `HorseSteer` disabled. Which
hook causes it, and how, is unknown. No branch in the exe lands inside either hook's patched bytes. Logs are in
`scratch/probe-runs/` (`f5-off-win`, `no-stub-win`, `no-hooks-win`).

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
- **HorseProbe freezes:** the game froze near the end of the F10 write watch in run 1, during Esc and Alt+Tab, with a probe cause not ruled out.
  A freeze at 13:57 was logged by Windows as an application hang (`AppHangB1`), not a crash.

## DMM

- **Stale manifest:** a stale `bin64\dmm\manifest.tsv` pointing at a moved mods folder stops saves loading, with a missing
  `decorationparam_cclite_female.xml` error. A full mount rebuilds it.
- **Logs:** disabling an ASI in DMM deletes its logs.
