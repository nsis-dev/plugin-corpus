@ECHO OFF
Set MSSDK=D:\Program Files\Microsoft Platform SDK
Set VCDIR=D:\Program Files\Microsoft Visual C++ Toolkit 2003

::###################################::
Set PATH=%VCDIR%\bin;%MSSDK%\bin;%PATH%
Set INCLUDE=%VCDIR%\include;%MSSDK%\include;%INCLUDE%
Set LIB=%VCDIR%\lib;%MSSDK%\lib;%LIB%

::msvcrt.lib libcmt.lib
::/def:windll32.def
rc /r /Fo"nsUnzip.res" "Resources\nsUnzip.rc"
cl /D "NDEBUG" /D "WINDOWS" /D "WIN32" /D "WINDLL" /D "DLL" /D "USE_EF_UT_TIME" /O1 /LD "nsUnzip.c" "nsUnzip.res" "..\windll\windll.c" "..\win32\crc_i386.c" "..\win32\nt.c" "..\win32\win32.c" "..\api.c" "..\crc32.c" "..\crctab.c" "..\crypt.c" "..\explode.c" "..\extract.c" "..\fileio.c" "..\globals.c" "..\inflate.c" "..\list.c" "..\match.c" "..\process.c" "..\unreduce.c" "..\unshrink.c" "..\zipinfo.c" /link kernel32.lib user32.lib advapi32.lib /OPT:NOWIN98 /OUT:"..\..\nsUnzip.dll"
if exist *.obj del *.obj
if exist *.lib del *.lib
if exist *.exp del *.exp
if exist *.res del *.res
PAUSE