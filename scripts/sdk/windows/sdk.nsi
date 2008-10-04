; greenphone-sdk.nsi
;
; This script installs greenphone sdk on windows.
!include "Sections.nsh"
!include "LogicLib.nsh"
!include "TextFunc.nsh"

SetCompress off

;--------------------------------

Name "Qt Extended SDK Version: !QPE_VERSION! Build: !CHANGENO!"
Caption "Qt Extended SDK"
Icon "install.ico"
OutFile "autorun.exe"

SetDateSave on
SetDatablockOptimize on
CRCCheck on
;SilentInstall silent
XPStyle on

InstallDir "$PROFILE\Trolltech\Qt Extended"
InstallDirRegKey HKLM "Software\Trolltech\Qt Extended" ""

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

Section "Qt Extended SDK"
  SetOutPath $INSTDIR
  DetailPrint "Writing Reg entries..."
  ; write reg info
  WriteRegStr HKLM SOFTWARE\Trolltech\Qt Extended "Install_Dir" "$INSTDIR"
  ; write uninstall strings
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Qt Extended" "DisplayName" "Qt Extended (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Qt Extended" "UninstallString" '"$INSTDIR\bt-uninst.exe"'
  WriteUninstaller "bt-uninst.exe"
  CreateDirectory "$SMPROGRAMS\Trolltech"
  CreateDirectory "$SMPROGRAMS\Trolltech\Qt Extended"
  CreateShortCut "$SMPROGRAMS\Trolltech\Qt Extended\Uninstall Qt Extended SDK.lnk" "$INSTDIR\bt-uninst.exe"
  CreateShortCut "$SMPROGRAMS\Trolltech\Qt Extended\Qt Extended SDK.lnk" "$INSTDIR\sdk.vmx"
  CreateShortCut "$SMPROGRAMS\Trolltech\Qt Extended\Dev Quick Start.lnk" "$INSTDIR\sdk_quickstart.pdf"
  CreateShortCut "$SMPROGRAMS\Trolltech\Qt Extended\Release Notes.lnk" "$INSTDIR\release.html"
  CreateShortCut "$DESKTOP\Qt Extended SDK.lnk" "$INSTDIR\sdk.vmx"
  ;File Qt Extended.vmx
  File install.ico
  File license.txt

  CopyFiles "$EXEDIR\sdk_quickstart.pdf" "$INSTDIR"
  CopyFiles "$EXEDIR\vmware_player.png" "$INSTDIR"
  CopyFiles "$EXEDIR\release.html" "$INSTDIR"
  CopyFiles "$EXEDIR\qtopia.iso" "$INSTDIR"
  CopyFiles "$EXEDIR\sdk.vmx" "$INSTDIR"


  DetailPrint "" 
  IfFileExists "$INSTDIR\qtopiasrc.vmdk" 0 doit1
  MessageBox MB_YESNO "Overwrite Qt Extended SDK source partition?" IDYES doit1 IDNO next1
  doit1:
  DetailPrint "Installing qtopia src... Step 1/5"
  untgz::extract -d "$INSTDIR" -zbz2 "$EXEDIR\qtopiasrc.dat"
  AddSize 10000
  next1:

  DetailPrint "" 
  IfFileExists "$INSTDIR\home.vmdk" 0 doit2
  MessageBox MB_YESNO "Overwrite Qt Extended SDK home partition?" IDYES doit2 IDNO next2
  doit2:
  DetailPrint "Installing home......... Step 2/5"
  untgz::extract -d "$INSTDIR" -zbz2 "$EXEDIR\home.dat"
  AddSize 50000
  next2:

  DetailPrint "" 
  DetailPrint "Installing qtopia SDK... Step 3/5"
  untgz::extract -d "$INSTDIR" -zbz2 "$EXEDIR\qtopia.dat"  
  AddSize 500000

  DetailPrint "" 
  DetailPrint "Installing toolchain.... Step 4/5"
  untgz::extract -d "$INSTDIR" -zbz2 "$EXEDIR\toolchain.dat"
  AddSize 220000

  DetailPrint "" 
  IfFileExists "$INSTDIR\rootfs.vmdk" 0 doit3
  MessageBox MB_YESNO "Overwrite Qt Extended SDK root filesystem partition?" IDYES doit3 IDNO next3
  doit3:
  DetailPrint "Installing root......... Step 5/5"
  untgz::extract -d "$INSTDIR" -zbz2 "$EXEDIR\rootfs.dat"
  AddSize 1100000
  next3:

SectionEnd

;--------------------------------
  
Function .onInit
 System::Call 'kernel32::CreateMutexA(i 0, i 0, t "gpMutex") i .r1 ?e'
 Pop $R0
 
 StrCmp $R0 0 +3
   MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
   Abort

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

UninstallText "This will uninstall Qt Extended SDK. Hit next to continue."
UninstallIcon "install.ico"

Section "Uninstall"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Qt Extended"
  DeleteRegKey HKLM "SOFTWARE\Trolltech\Qt Extended"
  Delete "$INSTDIR\bt-uninst.exe"
  Delete "$INSTDIR\license.txt"
  Delete "$INSTDIR\qtopia.vmdk"
  Delete "$INSTDIR\qtopiasrc.vmdk"
  Delete "$INSTDIR\Qt Extended.vmx"
  Delete "$INSTDIR\toolchain.vmdk"
  Delete "$INSTDIR\home.vmdk"
  Delete "$INSTDIR\rootfs.vmdk"
  Delete "$INSTDIR\qtopia.iso"
  Delete "$INSTDIR\*.vmsd"
  Delete "$INSTDIR\*.log"
  Delete "$INSTDIR\*.nvram"
  Delete "$INSTDIR\*.vmxf"
  Delete "$INSTDIR\*.vmss"
  Delete "$INSTDIR\*.vmem"
  Delete "$INSTDIR\*.lck"
  Delete "$INSTDIR\*.pdf"
  Delete "$INSTDIR\*.html"
  Delete "$INSTDIR\*.png"
  Delete "$INSTDIR\install.ico"
  Delete "$SMPROGRAMS\Trolltech\Qt Extended\Uninstall Qt Extended SDK.lnk"
  Delete "$SMPROGRAMS\Trolltech\Qt Extended\Qt Extended SDK.lnk"
  Delete "$SMPROGRAMS\Trolltech\Qt Extended\Dev Quick Start.lnk"
  Delete "$SMPROGRAMS\Trolltech\Qt Extended\Release Notes.lnk"
  Delete "$DESKTOP\Qt Extended SDK.lnk"
  RMDir  "$SMPROGRAMS\Trolltech\Qt Extended"
  RMDir  "$INSTDIR"
  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_OK "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg:

SectionEnd
