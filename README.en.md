<div align="center">

# Battle Tanks Remastered

**A childhood game, brought back to modern computers.**

[![Build and checks](https://github.com/OneDeadMachine-Dev/btanks-remastered/actions/workflows/ci.yml/badge.svg)](https://github.com/OneDeadMachine-Dev/btanks-remastered/actions/workflows/ci.yml)

[Русский](README.md) · [Build & run](docs/BUILDING.md) · [Credits](CREDITS.md) · [Contributing](CONTRIBUTING.md)

![Battle Tanks Remastered main menu featuring the original hand-drawn artwork](docs/images/battle-tanks-cover.png)

*Original Battle Tanks artwork by Netive Media Group. Screenshot from the current Web build.*

</div>

## Why I came back

Battle Tanks was part of my childhood. My sister and I loved playing it together, and years later I simply could not walk past those familiar tanks.

I wanted to play it again on a modern computer: keep the original artwork, music and feel, and give other people a way to return to it too. That is how **Battle Tanks Remastered** began.

This is a personal preservation and development project built on the work of **Netive Media Group**. The modern port is maintained by **[OneDeadMachine](https://github.com/OneDeadMachine-Dev)**.

## The game

A fast, top-down 2D arcade game with tanks, Shilkas, rocket launchers, infantry, bots and destructible environments. Fight other players, tackle cooperative missions, collect weapons and leave your vehicle to continue on foot.

The bundled content includes **13 maps** and deathmatch, team deathmatch, cooperative and capture-the-flag modes. The original sprites, maps, music and sounds are preserved.

<table>
<tr>
<td width="50%"><img src="docs/images/curfew.png" alt="Vehicles fighting through the streets of Curfew"><br><strong>Curfew</strong> — city combat</td>
<td width="50%"><img src="docs/images/baykonur.png" alt="The coastline and fortifications of Baykonur"><br><strong>Baykonur</strong> — a cooperative mission</td>
</tr>
</table>

## What the remaster adds

- **Modern macOS support:** SDL3, GPU rendering, widescreen presentation and a self-contained `.app` bundle.
- **A browser build:** WebAssembly and WebGL2, local modes, settings and profiles stored in IndexedDB.
- **An updated foundation:** texture atlases, fixed-step simulation, and repairs to collisions, input, networking and saving.
- **The original character:** the existing engine and content remain at the heart of the project.

| Platform | Status |
|---|---|
| macOS / Apple Silicon | Build and gameplay scenarios tested locally |
| Modern browser / WebGL2 | Build and all maps tested; networking disabled |
| Windows and Linux | Historical source retained; remaster builds not yet validated |
| Phones and tablets | Touch controls still need to be implemented |

This is a work in progress. Networking has been tested with two clients on one computer; real LAN/NAT, a physical gamepad and real-device audio still need validation. See [testing notes](docs/TESTING.md).

## Build and play

### macOS / Apple Silicon

Install Xcode Command Line Tools and Homebrew first. From the repository root:

```sh
brew install sdl3 libvorbis expat scons pkgconf
./build-mac.sh
./run-mac.sh
```

To package the executable, libraries and game data:

```sh
./make-app.sh
```

Output: `dist/Battle Tanks.app`. Local bundles use ad-hoc signing; distributing a signed macOS application requires Developer ID signing and notarization.

### Browser

```sh
brew install emscripten cmake ninja
./make-web.sh serve
```

Open [localhost:8765](http://localhost:8765/index.html?fps). The first build downloads Emscripten dependencies and Expat. This runs a local server; a hosted demo has not been published yet.

[Detailed build instructions and configuration](docs/BUILDING.md).

## Controls

| Key | Action |
|---|---|
| Arrow keys | Move |
| Space | Primary weapon |
| Shift | Secondary weapon |
| E | Enter / leave a vehicle |
| Tab | Scoreboard |
| M | Map |
| Esc | Menu |

The Web build has a **?** help button. Controls can be changed in the settings.

## Thank you to the original team

| Contributor | Original contribution |
|---|---|
| Vladimir **“Whoozle” Menshakov** | Programming |
| Alexander **“Methos” Wagner | Graphics |
| Vladimir **“VZ” Zhuravlev** | Level design |
| Vladimir **“Gold” Goldobin** | Tools |
| Vladimir **“Petrovich” Volkov** | Music |
| Leonid **“Dark Matter” Volkov** | Sound effects |
| **Netive Media Group, 2006–2009** | Game design |

Thank you for making a game worth coming back to after all these years.

[Full credits and provenance](CREDITS.md) · [Original GitHub project](https://github.com/whoozle/btanks) · [SourceForge](https://sourceforge.net/projects/btanks/)

This port starts from **SVN r8098, a 2010 snapshot**. That is the source revision date, not a claim that maintenance ended then: the project's [2013 announcement](https://sourceforge.net/p/btanks/news/) describes its move to GitHub.

## What comes next

Touch controls, an SDL3 port of the map editor, further native multiplayer testing, and research into browser networking. These are plans, not currently available features.

Bug reports, testing on other hardware and focused fixes are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md).

## For developers

[Engine guide](docs/PROJECT_GUIDE.md) · [Testing](docs/TESTING.md) · [Changelog](CHANGELOG.md) · [Publishing](docs/PUBLISHING.md)

Active source lives in `btanks-code/trunk`. Historical material in `btanks-code/branches` is retained and is not used by the current builds.

## Licenses

Original copyright notices and in-game credits are retained. The root [LICENSE](LICENSE) contains GNU GPL v2; game source headers allow version 2 or later. The historical [LICENSE.EXCEPTION](LICENSE.EXCEPTION) is preserved unchanged. Bundled libraries retain their own terms; see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
