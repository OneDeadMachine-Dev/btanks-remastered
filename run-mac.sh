#!/bin/zsh
# Запуск Battle Tanks Remastered (macOS, Metal через SDL3).
# Игра сама находит данные и библиотеки рядом с бинарником, откуда ни запускай.
set -e
DIR="${0:A:h}"
exec "$DIR/btanks-code/trunk/btanks" "$@"
