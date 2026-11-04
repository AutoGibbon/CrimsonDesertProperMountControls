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
