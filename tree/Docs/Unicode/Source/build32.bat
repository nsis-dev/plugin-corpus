@echo Building "unicode.dll"
cl /O1 unicode.c /LD /link kernel32.lib user32.lib "nsis/pluginapi-x86-ansi.lib" /OPT:NOWIN98 /NODEFAULTLIB /ENTRY:DllMain /OUT:"unicode.dll"
mkdir ".\Plugins\x86-ansi"
move unicode.dll ".\Plugins\x86-ansi\unicode.dll"
cl /D _UNICODE /D UNICODE /O1 unicode.c /LD /link kernel32.lib user32.lib "nsis/pluginapi-x86-unicode.lib" /OPT:NOWIN98 /NODEFAULTLIB /ENTRY:DllMain /OUT:"unicode.dll"
mkdir ".\Plugins\x86-unicode"
move unicode.dll ".\Plugins\x86-unicode\unicode.dll"
@PAUSE