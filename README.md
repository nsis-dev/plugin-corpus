# nsis-plugins-corpus

Every plugin listed on the NSIS wiki's [Category:Plugins][cat], downloaded and
laid out so a path that works against `NSISDIR` works here too.

**This is a shadow of `NSISDIR`, not a replacement for it.** Nothing here is
installed into the real NSIS tree, and nothing here is on `makensis`'s default
search path. Point at it explicitly when you want it:

```nsis
!addplugindir "path/to/nsis-plugins-corpus/tree/Plugins/x86-unicode"
```

[cat]: https://nsis.sourceforge.io/Category:Plugins

## Why it exists

A plugin's **output count cannot be discovered** — NSIS offers no way to ask a
DLL how many values it pushes, and getting it wrong unbalances the stack with no
diagnostic from anybody. Documentation is not a reliable substitute: for
`AccessControl`, the readme, the wiki page and the way real installers call it
each imply a *different* answer, and all three are wrong. Only the source
settles it.

So the number that matters most is this one:

| | |
| --- | --- |
| Plugin pages on the wiki | 200 |
| Archives that still download | 161 |
| **Plugins shipping source** | **129** |

690 `.c`/`.cpp`/`.pas`/`.h` files. That is the evidence base for anything that
has to state an arity.

## Layout

```
tree/           a shadow NSISDIR -- Plugins/ Docs/ Include/ Examples/ Contrib/
by-plugin/      each archive extracted on its own, provenance intact
archives/       the downloaded .zip/.7z, untouched
pages/          nothing (scratch, kept so the fetcher can resume)
plugins.json    the 200 wiki page names
links.json      page -> archive URLs found on it
status.json     page -> local archive, or why not
merge-report.json  files placed, and every colliding path
fetch.log       what happened, with timestamps
```

Use `tree/` to *build* against and `by-plugin/` to *read*. The merge is lossy on
purpose — two plugins shipping `Include/Foo.nsh` collide, and the collision is
recorded rather than resolved — so `by-plugin/` is the authority on who shipped
what.

### Arch directories

Twenty years of archives agree on nothing: `ANSI`, `Release_Unicode`,
`ReleaseU`, `i386-ansi`, `x64`, `Unicode/Plugins/` and a bare DLL in `Plugins/`
all appear. `tree/` is normalised onto NSIS 3's four directories: the nearest
directory above a DLL that names a charset or a bitness decides where it goes,
and a DLL with no such label is `x86-ansi`, because that is what "old enough to
be unlabelled" means. Debug builds are left out of `tree/`; `by-plugin/` still
has them.

| | DLLs |
| --- | ---: |
| `x86-ansi` | 158 |
| `x86-unicode` | 51 |
| `amd64-unicode` | 16 |
| `amd64-ansi` | 4 |

## Rebuilding

```sh
python3 fetch_plugins.py   # list pages, find archives, download, extract
python3 merge_tree.py      # build tree/ from by-plugin/
```

Both are resumable — they skip anything already on disk, so a re-run picks up
new plugins and repairs partial fetches without re-downloading. `fetch_plugins.py`
sleeps 0.6 s between requests and sends a real user agent; the wiki is
volunteer-run, so leave that alone.

## Known gaps

- **34 of the 200 pages link no archive.** Most describe a plugin hosted
  off-wiki (GitHub, a personal site) or document one that ships with NSIS.
- **HelpButton could not be recovered** (checked 2026-10-08). `darklogic.org`
  is now a parked domain that answers every URL with an HTML landing page, and
  the Wayback Machine has only the 2007 `/win32/nsis/` index, not
  `helpbutton_v0_9.zip`.
- **8 archives came from somewhere other than the first link.** Five wiki
  pages link a `File:` description page rather than the upload itself, so the
  file was taken from its `/mediawiki/images/` URL (Blowfish++, ButtonEvent,
  PopupListBox, RealProgress, SetCursor); `fetch_plugins.py` now does this
  itself. ApplicationID's own site now serves HTML, so it came from the GitHub
  release the page also links. Metadl came from the Wayback Machine's 2011
  capture, and Nsisdbg from its 2007 capture of the author's older
  `kverka.no/~saivert` mirror -- the `saivert.com` link on the wiki redirected
  there before it died.
- **The wiki is not the whole ecosystem.** `Inetc`, `NScurl` and `nsJSON` are
  actively developed on GitHub, and the wiki copy may lag. Check upstream before
  trusting a version number here.
- Nothing is verified, signed, or scanned. These are binaries downloaded from a
  public wiki; treat them as untrusted input and read the source, which is why
  the source is the point.
