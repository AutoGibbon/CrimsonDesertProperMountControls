# HorseProbe test results

Test steps: [test.md](test.md). Leave anything you didn't test blank.

## Session

- Date:
- Did the game update since the last session? (yes / no / don't know):
- Mods disabled, if any:
- Mount used (e.g. your usual horse, donkey):
- Did the game launch and load normally? (yes / no):
- Crashes (which part, and what you were doing):

## Part 1: what you saw on screen

For each step, describe which way the horse moved and turned relative to its facing and to the camera.

| Step                     | What the horse did                                                                      |
| ------------------------ | --------------------------------------------------------------------------------------- |
| Hold W, mouse still      | moved forward relative to camera                                                        |
| Hold W, swing camera     | moved forward relative to camera, with slow tracking toward new heading as camera moved |
| Stopped, rotate camera   | camera rotated                                                                          |
| Hold A from a standstill | horse turned to face and moved left, got stuck on a wall                                |
| Hold D from a standstill | horse turned to face and moved right,got stuck on a wall                                |
| Hold S from a standstill | horse turned to face and moved toward camera                                            |

- Does the horse ever turn on the spot, without travelling? (yes / no, and when): only when the horse was not facing the "desired heading"
- Does the camera drift back behind the horse by itself? (yes / no, and how quickly): no test was performed that could evaluate this behaviour.

## Part 2: input scan (F8)

- Heard the F8 beep? (yes / no): yes
- Heard a beep after each of W, D, S and A? (yes / no, and which were missing): yes
- Heard the three rising beeps at the end? (yes / no): yes
- Roughly how long did the W step take? a second or two
- Size of any game hitch: none noticed
- Anything go wrong, such as a mouse bump or a wrong key? not that I know of

## Part 3: write watch (F10)

- Number of F10 presses: one
- Heard two quick beeps after each press? (yes / no): repeated w a s d taps until first beep, then attempted ESC,Alt+Tab to reference test instructions, game froze
- Any stutter, crash or odd behaviour during or after it?
- Did another mod stop working afterwards? (which one): n/a, game froze, might be my fault

## Log files

- `HorseProbe.log` and `HorseProbe.csv` in `bin64`? (yes / no; if no, where):

## Notes

Anything else that seemed odd or relevant:
