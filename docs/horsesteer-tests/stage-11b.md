# Stage 11b test

In stage 11, the mod's mount check rejected the camel and reindeer, so the controls never turned on for them. This build scans each mount's memory for its name, so that the check can be fixed, and adds a key that turns the controls on regardless. The main things to get are a scan of every mount and the control tests that stage 11 couldn't run.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load the save with the mounts and wait until at least 25 seconds after launch.

## Keys

| Key | What it does |
| --- | --- |
| End | Turns the controls on for this mount (long beep) |
| F5 | Turns the mod off (low beep) or on (high beep) |
| Insert | Adds a numbered marker to the logs (short high beep) |
| Page Up / Page Down | Raises or lowers the backward walk's rate by 0.1 |

The scan runs in the background twice on each mount, 5 and 30 seconds after you mount, with a short low beep each time. You don't need to wait for it before testing. **Stay on each mount for at least 35 seconds**, or its scan is incomplete. The game keeps running during a scan, though F5 and the other keys may not respond until it ends.

## Steps

Ride the mounts in the order of the table below, because the logs may not be able to name them. Note any you skip or add.

On the horse, don't press End or run the steps. Stay mounted for about 35 seconds, then dismount.

On each other mount:

- Press **End** once you're mounted.
- Press **Insert** before each of the steps below.
- Dismount once the steps are done and you've been mounted for at least 35 seconds, then move on to the next mount.
- If you remount the same animal, press End again and stay on for another 35 seconds, because its scan files start again.

The steps:

1. **Mod off (F5), from standing, hold S for 3 seconds:** note whether the mount walks backward, backsteps, turns around or does nothing. Press F5 again to turn the mod back on.
2. **Hold W, moving the camera around, then add A or D:** the mount should keep its heading, and A or D should steer it. Try it at a walk and at a run.
3. **From standing, hold S with A or D:** the mount should pivot.
4. **From standing, hold S for 5 seconds, then release:** note whether it walks backward with a walking animation, slides in its pose, goes forward first or doesn't move.
5. **Hold S again and press Page Up or Page Down until the feet match the ground:** note the rate from the number of presses. The rate starts at 0.5, and every press is logged, so an approximate count is fine.
6. **During the backward walk, add A or D for 3 seconds, then release it while still holding S:** note whether it turns, and whether the walking animation comes back. The horse-only recovery won't run on other mounts, so a slide that lasts until S is released is useful to know about.
7. **During the backward walk, release S and hold W at once:** note how soon it moves forward. It may not respond until it has stopped.
8. **At a run, release W and hold S:** it should stop, then walk backward.

If the first two or three mounts fail the same way at step 2 or 4, you can stop running the steps and only collect scans from the rest: mount each one and stay for 35 seconds.

If you have an alpine ibex, donkey, bull or anything else that isn't in the table, mount it for 35 seconds, and try W after pressing End.

Close the game and leave the logs in place until they've been read. Each mount adds two scan files, `bin64\HorseSteer_scan_*.txt`.

## Stage 11b results

y = good, n = bad, empty = not tested. The rate is the one where the feet matched.

| Mount | Scanned | 1. S, mod off | 2. Forward, steering | 3. Pivot | 4. S from standing | 5. Rate | 6. Reverse steering | 7. W after reverse | 8. From a run |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Horse | y | y | y | y | y | y | y | y | y |
| Camel | y | y | y | y | y | y | y | y | y | best speed found in test 6
| Reindeer | y | y | y | y | y | y | y | y | y |
| Elephant | n |  |  |  |  |  |  |  |  | was added to save but not found in game
| Boar or warthog | y | y | y | y | y | y | y | y | y | 
| Bear | y | y | y | y | y | y | y | y | y |
| Wolf | y | y | y | y | y | y | y | y | y |
| Iguana | y | y | y | y | y | y | y | y | y |
| Carmabirdsaurus | y | y | y | y | y | y | y | y | y | "red feathered raptor" ?
| Cucubird |  |  |  |  |  |  |  |  |  | unable to spawn

- Order ridden, if different: in order after carmabird

snow deer (tried to type "snow deer" but jumped off it lol sorry. after scan, found goot reverse speed, pressed INS once, dismounted)

tiger (same tuning pattern: scanned, found good speed, pressed INS once, dismounted)

ibex (same pattern)


- Other mounts tried:
- Any crash or freeze? no
- Notes: some mounts appear to need the steer recovery, some don't. seems like it would not hurt to just enable it for all.

## What the logs show

The logs and scans are in `scratch/probe-runs/stage11b/`. Mount labels come from the order ridden. The horse row's steps weren't run, as the instructions said, so its evidence is from stage 11. Results are in [mounts-and-vehicles.md](../findings/mounts-and-vehicles.md) and [backward-walk.md](../findings/backward-walk.md).
