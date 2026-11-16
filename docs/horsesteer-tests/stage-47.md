# Stage 47 test

Controls for the character the player controls. The mod now takes the rider from `[[manager+0x58]+0xD8]`, found in
[stage 46](stage-46.md), instead of `[manager+0x50]`, which stays on Kliff after switching to a follower. Ridden as a
follower, the follower's horse should get the mod's controls, and Kliff's horse should stay under AI control. The pointer
hunt is gone. The input census stays.

## Setup

1. Copy any `bin64\ProperHorseControls.log` and `.csv` to `scratch/probe-runs/` if they're needed.
2. The build is already in `DMM\mods\`. In DMM, turn `ProperHorseControls` off and on again and apply.
3. In `bin64\ProperHorseControls.ini`, set `Logging=1`.

## Steps

1. Load in as Kliff and summon the follower, so that it can be switched to.
2. As Kliff, mount and ride with W, A, D and an S tap, then dismount. This checks the usual controls.
3. Switch to the follower.
4. As the follower, mount its horse and repeat step 2's riding. Note what Kliff's horse does.
5. Dismount, switch back to Kliff, and repeat step 2.

Close the game and copy the log and CSV to `scratch/probe-runs/stage47/`.

## Logs

- **`mounted=1`:** for the follower's ride it should show the follower's horse key, `B0100005` in stage 46, not Kliff's
  `B0100002`.
- **`input actors:`:** while riding as the follower, the follower and its horse should show `applied` above 0, and
  `(rider)` should mark the follower.

## Reading the results

| Outcome | Meaning |
| --- | --- |
| The follower's horse rides like Kliff's, and Kliff's horse follows on its own | The fix |
| `mounted=1` shows Kliff's key during the follower's ride | `[[manager+0x58]+0xD8]` wasn't readable and the fallback was used |
| The follower's horse gets no controls | `FindActor` didn't find its mount, or its package isn't a ground mount |

tester notes:

in all tested sequences the player controlled mount behaved as expected, and the AI controlled mount did not exhibit any strange behaviours. the mod also handled switching characters while still mounted, an unexpected bonus (I expected to need to test for this and need more probe runs).
## Results

Passed on 2026-11-16, with Damian as the follower. Logs are in `scratch/probe-runs/stage47/`.

- **Rides:** Kliff on `B0100002`, Damian on his own horse `B0100005`, then Kliff again. Each `mounted=1` showed the
  controlled character's horse, with controls on, and brake slowdowns ended on the next state.
- **Rewrites:** only the controlled character and its mount had input rewritten. Kliff's horse got no input while
  Damian was controlled. 1,400 of 1,403 mounted CSV rows had the rewrite applied.
- **Switching while mounted:** at 16:04:22 and 16:05:01, with both characters mounted, the mount and the rewrites moved
  to the newly controlled pair within a second.
- **No problems logged.**
