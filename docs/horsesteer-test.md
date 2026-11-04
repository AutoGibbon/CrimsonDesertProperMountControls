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

# Stage 5b test

This repeats the backward clip test with the same chart edits, delivered as a JSON patch after the crash in stage 5. The S brake has
been removed from `HorseSteer`.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again, so the build without the brake is copied to `bin64`.
2. Turn off `HorseSteer Backward Walk Test` and apply.

## Steps

1. **Load your save:** it should load normally. Ride briefly, checking that S at speed only coasts and S with A or D doesn't drift.
   Close the game. If this load crashes, stop and report it, because the chart isn't the cause.
2. In DMM, turn on `HorseSteer Backward Walk Test`, which now has a JSON file inside it, and apply. Check it's listed as enabled.
3. **Load your save again:** if it crashes during loading, stop and report it.
4. **Hold W at a walk, then release it, and don't press anything:** after the stop, the horse should play three backward clips and
   return to idle. Repeat this a few times.
5. Note whether the horse moves backward, how far, and how the rider looks.
6. Close the game. Leave the logs in place until they've been read, then turn the test mod off in DMM and apply.

## Stage 5b results

| Step | What happened |
| --- | --- |
| 1. Load and ride, test mod off | skipped, trust |
| 3. Load, test mod on | loads ok |
| 4. Backward clips play and return to idle | first time saw 3 clips play, subsequent stops saw 2 clips play |
| 5. Horse movement and rider | horse animation good, 2 clip appears to have rider animation so would be suitable for use, horse does not move |

- Any crash or freeze? no crash or freeze
- Notes:

# Taming minigame results -- we don't need to keep moving this to the end of the file btw!

Tested on the build that pauses the mod during the minigame.

| Check | What happened |
| --- | --- |
| Minigame controls | Normal, with no delay noticed |
| Riding off after a win | The horse wouldn't move until a dismount and remount. Accepted as a known issue |
| Riding off after a win, `HorseSteer` disabled | Normal |

# Stage 5c test

This gives the horse's backward loop move speed data, to see whether that makes it move. It's the same test as stage 5b otherwise, and
S still does nothing from standing.

## Setup

1. Close the game. In DMM, turn `HorseSteer Backward Walk Test` off and on again and apply, so the new version of the patch is built.
2. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

## Steps

1. **Hold W at a walk, then release it, and don't press anything:** the horse should stop, then play the backward clips. Repeat this
   about 5 times, keeping track of which clips play.
2. **During the second clip:** note whether the horse moves, which way, how far, and whether its feet match the ground.
3. **During the second clip:** note whether the rider still has the secondary motion you saw before.
4. **Stop with a wall or slope close behind the horse:** note whether it stops at the wall and follows the slope.
5. **After the clips:** check the horse returns to idle and rides off normally with W.

Close the game and leave the logs in place until they've been read. Then turn the test mod off in DMM and apply.

## Stage 5c results

| Step | What happened |
| --- | --- |
| Load | |
| 1. Which clips play | 3 clips played, I think it was just hard to see difference in prior 3 clips used |
| 2. Horse movement in the second clip | no movement |
| 3. Rider in the second clip | no apparent movement |
| 4. Wall and slope | untested since no movement |
| 5. Return to idle and riding off | fine |

- Any crash or freeze? none
- Notes:

# Stage 5d test

Stage 5c was run with no keys held. The one rideable vehicle that reverses in the game data does so with the stick held back, so the move
speed data may need input. This uses the same patch, with a key held during the clips.

## Setup

1. Leave `HorseSteer Backward Walk Test` and `HorseSteer` on. Nothing needs to change in DMM.
2. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

The sequence ignores input once the first backward clip starts, but pressing a key before then cancels it. Each attempt starts by holding W at
a walk, then releasing it, and waiting for the horse to stop and start the first clip, which lasts about 3 seconds.

## Steps

1. **Hold W through the clips, with the mod on:** after the first clip starts, hold W until the clips end. Repeat 3 times.
2. **Hold S through the clips, with the mod off:** press F5 (low beep) once the horse has stopped, so the mod is off for the clips. After the
   first clip starts, hold S until the clips end. Press F5 again (high beep) to turn the mod back on before riding off. Repeat 3 times.
3. For each, note whether the horse moves during the second clip, which way, how far, and whether it turns.

Close the game and leave the logs in place until they've been read.

## Stage 5d results

| Step | What happened |
| --- | --- |
| Load | |
| 1. W through the clips, mod on | no move |
| 2. S through the clips, mod off | no move |

- Any crash or freeze? no
- Notes: n/a

# Stage 5e test

This fixes a field in the patch that was left at zero in stages 5c and 5d, and which may be what makes the horse move. The clips run as before.

## Setup

1. Close the game. In DMM, turn `HorseSteer Backward Walk Test` off and on again and apply, so the new version of the patch is built. Leave
   `HorseSteer` on.
2. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

Each attempt starts by holding W at a walk, then releasing it, and waiting for the horse to stop and start the first clip. Pressing a key before
then cancels the sequence.

## Steps

1. **No keys, mod on:** let the clips play. Repeat 3 times.
2. **Hold W through the clips, mod on:** after the first clip starts, hold W until the clips end. Repeat 3 times.
3. **Hold S through the clips, mod off:** press F5 (low beep) once the horse has stopped. After the first clip starts, hold S until the clips
   end. Press F5 again (high beep) before riding off. Repeat 3 times.
4. For each, note whether the horse moves during the second clip, which way, how far, and whether its feet match the ground.
5. **If it moves:** stop with a wall or slope close behind the horse, and note whether it stops at the wall and follows the slope.

Close the game and leave the logs in place until they've been read.

## Stage 5e results

| Step | What happened |
| --- | --- |
| Load | |
| 1. No keys, mod on | no move |
| 2. W through the clips, mod on | no move |
| 3. S through the clips, mod off | no move |
| 4. Feet and rider, if it moves | n/a |
| 5. Wall and slope, if it moves | n/a |

- Any crash or freeze? no
- Notes: n/a

# Stage 5f test

This build moves the horse backward at 1 m/s from the mod itself while S alone is held. It writes the horse's position directly, so it ignores
collisions and may look wrong. This is a one-off test, and the code won't stay in the mod unless it's worth keeping.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, so the new build is copied to `bin64`. Leave
   `HorseSteer Backward Walk Test` on, so the clips can be tried.
2. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

Save before this test. The horse may end up inside something, so be ready to dismount or reload.

## Steps

1. **From standing, hold S for about 5 seconds, then release:** note whether the horse moves backward smoothly, jitters or snaps back on
   release, and whether the rider and camera stay with it.
2. **Hold W at a walk, release it, and once the first backward clip starts, hold S until the clips end:** note how the movement and clips look
   together, and whether the feet match the ground.
3. **Ride off with W:** the horse should set off normally from where it ended up.
4. **Hold S on a slope, uphill and downhill:** note whether the horse follows the ground.
5. **Hold S with a wall or tree close behind the horse:** note whether it stops or goes through.

Close the game and leave the logs in place until they've been read.

## Stage 5f results

| Step | What happened |
| --- | --- |
| Load | |
| 1. S from standing | smooth movement |
| 2. S during the clips | movement is too fast for the animations |
| 3. Riding off | yes |
| 4. Slope | follows ground |
| 5. Wall or tree | stops at walls |

- Any crash or freeze?
- Is this worth keeping, if the problems can be fixed?
- Notes: turning with reverse movement not possible, might be impossible given no ground mount has built in working backward movement states and only half baked unused states available.

# Stage 5g test

This restores the backward loop's original data, so the rider's motion should be back, and slows the backward movement to about 1 m/s. You can
change the speed in game. S with A or D now moves the horse backward as well. The clips still only start after a stop from a walk, and S alone
from standing still slides the horse in its idle pose.

Keys, while the game is focused:

- **Numpad +:** faster, with a high beep.
- **Numpad -:** slower, with a low beep.

## Setup

1. Close the game. In DMM, turn `HorseSteer` and `HorseSteer Backward Walk Test` off and on again and apply, so both new versions are used.
2. Save, then load and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

## Steps

1. **Hold W at a walk, release it, and once the first backward clip starts, hold S until the clips end:** press numpad + or - between attempts,
   until the horse's feet best match the ground. Note how many presses from the start, or the setting in the log will show it.
2. **On the second clip:** note whether the rider's motion is back.
3. **From standing, hold S with A, then S with D, for about 5 seconds each:** note the path the horse takes, and how it looks.
4. **During the clips, hold S with A or D:** note whether the horse turns at all.
5. **Ride off with W:** the horse should set off normally.

Close the game and leave the logs in place until they've been read.

## Stage 5g results

| Step | What happened |
| --- | --- |
| Load | |
| 1. Best speed, and how the feet look | initial speed was near perfect match for clip 2. |
| 2. Rider on the second clip | rider looks passable |
| 3. S with A or D from standing | it does the pivot we've seen before, this time with a little backwards movement while it pivots. doesn't look very natural though |
| 4. S with A or D during the clips | no turn during this test |
| 5. Riding off | works fine |

- Any crash or freeze? no
- Does this look good enough to build the S trigger for? clip 2 looked great but see note!
- Notes: SOMETHING UNEXPECTED HAPPENED! when pressing W, if release and immediately hold S before horse settles to idle state, it actually activated a backwards walking animation all on its own?????

# Stage 5h test

The backward walk in stage 5g may come from the game's own walking blend, driven by the mod moving the horse backward. This checks that without the
test chart. The `HorseSteer` build doesn't change.

## Setup

1. Close the game. In DMM, turn off `HorseSteer Backward Walk Test` and apply. Leave `HorseSteer` on.
2. Load your save and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

## Steps

1. **Hold W at a walk, release it, and hold S straight away, for about 10 seconds:** repeat 5 times. Note whether the backward walk plays, how long it
   lasts, and what the horse does when it ends.
2. **Repeat, waiting about 0.5, then 1, then 2 seconds between releasing W and pressing S:** note the longest wait that still gives the walk.
3. **While the backward walk plays, add A, then D:** note whether the horse's body or path changes.
4. **While it plays, press numpad + or - and try again:** note the setting where the feet best match the ground. The test starts at 0.50.
5. **From a trot, release W and hold S straight away:** note what plays.

Close the game and leave the logs in place until they've been read.

## Stage 5h results

| Step | What happened |
| --- | --- |
| Load | |
| 1. How long the walk lasts, and how it ends | looks like it can loop indefinitely once triggered and the backward speed is maintained |
| 2. Longest wait that still works | there seems to be a small window of opportunity to get the blend to activate, log data will show long backward walks these were the ones that had successful activations |
| 3. Adding A or D | branches into the known pivoting on the spot state |
| 4. Best speed, and how the feet and rider look | one back walk was the longest, and had the best matched speed |
| 5. From a trot | harder to reliably activate, seems to be highly dependent on timing in its current state |

- Any crash or freeze? one CTD while alt tabbing back to game between note writing
- Notes: good looking animation if it can be leveraged.

# Stage 5i test

This tries to start the backward walk reliably, from standing, without a chart patch. When you press S alone from standing, the mod sends a brief
forward input, about 0.15 seconds, then moves the horse backward. You may see a small forward twitch first. S alone after releasing W now also starts
at any speed under 2.5 m/s. S with A or D doesn't move the horse backward any more, so it's the plain pivot again.

Keys, while the game is focused:

- **Numpad + and -:** backward speed, starting at 0.50.
- **Numpad * and /:** forward pulse length, up or down 25 ms, starting at 150 ms. High beep for up, low beep for down.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, and leave `HorseSteer Backward Walk Test` off.
2. Save, then load and wait until at least 20 seconds after launch. Stay on flat, open ground, with some clear space behind the horse.

## Steps

1. **From standing, wait for the horse to settle, then hold S for about 5 seconds and release:** repeat 10 times. Count how many give the backward walk,
   and note any twitch at the start and what happens on release.
2. **If it's unreliable or the twitch is bad:** change the pulse with numpad * or / and repeat. Note the setting that works best.
3. **At a walk, release W and hold S at once, 5 times:** count how many give the backward walk.
4. **At a trot, release W and hold S at once, 5 times:** count how many give the backward walk, and describe the transition.
5. **While walking backward, add A or D, then release them with S still held:** describe what happens.
6. **While walking backward, press W:** the horse should go forward normally.
7. **Hold S on a slope, and with a wall behind the horse:** note how it looks.

Close the game and leave the logs in place until they've been read.

## Stage 5i results

| Step | What happened |
| --- | --- |
| Load | |
| 1. S from standing: walks out of 10, twitch, release | no twitch noticed. nearly all activations worked. on release horse goes back to idle. |
| 2. Best pulse setting | initial pulse worked every time from idle state. only time it did not work from idle was on game refocus, horse moved with no animation |
| 3. From a walk: walks out of 5 | no succesful activations |
| 4. From a trot: walks out of 5, transition | as above |
| 5. Adding and releasing A or D | horse reverts to pivot or small radius turn walk |
| 6. W while walking backward | blends into  forward animation with correct movement |
| 7. Slope and wall | hard to even find flat ground so nearly every test is on an incline. at a wall the horse will reverse into the wall as far as the riders collider. |

- Any crash or freeze? no
- Notes: good result so far. if we can track the horse's movement to know exactly when to trigger the pulse we can get reliable backward blend activations from W > release > immediate S flows.

# Stage 5j test

If you hold S while the horse is moving, it now slows down first, then starts walking backward once it has nearly stopped. From standing, this should
work as it did in stage 5i.

The numpad keys from stage 5i still work.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply, and leave `HorseSteer Backward Walk Test` off.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **At a walk, release W and hold S at once, 10 times:** count how many give the backward walk, and note how long the wait feels.
2. **At a trot, release W and hold S at once, 5 times:** as above.
3. **At a gallop, release W and hold S at once, 5 times:** as above.
4. **From standing, hold S for about 5 seconds, 5 times:** count how many give the backward walk.
5. **Alt-tab away and back, then hold S from standing, 3 times:** note whether the horse ever moves with no animation.
6. **While riding at speed, tap S with A or D to make a tight turn:** this should feel the same as before, with no backward walk.
7. **While walking backward, add A or D, then release them with S still held:** note whether the backward walk comes back cleanly.

Close the game and leave the logs in place until they've been read.

## Stage 5j results

| Step | What happened |
| --- | --- |
| Load | |
| 1. From a walk: walks out of 10, wait | perfect. maybe drop the min speed to 0.2, sometimes activation is little choppy as it forces blend too quickly from the forward settling state - this applies to trot and gallop |
| 2. From a trot: walks out of 5, wait | as above |
| 3. From a gallop: walks out of 5, wait | as above |
| 4. From standing: walks out of 5 | perfect |
| 5. After alt-tab | perfect |
| 6. S with A or D at speed | perfect |
| 7. Adding and releasing A or D | backward walk returns cleanly but the A/D inputs produce forward motion making steering in reverse look clunky |

- Any crash or freeze? no
- Notes: n/a

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

# Stage 6 test

This is the final version of the backward walk, with the test keys and test chart removed. The speed and steering are fixed at the values you settled on, and the
walk now only applies to your horse.

## Setup

1. Close the game. In DMM, turn `HorseSteer` off and on again and apply. The `HorseSteer Backward Walk Test` mod has been removed, so check DMM doesn't list it.
2. Load your save and wait until at least 20 seconds after launch.

## Steps

1. **From standing, hold S for about 5 seconds, then release:** the horse should walk backward, then return to idle. Repeat 5 times.
2. **At a walk, trot and gallop, release W and hold S at once:** the horse should stop, then walk backward.
3. **While walking backward, hold A, then D, then release them with S still held:** the horse should curve as in stage 5k, then walk straight backward.
4. **While walking backward, press W:** the horse should go forward normally.
5. **From standing, hold S with A or D:** the horse should pivot.
6. **While riding at speed, tap S with A or D to make a tight turn:** this should feel the same as before.
7. **Press numpad +, -, * and /:** nothing should happen, with no beeps.
8. **Win or start the taming minigame, if you get a chance:** the controls should be normal during it.
9. **Any other mount, if you get access to one:** hold S from standing. It shouldn't walk backward, and it should behave as it did before this build.

Close the game and leave the logs in place until they've been read.

## Stage 6 results

| Step | What happened |
| --- | --- |
| Load | |
| 1. S from standing | good |
| 2. From a walk, trot and gallop | good, even after a jump |
| 3. Steering in reverse | good |
| 4. W while walking backward | good |
| 5. Pivot from standing | good |
| 6. S with A or D at speed | good |
| 7. Numpad keys | good |
| 8. Taming minigame | will test later, takes ages to find a wild horse, we know it works |
| 9. Other mount | cannot test, fresh save with only one basic mount |

- Any crash or freeze? no
- Notes: excellent progress!
