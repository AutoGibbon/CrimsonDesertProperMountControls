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
