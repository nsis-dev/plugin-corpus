Name "PPC-RegistryTest"
OutFile "PPC-RegistryTest.exe"

!include "PPC-Registry.nsh"
!include "Sections.nsh"

Var RADIOBUTTON

Page components
Page instfiles


Section "Basic registry functions" Basic
	Connect:
	${PPC-registry::CeRapiInitEx} $R0
	StrCmp $R0 0 0 +3
	MessageBox MB_OK "PPC-registry::CeRapiInitEx$\n$\n\
	                  Errorlevel: [$R0]"
	goto +3
	MessageBox MB_YESNO "PPC-registry::CeRapiInitEx$\n$\n\
	                  Errorlevel: [$R0]. Retry?" IDYES Connect
	goto end

	${PPC-registry::CreateKey} "HKEY_CURRENT_USER\SOFTWARE\NSIS\new1\new2\new3" $R0
	MessageBox MB_OK "PPC-registry::CreateKey$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::CreateKey} "HKEY_CURRENT_USER\SOFTWARE\NSIS\new1\new2\new3" $R0
	MessageBox MB_OK "PPC-registry::CreateKey$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::Write} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "MakeNSISWPlacement" "2c0000000000000001000000ffffffffffffffffffffffffffffffff7c0000003f000000a3020000fd010000" "REG_BINARY" $R0
	MessageBox MB_OK "PPC-registry::Write$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::Read} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "MakeNSISWPlacement" $R0 $R1
	MessageBox MB_OK 'PPC-registry::Read$\n$\n\
			$$R0    "string" =[$R0]$\n\
			$$R1    "type"   =[$R1]$\n'

	${PPC-registry::KeyExists} "HKEY_CURRENT_USER\SOFTWARE\NSIS" $R0
	MessageBox MB_OK "PPC-registry::KeyExists$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::DeleteKeyEmpty} "HKEY_CURRENT_USER\SOFTWARE\NSIS\new1\new2" $R0
	MessageBox MB_OK "PPC-registry::DeleteEmptyKey$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::DeleteKeyEmpty} "HKEY_CURRENT_USER\SOFTWARE\NSIS\new1\new2\new3" $R0
	MessageBox MB_OK "PPC-registry::DeleteEmptyKey$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::DeleteKey} "HKEY_CURRENT_USER\SOFTWARE\NSIS\new1" $R0
	MessageBox MB_OK "PPC-registry::DeleteKey$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::CopyValue} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "MakeNSISWPlacement" "HKEY_CURRENT_USER\SOFTWARE\NSIS\Symbols" "copied_MakeNSISWPlacement" $R0
	MessageBox MB_OK "PPC-registry::CopyValue$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::MoveValue} "HKEY_CURRENT_USER\SOFTWARE\NSIS\Symbols" "copied_MakeNSISWPlacement" "HKEY_CURRENT_USER\SOFTWARE\NSIS" "moved_MakeNSISWPlacement" $R0
	MessageBox MB_OK "PPC-registry::MoveValue$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::DeleteValue} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "moved_MakeNSISWPlacement" $R0
	MessageBox MB_OK "PPC-registry::DeleteValue$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::CopyKey} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "HKEY_CURRENT_USER\SOFTWARE\_NSIS" $R0
	MessageBox MB_OK "PPC-registry::CopyKey$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::MoveKey} "HKEY_CURRENT_USER\SOFTWARE\_NSIS" "HKEY_CURRENT_USER\SOFTWARE\NSIS_" $R0
	MessageBox MB_OK "PPC-registry::MoveKey$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::DeleteKey} "HKEY_CURRENT_USER\SOFTWARE\NSIS_" $R0

	${PPC-registry::StrToHexA} "Some Value" $0

	${PPC-registry::Write} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "new" "$0" "REG_BINARY" $R0
	MessageBox MB_OK "PPC-registry::StrToHexA$\n$\n\
			Hex string: [$0]"

	${PPC-registry::Read} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "new" $R0 $R1

	${PPC-registry::HexToStrA} "$R0" $R1
	MessageBox MB_OK "PPC-registry::HexToStrA$\n$\n\
			String: [$R1]"

	${PPC-registry::DeleteValue} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "new" $R0


	#Write extra string
	StrCpy $0 0
	StrCpy $1 0
	IntOp $0 $0 + 1
	StrCpy $1 '$1.$0'
	StrLen $2 $1
	IntCmp $2 1022 0 -3

	${PPC-registry::Write} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "new" "$1" "REG_SZ" $R0
	MessageBox MB_OK "PPC-registry::Write$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::WriteExtra} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "new" "----More text----" $R0
	MessageBox MB_OK "PPC-registry::WriteExtra$\n$\n\
			Errorlevel: [$R0]"


	#Read extra string
	StrCpy $0 0

	loop:
	${PPC-registry::ReadExtra} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "new" "$0" $R0 $R1
	IntOp $0 $0 + 1022   #if binary data then +511

	MessageBox MB_YESNO 'Read next part?$\n$\n\
			PPC-registry::ReadExtra$\n$\n\
			$$R0    "string" =[$R0]$\n\
			$$R1    "type"   =[$R1]' IDYES loop

	${PPC-registry::DeleteValue} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "new" $R0

	${PPC-registry::CeRapiUninit}

	end:
	${PPC-registry::Unload}
SectionEnd


Section /o "Search for value 'Content Type'" SearchValue
	${PPC-registry::CeRapiInit} $R0
	MessageBox MB_OK "PPC-registry::CeRapiInit$\n$\n\
			Errorlevel: [$R0]"
	StrCmp $R0 -1 end

	${PPC-registry::Open} "HKEY_LOCAL_MACHINE" "/K=0 /V=1 /S=0 /B=1 /N='Content Type'" $0
	StrCmp $0 0 0 loop
	MessageBox MB_OK "Error" IDOK close

	loop:
	${PPC-registry::Find} "$0" $1 $2 $3 $4

	MessageBox MB_OKCANCEL '$$1    "path"   =[$1]$\n\
				$$2    "value" =[$2]$\n\
				$$3    "string" =[$3]$\n\
				$$4    "type"   =[$4]$\n\
								$\n\
				Find next?' IDOK loop
	close:
	${PPC-registry::Close} "$0"
	${PPC-registry::CeRapiUninit}

	end:
	${PPC-registry::Unload}
SectionEnd


Section /o "Search and write founded in text file" SearchAndWriteInFile
	${PPC-registry::CeRapiInit} $R0
	MessageBox MB_OK "PPC-registry::CeRapiInit$\n$\n\
	                  Errorlevel: [$R0]"
	StrCmp $R0 -1 end

	${PPC-registry::CreateKey} "HKEY_CURRENT_USER\SOFTWARE\NSIS\new1\new2\new3" $R0
	${PPC-registry::Write} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "MakeNSISWPlacement" "2c0000000000000001000000ffffffffffffffffffffffffffffffff7c0000003f000000a3020000fd010000" "REG_BINARY" $R0
	${PPC-registry::Write} "HKEY_CURRENT_USER\SOFTWARE\NSIS\Symbols" "abc" "C:\Temp" "REG_SZ" $R0

	GetTempFileName $R0
	FileOpen $R1 $R0 w
	FileWrite $R1 'HKEY_CURRENT_USER\SOFTWARE\NSIS$\r$\n$\r$\n'

	${PPC-registry::Open} "HKEY_CURRENT_USER\SOFTWARE\NSIS" "/B=1" $0
	StrCmp $0 0 0 loop
	MessageBox MB_OK "Error" IDOK close

	loop:
	${PPC-registry::Find} "$0" $1 $2 $3 $4

	StrCmp $4 '' close
	StrCmp $4 'REG_KEY' 0 +3
	FileWrite $R1 '$4:"$1\$2"$\r$\n'
	goto +2
	FileWrite $R1 '$4:"$1" "$2"="$3"$\r$\n'
	goto loop

	close:
	${PPC-registry::Close} "$0"
	${PPC-registry::CeRapiUninit}
	FileClose $R1

	Exec '"notepad.exe" "$R0"'

	end:
	${PPC-registry::Unload}
SectionEnd


### If nsx.dll not found then "Search with banner" example will not be compile ###
!system 'ECHO.>"%TEMP%\Temp$$$.nsh"'
!system 'IF EXIST "${NSISDIR}\Plugins\nxs.dll" ECHO !define nxs_exist>>"%TEMP%\Temp$$$.nsh"'
!include "$%TEMP%\Temp$$$.nsh"
!system 'DEL "%TEMP%\Temp$$$.nsh"'

!ifdef nxs_exist
Section /o "Search with banner - 'NxS' plugin required" SearchWithBanner
	HideWindow

	${PPC-registry::CeRapiInit} $R0
	MessageBox MB_OK "PPC-registry::CeRapiInit$\n$\n\
			Errorlevel: [$R0]"
	StrCmp $R0 -1 end

	nxs::Show /NOUNLOAD `$(^Name) Setup`\
		/top `Setup searching something$\nPlease wait... If you can...`\
		/pos 78 /h 0 /can 1 /end

	${PPC-registry::Open} "HKLM\SOFTWARE" '/K=0 /V=1 /S=0 /N="Unexisted Name" /T=REG_DWORD /B=2' $0
	StrCmp $0 0 0 loop
	MessageBox MB_OK "Error" IDOK close

	loop:
	${PPC-registry::Find} "$0" $1 $2 $3 $4

	StrCmp $4 '' close
	StrCmp $4 'BANNER' 0 code
	nxs::Update /NOUNLOAD /sub "$1" /end
	nxs::HasUserAborted /NOUNLOAD
	Pop $5
	StrCmp $5 1 close loop

	code:
	;... code
	goto loop

	close:
	${PPC-registry::Close} "$0"
	${PPC-registry::CeRapiUninit}
	nxs::Destroy

	end:
	${PPC-registry::Unload}

	BringToFront
SectionEnd
!endif


Section /o "Save to the file" SaveKey
	${PPC-registry::CeRapiInit} $R0
	MessageBox MB_OK "PPC-registry::CeRapiInit$\n$\n\
			Errorlevel: [$R0]"
	StrCmp $R0 -1 end

	${PPC-registry::SaveKey} "HKCU\SOFTWARE\NSIS" "saved.reg" "/G=1 /D=2" $R0
	MessageBox MB_OK "PPC-registry::SaveKey$\n$\n\
			Errorlevel: [$R0]"

	${PPC-registry::CeRapiUninit}

	end:
	${PPC-registry::Unload}
SectionEnd


Function .onInit
	StrCpy $RADIOBUTTON ${Basic}
FunctionEnd

Function .onSelChange
	!insertmacro StartRadioButtons $RADIOBUTTON
	!insertmacro RadioButton ${Basic}
	!insertmacro RadioButton ${SearchValue}
	!insertmacro RadioButton ${SearchAndWriteInFile}
	!ifdef nxs_exist
		!insertmacro RadioButton ${SearchWithBanner}
	!endif
	!insertmacro RadioButton ${SaveKey}
	!insertmacro EndRadioButtons
FunctionEnd
