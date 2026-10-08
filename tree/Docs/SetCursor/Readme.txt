
 ==============================================================

 SetCursor.dll v0.2 (4KB) by Afrow UK

  Last build: 12th April 2006

  An NSIS plugin to set the cursor or change its position.

 --------------------------------------------------------------

 Place SetCursor.dll in your NSIS\Plugins folder or
 simply extract all files in the Zip to NSIS\

 See Examples\SetCursor\ExampleMUI.nsi for example of
 use.

 ==============================================================
 The Functions:

  SetCursor::System [cursor_id]

   Sets the cursor to a system defined cursor.

   [cursor_id]   : One of the following...

   * NORMAL
   * APPSTARTING
   * ARROW
   * CROSS
   * HAND
   * HELP
   * IBEAM
   * ICON
   * NO
   * SIZE
   * SIZEALL
   * SIZENESW
   * SIZENS
   * SIZENWSE
   * SIZEWE
   * UPARROW
   * WAIT

   Note: If an error occurs, an error message is pushed
         to the stack.

          -------------------------------------------

  SetCursor::Position [x] [y]

   Sets the screen coordinates of the cursor.

          -------------------------------------------

  SetCursor::File [file_name]

   Sets the cursor to a .CUR or .ANI file.

   [file_name]   : Path to a .CUR or .ANI file.

 ==============================================================
 Change Log:

  v0.2 [12th April 2006]
   * Fixed crashes with File.
   * Now using SetClassLong instead of sub-classing windows.
   * Removed all IDC_.
   * Changed DEFAULT to NORMAL.

  v0.1 [12th April 2006]
   * First build.