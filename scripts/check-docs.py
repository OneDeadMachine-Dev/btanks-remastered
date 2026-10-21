#!/usr/bin/env python3
"""Check the publishable documentation and repository-only packaging rules."""
import os
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
result = subprocess.run(
    ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
    cwd=ROOT, check=True, capture_output=True,
)
files = sorted({os.fsdecode(p) for p in result.stdout.split(b"\0") if p})
errors = []
local_only = (".claude/", ".playwright-cli/", "docs/evidence/", "build-web/", "dist/")
for name in files:
    path = ROOT / name
    if ".svn" in path.relative_to(ROOT).parts or name.startswith(local_only):
        errors.append(f"Local-only file in publication set: {name}")
    if not path.is_file():
        continue
    if path.stat().st_size > 50 * 1024 * 1024:
        errors.append(f"File exceeds this repository's 50 MiB policy: {name}")
    if path.suffix != ".md":
        continue
    content = path.read_text(encoding="utf-8")
    # Ignore code examples; validate Markdown links and inline HTML images/links.
    content = re.sub(r"^```.*?^```[^\n]*$", "", content, flags=re.M | re.S)
    links = re.findall(r"!?\[[^\]]*\]\((<[^>]+>|[^)\s]+)(?:\s+[^)]*)?\)", content)
    links += re.findall(r"(?:src|href)=[\"']([^\"']+)[\"']", content)
    for link in links:
        link = link.strip("<>")
        if urlsplit(link).scheme or link.startswith("//") or link.startswith("#"):
            continue
        target = unquote(urlsplit(link).path)
        if not target:
            continue
        resolved = (path.parent / target).resolve()
        if not resolved.is_relative_to(ROOT) or not resolved.exists():
            errors.append(f"Broken local link in {name}: {link}")
            continue
        relative = resolved.relative_to(ROOT).as_posix()
        if resolved.is_file() and relative not in files:
            errors.append(f"Link in {name} points outside publication set: {link}")
for name in ("LICENSE", "LICENSE.EXCEPTION"):
    root_license = ROOT / name
    if not root_license.is_file() or root_license.read_bytes() != (ROOT / "btanks-code/trunk" / name).read_bytes():
        errors.append(f"Root {name} must match the original license bytes")
if errors:
    print("\n".join(errors), file=sys.stderr)
    sys.exit(1)
print(f"PASS: {sum(name.endswith('.md') for name in files)} Markdown files, local links/images, original license copies and publication file rules")
