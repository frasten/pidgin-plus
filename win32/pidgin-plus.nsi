;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

!include "MUI.nsh"

;--------------------------------
;General

;Name and file
Name "Pidgin Plus! Plugin"
OutFile "pidgin-plus-0.2.0.exe"

;Default installation folder
InstallDir "$PROGRAMFILES\Pidgin\plugins"

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\Pidgin Plus Plugin" ""

;--------------------------------
;Interface Settings

!define MUI_ABORTWARNING

;--------------------------------
;Pages

;!insertmacro MUI_PAGE_LICENSE "Basic.nsi"
;!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "PidginPrivacyPlease" SecPidginPlus

SetOutPath "$INSTDIR"

File "pidgin-plus.dll"

;ADD YOUR OWN FILES HERE...

;Store installation folder
WriteRegStr HKCU "Software\Pidgin Plus Plugin" "" $INSTDIR

;Create uninstaller
WriteUninstaller "$INSTDIR\Uninstall-pidgin-plus.exe"

SectionEnd

;--------------------------------
;Descriptions

;Language strings
LangString DESC_SecPidginPlus ${LANG_ENGLISH} "The Pidgin Plus Plugin."

;Assign language strings to sections
;!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;	!insertmacro MUI_DESCRIPTION_TEXT ${SecPidginPlus} $(DESC_SecPidginPlus)
;!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

;ADD YOUR OWN FILES HERE...

Delete "$INSTDIR\Uninstall-pidgin-plus.exe"
Delete "$INSTDIR\pidgin-plus.dll"

DeleteRegKey /ifempty HKCU "Software\Pidgin Plus Plugin"

SectionEnd
