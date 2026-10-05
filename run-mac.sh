#!/bin/zsh
# Запуск Battle Tanks на macOS (Apple Silicon).
# Игра ищет свои библиотеки по относительным путям build/release/*, поэтому
# запускать нужно из btanks-code/trunk — скрипт делает это сам.
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR/btanks-code/trunk"
export DYLD_LIBRARY_PATH="$HOME/btanks-deps/lib"
exec ./btanks --no-gl "$@"
