# Microsoft Developer Studio Project File - Name="SoftDrv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SoftDrv - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SoftDrv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoftDrv.mak" CFG="SoftDrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SoftDrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SoftDrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Duke4_UT400/SoftDrv", MUJAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SoftDrv - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Lib"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MD /W4 /WX /vd0 /GX /Zi /Ox /Ot /Oa /Og /Oi /Ob2 /I "..\..\DirectX8\Inc" /I "..\..\Core\Inc" /I "..\..\Engine\Inc" /I "..\Inc" /I "..\..\Engine\Src" /D "_WINDOWS" /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /Yu"SoftDrvPrivate.h" /FD /O2x /Zm256 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /libpath:"..\..\DirectX8\Lib" ..\..\Engine\Lib\Engine.lib /nologo /base:"0x10D00000" /subsystem:windows /dll /pdb:"..\..\system\SoftDrv.pdb" /debug /machine:I386 /out:"..\..\System\SoftDrv.dll"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "SoftDrv - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Lib"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MDd /W4 /WX /vd0 /GX /Zi /Od /I "..\..\DirectX8\Inc" /I "..\..\Core\Inc" /I "..\..\Engine\Inc" /I "..\Inc" /I "..\..\Engine\Src" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_DEBUG" /D "WIN32" /Yu"SoftDrvPrivate.h" /FD /Zm256 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /libpath:"..\..\DirectX8\Lib" ..\..\Engine\Lib\Engine.lib /nologo /base:"0x10D00000" /subsystem:windows /dll /pdb:"..\..\system\SoftDrv.pdb" /debug /machine:I386 /out:"..\..\System\SoftDrv.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "SoftDrv - Win32 Release"
# Name "SoftDrv - Win32 Debug"
# Begin Source File

SOURCE=.\DrawPoly.cpp
# End Source File
# Begin Source File

SOURCE=.\DrawSurf.cpp
# End Source File
# Begin Source File

SOURCE=.\DrawTile.cpp
# End Source File
# Begin Source File

SOURCE=.\Hit.cpp
# End Source File
# Begin Source File

SOURCE=.\Line.h
# End Source File
# Begin Source File

SOURCE=.\Line1.h
# End Source File
# Begin Source File

SOURCE=.\Line2D.cpp
# End Source File
# Begin Source File

SOURCE=.\SoftDrv.cpp
# ADD CPP /Yc"SoftDrvPrivate.h"
# End Source File
# Begin Source File

SOURCE=.\SoftDrvPrivate.h
# End Source File
# End Target
# End Project
