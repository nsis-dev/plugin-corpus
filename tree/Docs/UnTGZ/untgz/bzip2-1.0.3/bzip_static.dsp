# Microsoft Developer Studio Project File - Name="bzip_static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=bzip_static - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bzip_static.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bzip_static.mak" CFG="bzip_static - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bzip_static - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "bzip_static - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "bzip_static - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f bzip_static.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "bzip_static.exe"
# PROP BASE Bsc_Name "bzip_static.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "nmake -f makefile.msc"
# PROP Rebuild_Opt "/a"
# PROP Target_File "bzip_static.exe"
# PROP Bsc_Name "bzip_static.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "bzip_static - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f bzip_static.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "bzip_static.exe"
# PROP BASE Bsc_Name "bzip_static.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "nmake -f makefile.msc"
# PROP Rebuild_Opt "/a"
# PROP Target_File "libbz2.lib"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "bzip_static - Win32 Release"
# Name "bzip_static - Win32 Debug"

!IF  "$(CFG)" == "bzip_static - Win32 Release"

!ELSEIF  "$(CFG)" == "bzip_static - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\blocksort.c
# End Source File
# Begin Source File

SOURCE=.\bzip2.c
# End Source File
# Begin Source File

SOURCE=.\bzip2recover.c
# End Source File
# Begin Source File

SOURCE=.\bzlib.c
# End Source File
# Begin Source File

SOURCE=.\compress.c
# End Source File
# Begin Source File

SOURCE=.\crctable.c
# End Source File
# Begin Source File

SOURCE=.\decompress.c
# End Source File
# Begin Source File

SOURCE=.\dlltest.c
# End Source File
# Begin Source File

SOURCE=.\huffman.c
# End Source File
# Begin Source File

SOURCE=.\mk251.c
# End Source File
# Begin Source File

SOURCE=.\randtable.c
# End Source File
# Begin Source File

SOURCE=.\spewG.c
# End Source File
# Begin Source File

SOURCE=.\unzcrash.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bzlib.h
# End Source File
# Begin Source File

SOURCE=.\bzlib_private.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
