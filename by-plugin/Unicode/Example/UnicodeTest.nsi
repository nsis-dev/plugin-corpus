/**********************************************************************
 ***          NSIS plugin for Unicode files conversion v1.0         ***
 **********************************************************************

2005 Shengalts Aleksander aka Instructor (Shengalts@mail.ru)
2009 Axel Mock (amock@marvell.com)  added FileUnicode2UTF8
2022 Jason Ross aka JasonFriday13, updated for NSIS v3.x

-Convert file from Unicode to ANSI

  unicode::FileUnicode2Ansi "Input" "Output" Type
  Pop $var

  Input:
     Unicode file
  Ouput:
     ANSI file
  Type:
     "AUTO"     - autodetect Unicode type
     "UTF-8"    - force Unicode type as UTF-8
     "UTF-16LE" - force Unicode type as UTF-16LE
     "UTF-16BE" - force Unicode type as UTF-16BE
  $var:
     ErrorLevel output

-Convert file from Unicode to UTF-8

  unicode::FileUnicode2UTF8 "Input" "Output" Type
  Pop $var

  Input:
     Unicode file
  Ouput:
     UTF-8 file
  Type:
     "AUTO"     - autodetect Unicode type
     "UTF-8"    - force Unicode type as UTF-8
     "UTF-16LE" - force Unicode type as UTF-16LE
     "UTF-16BE" - force Unicode type as UTF-16BE
  $var:
     ErrorLevel output

-Convert file from ANSI to Unicode

  unicode::FileAnsi2Unicode "Input" "Output" Type
  Pop $var

  Input:
     ANSI file
  Ouput:
     Unicode file
  Type:
     "UTF-8"    - convert as UTF-8
     "UTF-16LE" - convert as UTF-16LE
     "UTF-16BE" - convert as UTF-16BE
  $var:
     ErrorLevel output

-Get file unicode type:

  unicode::UnicodeType "Input"
  Pop $0

  Input:
     File to check
  $var:
     "NONE"             - None Unicode
     "UTF-8"            - 8-bit  Variable Width (Web)
     "UTF-16LE|UCS-2LE" - 16-bit Little Endian (Default for Windows)
     "UTF-16BE|UCS-2BE" - 16-bit Big Endian (Default for Linux)
     "UTF-32LE|UCS-4LE" - 32-bit Little Endian
     "UTF-32BE|UCS-4BE" - 32-bit Big Endian

-ErrorLevels output:
  "0" - no errors or UnicodeType output
  "1" - parameter is missed
  "2" - wrong UnicodeType specified
  "3" - file is not ANSI
  "4" - file is not UNICODE
  "5" - conversion of this UnicodeType don't supported
  "6" - input file couldn't be opened
  "7" - output file couldn't be opened
*/

Name "UnicodeTest"
OutFile "UnicodeTest.exe"
ShowInstDetails show
RequestExecutionLevel user

Section
  System::Call "kernel32::GetCurrentDirectory(i ${NSIS_MAX_STRLEN}, t .r9)"
  DetailPrint "Current directory: $9"

  #Convert file from Unicode to ANSI
  StrCpy $0 "$EXEDIR\UTF-8.txt"
  StrCpy $1 "$EXEDIR\_ResultANSI_.txt"
  StrCpy $2 AUTO

  unicode::FileUnicode2Ansi "$0" "$1" $2
  Pop $3

  DetailPrint 'unicode::FileUnicode2Ansi "$0" "$1" $2'
  DetailPrint "$3"
  DetailPrint ""


  #Convert file from ANSI to Unicode
  StrCpy $0 "$EXEDIR\ANSI.txt"
  StrCpy $1 "$EXEDIR\_ResultUnicode_.txt"
  StrCpy $2 UTF-16LE

  unicode::FileAnsi2Unicode "$0" "$1" $2
  Pop $3

  DetailPrint 'unicode::FileAnsi2Unicode "$0" "$1" $2'
  DetailPrint "$3"
  DetailPrint ""


  #Convert file from Unicode to UTF-8
  StrCpy $0 "$EXEDIR\_ResultUnicode_.txt"
  StrCpy $1 "$EXEDIR\_ResultUTF8_.txt"
  StrCpy $2 "AUTO"

  unicode::FileUnicode2UTF8 "$0" "$1" $2
  Pop $3

  DetailPrint 'unicode::FileUnicode2UTF8 "$0" "$1" $2'
  DetailPrint "$3"
  DetailPrint ""


  #Get file unicode type
  StrCpy $0 "$EXEDIR\UTF-16BE.txt"

  unicode::UnicodeType "$0"
  Pop $1

  DetailPrint 'unicode::UnicodeType "$0"'
  DetailPrint "$1"
  DetailPrint ""
SectionEnd
