;GT2 Setup Script
;--------------------------------

; ensure one version is always present
!ifndef FEATURE_UNICODE
!define FEATURE_ANSI
!endif

; build information
!define VER_MAJOR 0
!define VER_MINOR 37
!define VER_REVISION 1
!define VER_BUILD 1

; Unicode? yes|no
!ifdef FEATURE_UNICODE
!define VER_FILE_U  "u"
!else
!define VER_FILE_U  ""
!endif

; Debug? yes|no
!ifdef FEATURE_DEBUG
!define VER_FILE_D  "d"
!else
!define VER_FILE_D  ""
!endif

!define VER_FILE    "-${VER_MAJOR}.${VER_MINOR}${VER_FILE_U}${VER_FILE_D}"
!define VER_DISPLAY "${VER_MAJOR}.${VER_MINOR}"
!define PROG_NAME   "GT2 ${VER_DISPLAY}"
!define REG_KEY     "GT2"

; Use version from VS 2003
!define PROG_PATH            "..\target\7"
!define ADD_REDISTRIBUTABLE  0

;--------------------------------
;Configuration

OutFile gt2setup${VER_FILE}.exe
SetCompressor lzma

; available installation type (Custom is contained anyway)
InstType "Full"
InstType "Minimum"

InstallDir       "$PROGRAMFILES\${REG_KEY}"
InstallDirRegKey HKLM "Software\${REG_KEY}" ""

;--------------------------------

;Header Files

!include "MUI.nsh"
!include "Sections.nsh"

!ifdef FEATURE_UNICODE
!include "gt2.nsh"
!endif

;--------------------------------
;Configuration

;Names
Name "GT2"
Caption "${PROG_NAME} Setup"

;Interface Settings
!define MUI_ABORTWARNING

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP       "gt2header_left.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "gt2setup.bmp"

!define MUI_COMPONENTSPAGE_SMALLDESC

;Pages
!define MUI_WELCOMEPAGE_TITLE "Welcome to the ${PROG_NAME} Setup Wizard"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${PROG_NAME}, a small file format detection utility.\r\n\r\n$_CLICK"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "gt2license.txt"
Page custom PageReinstall PageLeaveReinstall
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "https://github.com/phax/gt"
!define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/phax/gt"

!define MUI_FINISHPAGE_NOREBOOTSUPPORT

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Reserve Files

  ;These files should be inserted before other files in the data block

  ReserveFile "gt2.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
;Installer Sections

!ifdef FEATURE_ANSI
Section "GT2 Core Files [ANSI] (required)" SecCoreA
  SetDetailsPrint textonly
  DetailPrint "Installing GT2 Core Files [ANSI]..."
  SetDetailsPrint listonly

  ; contained in InstallType 1 and 2; ReadOnly
  SectionIn 1 2 RO
  SetOutPath $INSTDIR

  ; see MS KB 326922: copy to local
  File Runtimes\msvcr71.dll
  File Runtimes\msvcp71.dll
  File Commons\dbghelp.dll

  File ${PROG_PATH}\bin\gt2.exe
  File ${PROG_PATH}\bin\gt2.exe.manifest
  File ${PROG_PATH}\bin\gtbasic.dll
  File ${PROG_PATH}\bin\gtbasic.dll.manifest
  File ${PROG_PATH}\bin\gtdata.dll
  File ${PROG_PATH}\bin\gtdata.dll.manifest
  File ${PROG_PATH}\bin\gtarc.dll
  File ${PROG_PATH}\bin\gtarc.dll.manifest
  File ${PROG_PATH}\bin\gtexe.dll
  File ${PROG_PATH}\bin\gtexe.dll.manifest
  File ${PROG_PATH}\bin\gtgen.dll
  File ${PROG_PATH}\bin\gtgen.dll.manifest
  File ..\docs\gt2.htm
  File ..\docs\gt2.his
  File /oname=gt2.cfg gt2.default.cfg
SectionEnd
!endif

!ifdef FEATURE_UNICODE
Section "GT2 Core Files [Unicode] (required)" SecCoreW

  SetDetailsPrint textonly
  DetailPrint "Installing GT2 Core Files [Unicode]..."
  SetDetailsPrint listonly

  ; contained in InstallType 1 and 2; ReadOnly
  SectionIn 1 2 RO

  ; see MS KB 326922: copy to local
  File Runtimes\msvcr71.dll
  File Runtimes\msvcp71.dll
  File Commons\dbghelp.dll

  File ${PROG_PATH}\binu\gt2u.exe
  File ${PROG_PATH}\binu\gt2u.exe.manifest
  File ${PROG_PATH}\binu\gtbasicu.dll
  File ${PROG_PATH}\binu\gtbasicu.dll.manifest
  File ${PROG_PATH}\binu\gtdatau.dll
  File ${PROG_PATH}\binu\gtdatau.dll.manifest
  File ${PROG_PATH}\binu\gtarcu.dll
  File ${PROG_PATH}\binu\gtarcu.dll.manifest
  File ${PROG_PATH}\binu\gtexeu.dll
  File ${PROG_PATH}\binu\gtexeu.dll.manifest
  File ${PROG_PATH}\binu\gtgenu.dll
  File ${PROG_PATH}\binu\gtgenu.dll.manifest
  File ..\docs\gt2.htm
  File ..\docs\gt2.his
  File /oname=gt2.cfg gt2.default.cfg
SectionEnd
!endif

Section "Related tools" SecTools
  SetDetailsPrint textonly
  DetailPrint "Installing GT2 related tools..."
  SetDetailsPrint listonly

  ; contained in InstallType 1
  SectionIn 1

!ifdef FEATURE_ANSI
  File ${PROG_PATH}\bin\geew.exe
  File ${PROG_PATH}\bin\geew.exe.manifest
;  File ${PROG_PATH}\bin\gt_peinfo.exe
;  File ${PROG_PATH}\bin\gt_listdll.exe
;  File ${PROG_PATH}\bin\gt_xovr.exe
!endif
!ifdef FEATURE_UNICODE
  File ${PROG_PATH}\binu\geewu.exe
  File ${PROG_PATH}\binu\geewu.exe.manifest
;  File ${PROG_PATH}\binu\gt_peinfou.exe
;  File ${PROG_PATH}\binu\gt_listdllu.exe
;  File ${PROG_PATH}\binu\gt_xovru.exe
!endif
SectionEnd

!ifdef FEATURE_DEBUG
SubSection "Debugging support" SecDebug

!ifdef FEATURE_ANSI
Section "Debug symbols [ANSI]" SecDebugSymbolsA
  SetDetailsPrint textonly
  DetailPrint "Installing GT2 debug symbols [ANSI]..."
  SetDetailsPrint listonly

  ; contained in InstallType 1
  SectionIn 1

  File ${PROG_PATH}\bin\gt2.pdb
  File ${PROG_PATH}\bin\gtbasic.pdb
  File ${PROG_PATH}\bin\gtdata.pdb
  File ${PROG_PATH}\bin\gtarc.pdb
  File ${PROG_PATH}\bin\gtexe.pdb
  File ${PROG_PATH}\bin\gtgen.pdb
SectionEnd
!endif

!ifdef FEATURE_UNICODE
Section "Debug symbols [Unicode]" SecDebugSymbolsW
  SetDetailsPrint textonly
  DetailPrint "Installing GT2 debug symbols [Unicode]..."
  SetDetailsPrint listonly

  ; contained in InstallType 1
  SectionIn 1

  File ${PROG_PATH}\binu\gt2u.pdb
  File ${PROG_PATH}\binu\gtbasicu.pdb
  File ${PROG_PATH}\binu\gtdatau.pdb
  File ${PROG_PATH}\binu\gtarcu.pdb
  File ${PROG_PATH}\binu\gtexeu.pdb
  File ${PROG_PATH}\binu\gtgenu.pdb
SectionEnd
!endif

SubSectionEnd
!endif

Section "Register Shell Extensions" SecShellExt

  SetDetailsPrint textonly
  DetailPrint "Installing Shell Extensions..."
  SetDetailsPrint listonly

  SectionIn 1
  
  ; all files
  WriteRegStr HKCR "*\shell\${REG_KEY}" "" "Detect with GT2..."
  WriteRegStr HKCR "*\shell\${REG_KEY}\command" "" '"$INSTDIR\gt2" "%L" /wf'
  
  ; special DLL version
  WriteRegStr HKCR "dllfile\shell\${REG_KEY}" "" "Detect with GT2 [DLL]..."
  WriteRegStr HKCR "dllfile\shell\${REG_KEY}\command" "" '"$INSTDIR\gt2" "%L" /wf /peall'
  
  ; special EXE version
  WriteRegStr HKCR "exefile\shell\${REG_KEY}" "" "Detect with GT2 [EXE]..."
  WriteRegStr HKCR "exefile\shell\${REG_KEY}\command" "" '"$INSTDIR\gt2" "%L" /wf /peall'
  
  ; special OCX version
  WriteRegStr HKCR "ocxfile\shell\${REG_KEY}" "" "Detect with GT2 [OCX]..."
  WriteRegStr HKCR "ocxfile\shell\${REG_KEY}\command" "" '"$INSTDIR\gt2" "%L" /wf /peall'
  
  ; special SCR version
  WriteRegStr HKCR "scrfile\shell\${REG_KEY}" "" "Detect with GT2 [SCR]..."
  WriteRegStr HKCR "scrfile\shell\${REG_KEY}\command" "" '"$INSTDIR\gt2" "%L" /wf /peall'
SectionEnd

Section "Add to the users PATH EnvVar" SecPathUser

  SetDetailsPrint textonly
  DetailPrint "Adding to path..."
  SetDetailsPrint listonly

  SectionIn 1
  
  ; extract and execute
  File ${PROG_PATH}\bin\gt_pathmod.exe
  File ${PROG_PATH}\bin\gt_pathmod.exe.manifest
  ExecWait '"$INSTDIR\gt_pathmod.exe" -a "-p$INSTDIR" -q'
SectionEnd

Section -post
  SetDetailsPrint textonly
  DetailPrint "Creating Registry Keys..."
  SetDetailsPrint listonly

  WriteRegStr   HKLM "Software\${REG_KEY}" "" $INSTDIR
  WriteRegDword HKLM "Software\${REG_KEY}" "VersionMajor" "${VER_MAJOR}"
  WriteRegDword HKLM "Software\${REG_KEY}" "VersionMinor" "${VER_MINOR}"
  WriteRegDword HKLM "Software\${REG_KEY}" "VersionRevision" "${VER_REVISION}"
  WriteRegDword HKLM "Software\${REG_KEY}" "VersionBuild" "${VER_BUILD}"

  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "UninstallString" '"$INSTDIR\uninst-gt2${VER_FILE_U}${VER_FILE_D}.exe"'
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr       HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "DisplayName" "${PROG_NAME} uninstall"
  WriteRegStr       HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "DisplayIcon" "$INSTDIR\gt2.exe,0"
  WriteRegStr       HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "DisplayVersion" "${VER_DISPLAY}"
  WriteRegDWORD     HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "VersionMajor" "${VER_MAJOR}"
  WriteRegDWORD     HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "VersionMinor" "${VER_MINOR}.${VER_REVISION}"
  WriteRegStr       HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "URLInfoAbout" "http://philip.helger.com/gt/"
  WriteRegDWORD     HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "NoModify" "1"
  WriteRegDWORD     HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "NoRepair" "1"

  ; will only be removed if empty
  SetDetailsPrint none
  SetDetailsPrint lastused

  WriteUninstaller $INSTDIR\uninst-gt2${VER_FILE_U}${VER_FILE_D}.exe

  SetDetailsPrint both

SectionEnd

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!ifdef FEATURE_ANSI
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCoreA} "The core files required to use GT2"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTools} "Some additional (maybe helpful) commandline tools"
!endif
!ifdef FEATURE_UNICODE
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCoreW} "The core files required to use GT2 as Unicode version (Windows NT or higher only)"
!endif
!ifdef FEATURE_DEBUG
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDebug} "The debugging symbols [optional]"
!ifdef FEATURE_ANSI
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDebugSymbolsA} "The debugging symbols for the ANSI version"
!endif
!ifdef FEATURE_UNICODE
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDebugSymbolsW} "The debugging symbols for the Unicode version"
!endif
!endif
  !insertmacro MUI_DESCRIPTION_TEXT ${SecShellExt} "Explorer shell extension registry entries"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPathUser} "Add the installation path to the user PATH environment variable"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Installer Functions

Function .onInit

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "gt2.ini"

!ifdef FEATURE_UNICODE
  Call IsWindowsNT
  Pop $R0
  StrCmp $R0 "1" lbl_have_unicode

!ifndef FEATURE_ANSI
    MessageBox MB_YESNO "The Unicode version of ${PROG_NAME} is not supported on your operating system. Continue anyway?" IDYES lbl_no_abort
      Abort ; causes installer to quit.
lbl_no_abort:
!endif  ; FEATURE_ANSI

    ; Win 9X/ME: disable Unicode sections
    SectionGetFlags ${SecCoreW} $R0
    IntOp $R0 $R0 & ${SECTION_OFF}
    IntOp $R0 $R0 | ${SF_RO}
    SectionSetFlags ${SecCoreW} $R0

!ifdef FEATURE_DEBUG
    SectionGetFlags ${SecDebugSymbolsW} $R0
    IntOp $R0 $R0 & ${SECTION_OFF}
    IntOp $R0 $R0 | ${SF_RO}
    SectionSetFlags ${SecDebugSymbolsW} $R0
!endif  ; debug

lbl_have_unicode:
!endif ; FEATURE_UNICODE

  SetOverwrite on          ; enable overwrite mode
FunctionEnd

Function PageReinstall

  ReadRegStr $R0 HKLM "Software\${REG_KEY}" ""

  StrCmp $R0 "" 0 +2
    Abort

  ;Detect version
    ReadRegDWORD $R0 HKLM "Software\${REG_KEY}" "VersionMajor"
    IntCmp $R0 ${VER_MAJOR} minor_check new_version older_version
  minor_check:
    ReadRegDWORD $R0 HKLM "Software\${REG_KEY}" "VersionMinor"
    IntCmp $R0 ${VER_MINOR} revision_check new_version older_version
  revision_check:
    ReadRegDWORD $R0 HKLM "Software\${REG_KEY}" "VersionRevision"
    IntCmp $R0 ${VER_REVISION} build_check new_version older_version
  build_check:
    ReadRegDWORD $R0 HKLM "Software\${REG_KEY}" "VersionBuild"
    IntCmp $R0 ${VER_BUILD} same_version new_version older_version

  new_version:

   !insertmacro MUI_INSTALLOPTIONS_WRITE "gt2.ini" "Field 1" "Text" "An older version of GT2 is installed on your system. It's recommended that you uninstall the current version before installing. Select the operation you want to perform and click Next to continue."
   !insertmacro MUI_INSTALLOPTIONS_WRITE "gt2.ini" "Field 2" "Text" "Uninstall before installing"
   !insertmacro MUI_INSTALLOPTIONS_WRITE "gt2.ini" "Field 3" "Text" "Do not uninstall"
   !insertmacro MUI_HEADER_TEXT "Already Installed" "Choose how you want to install GT2."
   StrCpy $R0 "1"
   Goto reinst_start

  older_version:

   !insertmacro MUI_INSTALLOPTIONS_WRITE "gt2.ini" "Field 1" "Text" "A newer version of GT2 is already installed! It is not recommended that you install an older version. If you really want to install this older version, it's better to uninstall the current version first. Select the operation you want to perform and click Next to continue."
   !insertmacro MUI_INSTALLOPTIONS_WRITE "gt2.ini" "Field 2" "Text" "Uninstall before installing"
   !insertmacro MUI_INSTALLOPTIONS_WRITE "gt2.ini" "Field 3" "Text" "Do not uninstall"
   !insertmacro MUI_HEADER_TEXT "Already Installed" "Choose how you want to install GT2."
   StrCpy $R0 "1"
   Goto reinst_start

  same_version:

   !insertmacro MUI_INSTALLOPTIONS_WRITE "gt2.ini" "Field 1" "Text" "${PROG_NAME} is already installed. Select the operation you want to perform and click Next to continue."
   !insertmacro MUI_INSTALLOPTIONS_WRITE "gt2.ini" "Field 2" "Text" "Add/Reinstall components"
   !insertmacro MUI_INSTALLOPTIONS_WRITE "gt2.ini" "Field 3" "Text" "Uninstall ${PROG_NAME}"
   !insertmacro MUI_HEADER_TEXT "Already Installed" "Choose the maintenance option to perform."
   StrCpy $R0 "2"

  reinst_start:

  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "gt2.ini"

FunctionEnd

Function PageLeaveReinstall

  !insertmacro MUI_INSTALLOPTIONS_READ $R1 "gt2.ini" "Field 2" "State"

  StrCmp $R0 "1" 0 +2
    StrCmp $R1 "1" reinst_uninstall reinst_done

  StrCmp $R0 "2" 0 +3
    StrCmp $R1 "1" reinst_done reinst_uninstall

  reinst_uninstall:
  ReadRegStr $R1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}" "UninstallString"

  ;Run uninstaller
  HideWindow

    ClearErrors
    ExecWait '$R1 _?=$INSTDIR'

    IfErrors no_remove_uninstaller
    IfFileExists "$INSTDIR\makensis.exe" no_remove_uninstaller

      Delete $R1
      RMDir $INSTDIR

    no_remove_uninstaller:

  StrCmp $R0 "2" 0 +2
    Quit

  BringToFront

  reinst_done:

FunctionEnd

;--------------------------------
;Uninstaller Section

Section Uninstall

  SetDetailsPrint textonly
  DetailPrint "Uninstalling ${PROG_NAME} ..."
  SetDetailsPrint listonly

  IfFileExists $INSTDIR\gt2.exe gt2_installed
    MessageBox MB_YESNO "It does not appear that ${PROG_NAME} is installed in the directory '$INSTDIR'.$\r$\nContinue anyway (not recommended)?" IDYES gt2_installed
    Abort "Uninstall aborted by user"
  gt2_installed:

  SetDetailsPrint textonly
  DetailPrint "Deleting Registry Keys..."
  SetDetailsPrint listonly

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REG_KEY}"
  DeleteRegKey HKLM "Software\${REG_KEY}"
  DeleteRegKey HKCR "*\shell\${REG_KEY}"
  DeleteRegKey HKCR "dllfile\shell\${REG_KEY}"
  DeleteRegKey HKCR "exefile\shell\${REG_KEY}"
  DeleteRegKey HKCR "ocxfile\shell\${REG_KEY}"
  DeleteRegKey HKCR "scrfile\shell\${REG_KEY}"

  SetDetailsPrint textonly
  DetailPrint "Deleting Files..."
  SetDetailsPrint listonly

  ; delete ANSI version
!ifdef FEATURE_ANSI
  Delete $INSTDIR\gt2.exe.manifest
  Delete $INSTDIR\gt2.exe
  Delete $INSTDIR\gtbasic.dll.manifest
  Delete $INSTDIR\gtbasic.dll
  Delete $INSTDIR\gtdata.dll.manifest
  Delete $INSTDIR\gtdata.dll
  Delete $INSTDIR\gtarc.dll.manifest
  Delete $INSTDIR\gtarc.dll
  Delete $INSTDIR\gtexe.dll.manifest
  Delete $INSTDIR\gtexe.dll
  Delete $INSTDIR\gtgen.dll.manifest
  Delete $INSTDIR\gtgen.dll
!ifdef FEATURE_DEBUG
  Delete $INSTDIR\gt2.pdb
  Delete $INSTDIR\gtbasic.pdb
  Delete $INSTDIR\gtdata.pdb
  Delete $INSTDIR\gtarc.pdb
  Delete $INSTDIR\gtexe.pdb
  Delete $INSTDIR\gtgen.pdb
!endif
  ; delete common files
  Delete $INSTDIR\gt2.his
  Delete $INSTDIR\gt2.htm
  Delete $INSTDIR\gt2.cfg
!endif

  ; delete unicode version
!ifdef FEATURE_UNICODE
  Delete $INSTDIR\gt2u.exe.manifest
  Delete $INSTDIR\gt2u.exe
  Delete $INSTDIR\gtbasicu.dll.manifest
  Delete $INSTDIR\gtbasicu.dll
  Delete $INSTDIR\gtdatau.dll.manifest
  Delete $INSTDIR\gtdatau.dll
  Delete $INSTDIR\gtarcu.dll.manifest
  Delete $INSTDIR\gtarcu.dll
  Delete $INSTDIR\gtexeu.dll.manifest
  Delete $INSTDIR\gtexeu.dll
  Delete $INSTDIR\gtgenu.dll.manifest
  Delete $INSTDIR\gtgenu.dll
!ifdef FEATURE_DEBUG
  Delete $INSTDIR\gt2u.pdb
  Delete $INSTDIR\gtbasicu.pdb
  Delete $INSTDIR\gtdatau.pdb
  Delete $INSTDIR\gtarcu.pdb
  Delete $INSTDIR\gtexeu.pdb
  Delete $INSTDIR\gtgenu.pdb
!endif
  ; delete common files
  Delete $INSTDIR\gt2.his
  Delete $INSTDIR\gt2.htm
  Delete $INSTDIR\gt2.cfg
!endif

  ; delete tools
!ifdef FEATURE_ANSI
  Delete $INSTDIR\geew.exe.manifest
  Delete $INSTDIR\geew.exe
;  Delete $INSTDIR\gt_peinfo.exe
;  Delete $INSTDIR\gt_listdll.exe
;  Delete $INSTDIR\gt_xovr.exe
!endif
!ifdef FEATURE_UNICODE
  Delete $INSTDIR\geewu.exe.manifest
  Delete $INSTDIR\geewu.exe
;  Delete $INSTDIR\gt_peinfou.exe
;  Delete $INSTDIR\gt_listdllu.exe
;  Delete $INSTDIR\gt_xovru.exe
!endif

  ExecWait '"$INSTDIR\gt_pathmod.exe" -r "-p$INSTDIR" -q'
  Delete $INSTDIR\gt_pathmod.exe.manifest
  Delete $INSTDIR\gt_pathmod.exe

  Delete $INSTDIR\msvcr71.dll
  Delete $INSTDIR\msvcp71.dll
  Delete $INSTDIR\dbghelp.dll

  SetDetailsPrint both

SectionEnd
