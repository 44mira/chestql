# chestql

## Dependencies

- `meson` (build tool)
- `sqlite3` (db)
- a C compiler like `gcc` or `clang`

This program also uses port *4499* for socket communication.

> [!NOTE]
> It makes use of Unix system calls for handling sockets and I could not be bothered
> to make a portable version that works with Windows `winsocket` 
> (PRs open if you want that learning experience lol)

## Building from source

### SERVER:
```bash
# sets up build directory
meson setup build

# compiles and links all dependencies
meson compile -C build

# run tests in interactive mode
meson test -C build -i

# run server
./build/chestql
```

### CLIENT

The clients for this program are going to be from `http` calls made by [ComputerCraft:Tweaked](https://www.curseforge.com/minecraft/mc-mods/cc-tweaked)

So it goes without saying, the clients are run within a Minecraft client inside of a computer from the mod.
Simply copy-paste `chestql.lua` into a file in the CraftOS system and run it.

```bash
./chestql.lua
```
