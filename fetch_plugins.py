#!/usr/bin/env python3
"""Build a local corpus of NSIS plugins from the wiki's Category:Plugins.

Three phases, each resumable: list the plugin pages, find the archive each one
links to, download and extract them. Nothing is written inside NSISDIR -- the
merged tree is a *shadow* of it, so `Plugins/`, `Docs/`, `Include/` and
`Examples/` line up with the real thing without touching it.
"""

import html
import json
import os
import re
import subprocess
import sys
import time
import urllib.parse
import urllib.request

BASE = "https://nsis.sourceforge.io"
DEST = os.path.dirname(os.path.abspath(__file__))
UA = "installua-plugin-corpus/1.0 (research; contact yathosho@gmail.com)"
DELAY = 0.6

ARCHIVE = re.compile(r"\.(zip|7z|rar|tar\.gz|tgz)$", re.I)

pages_dir = os.path.join(DEST, "pages")
arch_dir = os.path.join(DEST, "archives")
by_plugin = os.path.join(DEST, "by-plugin")
tree = os.path.join(DEST, "tree")
for d in (pages_dir, arch_dir, by_plugin, tree):
    os.makedirs(d, exist_ok=True)

log_path = os.path.join(DEST, "fetch.log")


def log(msg):
    line = f"[{time.strftime('%H:%M:%S')}] {msg}"
    print(line, flush=True)
    with open(log_path, "a", encoding="utf-8") as fh:
        fh.write(line + "\n")


def get(url, binary=False):
    req = urllib.request.Request(url, headers={"User-Agent": UA})
    with urllib.request.urlopen(req, timeout=60) as resp:
        data = resp.read()
    return data if binary else data.decode("utf-8", "replace")


# -- phase 1: the list of plugin pages --------------------------------------

list_path = os.path.join(DEST, "plugins.json")
if os.path.exists(list_path):
    pages = json.load(open(list_path))
else:
    text = get(f"{BASE}/Category:Plugins")
    seg = re.search(r"mw-category(.*?)printfooter", text, re.S)
    seg = seg.group(1) if seg else text
    pages = []
    for href, title in re.findall(r'href="/([^"#]+)" title="([^"]+)"', seg):
        href = html.unescape(href)
        if ":" in href or href in pages:
            continue
        pages.append(href)
    json.dump(pages, open(list_path, "w"), indent=1)
log(f"phase 1: {len(pages)} plugin pages")

# -- phase 2: the archive each page links to --------------------------------

links_path = os.path.join(DEST, "links.json")
links = json.load(open(links_path)) if os.path.exists(links_path) else {}

for i, page in enumerate(pages, 1):
    if page in links:
        continue
    try:
        body = get(f"{BASE}/{page}")
    except Exception as exc:  # a dead page is data, not a stop
        log(f"  page FAIL {page}: {exc}")
        links[page] = []
        continue
    found = []
    for href in re.findall(r'href="([^"]+)"', body):
        href = html.unescape(href)
        clean = href.split("?")[0]
        if not ARCHIVE.search(clean):
            continue
        found.append(urllib.parse.urljoin(f"{BASE}/{page}", href))
    # De-duplicate, keep wiki-hosted first: those are the ones that still exist.
    seen, ordered = set(), []
    for url in sorted(found, key=lambda u: (0 if "/mediawiki/images/" in u else 1, u)):
        if url not in seen:
            seen.add(url)
            ordered.append(url)
    links[page] = ordered
    if i % 10 == 0:
        json.dump(links, open(links_path, "w"), indent=1)
        log(f"  scanned {i}/{len(pages)}")
    time.sleep(DELAY)

json.dump(links, open(links_path, "w"), indent=1)
withlinks = sum(1 for v in links.values() if v)
log(f"phase 2: {withlinks}/{len(pages)} pages link an archive")

# -- phase 3: download ------------------------------------------------------


def slug(page):
    name = urllib.parse.unquote(page)
    name = re.sub(r"[_ ]?plug-?in$", "", name, flags=re.I)
    return re.sub(r"[^A-Za-z0-9._+-]", "_", name).strip("_") or "unnamed"


status = {}
for page, urls in links.items():
    if not urls:
        status[page] = "no-archive"
        continue
    name = slug(page)
    got = None
    for url in urls[:3]:  # a page may list several; the first that works wins
        ext = ARCHIVE.search(url.split("?")[0]).group(0).lower()
        target = os.path.join(arch_dir, name + ext)
        if os.path.exists(target) and os.path.getsize(target) > 0:
            got = target
            break
        try:
            if "/File:" in url:  # a wiki description page, not the upload
                m = re.search(r'href="(/mediawiki/images/[^"]+)"', get(url))
                if not m:
                    raise ValueError("no upload linked from File: page")
                url = urllib.parse.urljoin(BASE, html.unescape(m.group(1)))
            data = get(url, binary=True)
            if len(data) < 64:
                raise ValueError(f"suspiciously small ({len(data)} bytes)")
            if data.lstrip()[:1] == b"<":  # dead hosts answer with a 200 page
                raise ValueError("got HTML, not an archive")
            with open(target, "wb") as fh:
                fh.write(data)
            log(f"  got {name}{ext} ({len(data) // 1024} KiB)")
            got = target
            time.sleep(DELAY)
            break
        except Exception as exc:
            log(f"  dl FAIL {name} <- {url}: {exc}")
            time.sleep(DELAY)
    status[page] = os.path.relpath(got, DEST) if got else "download-failed"

json.dump(status, open(os.path.join(DEST, "status.json"), "w"), indent=1)
ok = sum(1 for v in status.values() if v.startswith("archives/"))
log(f"phase 3: {ok} archives downloaded")

# -- phase 4: extract -------------------------------------------------------


def extract(archive, into):
    os.makedirs(into, exist_ok=True)
    low = archive.lower()
    if low.endswith(".rar"):
        cmd = ["unrar", "x", "-o+", "-inul", archive, into + os.sep]
    elif low.endswith((".tar.gz", ".tgz")):
        cmd = ["bsdtar", "-xf", archive, "-C", into]
    else:  # zip and 7z alike -- 7z reads both, and many ".zip" here are not
        cmd = ["7z", "x", "-y", "-bso0", "-bsp0", f"-o{into}", archive]
    return subprocess.run(cmd, capture_output=True, timeout=300).returncode


extracted = 0
for page, path in status.items():
    if not path.startswith("archives/"):
        continue
    name = slug(page)
    into = os.path.join(by_plugin, name)
    if os.path.isdir(into) and os.listdir(into):
        extracted += 1
        continue
    rc = extract(os.path.join(DEST, path), into)
    if rc != 0:
        log(f"  extract FAIL {name} (rc={rc})")
    else:
        extracted += 1

log(f"phase 4: {extracted} archives extracted to by-plugin/")
log("done -- run merge_tree.py to build the shadow NSISDIR")
