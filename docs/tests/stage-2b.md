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
