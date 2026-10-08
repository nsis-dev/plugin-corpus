#!/usr/bin/env python3
"""Merge by-plugin/ into tree/, a shadow of NSISDIR.

Plugin archives agree on nothing. Some are rooted at `Plugins/x86-ansi/Foo.dll`,
some carry a `Contrib/Foo/` tree, and the oldest are a bare DLL beside a readme.
This normalises all three onto the layout `makensis -HDRINFO` reports, so that a
path that works against NSISDIR works here -- without writing into NSISDIR.

Collisions are recorded rather than resolved: two plugins shipping the same
`Include/Foo.nsh` is a fact about the ecosystem worth being able to read back.
"""

import json
import os
import re
import shutil

DEST = os.path.dirname(os.path.abspath(__file__))
BY = os.path.join(DEST, "by-plugin")
TREE = os.path.join(DEST, "tree")

# The directories NSISDIR actually has. `Contrib` holds the space in
# "Language files", so matching is case-insensitive on the segment only.
ROOTS = {"plugins", "include", "docs", "examples", "contrib", "stubs"}

# Archives label arch with whatever their build produced: `x64-unicode`,
# `_release_ANSI`, `ReleaseU`, a bare `Unicode`. A directory counts as an arch
# label if it names a charset or a bitness; NSIS 3's four names are the target.
ARCHY = re.compile(r"ansi|unicode|x64|amd64|x86|i386|^(release|debug)[au]$")

collisions = {}
placed = 0
loose = {}


def copy(src, rel, owner):
    global placed
    dst = os.path.join(TREE, rel)
    if os.path.exists(dst):
        if os.path.getsize(dst) == os.path.getsize(src):
            return  # same file shipped twice; not worth recording
        collisions.setdefault(rel, []).append(owner)
        return
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    shutil.copy2(src, dst)
    placed += 1


def arch_of(src, root):
    """The arch a DLL is for, from the nearest arch-labelled directory above it
    -- `Unicode/Plugins/Foo.dll` and `Plugins/Unicode/Foo.dll` alike. No label
    means x86-ansi, which is what an unlabelled old DLL is. None for a debug
    build: the release build next to it is the one anybody ships."""
    dirs = [d.lower() for d in os.path.relpath(os.path.dirname(src), root).split(os.sep)]
    if any(d.startswith("debug") for d in dirs):
        return None
    for d in reversed(dirs):
        if ARCHY.search(d):
            bits = "amd64" if re.search(r"x64|amd64", d) else "x86"
            cs = "unicode" if re.search(r"unicode|^releaseu$", d) else "ansi"
            return f"{bits}-{cs}"
    return "x86-ansi"


for name in sorted(os.listdir(BY)):
    root = os.path.join(BY, name)
    if not os.path.isdir(root):
        continue

    # Every NSIS-shaped subtree in this archive, at whatever depth it sits.
    anchors = []
    for dirpath, dirnames, _ in os.walk(root):
        for d in list(dirnames):
            if d.lower() in ROOTS:
                anchors.append(os.path.join(dirpath, d))
    # Drop anchors nested inside another anchor: the outermost wins, so a
    # `Contrib/Foo/Docs` is copied as part of Contrib rather than twice.
    anchors = [
        a
        for a in anchors
        if not any(a != b and a.startswith(b + os.sep) for b in anchors)
    ]

    covered = set()
    for anchor in anchors:
        seg = os.path.basename(anchor)
        canon = seg.capitalize() if seg.lower() != "contrib" else "Contrib"
        for dirpath, _, files in os.walk(anchor):
            for f in files:
                src = os.path.join(dirpath, f)
                covered.add(src)
                if canon == "Plugins" and f.lower().endswith(".dll"):
                    arch = arch_of(src, root)
                    if not arch:
                        continue
                    rel = os.path.join(canon, arch, f)
                else:
                    rel = os.path.join(canon, os.path.relpath(src, anchor))
                copy(src, rel, name)

    # Whatever the anchors did not cover. A bare `.dll` in an old archive is a
    # plugin for the ANSI target -- that is what "old" meant -- and a bare
    # `.nsh` is a header. Everything else is documentation by elimination.
    for dirpath, _, files in os.walk(root):
        for f in files:
            src = os.path.join(dirpath, f)
            if src in covered:
                continue
            ext = os.path.splitext(f)[1].lower()
            if ext == ".dll":
                arch = arch_of(src, root)
                if not arch:
                    continue
                rel = os.path.join("Plugins", arch, f)
            elif ext == ".nsh":
                rel = os.path.join("Include", f)
            else:
                rel = os.path.join("Docs", name, os.path.relpath(src, root))
            loose[name] = loose.get(name, 0) + 1
            copy(src, rel, name)

report = {
    "files_placed": placed,
    "plugins": len(
        [d for d in os.listdir(BY) if os.path.isdir(os.path.join(BY, d))]
    ),
    "loose_files_by_plugin": loose,
    "collisions": collisions,
}
json.dump(report, open(os.path.join(DEST, "merge-report.json"), "w"), indent=1)
print(f"placed {placed} files from {report['plugins']} plugins")
print(f"{len(collisions)} colliding paths -- see merge-report.json")
