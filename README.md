# SM-EnableJIT

A DLL mod for Scrap Mechanic that enables the LuaJIT JIT compiler.

> [!NOTE]
> Inject as early as possible, and always outside of a world (e.g. in the main menu). Lua states created before the hook is installed keep JIT disabled, so injecting while inside a world will not apply properly.

> [!IMPORTANT]
> This was ORIGINALLY discovered by [**Nothing Mod**](https://github.com/Nikodem2), but days later rediscovered by [**Comeone / \_uchstalker\_**](https://github.com/UchStalker) & got the information, credits to him & Nothing Mod!

## The problem

Scrap Mechanic ships with LuaJIT's JIT compiler turned off, so all Lua code runs in the interpreter. Script-heavy creations and mods pay for it in performance.

## What it does

Hooks the `luaL_newstate` import of `ScrapMechanic.exe` (IAT hook). Every time the game creates a new Lua state, the hook calls `luaopen_jit` from `lua51.dll` on it, which loads LuaJIT's `jit` library and enables the compiler. Hot Lua code then gets compiled to native machine code instead of being interpreted.

The hook is removed again when the DLL is unloaded.

## How to Download and Enable

There are 2 ways to enable the mod:

**SM-DLL-Injector**

1. Download the latest release of [SM-DLL-Injector](https://github.com/QuestionableM/SM-DLL-Injector/releases/latest) and follow the instructions in its README.
2. Download the latest release of `SM-EnableJIT.dll` from the Releases page of this repository.
3. Move `SM-EnableJIT.dll` to the `Steam/steamapps/common/Scrap Mechanic/Release/DLLModules` directory created by the SM-DLL-Injector installer.
4. Launch the game.

**Manual Injection**

1. Download the latest release of `SM-EnableJIT.dll`.
2. Launch the game.
3. Inject `SM-EnableJIT.dll` with a DLL injector of your choice.

## License

[MPL-2.0](LICENSE) <!-- TODO: confirm license -->
