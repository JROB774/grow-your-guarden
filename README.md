# Grow Your Guarden (My First Game Jam: Winter 2023)

A survival tower defence game made for **[My First Game Jam: Winter 2023 Edition](https://itch.io/jam/my-first-game-jam-winter-2023)**,
where the goal of the jam was to create a game around the theme of **Growth**. As a result the game features the concept of growing
plants as defences to defend a tree from waves of incoming enemies. A download for the game can found over on
**[itch.io](https://itch.io/game/summary/1920412)** where there is also a browser version available to be played.

## Development

The game was developed in C++ using **[SDL2](https://libsdl.org/)**, with the web port being developed using
**[emscripten](https://github.com/emscripten-core/emscripten)**.

## Building

After cloning the repository you will need to run `git submodule update --init --recursive` to setup submodules.

The game can then be built by running the `build\win32\build.bat` script with a build profile as the first argument. The following build
profiles are available:

- `win32` which builds the Windows version of the game.
- `web` which builds the Web version of the game.
- `tools` which builds auxiliary tools used for development.

Builds also accept an extra argument `release` which can be used to build the optimized release version of the game.

There are also scripts available for building a version of the game on MacOS, this version has not been properly tested or released but
is available for anyone who wants to build the game locally for their Mac. Due to the code using SDL2 it should be relatively easy to
also build and port the game to a variety of other platforms (e.g. Linux).

## License

The project's code is available under the **[MIT License](https://github.com/JROB774/grow-your-guarden/blob/master/LICENSE)**.
