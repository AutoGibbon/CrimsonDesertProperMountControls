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
