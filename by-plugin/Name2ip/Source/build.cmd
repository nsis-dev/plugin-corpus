@ECHO OFF
Set MSSDK=D:\Program Files\Microsoft Platform SDK
Set VCDIR=D:\Program Files\Microsoft Visual C++ Toolkit 2003

Set PATH=%VCDIR%\bin;%PATH%
Set INCLUDE=%MSSDK%\include;%VCDIR%\include;%INCLUDE%
Set LIB=%MSSDK%\lib;%VCDIR%\lib;%LIB%

cl /O1 name2ip.c /LD /link kernel32.lib user32.lib ws2_32.lib /OPT:NOWIN98 /NODEFAULTLIB /ENTRY:DllMain

del name2ip.obj
del name2ip.lib
del name2ip.exp
@PAUSE