# Microsoft Developer Studio Project File - Name="nsUnzip" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=nsUnzip - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nsUnzip.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nsUnzip.mak" CFG="nsUnzip - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nsUnzip - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "nsUnzip - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "nsUnzip_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "DEBUG" /D "WINDOWS" /D "WINDLL" /D "DLL" /D "USE_EF_UT_TIME" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib advapi32.lib /nologo /dll /debug /machine:I386 /out:"../../nsUnzip.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "nsUnzip_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "WINDOWS" /D "WINDLL" /D "DLL" /D "USE_EF_UT_TIME" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib advapi32.lib /nologo /dll /machine:I386 /out:"../../nsUnzip.dll" /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "nsUnzip - Win32 Debug"
# Name "nsUnzip - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\api.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\crc32.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\crc_i386.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\crctab.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\crypt.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\explode.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\extract.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\fileio.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\globals.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\inflate.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\list.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\match.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nsUnzip.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\nt.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\process.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\unreduce.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\unshrink.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\win32.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\windll\windll.c
# End Source File
# Begin Source File

SOURCE=..\zipinfo.c

!IF  "$(CFG)" == "nsUnzip - Win32 Debug"

!ELSEIF  "$(CFG)" == "nsUnzip - Win32 Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
