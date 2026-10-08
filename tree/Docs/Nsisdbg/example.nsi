; Example script for NSIS Debugger
; Symbol: nsisdbg
;
; Written by Saivert
; http://members.tripod.com/files_saivert/
!addplugindir "."
OutFile example.exe

Name "NSIS Debugger test"

LicenseText "This is not a license, just some important information..."
LicenseData examplic.txt
Page license
Page instfiles

; Following uservar is used in the section below
Var NUM

Function .onGUIInit
  nsisdbg::init /NOUNLOAD
  nsisdbg::sendtolog /NOUNLOAD ".onGUIInit called!"
FunctionEnd

Function .onGUIEnd
  nsisdbg::sendtolog /NOUNLOAD ".onGUIEnd called!"
  nsisdbg::shutdown
FunctionEnd

Section "Test NSIS Debugger"
  Pop $0
  DetailPrint "Top of stack = $0"
  DetailPrint "Register variables:"
  DetailPrint "$$0 = $0; $$1 = $1; $$R0 = $R0; $$R1 = $R1"
  DetailPrint "$$OUTDIR (outpur dir) is $OUTDIR"
  Push "... in the debugger"
  Push "Do you see this ..."
;Fill registers with numbers
  StrCpy $0 1
  StrCpy $1 2
  StrCpy $2 3
  StrCpy $3 4
  StrCpy $4 5
  StrCpy $5 6
  StrCpy $6 7
  StrCpy $7 8
  StrCpy $8 9
  StrCpy $9 10
  StrCpy $R0 11
  StrCpy $R1 12
  StrCpy $R2 13
  StrCpy $R3 14
  StrCpy $R4 15
  StrCpy $R5 16
  StrCpy $R6 17
  StrCpy $R7 18
  StrCpy $R8 19
  StrCpy $R9 20
  
  StrCpy $NUM 5
  loop:
    Push "Pushed $$NUM = $NUM"
    IntOp $NUM $NUM - 1
  IntCmp $NUM "0" 0 0 loop
  

SectionEnd
