# Microsoft Developer Studio Project File - Name="Realpolitik" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Realpolitik - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Realpolitik.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Realpolitik.mak" CFG="Realpolitik - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Realpolitik - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Realpolitik - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Realpolitik - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /machine:I386 /out:"..\output\Realpolitik.exe"

!ELSEIF  "$(CFG)" == "Realpolitik - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./Common" /I "./Win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\output\Realpolitik_d.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Realpolitik - Win32 Release"
# Name "Realpolitik - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Win32\Dibapi.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Dibfile.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\EditMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\EventUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\FileFinder.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Image.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\InfoDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\ListUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\LWindows.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\MacTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Menuutil.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Offscreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\PieceIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Preferences.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Print.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Rgn.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\ScrollableWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\StatusDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Strings.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\TextFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Util.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\WindowUtil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Win32\Dibapi.h
# End Source File
# Begin Source File

SOURCE=.\Win32\EditMenu.h
# End Source File
# Begin Source File

SOURCE=.\Win32\EventUtil.h
# End Source File
# Begin Source File

SOURCE=.\Win32\FileFinder.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Image.h
# End Source File
# Begin Source File

SOURCE=.\Win32\InfoDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Listutil.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Lwindows.h
# End Source File
# Begin Source File

SOURCE=.\Win32\MacTypes.h
# End Source File
# Begin Source File

SOURCE=.\Win32\MenuUtil.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Offscreen.h
# End Source File
# Begin Source File

SOURCE=.\Win32\OldGame.h
# End Source File
# Begin Source File

SOURCE=.\Win32\PieceIcon.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Preferences.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Print.h
# End Source File
# Begin Source File

SOURCE=.\Win32\resource.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Rgn.h
# End Source File
# Begin Source File

SOURCE=.\Win32\ScrollableWindow.h
# End Source File
# Begin Source File

SOURCE=.\Win32\StatusDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Strings.h
# End Source File
# Begin Source File

SOURCE=.\Win32\TextFile.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Util.h
# End Source File
# Begin Source File

SOURCE=.\Win32\WindowUtil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Win32\Realpolitik.ico
# End Source File
# Begin Source File

SOURCE=.\Win32\Realpolitik.rc
# End Source File
# Begin Source File

SOURCE=.\Win32\RPDoc.ico
# End Source File
# End Group
# End Target
# End Project
