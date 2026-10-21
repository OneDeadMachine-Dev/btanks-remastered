# Contributing

Thanks for helping keep Battle Tanks playable. This project aims to preserve the feel and content of the original game while improving its compatibility and reliability.

## Report a problem

Use the bug report form in the repository's Issues tab. Include your OS, CPU architecture, native/Web build, map and mode, exact steps, expected behavior and what actually happened. A short recording or screenshot helps with input and rendering problems.

If a crash is intermittent, include how often it happens. Remove private paths or profile data from logs before attaching them. Please do not upload your personal save/configuration by default.

## Work on a change

1. Read [BUILDING.md](docs/BUILDING.md) and the [engine guide](docs/PROJECT_GUIDE.md).
2. Keep each pull request focused. Preserve unrelated work, original credits and license notices.
3. Explain the trigger and the resulting behavior. For changes to rendering, input or menus, include a screenshot or short recording from a real build.
4. Run checks relevant to the change. The local regression suite is described in [TESTING.md](docs/TESTING.md). State clearly which platforms or scenarios you could not test.
5. For new assets or copied code, include their source, author and license.

Use disposable profiles or copied save state for destructive game scenarios. Avoid using your primary progress for crash, migration or save-failure tests.

## Useful contributions

Testing native multiplayer on different computers, real gamepad checks, audio validation, mobile input, compiler/platform fixes and editor work are all useful. Discuss large architecture changes or major visual changes in an issue first so they fit the preservation goal.

The Git history contains the imported original code and later port work. Keep historical branches under `btanks-code/branches` separate from active code in `btanks-code/trunk`.

## Pull requests

Describe what changed, why it matters and how you verified it. Passing compilation does not establish that gameplay, networking or persistence works. Small reproducible examples and honest limits are more useful than broad stability claims.

Русскоязычные issues и pull requests тоже приветствуются.
