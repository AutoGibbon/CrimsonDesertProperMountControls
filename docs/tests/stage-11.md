# Stage 11 test

This build turns on the backward walk for every ground mount, and logs which mount is being ridden. The main things to find out are what S does on each mount without the mod, whether the mod's backward walk starts on each one, and what rate makes the feet match.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load the save with the mounts and wait until at least 25 seconds after launch.

## Keys

| Key | What it does |
| --- | --- |
| F5 | Turns the mod off (low beep) or on (high beep) |
| Home | Adds a numbered marker to the logs (short high beep) |
| Page Up / Page Down | Raises or lowers the backward walk's rate by 0.1 |

The rate starts at 0.5 on each mount, and goes back to 0.5 if you dismount. If Home, Page Up or Page Down does something in the game or another mod, note it in the results.

## Steps

Do these on each mount, starting with the horse so that there's a known-good result to compare with. Press **Home** before each step, so that the logs can be split up. Skip any step that a mount makes awkward, and move on if a mount takes too long.

1. **Mod off (F5), from standing, hold S for 3 seconds:** note whether the mount walks backward, backsteps, turns around or does nothing. Press F5 again to turn the mod back on.
2. **Hold W, moving the camera around, then add A or D:** the mount should keep its heading, and A or D should steer it. Try it at a walk and at a run.
3. **From standing, hold S with A or D:** the mount should pivot.
4. **From standing, hold S for 5 seconds, then release:** note whether it walks backward with a walking animation, slides in its pose, goes forward first or doesn't move.
5. **Hold S again and press Page Up or Page Down until the feet match the ground:** note the rate from the number of presses. Every press is logged, so an approximate count is fine.
6. **During the backward walk, add A or D for 3 seconds, then release it while still holding S:** note whether it turns, and whether the walking animation comes back. The horse-only recovery won't run on other mounts, so a slide that lasts until S is released is useful to know about.
7. **During the backward walk, release S and hold W at once:** note how soon it moves forward. It may not respond until it has stopped.
8. **At a run, release W and hold S:** it should stop, then walk backward.

If you have an alpine ibex, donkey, bull or anything else that isn't in the table, mount it and try W. The mod may stay inactive on it, but the log will show its package name.

Close the game and leave the logs in place until they've been read. A long session is fine: the logs grow by about 10 MB an hour.

## Stage 11 results

Use `ok` or a few words. The rate is the one where the feet matched.
y = good, n = bad, empty = not tested

| Mount | 1. S, mod off | 2. Forward, steering | 3. Pivot | 4. S from standing | 5. Rate | 6. Reverse steering | 7. W after reverse | 8. From a run |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Horse | y | y | y | y | y | y | y | y |
| Camel | y | n, control hooks failed? other tests skipped | | | | | | |
| Reindeer | y | n, control hooks failed? other tests skipped  |  |  |  |  |  |  |
| Elephant |  |  |  |  |  |  |  |  |
| Boar or warthog |  |  |  |  |  |  |  |  |
| Bear |  |  |  |  |  |  |  |  |
| Wolf |  |  |  |  |  |  |  |  |
| Iguana |  |  |  |  |  |  |  |  |
| Carmabirdsaurus |  |  |  |  |  |  |  |  |
| Cucubird |  |  |  |  |  |  |  |  |

- Other mounts tried:
- Any crash or freeze?
- Notes:

## What the logs show

The logs are in `scratch/probe-runs/stage11/`. The hooks ran on every mount, but `ReadPackageName` only gave a package name on the horses, so `IsGroundMountPackage` rejected the other two and `i0_applied` stayed 0. The mount labels are inferred from the order ridden.

| Key | Likely mount | `ReadPackageName` | `i0_applied` | `ReadMountIdentity` description |
| --- | --- | --- | --- | --- |
| `0xB0100002` | Horse | `CD_R0001_Horse` | 1 with the mod on | `animal_horse.xml` |
| `0xB0100003` | Second horse | `CD_R0001_Horse` | 1 with the mod on | `animal_horse.xml` |
| `0xB0100004` | Camel | empty for 47 s, then garbage bytes | 0 in 869 rows | `animal_horse.xml` |
| `0xB0100005` | Reindeer | `ListItemFocusEffectBg` | 0 in 720 rows | `animal_deer.xml` |
