# Stage 10b test

This build has one deliberately broken address lookup, so the mod should do nothing at all and the game should play as if it weren't installed. The main thing to test is that the game is normal. With no hooks installed, this build can also show whether the freeze after a taming win happens without the mod's hooks.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply.
2. Load your save and wait until at least 25 seconds after launch.

## Steps

### The mod is idle

1. **Press F5:** there should be no beep.
2. **Mount your horse and hold W, moving the camera around:** the horse should follow the camera, as in the unmodded game.
3. **At a gallop, release W and hold S:** the game's own brake should apply. From standing, S shouldn't start the mod's backward walk.
4. **Dismount and play for a minute or two:** nothing should behave oddly.

### Freeze after a taming win, if you can find wild horses

5. **Tame a horse, letting go of every key as soon as you win and pressing nothing except what closes the end UI:** once it has closed, hold W. Note whether the horse moves. If it doesn't, try A, D and S as well.
6. **Tame another horse, tapping A and D a few times while the end UI is showing, before closing it:** then hold W. Note whether the horse moves.

Repeat steps 5 and 6 if you have the time, because each result counts for more when it's repeated.

Close the game and leave the logs in place until they've been read.

## Stage 10b results

| Step | What happened |
| --- | --- |
| Load | |
| 1. F5 | no beep |
| 2. W and the camera | yep yucky controls confirmed |
| 3. S | ick |
| 4. General play | ok |
| 5. Tame with no keys | not sure if I let go soon enough, horse moved |
| 6. Tame with keys | no second horse located for test |

- Any crash or freeze? no
- Notes: n/a
