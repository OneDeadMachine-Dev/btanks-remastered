#!/bin/bash
set -euo pipefail
test_repo="$(cd "$(dirname "$0")/.." && pwd)"
test_trunk="${BTANKS_TEST_TRUNK:-$test_repo/btanks-code/trunk}"
test_build="$(mktemp -d "${TMPDIR:-/tmp}/btanks-regression.XXXXXX")"
trap 'rm -rf "$test_build"' EXIT
test_flags=(-std=gnu++17 -I"$test_trunk" -I"$test_trunk/engine" -I"$test_trunk/engine/src" -I/opt/homebrew/include -L"$test_trunk" -L/opt/homebrew/lib -Wl,-rpath,"$test_trunk")
if [[ "${BTANKS_TEST_SANITIZERS:-0}" == 1 ]]; then
  test_flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
"${CXX:-clang++}" "${test_flags[@]}" "$test_repo/tests/collision_regression.cpp" -lsdlx -lmrt -lSDL3 -o "$test_build/collision"
"$test_build/collision"
"${CXX:-clang++}" "${test_flags[@]}" "$test_repo/tests/runtime_regression.cpp" \
  "$test_trunk/engine/controls/joyplayer.cpp" "$test_trunk/engine/controls/control_method.cpp" "$test_trunk/engine/controls/simple_joy_bindings.cpp" \
  -lbtanks_engine -lsdlx -lmrt -lSDL3 -o "$test_build/runtime"
mkdir "$test_build/profile"
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software "$test_build/runtime" "$test_build/profile"
