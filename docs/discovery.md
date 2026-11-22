# Crimson Desert: horse-relative controls

## Problem

All riding movement is camera-relative: forward input moves the horse towards the camera heading, and left/right input
moves it sideways relative to the camera instead of steering it.

## Objective

Replicate Red Dead Redemption 2's horse-relative controls, through a native ASI mod.

## Requirements

- **Forward/back:** W moves the horse along its own facing. S from standing walks the horse backward, as in RDR2. Tapping S with W held, pulling the stick back, or pressing the game's brake key slows the mount by a gait, on wagons too, and while steering, which keeps turning the mount. S must not answer the game's brake key: players press
  S while riding to turn, which would start unwanted drifts, and it makes escaping a drift harder. Alt still brakes and drifts as in the base game, since a player who wants to drift does it on purpose.
- **Steering:** A and D change the horse's heading while held. With no other input the horse walks in a small radius,
  and releasing W while steering at speed gives a tighter turn. The user wants to keep both behaviours.
- **Pivot:** S with A or D turns the horse nearly on the spot, at about 52°/s and 0.5 m/s, and the user wants to keep it.
- **Camera:** looking around freely never changes the horse's course.
- **Input:** keyboard and mouse, and gamepad. Gamepad users are never worse off: the stick pulled back works wherever S
  does.
- **Mounts:** a public release needs every legged ground mount to work. Carriages should get similar controls if possible. The exosuit/mech suit is excluded.

## Done when

- All requirements have been tested in game on the current build.
- The hook points and evidence are documented, with verified findings kept separate from hypotheses.

## Approach

`src/HorseSteer` hooks `ClientInputActorComponent`'s per-frame update while the player is mounted, and replaces its copied
camera forward and right vectors with ones aligned to the mount. The game's own angle calculation then resolves WASD
against the mount's facing, so the camera stays free. How it works is in [horsesteer-design.md](horsesteer-design.md),
and test results are in [state/tests.json](state/tests.json).

The game setting and camera preset routes are ruled out: both work by keeping the camera behind the horse, which breaks
free look. No existing mod does this: the user searched Nexus and found none that do anything like this, and GitHub and web
searches found none either.

## Findings

In [findings/](findings/), by topic, each section naming its build:

- [input-and-steering.md](findings/input-and-steering.md): input, camera vectors, turn rates and the taming minigame
- [charts-and-braking.md](findings/charts-and-braking.md): chart data, brake and drift, and reading or overriding keys
- [mounts-and-vehicles.md](findings/mounts-and-vehicles.md): the mount link, packages, vehicle and character tables, and carriages
- [backward-walk.md](findings/backward-walk.md): how the horse's backward walk works
- [stability.md](findings/stability.md): the freeze after a win, crashes and DMM problems
- [offsets.md](findings/offsets.md): every address and chain, and the Horse Tap Brake reads
- [horse-call-teleport.md](findings/horse-call-teleport.md): the distance at which a called horse teleports

## Resources

- [CDMW](https://github.com/Ratty123/CDMW-Full): archive and action chart decoding
- [CrimsonDesertCoop](https://github.com/macauleywilliams96-dotcom/CrimsonDesertCoop): research on this build, including
  `camera_reader`, `horse_diag` and `docs/research/caballo-coop.md`
- [CrimsonDesertTelemetry](https://github.com/ZappendusterFX/CrimsonDesertTelemetry): camera layout and write-watch
  tracing in `docs/ENGINE_CAMERA_RESEARCH.md`
- [CrimsonDesertTools](https://github.com/tkhquang/CrimsonDesertTools): maintained native mods and shared code
- Horse Tap Brake 1.0.2: chart state, velocity and key query reads, update-safe lookups, and the tap-to-slow idea and
  its release condition, with its author's permission. The slowdown itself zeroes the stick instead of holding the brake. See `findings/offsets.md`
- [CrimsonDesertModdingTools](https://github.com/NattKh/CrimsonDesertModdingTools): field names for static tables in `schemas/`. Its `vehicleinfo` and `characterinfo` mount parsers
  don't match this build's row layout, and its PAZ tools add nothing to CDMW and DMM
- [Ghidra](https://github.com/NationalSecurityAgency/ghidra), [x64dbg](https://github.com/x64dbg/x64dbg),
  [Cheat Engine](https://github.com/cheat-engine/cheat-engine), [SafetyHook](https://github.com/cursey/safetyhook)
