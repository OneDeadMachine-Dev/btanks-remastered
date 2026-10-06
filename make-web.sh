#!/bin/zsh
# Веб-версия (Emscripten / WebGL2). Нужно: brew install emscripten cmake ninja
#   ./make-web.sh          # собрать в build-web/out
#   ./make-web.sh serve    # собрать и открыть на http://localhost:8765
# Параметры страницы: ?fps (счётчик кадров), ?args=--no-sound  (аргументы игры через запятую)
set -e
ROOT="${0:A:h}"
cd "$ROOT"
[[ -f build-web/build.ninja ]] || emcmake cmake -S web -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-web
echo "готово: $ROOT/build-web/out/index.html"
if [[ "$1" == serve ]]; then
	echo "http://localhost:8765/index.html?fps"
	exec python3 -m http.server 8765 --bind 127.0.0.1 -d build-web/out
fi
