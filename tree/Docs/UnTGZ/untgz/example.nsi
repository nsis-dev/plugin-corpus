; example.nsi
; Demonstrates (and mini test of) usage of untgz NSIS plugin
; KJD <jeremyd@computer.org>

; The name of the installer
Name "untgz Example"

; The file to write
OutFile "untgz_example.exe"

; Let user see messages in detail window
ShowInstDetails show

; The default installation directory
InstallDir $TEMP\Example

; The text to prompt the user to enter a directory
DirText "This will install the untgz example on your computer.  Please select a directory"

; The text to prompt the user to enter a directory
ComponentText "This will install untgz examples on your computer. Select which test to run."

; let user choose examples to run
InstType "All Examples"
InstType "extract all files"
InstType "extract all files alternate"
InstType "extract specific files"
InstType "extract a single file"
InstType "extract a corrupt or missing file"
InstType "extract a zero byte file"
InstType "extract all files from uncompressed tarball"
InstType "extract all files from lzma compressed tarball"
InstType "extract all files from bzip2 compressed tarball"

; Required, the sample tarball used for testing
Section ""
  SectionIn 1 2 3 4 5 6 7 RO
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File example.tgz
  File examplecorrupt.tgz
  File exampleempty.tgz
  ; the alternate compressed files
  File example.tar
  File example.tlz
  File example.tar.lzma
  ;File example.tbz
SectionEnd

Section "Test extract"
  SectionIn 1 2
  ; untgz::extract [-j] [-d basedir] tarball.tgz
  untgz::extract -j -d "$INSTDIR/junkedpaths" "$INSTDIR/example.tgz"
  untgz::extract -d "$INSTDIR/withpaths" "$INSTDIR/example.tgz"
SectionEnd

Section "Test extractV, same as extract"
  SectionIn 1 3
  ; untgz::extractV [-j] [-d basedir] tarball.tgz [-i {iList}] [-x {xList}] --
  untgz::extractV -j -d "$INSTDIR/junkedpathsAlt" "$INSTDIR/example.tgz" --
  untgz::extractV -d "$INSTDIR/withpathsAlt" "$INSTDIR/example.tgz" --
SectionEnd

Section "Test extractV"
  SectionIn 1 4
  ; untgz::extractV [-j] [-d basedir] tarball.tgz [-i {iList}] [-x {xList}] --
  untgz::extractV -j -d "$INSTDIR/junkedpathsI" "$INSTDIR/example.tgz" -i "another doc.txt" --
  untgz::extractV -j -d "$INSTDIR/junkedpathsX" "$INSTDIR/example.tgz" -x "another doc.txt" --
  untgz::extractV -d "$INSTDIR/withpathsI" "$INSTDIR/example.tgz" -i "another doc.txt" --
  untgz::extractV -d "$INSTDIR/withpathsX" "$INSTDIR/example.tgz" -x "another doc.txt" --
  untgz::extractV -d "$INSTDIR/withpathsX curious" "$INSTDIR/example.tgz" -x "subdir/*" --
SectionEnd

Section "Test extractFile"
  SectionIn 1 5
  ; untgz::extractFile [-d basedir] tarball.tgz file
  untgz::extractFile -d "$INSTDIR/singlefile" "$INSTDIR/example.tgz" "another doc.txt"
SectionEnd

Section "Test extract corrupt File"
  SectionIn 1 6
  ; attempt to extract from a corrupt [manually truncated] tarball
  untgz::extractFile -d "$INSTDIR/corrupt" "$INSTDIR/examplecorrupt.tgz" "fake doc.txt"

  ; attempt to extract from a nonexistant tarball
  untgz::extractFile -d "$INSTDIR/corrupt" "$INSTDIR/exampleNotExist.tgz" "my doc.txt"

  ; attempt a proper extraction, should succeed.
  untgz::extractFile -d "$INSTDIR/corruptSuccess" "$INSTDIR/example.tgz" "another doc.txt"
SectionEnd

Section "Test extract 0 byte file"
  SectionIn 1 7
  untgz::extract -d "$INSTDIR/zerofile" "$INSTDIR/exampleempty.tgz"
SectionEnd

Section "Test uncompressed tarball"
  SectionIn 1 8
  untgz::extract -d "$INSTDIR/plainTAR" "$INSTDIR/example.tar"
  ;untgz::extract -d "$INSTDIR/plainTAR" -znone "$INSTDIR/example.tar"
SectionEnd

Section "Test lzma compressed tarball"
  SectionIn 1 9
  untgz::extract -d "$INSTDIR/tlz_file" -zlzma "$INSTDIR/example.tlz"
  untgz::extract -d "$INSTDIR/tar_lzma_file" -zlzma "$INSTDIR/example.tar.lzma"
SectionEnd

Section "Test bzip2 compressed tarball"
  SectionIn 1 10
  ; no example yet
  ;untgz::extract -d "$INSTDIR/tbz_file" -zbz2 "$INSTDIR/example.tbz"
  ; this one should fail
  untgz::extract -d "$INSTDIR/tbz_file" -zbz2 "$INSTDIR/example.tgz"
SectionEnd

; eof
