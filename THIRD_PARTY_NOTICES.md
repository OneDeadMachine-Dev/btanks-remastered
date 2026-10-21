# Licenses and third-party notices

This file is an index to notices shipped with this checkout. It does not replace or amend those notices.

## Battle Tanks

The original [GNU GPL v2 text](LICENSE) and [historical exception](LICENSE.EXCEPTION) are reproduced byte-for-byte from `btanks-code/trunk`. Game source headers generally specify **GPL version 2 or later**. Retain the original copyright notices and [credits](CREDITS.md) with the source and distributed game.

The exception names Netive Company Ltd. and contains terms specific to that rights holder. It is preserved as historical license text, without extending its permissions to the remaster maintainer.

## Bundled source

| Component | Notice included in this checkout |
|---|---|
| Historical Clunk r125 | [GNU LGPL 2.1 text](btanks-code/trunk/clunk/LICENSE); consult the individual file headers as well |
| MRT and SDLX wrappers | LGPL 2.1 or later notices in file headers, e.g. [mrt/file.cpp](btanks-code/trunk/mrt/file.cpp) and [sdlx/c_map.cpp](btanks-code/trunk/sdlx/c_map.cpp) |
| SDL graphics helpers | LGPL notices in the [bundled gfx source](btanks-code/trunk/sdlx/gfx/SDL_rotozoom.c) |
| Lua 5.1.5 | Copyright and MIT permission notice at the end of [lua.h](btanks-code/trunk/third_party/lua/lua.h) |
| stb_image | MIT / public-domain alternatives at the end of [stb_image.h](btanks-code/trunk/third_party/stb/stb_image.h) |

Game graphics, audio and maps are the original Battle Tanks content retained with its original distribution notices. The remaster does not claim to have created that content or assign it a new blanket license. Historical material under `btanks-code/branches` keeps its existing notices too.

## Build and runtime dependencies

The native build links dependencies installed with Homebrew, including SDL3, libvorbis, libogg, Expat and zlib. The Web build uses Emscripten ports and fetches the pinned Expat release declared in [web/CMakeLists.txt](web/CMakeLists.txt).

When distributing binaries, include the notices and any corresponding source required by the licenses of the exact dependency versions in that release. The local `.app` packaging script is a development packaging tool; signing, release notices and source distribution are separate release work described in [PUBLISHING.md](docs/PUBLISHING.md).
