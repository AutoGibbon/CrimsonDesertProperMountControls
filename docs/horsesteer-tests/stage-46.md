# Stage 46 test

Probe for the actor the player controls after switching to a follower. The mod takes the rider from
`[manager+0x50]`, which stayed on Kliff after a switch in the session logged in
`scratch/probe-runs/follower-mount-20261116/`. When the player mounted as the follower, Kliff mounted his own horse, and
the mod treated Kliff's horse as the player's. The build that session ran rewrote every actor's input against that
horse's facing.

This build only rewrites the input of `[manager+0x50]` and its mount, so after a switch the follower's horse should
ride with the game's camera-relative controls and Kliff's horse may still follow the player's keys. It changes nothing
else. With `Logging=1`, it adds:

- **Input census:** each actor that the input update runs for, and how often.
- **Pointer hunt:** once a second, it finds memory within three pointer steps of WorldSystem and `ClientActorManager`
  that points at one of those actors, its input component or its components object, and logs each one that changes.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`.

## Steps

Wait about 5 s between steps, so that each step's changes land in a different scan.

1. Load in as Kliff, on foot, with the follower summoned.
2. Switch to the follower.
3. As the follower, mount a horse, ride a short way, then dismount.
4. Switch back to Kliff.
5. As Kliff, mount, ride a short way, then dismount.

Note what Kliff's horse did during step 3. Close the game and copy the log and CSV to `scratch/probe-runs/stage46/`.

## Logs

- **`input actor aN=ADDRESS`:** each actor's first input update, with its package and appearance. `aN` names the actor
  in the lines below.
- **`input actors:`:** the actors with input updates in the last second, each with its update count `n`, the mod's
  rewrites `applied`, and its vehicle link `key` and `seat`. `(rider)` marks `[manager+0x50]`. A line is only written
  when the set, a link or `applied` changes.
- **`ref PATH = TAG ADDRESS`:** a qword that holds a known actor. `W` is WorldSystem and `M` is `ClientActorManager`,
  and each `>` is a pointer step, so `M+50` is the rider field and `W+18>+50>+10` is `[[[W+0x18]+0x50]+0x10]`. `TAG` is
  `aN`, `aN.input` or `aN.components`, or `rider` and `mount` before they've had an input update. `was` gives the old
  value, and `gone` means the path no longer holds a known actor. Each path logs up to 10 changes.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| A `ref` path changes from Kliff's tag to the follower's at step 2 and back at step 4 | The controlled actor's pointer. Check that it doesn't change at the mounts |
| Only `M+50` holds Kliff, with no path following the switch | The pointer is further than three steps from these roots, or on another object. The census still shows which actors the player's input reaches |

Tester notes.

1. Game loads, then player summons the follower, then fast travel to a meeting location. After this the follower can be switched to.
3. kliff horse appeared to be fully controlled by the AI.
## Results

Run on 2026-11-16, with Damian as the follower. Logs are in `scratch/probe-runs/stage46/`.

- **Input:** the input update only runs for the controlled character and, while it rides, its mount. After the switch,
  Damian got every update and Kliff none. Mounted, Damian read `key=B0100005 seat=0` and his own horse (`cd_r0001_horse`,
  appearance `_02007`) got updates too. Kliff's horse got none while Damian was controlled, and no input was rewritten
  during that ride, matching the tester's note that Kliff's horse was fully under AI control.
- **Controlled actor:** `[[manager+0x58]+0xD8]` held Kliff at load and changed to Damian, Kliff and Damian again at
  each switch, and didn't change at any mount or dismount. `[[manager+0x148]]`, `[[manager+0x1D8]]`,
  `[[manager+0x268]]`, `[[[manager+0x58]+0x8]+0x38]` and `[[[manager+0x58]+0xE8]+0x38]` did the same. `[manager+0x50]`
  stayed on Kliff.
- **The earlier break:** the build in that session, from 15 Nov, rewrote the input of every actor against the facing of
  Kliff's horse, which `[manager+0x50]` led to. This build only rewrote `[manager+0x50]` and its mount.
- **No problems logged,** and no hitches were reported from the pointer hunt.
