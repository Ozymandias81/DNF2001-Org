# Microsoft Developer Studio Project File - Name="xcore_winapp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=xcore_winapp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xcore_winapp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xcore_winapp.mak" CFG="xcore_winapp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xcore_winapp - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "xcore_winapp - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "xcore_winapp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f xcore_winapp.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "xcore_winapp.exe"
# PROP BASE Bsc_Name "xcore_winapp.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj"
# PROP Intermediate_Dir "obj"
# PROP Cmd_Line "nmake "debug=0" /f "makefile""
# PROP Rebuild_Opt "/a"
# PROP Target_File "xcore_winapp.lib"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "xcore_winapp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f xcore_winapp.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "xcore_winapp.exe"
# PROP BASE Bsc_Name "xcore_winapp.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "obj"
# PROP Intermediate_Dir "obj"
# PROP Cmd_Line "nmake "debug=1" /f "makefile""
# PROP Rebuild_Opt "/a"
# PROP Target_File "xcore_winapp.lib"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "xcore_winapp - Win32 Release"
# Name "xcore_winapp - Win32 Debug"

!IF  "$(CFG)" == "xcore_winapp - Win32 Release"

!ELSEIF  "$(CFG)" == "xcore_winapp - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\winapp.cpp
# End Source File
# Begin Source File

SOURCE=.\xwnd.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\winapp.h
# End Source File
# Begin Source File

SOURCE=.\xwnd.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
