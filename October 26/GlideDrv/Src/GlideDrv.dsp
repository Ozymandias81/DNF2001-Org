# Microsoft Developer Studio Project File - Name="GlideDrv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GlideDrv - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GlideDrv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GlideDrv.mak" CFG="GlideDrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GlideDrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GlideDrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Unreal/GlideDrv/Src", KNBAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GlideDrv - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /Zp4 /MD /W4 /WX /vd0 /GX /Zi /O2 /Ob2 /I "..\..\xcore" /I "..\..\cannibal" /I "..\..\Core\Inc" /I "..\..\Engine\Inc" /I "." /I "..\..\Engine\Src" /D "_WINDOWS" /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /FR /YX"Engine.h" /FD /Zm256 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 glide2x.lib ..\..\Core\Lib\Core.lib ..\..\Engine\Lib\Engine.lib kernel32.lib user32.lib /nologo /base:"0x10700000" /subsystem:windows /dll /pdb:"..\..\system\GlideDrv.pdb" /debug /machine:I386 /out:"..\..\System\GlideDrv.dll"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "GlideDrv - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /Zp4 /MDd /W4 /WX /Gm /vd0 /GX /Zi /Od /I "..\..\xcore" /I "..\..\cannibal" /I "..\..\Core\Inc" /I "..\..\Engine\Inc" /I "." /I "..\..\Engine\Src" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_DEBUG" /D "WIN32" /YX"Engine.h" /FD /Zm256 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glide2x.lib ..\..\Core\Lib\Core.lib ..\..\Engine\Lib\Engine.lib kernel32.lib user32.lib /nologo /base:"0x10700000" /subsystem:windows /dll /pdb:"..\..\system\GlideDrv.pdb" /debug /machine:I386 /out:"..\..\System\GlideDrv.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "GlideDrv - Win32 Release"
# Name "GlideDrv - Win32 Debug"
# Begin Group "Src"

# PROP Default_Filter "*.cpp *.h"
# Begin Source File

SOURCE=.\UnGlide.cpp
# End Source File
# End Group
# End Target
# End Project
