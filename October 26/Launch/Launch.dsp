# Microsoft Developer Studio Project File - Name="Launch" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Launch - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Launch.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Launch.mak" CFG="Launch - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Launch - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Launch - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Duke4_UT400/Launch", RQJAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Launch - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /Zp4 /MD /W4 /WX /vd0 /GX /Zi /O2 /Ob2 /I "..\..\Core\Inc" /I "..\..\Engine\Inc" /I "..\..\Window\Inc" /I "." /D "_WINDOWS" /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /FR /Yu"LaunchPrivate.h" /FD /Zm256 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\..\Engine\Lib\Engine.lib user32.lib kernel32.lib gdi32.lib advapi32.lib shell32.lib /nologo /base:"0x10900000" /subsystem:windows /pdb:"..\..\system\DukeForever.pdb" /debug /machine:I386 /out:"..\..\System\DukeForever.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Launch - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MDd /W4 /WX /Gm /vd0 /GX /Zi /Od /I "..\..\Core\Inc" /I "..\..\Engine\Inc" /I "..\..\Window\Inc" /I "." /D "_WINDOWS" /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /Yu"LaunchPrivate.h" /FD /Zm256 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\..\Engine\Lib\Engine.lib user32.lib kernel32.lib gdi32.lib advapi32.lib shell32.lib /nologo /base:"0x10900000" /subsystem:windows /pdb:"..\..\system\DukeForever.pdb" /debug /machine:I386 /out:"..\..\System\DukeForever.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Launch - Win32 Release"
# Name "Launch - Win32 Debug"
# Begin Group "Src"

# PROP Default_Filter "*.cpp;*.h"
# Begin Source File

SOURCE=.\Launch.cpp
# ADD CPP /Yc"LaunchPrivate.h"
# End Source File
# Begin Source File

SOURCE=.\LaunchPrivate.h
# End Source File
# Begin Source File

SOURCE=.\Res\LaunchRes.h
# End Source File
# End Group
# Begin Group "Res"

# PROP Default_Filter "*.rc"
# Begin Source File

SOURCE=.\Res\LaunchRes.rc
# End Source File
# Begin Source File

SOURCE=.\Res\Unreal.ico
# End Source File
# End Group
# Begin Group "Fortify"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Fortify.c

!IF  "$(CFG)" == "Launch - Win32 Release"

# ADD CPP /W3
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "Launch - Win32 Debug"

# ADD CPP /W1 /D "FORTIFY"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Fortify.doc
# End Source File
# Begin Source File

SOURCE=.\Fortify.h
# End Source File
# Begin Source File

SOURCE=.\Ufortify.h
# End Source File
# Begin Source File

SOURCE=.\Ufortify.hpp
# End Source File
# Begin Source File

SOURCE=.\Zfortify.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Zfortify.hpp
# End Source File
# End Group
# Begin Group "Bugslayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BugslayerUtil.h
# End Source File
# Begin Source File

SOURCE=.\CrashHandler.h
# End Source File
# Begin Source File

SOURCE=.\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\DiagAssert.h
# End Source File
# Begin Source File

SOURCE=.\MemDumperValidator.h
# End Source File
# Begin Source File

SOURCE=.\MemStress.h
# End Source File
# Begin Source File

SOURCE=.\MSJDBG.h
# End Source File
# Begin Source File

SOURCE=.\PSAPI.H
# End Source File
# Begin Source File

SOURCE=.\SymbolEngine.h
# End Source File
# Begin Source File

SOURCE=.\WarningsOff.h
# End Source File
# Begin Source File

SOURCE=.\WarningsOn.h
# End Source File
# Begin Source File

SOURCE=.\BugslayerUtil.lib
# End Source File
# End Group
# Begin Source File

SOURCE=.\Res\Logo.bmp
# End Source File
# End Target
# End Project
