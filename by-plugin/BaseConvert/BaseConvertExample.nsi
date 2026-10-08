Name "testScript"
#More plugins

!addplugindir ".\plugin"

!include 'LogicLib.nsh'
!include 'FileFunc.nsh'
!include "WordFunc.nsh"

# General Symbol Definitions
!define VERSION 1.0.0
!define APP_NAME testScript
!define COMPANY ""
!define URL ""

# Included files
!include Sections.nsh

# Installer pages
Page instfiles

# Installer attributes
OutFile ${APP_NAME}.exe
InstallDir $PLUGINSDIR
;InstallDir $DESKTOP
CRCCheck on
XPStyle on
Icon "${NSISDIR}\Contrib\Graphics\Icons\classic-install.ico"
SilentInstall normal
VIProductVersion 1.0.0.0
VIAddVersionKey ProductName "${APP_NAME}"
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey CompanyName "${COMPANY}"
VIAddVersionKey FileVersion "${VERSION}"
VIAddVersionKey FileDescription ""
VIAddVersionKey LegalCopyright ""

# Installer sections
Section -Main SEC0000
BaseConvert::Convert 36 2 "0"
pop $R1
DetailPrint "Convert 0 from base 36 to 2: $R1"

BaseConvert::Convert 2 36 "0000"
pop $R1
DetailPrint "Convert 0 from base 2 to 36: $R1"

BaseConvert::Convert 2 16 "0000000000000000"
pop $R1
DetailPrint "Convert 0 from base 2 to 16: $R1"

BaseConvert::Convert 16 2 "0000000000000000000000000000000000000"
pop $R1
DetailPrint "Convert 0 from base 16 to 2: $R1"

push "a"
call Hex2Dec
pop $R0

push "123"
call Hex2Dec
pop $R0

push "513498571495"
call Hex2Dec
pop $R0

push "fffffffffffffffffffffffffffff"
call Hex2Dec
pop $R0

BaseConvert::Convert 36 2 "asfjewof4fdf0g0dsaxz"
pop $R1
DetailPrint "Convert asfjewof4fdf0g0dsaxz from base 36 to 2: $R1"

BaseConvert::Convert 2 36 "$R1"
pop $R2
DetailPrint "Convert $R1 from base 2 to 36: $R2"

!define base_7 123435134

BaseConvert::Convert 7 17 "${base_7}"
pop $R2
DetailPrint "Convert ${base_7} from base 7 to 17: $R2"

BaseConvert::Convert 17 10 "$R2"
pop $R1
DetailPrint "Convert $R2 from base 17 to 10: $R1"

BaseConvert::Convert 10 11 "$R1"
pop $R2
DetailPrint "Convert $R1 from base 10 to 11: $R2"

BaseConvert::Convert 11 7 "$R2"
pop $R1
DetailPrint "Convert $R2 from base 11 to 7: $R1"

DetailPrint "input: ${base_7}  output: $R1"


SectionEnd

# Installer functions
Function .onInit
    InitPluginsDir
    SetSilent normal
FunctionEnd

Function Hex2Dec
    exch $R0
	push $R1
	DetailPrint "Converting (Hex2Dec): $R0"
	BaseConvert::Convert 16 10 "$R0"
	pop $R1
	DetailPrint "To (dll): $R1"
    IntFmt $R0 "%ld" "0x$R0"
	DetailPrint "To (intfmt): $R0"
	pop $R1
    pop $R0
FunctionEnd


