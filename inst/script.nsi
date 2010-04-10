; NSIS script to create an installer for Virtual Tuner

;--------------------------------
; The name of the installer
Name "Virtual Tuner"

; The file to write
OutFile "setup.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Virtual Tuner"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\Virtual_Tuner "Install_Dir"

LicenseData "..\license.txt"
LicenseText "GNU General Public License"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
; Pages

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------
; The stuff to install

Section "Virtual Tuner"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\tuner.exe"
  File "..\license.txt"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Virtual_Tuner "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Virtual Tuner" "DisplayName" "Virtual Tuner"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Virtual Tuner" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Virtual Tuner" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Virtual Tuner" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Virtual Tuner"
  CreateShortCut  "$SMPROGRAMS\Virtual Tuner\Virtual Tuner.lnk" "$INSTDIR\tuner.exe" "" "$INSTDIR\tuner.exe" 0
  CreateShortCut  "$SMPROGRAMS\Virtual Tuner\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcuts"

   CreateShortCut  "$DESKTOP\Virtual Tuner.lnk" "$INSTDIR\tuner.exe" "" "$INSTDIR\tuner.exe" 0

SectionEnd  

;--------------------------------
; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Virtual Tuner"
  DeleteRegKey HKLM SOFTWARE\Virtual_Tuner

  ; Remove files and uninstaller
  Delete $INSTDIR\tuner.exe
  Delete $INSTDIR\license.txt
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Virtual Tuner\*.*"
  Delete "$DESKTOP\Virtual Tuner.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Virtual Tuner"
  RMDir "$INSTDIR"

SectionEnd
