# Changelog

This file tracks the modern remaster. Historical upstream changes remain in [the original ChangeLog](btanks-code/trunk/ChangeLog).

## Unreleased

### Repository presentation

- Russian and English introductions, the maintainer's childhood story, original-team credits and actual game screenshots.
- Build, testing, contribution and publishing guides; preserved root license texts and a dependency-notice index.
- GitHub issue/PR templates and a macOS/Web build workflow; current results are linked from the README badge.
- Exclusion of local tooling, audit artifacts and SVN working-copy metadata from the current tracked tree.

### Repairs verified on 21 October 2026

- Bounded collision-mask reads, corrected partial masks and clipping to both source images.
- A valid default team count for Team Deathmatch.
- UDP peer association for clients sharing an IP, safe TCP socket lifetime during disconnect and protected send-queue access.
- Clock synchronization before timestamp-based rewind; signed packet age and timer rollover handling.
- Dedicated-server respawn without a campaign profile.
- SDL3 joystick instance IDs, logical mouse coordinates, cursor hiding and initialized mouse-control state.
- Atomic configuration replacement with failed-write preservation.
- Float positive exponents and aligned integer serialization.
- Pending world commands and detached objects discarded when changing maps.
- SCons debug arguments, invalid FPS-limit fallback and consistent native/Web version strings.
- Map assignment compatibility with the libc++ shipped in Xcode 16.4; redundant `const` key template arguments removed consistently, with the ordered serialization format preserved.

### Modern port, October 2026

- SDL3 platform migration and GPU sprite rendering.
- Widescreen support, texture atlases and fixed-step simulation.
- Relocatable native libraries and self-contained macOS packaging.
- Emscripten/WebGL2 build, browser help and IndexedDB settings/profile persistence.
- Updated controls and removal of the defunct master-server address as the default.

This is a development snapshot, not a tagged release. See [testing scope](docs/TESTING.md) for the current acceptance limits.
