# Testing and current validation

Local results recorded on **21 October 2026**. These are concrete checks of a development snapshot, not a claim that every game scenario is stable.

## What passed locally

| Check | Scope and result |
|---|---|
| Native release build | Apple Silicon; executable, libraries and `.app` built; ad-hoc signature verified |
| Web build | Emscripten / WebGL2 build and game startup |
| Collision regression | 40,000 deterministic comparisons against an independent pixel oracle |
| Component regressions | Release and ASan/UBSan: save failure preservation, XML escaping, float/integer roundtrips, clocks, pending world commands, cursor coordinates/visibility and virtual joystick routing |
| Native maps | All 13 maps, 3 AI, 31 simulation seconds per map, SDL dummy/software with ASan/UBSan; no reported error |
| Web maps | All 13 maps, at least 32 wall-clock seconds after loading each; Chrome, up to three tabs at once; no captured runtime error |
| Team Deathmatch | Fresh browser profile → default two teams → team selection → movement and firing |
| Networking | Two same-IP clients, 90-second scenario with movement/firing and disconnect/reconnect; world updates and respawns continued |
| Sanitized networking | Separate 45-second run with a diagnostic server and two clients, without autopilot; no ASan/UBSan report |
| Browser persistence | Changed profile name → save → completed syncfs callback → reload; name preserved, no temporary save files left |
| Extended native harness | 600 simulation seconds, 9 map changes, scripted movement/fire and death events; exit 0 after the world-clear repair |

The extended run exercised the repaired world/network code. A later serializer alignment change was checked separately by the component and sanitized network tests. Final packaged builds include that change. Full memory-leak detection was not run (`detect_leaks=0`); observing RSS or WASM capacity is not proof that no leaks exist.

## Run the component suite

From the repository root on the supported Homebrew/Apple Silicon setup:

```sh
./build-mac.sh
./tests/run-regressions.sh
```

The runner creates and removes a temporary directory. It exercises the real collision, configuration, serialization and input components; it does not drive a complete match.

For a separately prepared sanitizer build:

```sh
BTANKS_TEST_TRUNK=/absolute/path/to/diagnostic/btanks-code/trunk \
BTANKS_TEST_SANITIZERS=1 ASAN_OPTIONS=detect_leaks=0 \
UBSAN_OPTIONS=halt_on_error=1 ./tests/run-regressions.sh
```

The optional path selects built libraries; it is not an automatic sanitizer-build command. Gameplay crash/respawn harnesses also require isolated test profiles and should not be run against your primary save.

## Limits

Physical gamepad behavior, real-device audio, separate computers and real NAT/LAN, full CTF completion, touch controls and the historical editor remain unvalidated. The Windows implementation of atomic replacement has not been built or run on Windows. The failed-write regression does not simulate sudden power loss or every possible filesystem failure.

The historical native network protocol remains in place; the remaster has no hosted multiplayer service or browser networking yet.

## Publication-source check

On 21 October, the exact publication set (1,507 files) was copied into a fresh directory without old SVN metadata, existing build outputs, local assistant settings or raw audit files. Native and Web builds both completed successfully, and the native component suite passed against the freshly built libraries. The standard Web build fetched/configured its dependencies without a source-cache override. This validates the local publication copy, not a hosted Actions run.

## GitHub checks

The [CI workflow](../.github/workflows/ci.yml) prepares a documentation check, a native macOS build/component-test job and a Web compile job. These are distinct from the interactive and extended checks above. The README badge reads the live result from [GitHub Actions](https://github.com/OneDeadMachine-Dev/btanks-remastered/actions/workflows/ci.yml); consult the individual job logs for the actual result of a revision.

Detailed local audit logs, test-copy patches and machine-specific observations stay outside the publication set. Public screenshots and this summary preserve the relevant results without making raw diagnostic dumps part of the repository's front page.
