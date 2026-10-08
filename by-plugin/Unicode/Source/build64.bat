@echo Building "unicode.dll"
cl /D _UNICODE /D UNICODE /O1 unicode.c /LD /link kernel32.lib user32.lib "nsis/pluginapi-amd64-unicode.lib" /MACHINE:x64 /OPT:NOWIN98 /NODEFAULTLIB /ENTRY:DllMain /OUT:"unicode.dll"
mkdir ".\Plugins\amd64-unicode"
move unicode.dll ".\Plugins\amd64-unicode\unicode.dll"
@PAUSE