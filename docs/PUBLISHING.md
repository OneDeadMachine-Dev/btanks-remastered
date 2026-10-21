# Publishing and maintaining the GitHub repository

Public source repository: **[OneDeadMachine-Dev/btanks-remastered](https://github.com/OneDeadMachine-Dev/btanks-remastered)**. The default development branch is `main`. Pages and binary releases remain separate publication steps.

## Repository metadata

- **Name:** `btanks-remastered`
- **Description:** A childhood favorite, brought back to modern macOS and the browser. An SDL3/WebAssembly remaster of Battle Tanks with original-team credits preserved.
- **Topics:** `battle-tanks`, `btanks`, `game-preservation`, `retrogaming`, `cpp`, `sdl3`, `macos`, `webassembly`, `webgl2`, `open-source-game`
- **Website:** leave unset until a real demo or project site is published.
- **Social preview:** use `docs/images/curfew.png`, an actual 1280×720 game screenshot, with its artwork attribution retained in the repository.

The primary README is Russian; [README.en.md](../README.en.md) provides the English version. Both carry the same childhood story, platform scope and original credits.

## Contents prepared for publication

Source, original game assets, licenses, build scripts, component regressions, screenshots, guides, issue forms, PR template and CI configuration.

The current index excludes old `.svn` working-copy metadata. Local copies remain on disk. This cleanup does not rewrite previous commits; historical SVN metadata still exists in the old Git history.

The `.gitignore` also excludes local assistant settings, build outputs, browser artifacts, raw diagnostic evidence and the three machine-specific audit reports. Those files stay on the maintainer's machine. Portable results live in [TESTING.md](TESTING.md) and [CHANGELOG.md](../CHANGELOG.md).

Before committing the publication set, inspect it explicitly:

```sh
python3 scripts/check-docs.py
git diff --check
git status --short
git diff --cached --stat
```

Review the existing gameplay repairs as part of the first public development snapshot. Commit only the intended files; no history rewrite is required for the prepared layout.

## Local preparation verification

The publishable source set was built in a separate clean copy: native release, Web and the component suite passed. Local documentation links, image loading and license-copy equality were checked. README previews were inspected at desktop and mobile widths in light/dark styling. The local preview approximates GitHub rendering; the hosted README is available in the public repository.

## First hosted run

Hosted build and check results are available in [GitHub Actions](https://github.com/OneDeadMachine-Dev/btanks-remastered/actions). The workflow uses macOS 15 arm64 as documented by [GitHub's runner reference](https://docs.github.com/en/actions/reference/runners/github-hosted-runners). Do not describe the hosted workflow as passing until it actually completes.

The source repository is public. This publication does not deploy a hosted game demo, add collaborators or create a signed binary release.

## Before a binary release

A public repository and a downloadable signed release are different milestones. For a release, build the reviewed revision, record its toolchain and validation limits, include the exact dependency notices/corresponding source required by its licenses, and complete signing/notarization for macOS distribution. Keep the source revision and generated artifacts clearly associated.

A Web build can later be deployed as static files from `build-web/out`; the current workflow only verifies compilation and does not deploy it.
