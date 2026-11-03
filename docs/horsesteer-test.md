# HorseSteer stage 1 test

`HorseSteer.asi` makes riding input follow the mount's facing instead of the camera. This test checks whether that works
in game. It changes game memory while running, and has no effect once disabled.

Only forward steering is the target of this stage. A, D and S are expected to feel wrong, so describe what they do rather
than treating it as a failure.

Keys, while the game is focused:

- **F5:** toggle the mod. A high beep means on and a low beep means off. It starts on.
- **Shift+F5:** flip the forward direction, with one medium beep. Use this only if step 2 tells you to.

## Setup

1. In DMM, disable `HorseProbe`, enable `HorseSteer` and apply.
2. Launch, load your save, and wait until at least 20 seconds after launch, when the hook installs.
3. Stay on flat, open ground. Ride only your horse: the mod currently applies to anything you ride.

## Steps

Spend about 10 seconds on each. Write what happened in the Results section below.

1. **On foot:** walk around with WASD and move the camera. This should feel normal.
2. **Mount up and hold W, mouse still:** the horse should go straight ahead without turning. If it turns around or spins,
   press Shift+F5 once, try again, and note it in the results.
3. **Hold W and swing the camera left and right:** this is the main test. The horse should keep going straight.
4. **Press F5 (low beep), then repeat step 3:** the old behaviour should return, with the horse following the camera. Press
   F5 again (high beep) to turn the mod back on.
5. **Hold W and A, then W and D:** the horse should curve left, then right.
6. **Standing still, hold A, then D:** describe whether the horse turns in place or moves off.
7. **Standing still, hold S:** describe what it does.
8. **Dismount and walk around:** this should feel normal again.

## Afterwards

1. Close the game.
2. Fill in the results below.
3. Leave `HorseSteer` enabled until the logs have been read. Disabling it in DMM deletes `HorseSteer.log` and
   `HorseSteer.csv` from `bin64`.

## Results

| Step                        | What happened                                                                          |
| --------------------------- | -------------------------------------------------------------------------------------- |
| 1. On foot                  | normal walking all good                                                                |
| 2. Hold W                   | went straight!                                                                         |
| 3. Hold W, swing camera     | went straight!                                                                         |
| 4. Mod off, swing camera    | horse turned to camera heading before beginning movement                               |
| 5. W with A, W with D       | horse turns toward the given steering input while moving forward, exactly what we want |
| 6. A, then D, from standing | horse does a small radius walk while turning toward the given steering input, perfect  |
| 7. S from standing          | horse has no idea what to do :D, maybe they didn't code a backwards walk?              |
| 8. After dismounting        | yes back to normal on foot controls                                                    |

- Did you need Shift+F5? (yes / no): no
- Any crash or freeze? (which step, and what you were doing): no
- Which way is "left" in step 5: the horse's left, or something else? A = left = horse's left, D = right = horse's right
- Notes: for this test I disabled the camera follow and camera lookahead settings.

# Stage 2 test

This build makes S a brake and limits the mod to horse-type mounts. Reversing isn't possible through input, because the horse chart has no route into its backward walk.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`. Check the log's first lines mention "hooks installed".
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **Ride with W, A and D at a trot:** this should feel the same as before. If nothing has changed from the game's normal controls, stop and report it, as the horse check has failed.
2. **Hold W, then release W and hold S at a trot or gallop:** the horse should slow down and stop, without turning around.
3. **Standing still, hold S:** the horse should stay put.
4. **Hold S with A, then S with D:** the horse should act as if only A or D were held.
5. **Other mounts, if you have one available that isn't horse-based, such as the exosuit or a bear:** the controls should be the game's normal ones.
6. **Other horse-type mounts, if you have one, such as a golem horse or donkey:** the horse controls should apply.

Leave `HorseSteer` enabled until the logs have been read.

## Stage 2 results

| Step | What happened |
| --- | --- |
| 1. W, A and D at a trot | has reverted to base controls, no more horse relative movement, other steps not tested due to regression |
| 2. S to brake | |
| 3. S from standing | |
| 4. S with A or D | |
| 5. Non-horse mount | |
| 6. Other horse-type mount | |

- Any crash or freeze?
- Notes:

# Stage 2b test

The first stage 2 run failed because the horse check couldn't read the mount's identity, which switched the mod off. This
build applies to every mount again, and writes `HorseSteer_mountscan.txt` the first time you ride each one, so a working
horse check can be built from it.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **Ride with W, A and D at a trot:** the horse-relative controls should be back.
2. **Hold W, then release W and hold S at a trot or gallop:** the horse should slow down and stop, without turning around.
3. **Standing still, hold S:** the horse should stay put.
4. **Hold S with A, then S with D:** the horse should act as if only A or D were held.
5. **Ride any other mounts you can reach, for about 10 seconds each, including the exosuit if available:** this gives the
   scan something to compare. The mod applies to them in this build, so expect odd controls. Note what each mount was.

F5 and logging pause for up to 8 seconds during each scan, though steering and the game should keep running. Leave
`HorseSteer` enabled until the logs have been read.

## Stage 2b results

| Step | What happened |
| --- | --- |
| 1. W, A and D at a trot | expected behaviour |
| 2. S to brake | no noticable difference between just releasing w on its own |
| 3. S from standing | nothing happens |
| 4. S with A or D | horse turns toward steer input, at 90 degree steps, appears to be doing best effort to turn in place with any forward movement at all, this is different from S and D alone where the horse will walk in small radius toward steering direction |
| 5. Other mounts ridden, in order | n/a |

- Any crash or freeze?
- Notes: holding S and a steer input while in motion behaves similarly to holding W and steer input (less steering)
other note. W and steer starting to feel counterproductive especially at low speed like moving from a standstill. turn radius is very big when setting off with W and A/D.

# Stage 3 test

This build limits the mod to land mounts by their action package, tightens W+A and W+D turns when setting off, and slightly
increases them in motion. The steering numbers are a starting point, so report how they feel.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **Mount your horse and hold W:** the horse should go straight. If the game's normal controls are back, stop and report it,
   as the mount check has failed.
2. **From standing, hold W and A, then W and D:** the turn should be much tighter than before.
3. **At a walk, trot and gallop, hold W and tap or hold A or D:** the turn should be slightly quicker than before. Check that it
   doesn't feel twitchy, especially when moving from walk to trot, where the steering angle changes.
4. **Hold W and swing the camera:** the horse should still go straight.
5. **From standing, hold S and A, then S and D:** the pivot should be unchanged.
6. **Any other mount, if you get access to one:** ride it briefly and note the name and how it behaves.

Leave `HorseSteer` enabled until the logs have been read.

## Stage 3 results

| Step | What happened |
| --- | --- |
| 1. W on horse | good |
| 2. W with A or D from standing | good, though if pressing W and steer at almost exactly the same time the horse gets stuck in a pivot without moving. |
| 3. W with A or D in motion | seems better, not too fast not too slow. though, it is noticably "instant" in application. if it could ramp up over 0.3s toward the maximum turn rate? assess for complexity cost before doing anything here |
| 4. W with camera swing | good |
| 5. S with A or D | good |
| 6. Other mounts | untested |

- Any crash or freeze?
- Too tight, too loose, or about right?
- Notes:

# Stage 4 test

This build eases steering in over 0.3 s instead of applying it at once, and should stop the horse getting stuck in a pivot when
W and A or D are pressed at the same moment.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **From standing, press W and A at exactly the same moment, then W and D:** repeat about 5 times each. The horse should set off
   and turn, without getting stuck in a pivot.
2. **From standing, hold W, then add A or D a moment later:** this should set off as well as before.
3. **From standing, hold A or D, then add W a moment later:** this should set off as well as before.
4. **At a trot and gallop, hold W and hold A or D:** the turn should build up smoothly instead of starting at once, and reach the
   same turn speed as before.
5. **At a trot and gallop, hold W and tap A or D:** the tap will turn less than before. Note whether small corrections still feel
   useful.
6. **Hold W, then swap directly between A and D:** the horse should change direction without a jerk or a pause.
7. **From standing, hold S and A, then S and D:** the pivot should be unchanged.

Leave `HorseSteer` enabled until the logs have been read.

## Stage 4 results

| Step | What happened |
| --- | --- |
| 1. W with A or D, same moment | seems good |
| 2. W, then A or D | good |
| 3. A or D, then W | good |
| 4. Hold A or D in motion | good |
| 5. Tap A or D in motion | feels good to me |
| 6. Swap between A and D | good |
| 7. S with A or D | good |

- Any crash or freeze?
- Is 0.3 s too slow, too quick, or about right? seems juust about right.
- Notes:
