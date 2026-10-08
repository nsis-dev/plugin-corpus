@echo off
setlocal
set bld_ok=0

if "%MSVCDir%"=="" echo.Bad build env!&goto:EOF

set CL=/nologo /Ogiy /Os /O1 /Gz /GF /Gi- /Gm- /Zl /WX /W3
set LINK=/NODEFAULTLIB /INCREMENTAL:NO /OPT:REF /OPT:ICF,9 /RELEASE
cl /LD /GD BgWorker.cpp /link /OUT:BgWorker.dll kernel32.lib user32.lib &&set bld_ok=1


if %bld_ok%==1 "%ProgramFiles%\DevTools\NSIS\NSISu\makensis.exe" BgWorker.nsi&&"%tmp%\BgWorker"

