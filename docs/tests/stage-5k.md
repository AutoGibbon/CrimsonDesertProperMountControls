# Stage 5k test

The backward walk now starts a little later in the stop, which may smooth the start. A or D during the backward walk now sends a much smaller steering
input, to try to turn the horse without it stepping forward.

Keys, while the game is focused:

- **Numpad + and -:** backward speed, starting at 0.50.
- **Numpad * and /:** reverse steering input, up or down 0.02, starting at 0.10. High beep for up, low beep for down. The pulse keys from stage 5i are gone.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, and leave `HorseSteer Backward Walk Test` off.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **At a walk, trot and gallop, release W and hold S at once, 3 times each:** note whether the start is smoother, and whether it ever fails.
2. **From standing, hold S, 3 times:** this should still start every time.
3. **While walking backward, hold S with A, then S with D, for about 5 seconds each:** note whether the horse turns, which way its head goes, and whether it
   steps forward.
4. **Try numpad * and / between attempts:** find the setting with the best turn and no forward steps. Note where forward steps begin, and the turn at the best
   setting.
5. **Standing still, hold S with A or D:** this should still be the pivot.
6. **Hold A or D first from standing, then add S, then release A or D:** note what happens.

Close the game and leave the logs in place until they've been read.

## Stage 5k results

| Step | What happened |
| --- | --- |
| Load | |
| 1. Start from a walk, trot and gallop | same occasional initial jitter between blends. non issue rly. |
| 2. Start from standing | good |
| 3. Steering while reversing | couldn't get low enough turn rate to look non jank |
| 4. Best steering setting | last recorded tweak is the one where the horse turns while reversing in a good away and recovers to the reversing animation after releasing steer input |
| 5. Pivot from standing | correct. steer + reverse simultaneously produces standing pivots at 90degree steps |
| 6. A or D first, then S | this depends on what state the horse last exited into idle. if was moving forward then idle, produces forward tight turn. if was reversing then idle, produces standing pivot. if was dismounted then mounted, produces standing pivot. |

- Any crash or freeze? no
- With A held in reverse, should the horse's head turn left, or should its rear swing left? as it stands the horse's head turns to the left, which is consistent with forward and stationary steering inputs and feels intuitive.
- Notes: pretty damned good!
