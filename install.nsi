############################################################################################
#      NSIS Installation Script created by NSIS Quick Setup Script Generator v1.09.18
#               Entirely Edited with NullSoft Scriptable Installation System
#              by Vlasis K. Barkas aka Red Wine red_wine@freemail.gr Sep 2006
############################################################################################

!define APP_NAME "JuceVLC"
!define COMP_NAME "Matthieu A."
!define WEB_SITE "http://jucevlc.sourceforge.net/"
!define VERSION "00.86.00.00"
!define V "0.86"
!define COPYRIGHT "Matthieu A. © 2013"
!define DESCRIPTION "JuceVLC Video Player"
!define INSTALLER_NAME "${APP_NAME}setup-${V}.exe"
!define MAIN_APP_EXE "juceVLC.exe"
!define INSTALL_TYPE "SetShellVarContext current"

!define REG_START_MENU "Start Menu Folder"

RequestExecutionLevel user
var SM_Folder

######################################################################

VIProductVersion  "${VERSION}"
VIAddVersionKey "ProductName"  "${APP_NAME}"
VIAddVersionKey "CompanyName"  "${COMP_NAME}"
VIAddVersionKey "LegalCopyright"  "${COPYRIGHT}"
VIAddVersionKey "FileDescription"  "${DESCRIPTION}"
VIAddVersionKey "FileVersion"  "${VERSION}"

######################################################################

SetCompressor /SOLID Bzip2
Name "${APP_NAME}"
Caption "${APP_NAME}"
OutFile "${INSTALLER_NAME}"
BrandingText "${APP_NAME}"
XPStyle on
InstallDir "$LOCALAPPDATA\${APP_NAME}-${V}"
AllowRootDirInstall false
######################################################################

!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING

; MUI Settings / Icons
!define MUI_ICON "src\vlc.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall-nsis.ico"

; MUI Settings / Header
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_HEADERIMAGE_BITMAP "installer\logo.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "installer\logo.bmp"

!insertmacro MUI_PAGE_DIRECTORY

!ifdef REG_START_MENU
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "JuceVLC"
!insertmacro MUI_PAGE_STARTMENU Application $SM_Folder
!endif

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\${MAIN_APP_EXE}"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "Slovak"


######################################################################

Section -MainProgram
${INSTALL_TYPE}
SetOverwrite ifnewer
SetOutPath "$INSTDIR"
File /a ${MAIN_APP_EXE}
File /a "libvlc.dll"
File /a "libvlccore.dll"
File /a "README.txt"
SetOutPath "$INSTDIR\plugins"
File /a /r "plugins\*.*"
SectionEnd

######################################################################

Section -Icons_Reg
SetOutPath "$INSTDIR"
WriteUninstaller "$INSTDIR\uninstall.exe"

!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
SetShellVarContext current
CreateDirectory "$SMPROGRAMS\$SM_Folder"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\Uninstall ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"

!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url"
!endif
!insertmacro MUI_STARTMENU_WRITE_END
!endif

!ifndef REG_START_MENU
CreateDirectory "$SMPROGRAMS\${APP_NAME}"
CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}"
CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"

!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url"
!endif
!endif

SectionEnd

######################################################################

Section Uninstall

        FindProcDLL::FindProc ${MAIN_APP_EXE}
        IntCmp $R0 1 0 notRunning
            MessageBox MB_OK|MB_ICONEXCLAMATION "JuceVLC is running. Please close it first" /SD IDOK
            Abort
        notRunning:
        
${INSTALL_TYPE}
Delete "$INSTDIR\${MAIN_APP_EXE}"
RmDir /r "$INSTDIR\plugins\*.*"
RmDir "$INSTDIR\plugins"
Delete "$INSTDIR\plugins.dat"
Delete "$INSTDIR\France.lang"
Delete "$INSTDIR\mediaTimes.xml"
Delete "$INSTDIR\settings.xml"
Delete "$INSTDIR\shortcuts.list"
Delete "$INSTDIR\libvlccore.dll"
Delete "$INSTDIR\libvlc.dll"
Delete  "$INSTDIR\uninstall.exe"
Delete  "$INSTDIR\France.lang.sample"
Delete  "$INSTDIR\thumbnails"
Delete  "$INSTDIR\README.txt"
!ifdef WEB_SITE
Delete "$INSTDIR\${APP_NAME} website.url"
!endif

RmDir "$INSTDIR"

!ifdef REG_START_MENU
SetShellVarContext current
!insertmacro MUI_STARTMENU_GETFOLDER "Application" $SM_Folder
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk"
Delete "$SMPROGRAMS\$SM_Folder\Uninstall ${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk"
!endif
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\$SM_Folder"
!endif

!ifndef REG_START_MENU
Delete "$SMPROGRAMS\JuceVLC\${APP_NAME}.lnk"
Delete "$SMPROGRAMS\JuceVLC\Uninstall ${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\JuceVLC\${APP_NAME} Website.lnk"
!endif
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\JuceVLC"
!endif

SectionEnd

######################################################################


######################################################################

Function .onInit
        ;File /oname=$TEMP\vlc.bmp "installer\vlc.bmp"

        ;advsplash::show 1000 600 400 0xFFFFFF $TEMP\vlc

        ;Pop $0          ; $0 has '1' if the user closed the splash screen early,
                        ; '0' if everything closed normally, and '-1' if some error occurred.

        ;Delete $TEMP\vlc.bmp

        FindProcDLL::FindProc ${MAIN_APP_EXE}
        IntCmp $R0 1 0 notRunning
            MessageBox MB_OK|MB_ICONEXCLAMATION "JuceVLC is running. Please close it first" /SD IDOK
            Abort
        notRunning:
	;Language selection dialog

	Push ""
	Push ${LANG_ENGLISH}
	Push English
	Push ${LANG_DUTCH}
	Push Dutch
	Push ${LANG_FRENCH}
	Push French
	Push ${LANG_GERMAN}
	Push German
	Push ${LANG_KOREAN}
	Push Korean
	Push ${LANG_RUSSIAN}
	Push Russian
	Push ${LANG_SPANISH}
	Push Spanish
	Push ${LANG_SWEDISH}
	Push Swedish
	Push ${LANG_TRADCHINESE}
	Push "Traditional Chinese"
	Push ${LANG_SIMPCHINESE}
	Push "Simplified Chinese"
	Push ${LANG_SLOVAK}
	Push Slovak
	Push A ; A means auto count languages
	       ; for the auto count to work the first empty push (Push "") must remain
	LangDLL::LangDialog "Installer Language" "Please select the language of the installer"

	Pop $LANGUAGE
	StrCmp $LANGUAGE "cancel" 0 +2
		Abort
FunctionEnd