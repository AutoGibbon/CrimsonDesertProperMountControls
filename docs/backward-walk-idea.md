# Idea: horse backward walk

Not started. S currently coasts, and pivots with A or D. RDR2 also backs up, and this would add that through a chart patch paired with
the ASI.
The ASI is still needed either way, because horse-relative movement is outside the chart data.

## Findings

Analysed on build 1.0.0.3479. The horse chart is `upperaction/4_riding/r0001_horserd2_movetest_03_upper.paac`.

| State | Node | Hash | Clip |
| --- | --- | --- | --- |
| Start | 4 | `0x0767D84A` | `cd_rd_prh_basic_00_00_nor_move_walk_b_stt_00.paa` |
| Loop | 43 | `0x2CC7DE9C` | `cd_rd_prh_basic_00_00_nor_move_walk_b_ing_00.paa` |
| End | 226 | `0xCD9A3CBD` | `cd_rd_prh_basic_00_00_nor_move_walk_b_end_00.paa` |

- **No route in:** no other state branches into these three. Start and End only branch to themselves, and nothing targets Loop.
- **Other end state:** node 69 (`walk_b_end_000`) has a branch from elsewhere, but it's a separate clip.
- **Conflicts:** no installed mod replaces this chart.
- **Wagon chart:** `r0012_wagon_upper.paac` contains the same three hashes, but not as states or branch targets. Their purpose is unknown.
- **Rider:** no rider clips for reversing on a horse were found.

## Chart format

Field names are inferred from the data. `scratch/cdmw-src/cdmw/core/action_chart_binary.py` parses the sections.

- **Node:** `+0x18` is the state hash. `+0xE4` (u16) and `+0xE8` (u16) are the first index and count in the 32-byte branch
  records.
- **Branch record (32 bytes):** `+0x0C` (u16) is the index into `branchInstanceArray`.
- **Branch (0x38 bytes):** `+0x14` is the target state hash. `+0x24` and `+0x26` (u16) each start a condition list in the condition
  indices, with lengths in the bytes at `+0x2A` and `+0x2B`. Key checks are in the second list.
- **Condition list:** each index is a dword offset into the condition bytecode, and tokens read as prefix expressions: `0x287` AND,
  `0x288` OR, `0x289` NOT.
- **Condition:** type `0xEA` looks like a key check, with a name table index and a hash after it. Combat Evolved's log also
  calls type 234 its key condition, and type 238 (`0xEE`) skill usable, which takes a skill ID.
- **Adding branches:** point a node's record range at a copy appended to the end of the table, so no other index moves. This
  is untested.

## Open questions

1. Does the clip move the horse backward, or only animate on the spot? If it doesn't move, the ASI would have to move the
   horse, and this is probably not worth doing.
2. Does the rider look acceptable?
3. What triggers it: a key name the game accepts for S, or the ASI answering an existing key check? `exe+0x3B0150` is a game
   key check, according to Combat Evolved's log.

## Next step

Run one experiment before building anything: retarget an existing idle branch that fires on a key press to `0x0767D84A`, a
4-byte change, and ship it as a DMM mod. The tester presses that key while standing on the horse. This answers questions 1
and 2. The horse may get stuck in the state, because it has no way out, so use a throwaway save or reload afterwards.

## If it works

- Add the missing links: idle to Start, Start to Loop, and End to idle. Loop to End already exists.
- While reversing, stop the ASI treating S as a brake or as "turn around".
- Generate the patched chart from the current game file, because a fixed copy can break or overwrite game changes on any
  update. CDMW refuses branch edits, so this needs its own patcher.
