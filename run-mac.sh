#!/bin/zsh
# Запуск Battle Tanks Remastered (macOS, Metal через SDL3).
# Игра ищет свои библиотеки по относительным путям build/release/*, поэтому
# запускать нужно из btanks-code/trunk — скрипт делает это сам.
set -e
DIR="${0:A:h}"
cd "$DIR/btanks-code/trunk"
exec ./btanks "$@"
