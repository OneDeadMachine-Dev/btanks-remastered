#!/bin/zsh
# Сборка Battle Tanks Remastered для macOS (Apple Silicon).
# Зависимости (всё из Homebrew):  brew install sdl3 libvorbis expat scons pkgconf
# Lua 5.1 вшит в репозиторий (btanks-code/trunk/third_party/lua).
set -e

BREW="$(brew --prefix)"
INC="-I$BREW/include -I$BREW/include/SDL3 -I$BREW/opt/expat/include"
export PKG_CONFIG_PATH="$BREW/lib/pkgconfig:$BREW/share/pkgconfig"
export CFLAGS="$INC"
export CXXFLAGS="$INC -std=gnu++17"
export LDFLAGS="-L$BREW/lib -L$BREW/opt/expat/lib"

cd "$(dirname "$0")/btanks-code/trunk"
exec scons -j"$(sysctl -n hw.ncpu)" mode=release "$@"
