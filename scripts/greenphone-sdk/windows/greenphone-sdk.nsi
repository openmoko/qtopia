; greenphone-sdk.nsi
;
; This script installs greenphone sdk on windows.
!include "Sections.nsh"
!include "LogicLib.nsh"
!include "TextFunc.nsh"

SetCompress off

;--------------------------------

Name "Greenphone SDK Version: !QPE_VERSION! Build: !CHANGENO!"
Caption "Greenphone SDK"
Icon "install.ico"
OutFile "autorun.exe"

SetDateSave on
SetDatablockOptimize on
CRCCheck on
;SilentInstall silent
XPStyle on

InstallDir "$PROGRAMFILES\Trolltech\Greenphone"
InstallDirRegKey HKLM "Software\Trolltech\Greenphone" ""

CheckBitmap "${NSISDIR}\Contrib\Graphics\Checks\classic-cross.bmp"

;--------------------------------

LicenseData "license.txt"

Page license
;Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

ShowInstDetails nevershow

;--------------------------------

Section "Greenphone SDK"
  SetOutPath $INSTDIR
  DetailPrint "Writing Reg entries..."
  ; write reg info
  WriteRegStr HKLM SOFTWARE\Trolltech\Greenphone "Install_Dir" "$INSTDIR"
  ; write uninstall strings
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Greenphone" "DisplayName" "Greenphone (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Greenphone" "UninstallString" '"$INSTDIR\bt-uninst.exe"'  
  WriteUninstaller "bt-uninst.exe" 
  CreateDirectory "$SMPROGRAMS\Trolltech"
  CreateDirectory "$SMPROGRAMS\Trolltech\Greenphone"
  CreateShortCut "$SMPROGRAMS\Trolltech\Greenphone\Uninstall Greenphone SDK.lnk" "$INSTDIR\bt-uninst.exe"
  CreateShortCut "$SMPROGRAMS\Trolltech\Greenphone\Greenphone SDK.lnk" "$INSTDIR\greenphone.vmx" 
  CreateShortCut "$SMPROGRAMS\Trolltech\Greenphone\Getting Started.lnk" "$INSTDIR\greenphone_gettingstarted.pdf"
  CreateShortCut "$SMPROGRAMS\Trolltech\Greenphone\User Guide.lnk" "$INSTDIR\greenphone_userguide.pdf" 
  CreateShortCut "$SMPROGRAMS\Trolltech\Greenphone\Dev Quick Start.lnk" "$INSTDIR\greenphone_quickstart.pdf"
  CreateShortCut "$SMPROGRAMS\Trolltech\Greenphone\Release Notes.lnk" "$INSTDIR\release.html"
  CreateShortCut "$DESKTOP\Greenphone SDK.lnk" "$INSTDIR\greenphone.vmx"
  File greenphone.vmx
  File install.ico
  File license.txt

  CopyFiles "$EXEDIR\greenphone_gettingstarted.pdf" "$INSTDIR"
  CopyFiles "$EXEDIR\greenphone_quickstart.pdf" "$INSTDIR"
  CopyFiles "$EXEDIR\greenphone_userguide.pdf" "$INSTDIR"
  CopyFiles "$EXEDIR\qtopia_greenphone-thm2.png" "$INSTDIR"
  CopyFiles "$EXEDIR\trolltech.png" "$INSTDIR"
  CopyFiles "$EXEDIR\vmware_player.png" "$INSTDIR"
  CopyFiles "$EXEDIR\release.html" "$INSTDIR"

  DetailPrint "" 
  DetailPrint "Installing qtopia src..."
  untgz::extract -d "$INSTDIR" "$EXEDIR\qtopiasrc.dat"
  AddSize 10000

  DetailPrint "" 
  DetailPrint "Installing home........."
  untgz::extract -d "$INSTDIR" "$EXEDIR\home.dat"
  AddSize 50000

  DetailPrint "" 
  DetailPrint "Installing qtopia SDK...3.5 minutes remaining"
  untgz::extract -d "$INSTDIR" "$EXEDIR\qtopia.dat"  
  AddSize 500000

  DetailPrint "" 
  DetailPrint "Installing toolchain....2.5 minutes remaining"
  untgz::extract -d "$INSTDIR" "$EXEDIR\toolchain.dat"
  AddSize 220000

  DetailPrint "" 
  DetailPrint "Installing root.........2.0 minutes remaining"
  untgz::extract -d "$INSTDIR" "$EXEDIR\rootfs.dat"
  DetailPrint "SDK extraction complete." 
  DetailPrint "" 
  AddSize 1100000

SectionEnd

;--------------------------------
  
Function .onInit
  SetOutPath $TEMP
  File /oname=spltmp.bmp "splash.bmp"
  ;File /oname=spltmp.wav "splash.wav"
  splash::show 5000 $TEMP\spltmp
  Pop $0
  Delete $TEMP\spltmp.bmp
  ;Delete $TEMP\spltmp.wav
  ExecShell "open" "$EXEDIR\release.html"
  sleep 3000
FunctionEnd

;--------------------------------

; Uninstaller

UninstallText "This will uninstall Greenphone SDK. Hit next to continue."
UninstallIcon "install.ico"

Section "Uninstall"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Greenphone"
  DeleteRegKey HKLM "SOFTWARE\Trolltech\Greenphone"
  Delete "$INSTDIR\bt-uninst.exe"
  Delete "$INSTDIR\license.txt"
  Delete "$INSTDIR\qtopia.vmdk"
  Delete "$INSTDIR\qtopiasrc.vmdk"
  Delete "$INSTDIR\greenphone.vmx"
  Delete "$INSTDIR\toolchain.vmdk"
  Delete "$INSTDIR\home.vmdk"
  Delete "$INSTDIR\rootfs.vmdk"
  Delete "$INSTDIR\*.vmsd"
  Delete "$INSTDIR\*.log"
  Delete "$INSTDIR\*.nvram"
  Delete "$INSTDIR\*.vmss"
  Delete "$INSTDIR\*.vmem"
  Delete "$INSTDIR\*.lck"
  Delete "$INSTDIR\*.pdf"
  Delete "$INSTDIR\*.html"
  Delete "$INSTDIR\*.png"
  Delete "$INSTDIR\install.ico"
  Delete "$SMPROGRAMS\Trolltech\Greenphone\Uninstall Greenphone SDK.lnk"
  Delete "$SMPROGRAMS\Trolltech\Greenphone\Greenphone SDK.lnk"
  Delete "$SMPROGRAMS\Trolltech\Greenphone\Getting Started.lnk"
  Delete "$SMPROGRAMS\Trolltech\Greenphone\User Guide.lnk"
  Delete "$SMPROGRAMS\Trolltech\Greenphone\Dev Quick Start.lnk"
  Delete "$SMPROGRAMS\Trolltech\Greenphone\Release Notes.lnk"
  Delete "$DESKTOP\Greenphone SDK.lnk"
  RMDir  "$SMPROGRAMS\Trolltech\Greenphone"
  RMDir  "$INSTDIR"
  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_OK "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg:

SectionEnd
