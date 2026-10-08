# Microsoft Developer Studio Generated NMAKE File, Based on dumpstate.dsp
!IF "$(CFG)" == ""
CFG=dumpstate - Win32 Release
!MESSAGE No configuration specified. Defaulting to $(CFG)
!ENDIF 

!IF "$(CFG)" != "dumpstate - Win32 Release" && "$(CFG)" != "dumpstate - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dumpstate.mak" CFG="dumpstate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dumpstate - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dumpstate - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "dumpstate - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
TARGETDIR=c:\progra~1\nsisv2\plugins
#TARGETDIR=..\..\nsis\plugins

ALL : "$(TARGETDIR)\dumpstate.dll"


CLEAN :
	-erase "$(INTDIR)\dumpstate.obj"
	-erase "$(INTDIR)\util.obj"
	-erase "$(INTDIR)\vc60.idb"
	-erase "$(OUTDIR)\dumpstate.exp"
	-erase "$(OUTDIR)\dumpstate.lib"
	-erase "$(TARGETDIR)\dumpstate.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "dumpstate_EXPORTS" /Fp"$(INTDIR)\dumpstate.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dumpstate.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup" /dll /incremental:no /pdb:"$(OUTDIR)\dumpstate.pdb" /machine:I386 /nodefaultlib /out:"$(TARGETDIR)/dumpstate.dll" /implib:"$(OUTDIR)\dumpstate.lib" /opt:nowin98 
LINK32_OBJS= \
	"$(INTDIR)\dumpstate.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\dumpstate.res"

"$(TARGETDIR)\dumpstate.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dumpstate - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\dumpstate.dll"


CLEAN :
	-@erase "$(INTDIR)\dumpstate.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dumpstate.dll"
	-@erase "$(OUTDIR)\dumpstate.exp"
	-@erase "$(OUTDIR)\dumpstate.ilk"
	-@erase "$(OUTDIR)\dumpstate.lib"
	-@erase "$(OUTDIR)\dumpstate.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "dumpstate_EXPORTS" /Fp"$(INTDIR)\dumpstate.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dumpstate.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\dumpstate.pdb" /debug /machine:I386 /out:"$(OUTDIR)\dumpstate.dll" /implib:"$(OUTDIR)\dumpstate.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dumpstate.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\dumpstate.res"

"$(OUTDIR)\dumpstate.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dumpstate.dep")
!INCLUDE "dumpstate.dep"
!ELSE 
!MESSAGE Warning: cannot find "dumpstate.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dumpstate - Win32 Release" || "$(CFG)" == "dumpstate - Win32 Debug"
SOURCE=.\dumpstate.c .\dumpstate.rc .\util.c

"$(INTDIR)\dumpstate.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

#################### resources
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dumpstate.res" /d "NDEBUG"
	
"$(INTDIR)\dumpstate.res" : .\dumpstate.rc "$(INTDIR)"
	$(RSC) $(RSC_PROJ) .\dumpstate.rc




