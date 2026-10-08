@echo off
setlocal
set CL=&set LINK=/nologo
set _CL=..\InetBgDL.cpp /W3 /WX /O1sgyb1 /Gz /GF /Gy /GL /LD /Zl /link /OPT:nowin98 /OPT:REF /OPT:ICF=99 /DLL /NODEFAULTLIB kernel32.lib user32.lib wininet.lib
set _CLEAN=del InetBgDL.lib^&del InetBgDL.exp^&del InetBgDL.obj
pushd Ansi
cl %_CL%&&(
	pushd ..\Unicode
	cl /DUNICODE %_CL%
	%_CLEAN%
	popd
	)
%_CLEAN%
popd
 