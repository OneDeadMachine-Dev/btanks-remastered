# Build and run

These instructions cover the currently exercised **macOS / Apple Silicon** toolchain and the Web build. Use the repository root as your working directory. Historical Windows/Linux build files are retained, but this remaster has not been validated on those platforms.

## Native macOS

Install Xcode Command Line Tools if needed:

```sh
xcode-select --install
```

With Homebrew installed:

```sh
brew install sdl3 libvorbis expat scons pkgconf
./build-mac.sh
./run-mac.sh
```

`build-mac.sh` discovers Homebrew with `brew --prefix`. Lua 5.1.5 and stb_image are bundled in the source tree. The game locates its libraries and `data/` relative to the binary, so `run-mac.sh` can also be invoked by its absolute path.

Build a local app bundle:

```sh
./make-app.sh
```

Output: `dist/Battle Tanks.app`. The bundle contains the executable, game data and runtime libraries. It uses ad-hoc signing for local use. Public signed binaries require a separate Developer ID/notarization and release process.

### Debug and regression checks

```sh
./build-mac.sh mode=debug
./tests/run-regressions.sh
```

Return to the release configuration before packaging:

```sh
./build-mac.sh mode=release
```

The component regression tests use temporary configuration files and SDL dummy/software rendering. More detail is in [TESTING.md](TESTING.md).

## WebAssembly / WebGL2

```sh
brew install emscripten cmake ninja
./make-web.sh
```

Output: `build-web/out/index.html`, JavaScript, WASM and the game-data archive. The first build needs network access for Emscripten ports and the Expat release declared in [CMakeLists.txt](../web/CMakeLists.txt). Do not open the HTML directly with a file URL.

To build and serve locally:

```sh
./make-web.sh serve
```

Open [localhost:8765](http://localhost:8765/index.html?fps). To serve an existing build without rebuilding:

```sh
python3 -m http.server 8765 --bind 127.0.0.1 --directory build-web/out
```

Useful query parameters:

- `?fps` — browser frame counter.
- `?args=--no-sound` — start with audio disabled.
- `?args=--no-sound,--map=arena` — preload a map.

Native TCP/UDP networking is disabled in the Web build. Browser settings and profiles are stored in IndexedDB for the page's origin; changing the hostname or port creates a different storage origin. Wait for a normal save before closing the page; abrupt close during an asynchronous sync has not been validated.

## Configuration and content

The native game stores configuration and progress in `~/.btanks/bt.xml`. Back up your own progress before manual migration or crash experiments. The Web build uses a virtual `~/.btanks` backed by IndexedDB.

The bundled content includes 13 TMX maps. Campaign support exists in the engine, but a `campaign.xml` campaign pack is not included in the base content. The historical map editor still needs its own SDL3 port.

## Repository documentation

```sh
python3 scripts/check-docs.py
```

This checks local documentation/image links, preserved root license copies and accidental inclusion of local-only files. It does not verify external websites or execute the game.
