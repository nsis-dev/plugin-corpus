!include MUI.nsh

OutFile SetCursorMUI.exe

# Pages...
!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

# Languages...
!insertmacro MUI_LANGUAGE English

# Test sections!
SectionGroup /e "System Cursors"

 Section "APPSTARTING Cursor"

  SetCursor::System APPSTARTING
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "ARROW Cursor"

  SetCursor::System ARROW
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "CROSS Cursor"

  SetCursor::System CROSS
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "HAND Cursor"

  SetCursor::System HAND
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "HELP Cursor"

  SetCursor::System HELP
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "IBEAM Cursor"

  SetCursor::System IBEAM
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "NO Cursor"

  SetCursor::System NO
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "SIZE Cursor"

  SetCursor::System SIZE
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "WAIT Cursor"

  SetCursor::System WAIT
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

SectionGroupEnd

SectionGroup /e "Custom Cursors"

 Section "Stop Watch Cursor"

  SetOutPath $EXEDIR
  SetCursor::File "$EXEDIR\stopwtch.ani"
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "Green Arrow Cursor"

  SetOutPath $EXEDIR
  SetCursor::File "$EXEDIR\Arrow.green.cur"
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

 Section "Red Hand Cursor"

  SetOutPath $EXEDIR
  SetCursor::File "$EXEDIR\Hand.red.cur"
  Sleep 3000
  SetCursor::System NORMAL

 SectionEnd

SectionGroupEnd

Section "Set Position: 99, 200"

 SetCursor::Position 99 200

SectionEnd
