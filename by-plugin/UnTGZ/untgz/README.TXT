untgz plugin for NSIS v2
untgz - unzip like replacement plugin, except for gzipped tarballs
KJD <jeremyd@computer.org>

  This plugin implements functionality similar to the untgz
  sample program distributed with zlib.  It will uncompress
  gzipped (zlib compatible) tar files (*.tgz/*.tar.gz/tarballs) and 
  extract one or more files as indicated by the called command.
  Compile time option to also support lzma (*.tlz/*.tar.lzma)
  and bzip2 (TODO) compressed tar files.

  See below for usage information.  Basic portable tarfiles
  are supported, though no particular checking is done, only
  regular files and directories are processed, so extensions
  in the tar file may confuse it, and at present the full
  tar spec is not handled.  Please verify the tarfiles you
  will use work with it before deploying; the pluginTester
  program will allow one to do this easily without the need
  to write and compile a NSIS script.  Only supports filenames
  up to 100 characters (standard tar short names).


  This software based on NSIS exDLL (example plugin) and zlib library.
  I personally add no additional copyright, so the license is the
  combination of NSIS exDLL (example plugin) and decompression
  libraries used.  For basic gzip'd tarballs (which rely on zlib),
  this is essentially MIT/BSD licensed.  Support for lzma compressed
  tarballs requires the LZMA files and their LGPL/CPL with exception
  license.
  
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors (or copyright holders) be
  held liable for any damages arising from the use of this software.


  To build:
  - extract in the NSIS2 Contrib\ directory (along side the other
    NSIS plugins).  All needed files are included in archive.
  - copy either the MSVC6\*.* or MSVC7\*.* project files to
    Contrib\untgz\ directory, then open the project file.
  - From the build menu select Release (or Debug) build and build.
    The Release build will place untgz.dll in the ../../Plugins directory.

  The primary release is built with MSVC 6 (optimizing compiler).
  The MSVC 7 project files are the MSVC 6 ones converted to MSVC.NET
  (aka 7.0) and tested with MSVC.NET standard (non-optimizing).
  The project files should work with MSVC.NET2003 (aka 7.1), though
  some adjustments to the the miniclib may be necessary for the
  optimizing version of MSVC 7.x  (MSVC 7 project may be out of date)

  For makefiles or generating your own project, the important settings
  are no default libraries (-nodefaultlib), but do link with Windows
  Kernel32 and User32 (kernel32.lib user32.lib), and may still be
  necessary to keep from needing helper function (-Gs16000).
  The include path must also be updated to include the zlib subdirectory
  (-I./zlib).
  E.g. -opt:nowin98 -machine:I386 -nodefaultlib -Gs16000 kernel32.lib
       user32.lib -I./zlib
  Then just include all the C/C++ files in the archive (.\untgz.cpp, 
  .\nsisUtils.c, .\miniclib.c, .\untar.c, .\zlib\*.c, .\lzma\*.c,
  and .\untgz.rc).


---
 * Please note: all material here other than those by KJD
 * have been taken from the respective author/copyright holder's
 * work, do not contact them regarding these [my] changes.
 * Corrections/additions/ or information to obtain original
 * versions, please contact me: jeremyd --at-- fdos.org
 * (defunct: jeremyd@computer.org)
---

/*
 * This plugin largely based on untgz sample included with zlib:
 *
 * untgz.c -- Display contents and/or extract file from
 * a gzip'd TAR file
 * written by "Pedro A. Aranda Guti\irrez" <paag@tid.es>
 * adaptation to Unix by Jean-loup Gailly <jloup@gzip.org>
 */

--- Large Portions from ZLIB general purpose compression library ---
    version 1.2.2, October 3rd, 2004
    Copyright (C) 1995-2004 Jean-loup Gailly and Mark Adler

    version 1.1.4, March 11th, 2002
    Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

--- Portions from NSIS, especially ExDLL (Example NSIS Plugin) ---
    Copyright (C) 1999-2002 Nullsoft, Inc.

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software. 

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.

--- Portions from ExtractDLL v0.1 beta ---
    Copyright 2002 
    by Tim Kosse
    tim.kosse@gmx.de

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

--- LZMA support from 7Zip's LZMA SDK ---
    LZMA SDK 4.23  Copyright (C) 1999-2005 Igor Pavlov
  LZMA SDK is licensed under two licenses:
  (see lzma directory for full license text of LGPL and CPL)
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and 
  follow rules of that license.

  SPECIAL EXCEPTION
  Igor Pavlov, as the author of this code, expressly permits you 
  to statically or dynamically link your code (or bind by name) 
  to the files from LZMA SDK without subjecting your linked 
  code to the terms of the CPL or GNU LGPL. 
  Any modifications or additions to files from LZMA SDK, however, 
  are subject to the GNU LGPL or CPL terms.

  SPECIAL EXCEPTION allows you to use LZMA SDK in applications with closed code, 
  while you keep LZMA SDK code unmodified.

  SPECIAL EXCEPTION #2: Igor Pavlov, as the author of this code, expressly permits 
  you to use this code under the same terms and conditions contained in the License 
  Agreement you have for any previous version of LZMA SDK developed by Igor Pavlov.

  SPECIAL EXCEPTION #2 allows owners of proprietary licenses to use latest version 
  of LZMA SDK as update for previous versions.

  SPECIAL EXCEPTION #3: Igor Pavlov, as the author of this code, expressly permits 
  you to use code of examples (LzmaTest.c, LzmaStateTest.c, LzmaAlone.cpp) as 
  public domain code. 

/****************************************************************************/

/*
  USAGE:
  untgz::extract [-j] [-d basedir] [-k] [-z<type>] tarball.tgz
    extracts files from tarball.tgz
	if -j is specified then ignore paths in tarball (junkpaths)
	if -d is specified will extract relative to basedir
	if -k is specified will not overwrite existing files (keep)
      if -z is specified determines compression used, see below
  untgz::extractV [-j] [-d basedir] [-k] [-z<type>] tarball.tgz [-i {iList}] [-x {xList}] --
    extracts files from tarball.tgz
	if -j is specified then ignore paths in tarball (junkpaths)
	if -d is specified will extract relative to basedir
	if -k is specified will not overwrite existing files (keep)
      if -z is specified determines compression used, see below
	if -i is specified will only extract files whose filename matches
	if -x is specified will NOT extract files whose filename matches
	the -- is required and marks the end of the file lists
  untgz::extractFile [-d basedir] [-z<type>] tarball.tgz file
    extracts just the file specified
	path information is ignored, implictly -j is specified (may also be explicit)

  For compatibility with tar command, the following option specifiers may be
  used, however, they are simply ignored.
      -x indicates action to perform is extraction (extract)
      -f archive-name indicates name of tarball (filename), note
         even when used, the filename must be last argument

  If the tarball uses a compression other than gz, then the -z<type> option
  must be specified to indicate how tar file is compressed.
  -z      indicates gzip (.tgz/.tar.gz) compression, uses zlib, 
          this is the default if -z<type> option is omitted
  -znone  indicates uncompressed tar file (.tar)
  -zlzma  indicates lzma (.tlz/.tar.lzma) compression
  -zbz2   indicates bzip2 (.tbz/.tar.bz2) compression, NOT YET IMPLEMENTED
  

  NOTES:
    Without -j there is a security issue as no checking is done to paths,
    allowing untrusted tarballs to overwrite arbitrary files (e.g. /bin/*).
    Also no checking is done to directory or file names.  In untar.c there
    is a hook so custom versions can modify/strip filepaths prior to opening.
    The -d option is currently implemented by a chdir to indicatd directory
    prior to extraction; future versions may instead prepend to extracted path.
*/

TODO:
- determine most compatible use of prefix (when and if to use, and how much)
  to support file names longer than 100 characters. [DOES ANYBODY CARE?]
- add support for gnu/star extension for Sparse files. [DOES ANYBODY CARE?]

NOTES:
- presently tested with NSIS v2
- does not require a c runtime library (important if say you compile with MS VC 7 and don't
  want to require its runtime present during install [or Windows 95 and compiled with MS VC 5 or 6])
- Up to and including 1.0.7 included a slightly modified zlib so as to use Win32 API and not c runtime
  library see below for exact patches, requires included miniclib.{h,c}
- 1.0.8 it mostly uses the heavily modified modified zlib included with NSIS, with certain
  adjustments to support gzio.c as needed by plugin, which is a combination the NSIS zlib with
  EXEHEAD both defined and undefined.
- From 1.0.9 updated to zlib 1.2.2 and using included modified zlib.
- to create a .tar.lzma (or .tlz) file, create a standard tarball (uncompressed) and then
  compress using lzma.exe within LZMA SDK (http://www.7-zip.org/sdk.html)


HISTORY:
- 20030101  KJD  1.0.0
  initial release (used for AbiWord v2 as a replacement for spawning unzip.exe)
- 20030103  KJD  1.0.1
  modified to not require c runtime library and added version resource
  (note still links with crt for some oddball VC functions stkchk, chkesp, & allmul)
- 20030109  KJD  1.0.2
  implemented extract (include) and don't extract (exclude) list parsing
  cleaned up compile some (no longer needs to link with MSVCRT for some static functions
  added MS VC 6 project file (makefile generated from this)
  moved extraction from tar.gz code (mostly untgz.c from zlib anyway) into separate file from plugin
- 20030115  KJD  1.0.3
  fixed stack bug, rearranged options so doesn't read past called options
  placed utility functions into separate file (so easier for others to reuse if wanted)
  added example/test files
- 20030920  KJD  1.0.4
  correct Makefile.abi, adjust source so doesn't use intrinsic memcpy/memset functions
  even when /Oi (enable intrinsics) is given (e.g. via /O2)
- 20040408  KJD  1.0.5
  patch by Tavella Gaston to correctly return an error 'status' when we fail
  to extract from the tarball
  updated example.nsi to demonstrate errors and added truncated examplecorrupt.tgz
- 20041128  KJD  1.0.6
  a little late but anyway, handle zero (0) byte files in tarballs as requested by 
  Brian Smith (amusing as I went to a college with a different Brian Smith :-)
- 20041226  KJD  1.0.7
  removed potential issue where plugin 1st attempted to create file (to extract)
  then only if that failed junked any path stored in tarball then try again,
  it now only tries once after either junking path or attempting to create path.
  Added but not compiled in by default function to strip paths of leading root (\)
  or parent (..) references.
  Added mkdummyfiles.cpp, creates set of dummy files for testing purposes, see source.
  Added plgntester.cpp, can be used to run (test) plugins outside of an installer;
  see plgntester.txt for more information.
- 20041227  KJD  1.0.8
  changed to use NSIS' included zlib (more-or-less), reduced size in half
  added check to ensure header chksum appears valid
  treat type '7' (contiguous files) as regular files instead of ignoring
  moved plgntester to own archive
  add MSVC 7 project files
  *tested and appears to work, however due to changes, problems may still exist*
- 20050629  KJD  1.0.9
  bugfix, some tarballs would fail to inflate correctly causing extraction
  failure due to issue with used zlib, therefore updated to zlib 1.2.2 and
  no longer using NSIS' included zlib (unfortunately plugin size increased).
  changed -d option so sets working directory after opening tarball; note:
  this may require changes in your installer if you relied on tarball being
  in same directory as specified by -d instead of providing path to it;
  so current directory at time of call is used for tarball without path
  as expected.  Additionally, as -d will attempt to create path if it does
  not exist, this has been moved so only final -d path creation tried.
  Added experimental support for GNU tar long names (max 255 char limit).
- 20050702  KJD  1.0.10
  gzipped (.tgz) and uncompressed (.tar) TAR file handling unchanged.
  added initial support for other compression methods, must be specified manually.
  implemented support for LZMA compressed tarballs (.tar.lzma or possibly? .tlz)


KJD
20050702


Note: zlib included is modifed from released versions to trim down its
size and use included mini-c-library.  For exact differences, diff with
files from zlib version 1.2.2 archive.
