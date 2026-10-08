!include MUI2.nsh
!include NSISArray.nsh

Name 'SortNumeric'
OutFile 'NSISArrayExample - SortNumeric.exe'
ShowInstDetails show

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE English

${Array} "TestArray" 7 6
${ArrayFunc} WriteList
${ArrayFunc} Debug
${ArrayFunc} SortNumeric

Section

  ${TestArray->Init}

  ${TestArray->WriteList} "'48' '29' '9' '53' '2' '78' '1001'"
  DetailPrint 'TestArray = [48, 29, 9, 53, 2, 78, 1001]'
  DetailPrint ""
  DetailPrint "View original Array:"
  ${TestArray->Debug}
  Sleep 3000

  DetailPrint ""
  DetailPrint "Sorting..."
  ${TestArray->SortNumeric} ""
  DetailPrint ""
  DetailPrint "Now View the sorted version:"
  ${TestArray->Debug}

  ${TestArray->Delete}

SectionEnd