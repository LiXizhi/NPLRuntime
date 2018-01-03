; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply
; prompts the user asking them where to install, and drops a copy of example1.nsi
; there.

;--------------------------------
!include "EnvVarUpdate.nsh"
!include "FileFunc.nsh"
!include "MUI2.nsh"
; The name of the installer
Name "NPLRuntime"
Var  StartMenuFolder
; The file to write
OutFile "nplruntime_v1.0-alpha.10_Windows_x86.exe"
RequestExecutionLevel admin

; The default installation directory
InstallDir $PROGRAMFILES32\npl
!define INSTDIR "$PROGRAMFILES32\npl"
; Request application privileges for Windows Vista
;RequestExecutionLevel user

!insertmacro DriveSpace
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TEXT  "This wizard will guide you through the installation of nplruntime. It is recommended that you close all other applications before starting Setup."
!define MUI_WELCOMEFINISHPAGE_BITMAP "sidebar.bmp"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP  "header.bmp"
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE $(myLicenseData)
Page directory dir_pre "" ""
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_LANGUAGE "SimpChinese" ;first language is the default language
!insertmacro MUI_LANGUAGE "English"

BrandingText "http://www.keepwork.com"
;--------------------------------

LangString DskCText ${LANG_ENGLISH} "The installer is already running."
LangString DskCText ${LANG_SIMPCHINESE} "安装程序已经在运行"
LicenseLangString myLicenseData ${LANG_ENGLISH} "LICENSE"
LicenseLangString myLicenseData ${LANG_SIMPCHINESE} "LICENSE"

;--------------------------------
; Test if Disk C free space is more than 10MB, if yes, donot disply directory choose page, if no give user the choice
Function dir_pre

 Var /GLOBAL  NeedSpace
 ;Var /GLOBAL  DskCEnough

 StrCpy $NeedSpace "10"
 ${DriveSpace} "C:\" "/D=F /S=M" $R0
 IntCmp $R0 $NeedSpace is1024 lessthan1024 morethan1024

 is1024:
	Goto diskCIsnotEnough

 lessthan1024:
	Goto diskCIsnotEnough

 morethan1024:
	Goto diskCIsEnough

 diskCIsEnough:
	;StrCpy $DskCEnough "1"
	## enable  following line to show directory page, otherwise it will skip the dir page.
	goto done
	abort

 diskCIsnotEnough:
	;StrCpy $DskCEnough "0"
	;MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(DskCText)" IDYES gogoInst IDNO quitInst
	MessageBox MB_OK|MB_ICONEXCLAMATION "$(DskCText)"
	goto done
	;Quit

 ;gogoInst:
	;Goto done

 ;quitInst:
	;Quit

 done:
Functionend

LangString InstallerAlreadyRunning ${LANG_ENGLISH} "The installer is already running."
LangString InstallerAlreadyRunning ${LANG_SIMPCHINESE} "安装程序已经在运行"

Function .onInit
	;----------------------
	;prevent multiple runs
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "myMutex") i .r1 ?e'
	Pop $R0

	StrCmp $R0 0 +3
	 MessageBox MB_OK|MB_ICONEXCLAMATION $(InstallerAlreadyRunning)
	 Abort

	;-----------------------
	;Language selection dialog
	;!insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

; The stuff to install
Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR"

  ; Put file there
  File "..\ParaWorld\bin32\ParaEngineClient.exe"
  File "..\ParaWorld\bin32\libcurl.dll"
  File "..\ParaWorld\bin32\lua.dll"
  File "..\ParaWorld\bin32\luasql.dll"
  File "..\ParaWorld\bin32\sqlite.dll"
  File "..\ParaWorld\bin32\PhysicsBT.dll"

    FileOpen $9 npl.bat w ;Opens a Empty File an fills it
	FileWrite $9 "@echo off $\n"
	FileWrite $9 '"%~dp0\paraengineclient.exe" %*'
	FileClose $9 ;Closes the filled file

	FileOpen $9 npls.bat w ;Opens a Empty File an fills it
	FileWrite $9 "@echo off $\n"
	FileWrite $9 '"%~dp0\paraengineclient.exe" %* servermode=true'
	FileClose $9 ;Closes the filled file

	FileOpen $9 nplc.bat w ;Opens a Empty File an fills it
	FileWrite $9 "@echo off $\n"
	FileWrite $9 setlocal
	FileWrite $9 set binder=+
	FileWrite $9 set options=%1
	FileWrite $9 shift
	FileWrite $9 :loop
	FileWrite $9 if "%1"=="" goto run_npl
	FileWrite $9 set options=%options%%binder%%1
	FileWrite $9 shift
	FileWrite $9 goto loop
	FileWrite $9 :run_npl
	FileWrite $9 npl bootstrapper="(gl)script/ide/System/nplcmd/cmd.npl" i="true" servermode="true" nplcmd=%options%
	FileWrite $9 endlocal
	FileClose $9 ;Closes the filled file

  SetOutPath "$INSTDIR\npl_packages"
  File /nonfatal /a /r "..\npl_packages\"
  ${EnvVarUpdate} $0 "PATH" "A" "HKCU" $INSTDIR

SectionEnd ; end the section