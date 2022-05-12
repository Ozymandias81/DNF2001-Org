# Microsoft Developer Studio Project File - Name="WinDrv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=WinDrv - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinDrv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinDrv.mak" CFG="WinDrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinDrv - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "WinDrv - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Unreal/WinDrv", FAAAAAAA"
# PROP Scc_LocalPath ".."
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WinDrv - Win32 Release"

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
# ADD CPP /nologo /Zp4 /MD /W4 /WX /vd0 /GX /Zi /O2 /Ob2 /I "." /I "..\Inc" /I "..\..\Core\Inc" /I "..\..\Engine\Inc" /I "..\..\Window\Inc" /D WINDRV_API=__declspec(dllexport) /D "_WINDOWS" /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /FR /Yu"WinDrv.h" /FD /Zm256 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ..\..\Window\Lib\Window.lib ..\..\Core\Lib\Core.lib ..\..\Engine\Lib\Engine.lib ..\Lib\ddraw.lib gdi32.lib kernel32.lib user32.lib winmm.lib dinput8.lib dxguid.lib ole32.lib /nologo /base:"0x11100000" /subsystem:windows /dll /incremental:yes /debug /machine:I386 /out:"..\..\System\WinDrv.dll"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "WinDrv - Win32 Debug"

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
# ADD CPP /nologo /Zp4 /MDd /W4 /WX /Gm /vd0 /GX /Zi /Od /I "." /I "..\Inc" /I "..\..\Core\Inc" /I "..\..\Engine\Inc" /I "..\..\Window\Inc" /D "_WINDOWS" /D WINDRV_API=__declspec(dllexport) /D "UNICODE" /D "_UNICODE" /D "_DEBUG" /D "WIN32" /Yu"WinDrv.h" /FD /Zm256 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\..\Window\Lib\Window.lib ..\..\Core\Lib\Core.lib ..\..\Engine\Lib\Engine.lib ..\Lib\ddraw.lib gdi32.lib kernel32.lib user32.lib winmm.lib dinput8.lib dxguid.lib ole32.lib /nologo /base:"0x11100000" /subsystem:windows /dll /debug /machine:I386 /out:"..\..\System\WinDrv.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "WinDrv - Win32 Release"
# Name "WinDrv - Win32 Debug"
# Begin Group "Src"

# PROP Default_Filter "*.cpp;*.h"
# Begin Source File

SOURCE=.\WinClient.cpp
# End Source File
# Begin Source File

SOURCE=.\WinDrv.cpp
# ADD CPP /Yc"WinDrv.h"
# End Source File
# Begin Source File

SOURCE=..\inc\WinDrv.h
# End Source File
# Begin Source File

SOURCE=.\Res\WinDrvRes.h
# End Source File
# Begin Source File

SOURCE=.\WinViewport.cpp
# End Source File
# End Group
# Begin Group "Res"

# PROP Default_Filter "*.rc"
# Begin Source File

SOURCE=.\Res\ADDACTOR.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\BRUSHFR.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\BRUSHMOV.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\BRUSHROT.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\BRUSHSCA.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\BRUSHSHR.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\BrushSna.cur
# End Source File
# Begin Source File

SOURCE=.\Res\BRUSHSTR.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\BrushWrp.cur
# End Source File
# Begin Source File

SOURCE=.\Res\CAMZOOM.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\MOVEACT.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\SELACTOR.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\SELECTPO.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\terrafor.cur
# End Source File
# Begin Source File

SOURCE=.\Res\TEXGRAB.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\TEXPAN.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\TEXROT.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\TEXSCALE.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\TEXSET.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\Unreal.ico
# End Source File
# Begin Source File

SOURCE=.\Res\WinDrvRes.rc
# End Source File
# End Group
# Begin Group "Int"

# PROP Default_Filter "*.int"
# Begin Source File

SOURCE=..\..\System\WinDrv.int
# End Source File
# End Group
# Begin Source File

SOURCE=.\Res\cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\Res\cur00002.cur
# End Source File
# Begin Source File

SOURCE=.\Res\cur00003.cur
# End Source File
# Begin Source File

SOURCE=.\Res\idcursor.cur
# End Source File
# End Target
# End Project
