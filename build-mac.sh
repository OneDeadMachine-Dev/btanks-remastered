#!/bin/zsh
# Сборка Battle Tanks (btanks) для macOS на Apple Silicon.
# Зависимости: Homebrew (sdl, smpeg, libvorbis, expat, scons, pkgconf),
# ~/btanks-deps (Lua 5.1.5 + SDL_image 1.2, собираются отдельно).
set -e

export PKG_CONFIG_PATH="$HOME/btanks-deps/lib/pkgconfig"
export CFLAGS="-I$HOME/btanks-deps/include -I$HOME/btanks-deps/include/SDL -I/opt/homebrew/include/SDL3 -I/opt/homebrew/include/SDL3_image -I/opt/homebrew/include -I/opt/homebrew/opt/expat/include"
export CXXFLAGS="-I$HOME/btanks-deps/include -I$HOME/btanks-deps/include/SDL -I/opt/homebrew/include/SDL3 -I/opt/homebrew/include/SDL3_image -I/opt/homebrew/include -I/opt/homebrew/opt/expat/include -std=gnu++17"
export LDFLAGS="-L$HOME/btanks-deps/lib -L/opt/homebrew/lib -L/opt/homebrew/opt/expat/lib"

cd "$(dirname "$0")/btanks-code/trunk"
exec scons -j8 mode=release "$@"
