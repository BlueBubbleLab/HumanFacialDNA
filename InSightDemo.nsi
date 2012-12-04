############################################################################################
#      NSIS Installation Script created by NSIS Quick Setup Script Generator v1.09.18
#               Entirely Edited with NullSoft Scriptable Installation System                
#              by Vlasis K. Barkas aka Red Wine red_wine@freemail.gr Sep 2006               
############################################################################################

!define APP_NAME "InSightDemo"
!define COMP_NAME "ThirdSight"
!define WEB_SITE "http://www.thirdsight.co"
!define VERSION "00.00.01.01"
!define COPYRIGHT "ThirdSight B.V. © 2013"
!define DESCRIPTION "InSight SDK demo"
!define LICENSE_TXT "inSight\LICENSE.txt"
!define INSTALLER_NAME "InSightDemoInstaller.exe"
!define MAIN_APP_EXE "insightdemo.exe"
!define INSTALL_TYPE "SetShellVarContext current"
!define MUI_ICON "inSight\resources\icons\insightInstaller.ico"
!define MUI_UNICON "inSight\resources\icons\insightUninstaller.ico"
!define MUI_SPECIALBITMAP "inSight\resources\icons\insightInstaller.bmp"
!define REG_ROOT "HKCU"
!define REG_APP_PATH "Software\Microsoft\Windows\CurrentVersion\App Paths\${MAIN_APP_EXE}"
!define UNINSTALL_PATH "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

######################################################################

VIProductVersion  "${VERSION}"
VIAddVersionKey "ProductName"  "${APP_NAME}"
VIAddVersionKey "CompanyName"  "${COMP_NAME}"
VIAddVersionKey "LegalCopyright"  "${COPYRIGHT}"
VIAddVersionKey "FileDescription"  "${DESCRIPTION}"
VIAddVersionKey "FileVersion"  "${VERSION}"

######################################################################

SetCompressor ZLIB
Name "${APP_NAME}"
Caption "${APP_NAME}"
OutFile "${INSTALLER_NAME}"
BrandingText "${APP_NAME}"
XPStyle on
InstallDirRegKey "${REG_ROOT}" "${REG_APP_PATH}" ""
InstallDir "$PROGRAMFILES\InSightDemo"

######################################################################

!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING

!insertmacro MUI_PAGE_WELCOME

!ifdef LICENSE_TXT
!insertmacro MUI_PAGE_LICENSE "${LICENSE_TXT}"
!endif

!insertmacro MUI_PAGE_DIRECTORY

!ifdef REG_START_MENU
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "InSightDemo"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${REG_ROOT}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${UNINSTALL_PATH}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${REG_START_MENU}"
!insertmacro MUI_PAGE_STARTMENU Application $SM_Folder
!endif

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\${MAIN_APP_EXE}"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

######################################################################

Section -MainProgram
${INSTALL_TYPE}
SetOverwrite ifnewer
SetOutPath "$INSTDIR"
File "build\Release\insightdemo.exe"
File "..\winbin\curl\7.23.1\vs2010\bin\libcurl.dll"
File "..\winbin\curl\7.23.1\vs2010\bin\libeay32.dll"
File "..\winbin\opencv\2.4.2\vs2010\bin\opencv_core242.dll"
File "..\winbin\opencv\2.4.2\vs2010\bin\opencv_highgui242.dll"
File "..\winbin\opencv\2.4.2\vs2010\bin\opencv_imgproc242.dll"
File "..\winbin\opencv\2.4.2\vs2010\bin\opencv_ml242.dll"
File "..\winbin\opencv\2.4.2\vs2010\bin\opencv_objdetect242.dll"
File "..\winbin\opencv\2.4.2\vs2010\bin\opencv_video242.dll"
File "..\winbin\opencv\2.4.2\vs2010\bin\opencv_calib3d242.dll"
File "..\winbin\opencv\2.4.2\vs2010\bin\opencv_flann242.dll"
File "..\winbin\opencv\2.4.2\vs2010\bin\opencv_features2d242.dll"
File "..\winbin\gsl\1.8\vs2010\bin\libgsl.dll"
File "..\winbin\gsl\1.8\vs2010\bin\libgslcblas.dll"
#File "settings.ini"
File "..\winbin\curl\7.23.1\vs2010\bin\ssleay32.dll"
File "..\winbin\tbb\3.0u7\vs2010\bin\tbb.dll"
SetOutPath "$INSTDIR\resources"
File "resources\demo_dashboard.png"
File "inSight\resources\icons\insightdemo.ico"
SetOutPath "$INSTDIR\data"
File /r /x ".." "inSight\data\*"
AccessControl::GrantOnFile "$INSTDIR" "(S-1-5-32-545)" "FullAccess"
SectionEnd

######################################################################

Section -Icons_Reg
SetOutPath "$INSTDIR"
WriteUninstaller "$INSTDIR\uninstall.exe"

!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
CreateDirectory "$SMPROGRAMS\$SM_Folder"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}" "" "$INSTDIR\resources\insightdemo.ico"
CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}" "" "$INSTDIR\resources\insightdemo.ico"
!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url" "" "$INSTDIR\resources\insightdemo.ico"
!endif
!insertmacro MUI_STARTMENU_WRITE_END
!endif

!ifndef REG_START_MENU
CreateDirectory "$SMPROGRAMS\InSightDemo"
CreateShortCut "$SMPROGRAMS\InSightDemo\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}" "" "$INSTDIR\resources\insightdemo.ico"
CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_APP_EXE}" "" "$INSTDIR\resources\insightdemo.ico"
!ifdef WEB_SITE
WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
CreateShortCut "$SMPROGRAMS\InSightDemo\${APP_NAME} Website.lnk" "$INSTDIR\${APP_NAME} website.url" "" "$INSTDIR\resources\insightdemo.ico"
!endif
!endif

WriteRegStr ${REG_ROOT} "${REG_APP_PATH}" "" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayName" "${APP_NAME}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayIcon" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayVersion" "${VERSION}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "Publisher" "${COMP_NAME}"

!ifdef WEB_SITE
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "URLInfoAbout" "${WEB_SITE}"
!endif
SectionEnd

######################################################################

Section Uninstall
${INSTALL_TYPE}
RmDir /r "$INSTDIR"

!ifdef REG_START_MENU
!insertmacro MUI_STARTMENU_GETFOLDER "Application" $SM_Folder
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\$SM_Folder\${APP_NAME} Website.lnk"
!endif
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\$SM_Folder"
!endif

!ifndef REG_START_MENU
Delete "$SMPROGRAMS\InSightDemo\${APP_NAME}.lnk"
!ifdef WEB_SITE
Delete "$SMPROGRAMS\InSightDemo\${APP_NAME} Website.lnk"
!endif
Delete "$DESKTOP\${APP_NAME}.lnk"

RmDir "$SMPROGRAMS\InSightDemo"
!endif

DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}"
DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}"
SectionEnd

######################################################################

