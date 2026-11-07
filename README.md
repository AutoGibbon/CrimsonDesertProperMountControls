# Proper Horse Controls

## HORSIE GO WHERE HORSIE LOOK\*.

RDR2 style "horse relative" mount controls for Crimson Desert.

<sub>\* and Wagon, Ibex, Camel, Snow Deer, Boar, Bear, Wolf, Tiger, Lion, Iguana, Raptor, Kuku Bird (steering works while
gliding too), elephant</sub>

I do not like horse being tied to the camera. I wanna look around while riding.

"Horse Relative" means the direction the horse travels is disconnected from the camera. Think of it like riding an
actual horse (I have never ridden a horse IRL but this feels like a good way to describe it).

## What do?

- Horsie go where horsie look! You look but horsie not care. Horsie go where horsie look. (why am I writing like this?)
- Change where horsie look with A and D (or stick). So easy.
- Horsie can reverse now. Like... walk backwards. Waow.
- Wagon go where horsie look!
- Wagon reverse with S key (or down on stick). Still reverse with normal button too.
- No change taming minigame (even though I could have left it and then with this mod you'd have been able to instantly
  win by holding one button).

## Controls

- Proper horse controls
- **F5** - Toggle the mod on/off.
- Reversing turns the wrong way for brain? Set **InvertSteer=0** in the ini.

<sub>Technically controller should be supported but I cannot test this. If it works on controller, cool! Otherwise,
feedback wanted.</sub>

## Settings

<details>
<summary>All settings</summary>

_This is all documented inside the **.ini**, but I know some folks want to know everything possible about a mod before
they choose to try it out. Restart game after editing your **.ini** file._

**`[Keys] Toggle`**
Key to turn the mod on/off. Default F5. F1-F24, A-Z, 0-9, Numpad0-Numpad9, Insert, Delete, Home, End, PageUp, PageDown,
Pause or ScrollLock. Not W, A, S or D.

**`[Reverse] InvertSteer`**
Which way A and D turn horsie when reversing. Default 1, set 0 to flip it. Works on wagon too.

**`[BackwardSpeed]`**
How fast each kind of mount walks backward, 0.2 to 6 m/s. Ibex uses the horse value, snow deer uses reindeer, tiger and
lion use Wolf_Elite.

**`[Debug] Logging`**
Default 0. Set 1 if you're reporting a bug and want to send me logs. About 10MB per hour of riding, and they get wiped
every game start, so save them before relaunching.

</details>

## Installation

- **[DMM](https://www.nexusmods.com/crimsondesert/mods/633):** add the downloaded zip as a mod, enable it and apply.
- **Manual:** copy **ProperHorseControls.asi** and **ProperHorseControls.ini** from the **ProperHorseControls** folder
  into the game's **bin64** folder. Requires ASI loader.
- Uninstalling?: remove both files from **bin64**.

## Known issues

- Wagons still control poorly in reverse, just like base game. I ain't touching that.
- After winning the taming minigame, the horse controls can freeze if you choose to ride it immediately. Dismount and
  remount to fix it.
- Should keep working with game updates, but if not, patch will come soon. Please wait :)

## Compatibility

- Any mod that changes mount or riding inputs will probably conflict.
- [Crimson Route](url=https://www.nexusmods.com/crimsondesert/mods/3175) - toggle this mod off when using auto run. Their automatic game inputs need the default camera coupled controls to work properly.

## AI Disclosure

The code for this mod was written with Claude (Anthropic's AI).

The design, documentation, reference material, testing approach, and silly sounding mod description was created by me.

Zero functions or features of any other person's work were cloned, stolen, or directly replicated to make this mod.

## Credits and Thanks

Horsie would not go where horsie look if not for these, so thank you for all your hard work.

- [CDMW](https://github.com/Ratty123/CDMW-Full) - tools for archive and action chart decoding
- [CrimsonDesertCoop](https://github.com/macauleywilliams96-dotcom/CrimsonDesertCoop) - horse and camera research
- [CrimsonDesertTelemetry](https://github.com/ZappendusterFX/CrimsonDesertTelemetry) - camera research
- [CrimsonDesertTools](https://github.com/tkhquang/CrimsonDesertTools) - native mod reference code
- [CrimsonDesertModdingTools](https://github.com/NattKh/CrimsonDesertModdingTools) - game table field name research
- [Horse Tap Brake](https://www.nexusmods.com/crimsondesert/mods/3666) - learned how to examine mount chart state,
  velocity and key reads, and update-safe callsite lookups
- [DMM](https://www.nexusmods.com/crimsondesert/mods/633) - mod manager, used for install and testing
- [SafetyHook](https://github.com/cursey/safetyhook) and [Zydis](https://github.com/zyantific/zydis) - function hooking

## License

[MIT](LICENSE). Third-party licenses are in [THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt).
