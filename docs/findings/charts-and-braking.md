# Action charts, keys and braking

Build 1.0.0.3479. `tools/chart_branches.py CDMW_SRC CHART_DIR` prints the horse chart's `key_brake` branches, the branches into
the slide and drift states, and its skill conditions, with conditions decoded.

## Verified from game data

- **Rotate events:** the horse chart has 76 unique `Rotate` frame events. Of 2,911 across all charts, 2,905 share the
  same header bytes, and no value there selects between camera, input and character frames.

## Hypotheses

- `Rotate` events set turn rates and angle limits while native code supplies the target direction. This is inferred from
  values such as 6.98 rad/s and 180.

## Brake and drift

- **`key_brake`:** bound to Alt, mouse X2 and gamepad B, in the same `CharacterAction` group that on-foot charts read, so S can't be
  added to it in `inputmap.xml`. The game's text describes it as "Drift ... while dashing". Most horse uses are `OR T10 key_brake`,
  and `T10` is probably "no stick input", which fits the tester finding that Alt at normal gaits feels the same as releasing W. The
  slide stop, `gallop_f_slip_end`, is only entered from the drift-end and `faststart` clips and two special gallop states. In those
  gallop states, the `T10` branch is tested before `key_brake`, so braking needs a stick input. Drift starts from the slide stop, with
  `key_brake`, a steering angle past 45° and `SKILL_USABLE(12007)`. These skill IDs have not been matched to names or unlock rules.
  The tester reports that brake with A or D drifts in game, quickly slowing the horse and turning it to that side.
- **`key_brake` on other mounts:** of the land-mount ride charts, only the wolf and warthog read it, in one walk branch each,
  `OR T10 key_brake`. The bear, camel, reindeer, elephant, iguana, Carma bird and cucu bird ride charts don't read it.
- **Stage 5 S brake (`scratch/probe-runs/stage5-brake/`):** speeds are noisy, and each comparison below has few samples.
  - **S alone:** 8.1 m/s to under 1 m/s in 2.3 s, against 2.3 s from 7.1 m/s for one clean W release. From 13.2 m/s, it reached
    1.4 m/s in 3.2 s. The tester saw no difference from releasing W, and no slide without steering.
  - **S with A or D:** at 13–15 m/s the horse turned at 110–200°/s on average during the press, and the tester saw a drift. At
    11–12 m/s it turned at 70–85°/s with no drift.
  - **Standing:** S does nothing, and S with D pivoted at about 50°/s and 0.4 m/s.
  - **Drift recovery:** many drifts fall to about 0 m/s within 2 s, then reach about 10 m/s by 6 s, even with W held. Speed fell
    below 3 m/s within 5 s in 6 of 17 Alt drifts with the mod on and 1 of 7 with it off, with W held about as long in both. It also
    did in 6 of 8 S drifts, but W was held for less of those. The cause is untraced.

## Reading and overriding keys

- **Key check:** `exe+0x3B0150` is the handler for condition type `0xEA`. The handler table comes from `exe+0x3A9FC0`, at index
  `type - 0x83`. The handler reads the key hash from the condition at `+8` and the actor from the context at `+8`. `key_brake`'s
  hash is `0x377B9DB8`. Its live code starts with a jump to a non-module address (`0x13FFD003C` in the logged runs), so something
  hooks it at runtime. It returns 0 when the key is pressed: the game's answer was 1 except during brief presses, and answering 0
  on S started drifts in game. Replacing the table slot worked alongside the existing hook. That code was removed with the brake and is saved in
  `scratch/probe-runs/stage5-brake/s-brake.patch`.
- **Installed mods:** Combat Evolved swaps riding charts such as `ride_upper` and `ride_weapon_upper`, and its strings show
  no movement or steering code. It logs a game key check at `exe+0x3B0150` on this build, which may help with reading
  game input.
- **Key query (Horse Tap Brake, checked against this build's exe):** the signature has one match, `exe+0x30C5640`, which takes a
  key hash and returns true if the key is held. It's slot 8 of the input object's vtable. That object comes from the first entry
  of the list at `[[WorldSystem+0xD8]+0x20]` (count at `+0x28`), then `+0x10`.
- **Key hash (Horse Tap Brake, checked against this build's exe):** Jenkins lookup3 with seed `0xC5EDE` on the lowercase name.
  `key_brake` gives `0x377B9DB8`, as logged in earlier tests. The mod overrides the result of the slot 8 call to hold a key, so the
  `0xEA` handler table doesn't need patching. This only covers actions read through that query, so it can't replace the stick
  changes. Nothing here has been tested in game with `HorseSteer`.
