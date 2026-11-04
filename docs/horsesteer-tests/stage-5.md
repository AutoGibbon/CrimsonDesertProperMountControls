# Stage 5 test

This build makes S the game's brake key on your horse at about a trot or faster: S alone acts like W with Alt, and S with A or D
like A or D with Alt. Below that speed, S behaves as before. A separate test mod makes the horse play its backward walk after
stopping from a walk.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`.
2. Leave `HorseSteer Backward Walk Test` off for part 1.
3. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground.

## Part 1: S brake

1. **Press Alt once while riding:** nothing needs to look different. This logs the game's own answer for the key.
2. **At a trot, canter and gallop, hold W, then release W and hold S:** compare the stop with just releasing W.
3. **While dashing, release W and hold S:** note whether the horse does the sliding stop.
4. **While dashing, release W and hold S with A, then S with D:** the horse should drift towards that side, as Alt with A or D does.
   If S ever speeds the horse up, stop and report it.
5. **At a trot and gallop, hold S with A or D:** describe the turn and how quickly the horse slows.
6. **From standing, hold S, then S with A or D:** S should do nothing, and S with A or D should still pivot.
7. **After each stop, ride off with W:** the horse should set off normally.

Close the game and leave `HorseSteer` enabled until the logs have been read.

## Part 2: backward walk clips

Use a throwaway save if you can, because this changes the horse's chart. Disabling the test mod restores the normal chart.

1. In DMM, turn on `HorseSteer Backward Walk Test` and apply, then load your save.
2. **Hold W at a walk, then release it, and don't press anything:** after the stop, the horse should play three backward clips
   and return to idle. Repeat this a few times.
3. Note whether the horse moves backward, how far, and how the rider looks.
4. Close the game, then turn the test mod off in DMM and apply.

## Stage 5 results

| Step | What happened |
| --- | --- |
| 1.2 S at a trot, canter and gallop | no difference, we knew this is true anyway |
| 1.3 S while dashing | it will not, sliding requires steering input plus brake |
| 1.4 S with A or D while dashing | drift activates. seems more prone to failure to resume correct movement than base game, but even base game movement recovery after drift is... wonky (logs should show this as I tested drift with input hook off an on) |
| 1.5 S with A or D at a trot and gallop | no activate. game require high speed to activate drift. |
| 1.6 S and pivot from standing | good pivot behaviour. |
| 1.7 Riding off after a stop | works |
| 2.2 Backward clips play and return to idle | |
| 2.3 Horse movement and rider | |

- Any crash or freeze? no bad in 5.1, 5.2 did not get past load screen. CTD.
- Is the stop at normal gaits hard enough? cannot answer as horse braking can only be activated with a slide.
- Notes: abandon braking concept, do not remap S key to slide activator, PC users will instinctively press S while normal riding and accidentally activating a drift when they just wanted to turn will be counterproductive. still keen on potential for backwards walk though when S key given from standstill.
