Name "install-nsPython-plugin"
OutFile "install-nsPython-plugin.exe"


InstallDir $PROGRAMFILES\NSIS
InstallDirRegKey HKLM SOFTWARE\NSIS ""

Section "Install Python Plugin"
    SetOutPath $INSTDIR\Plugins
    File nsPython.dll
SectionEnd
