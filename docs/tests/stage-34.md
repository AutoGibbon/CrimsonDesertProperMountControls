# Stage 34 test

`KeyRampMs` under `[Steering]` sets how long keyboard steering takes to reach its full angle. This checks low values,
including 0, at a standing start and at a gallop.

## Setup

1. Deploy the build with `KeyRampMs` and set `Logging=1` in `bin64\ProperHorseControls.ini`.
2. Use the horse on flat, open ground.

## Steps

1. Set `KeyRampMs=0` and press End. From standing, press W and A at the same moment several times. At a gallop, switch
   between W+A and W+D.
2. Set `KeyRampMs=50`, press End and repeat step 1.

## Stage 34 results

Logs are in `scratch/probe-runs/stage34/`. The tester rated steering response as expected at both values. At 0, simultaneous
standing starts can stick the horse turning on the spot, which the `KeyRampMs` comment in the ini warns about.

- **0 ms, standing starts:** 3 W+A presses. The one at tick 5078671 left `2592` for `A37D` and `A56E` and walked a
  57°/s circle at about 1.7 m/s. The ones at 5090484 and 5106890 entered `9884` instead and stayed there, at 0 to 0.9 m/s
  with the yaw rate pulsing between about 0 and 100°/s, until A was released 9.0 s and 1.3 s later. The horse then went
  to `A37D` and walked off. This matches stage 3's stuck pivot, and stage 30's spin also stayed in `9884`. The start
  state doesn't explain it, because the clean start and one stuck start both began in `2592`.
- **0 ms, gallop:** the mount input showed the full 55° angle, (±0.82, 0.57), on the first row after each A or D press, and
  the yaw rate reached about 60°/s within one or two rows.
- **50 ms, standing start:** 1 W+A press at tick 5159281 went from `2475` to `A37D` and `A56E`, with no `9884`. A single
  start doesn't show that 50 ms is safe.
- **50 ms, gallop:** the first row after a press sometimes still showed (0, 1), and the next row showed 55°. Rows were
  about 110 ms apart, so the delay is under one row.

### Second run at 50 ms

Logs are in `scratch/probe-runs/stage34/run2/`. The tester found that pressing A or D shortly before W can give a pivot,
and accepts it. 12 standing starts, with rows about 110 ms apart, so "same row" means within about 110 ms in either
order:

| First row | Starts | Into `9884` |
| --- | --- | --- |
| W with A or D | 6 | 1, at tick 7266437 |
| A or D, then W a row later | 4 | 3, at 7270812, 7334031 and 7338296 |
| W, then A or D a row later | 2 | 0 |

- **The pivot:** each `9884` entry came from `AAE5` and lasted until the keys were released, 2.9 to 3.1 s, at up to about
  1 m/s, then went through `99F1`. The clean starts went to `A37D` and then `A56E`, including three from `AAE5`, so the
  state before the press doesn't decide it.
- **Tick 7266437:** the mount input on the first row was (−0.41, 0.91), which at a 50 ms ramp means A had been held for
  about 15 ms. The CSV can't show whether A came before W within that row.
- **Combined with the first run:** at 50 ms, 1 of 7 same-row starts pivoted, against 2 of 3 at 0 ms.
