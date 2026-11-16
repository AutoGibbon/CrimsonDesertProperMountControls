# Horse call: teleport distance

PE timestamp `0x6AF5D198`, from static analysis of `scratch/exe-baseline/CrimsonDesert_6AF5D198.exe`. Not yet tested in game.
Addresses are RVAs and are specific to this build.

When the horse is called with H, it teleports to the player rather than pathfinding if it's farther away than a threshold. The
threshold is very likely the `pa::GlobalVariableFloat` below, with a default of **64.0** (presumably metres).

## The variable

| What | Address | Notes |
| --- | --- | --- |
| `GlobalVariableFloat` object | `0x71D1BB0` | Name hash `0xB0BAB4F0`; the name string isn't in the exe |
| Current value | `0x71D1BE8` (object `+0x38`) | In `.sxdata`, zero on disk; write this to change the threshold |
| Default copy | `0x71D1BEC` (object `+0x3C`) | |
| Default constant | `0x61BF02C` (`.00cfg`) | `64.0` |
| Static initializer | `0x2161EB` | Copies the constant into `+0x38` and `+0x3C`, then registers the object with `GlobalVariableManager` |

Two places read the value:

- `AIOperatorValue_GetFarDistanceForMercenaryTeleport` (vtable `0x5CF5988`), whose getter at `0x2560D40` returns the float
  unchanged to the AI chart. That operator is how the horse's AI chart reads the threshold.
- `0x2372AB0`, a range check that returns whether squared distance ≤ (threshold + a float argument)². It's called from `0x2372A63`.

## The 128 m variant

`0x2372AB0` uses a different global, at `0x71D1C38` (object `0x71D1C00`, hash `0x8CE70A0A`, default **128.0**), when either of its
two 16-bit ID arguments equals the word at `[[...]+0x168]+0x8C` on a global object. What that ID stands for hasn't been worked
out. The AI operator always returns the 64 value.

## Not established

- The horse's AI chart hasn't been decoded, so it isn't confirmed that the chart compares the leader distance against this value
  directly rather than scaling it. The link comes from the operator's name and the call flow in `scratch/caballo.md`
  (`CallVehicleMercenaryAck` → `StartAIFunctionReq` → `AITeleportReq` → `MoveForceAck`).
- The engine has a dev command, `/sg [GlobalVariableName] [value]`, and a `_globalVariableOverrides` list, but no shipped
  config file that sets this variable was found.

## Testing it

Set the float at `base+0x71D1BE8` to a small value such as 20, from an ASI or a memory editor. Then call the horse from about 30 m
and from about 70 m, and check whether it teleports at both distances instead of only the farther one.

## Finding the addresses again after an update

1. Find the RTTI type `.?AVAIOperatorValue_GetFarDistanceForMercenaryTeleport@pa@@`, then its vtable. Entry 6 is the getter, and
   its `vmovss xmm0, [rip+...]` gives the value's address.
2. Find RIP-relative references to that address. The one that stores to it is the static initializer, and its `vmovss` load
   gives the default constant. The neighbouring initializers are `0x100` bytes apart.
3. `check_rtti.py` on the initializer's stored vtable should print `.?AVGlobalVariableFloat@pa@@`.
