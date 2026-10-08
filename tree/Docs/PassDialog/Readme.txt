
 ==============================================================

 PassDialog.dll v0.4 (8.5kB) by Afrow UK

  Last build: 10th August 2015

  A NSIS plugin that allows you to use the following dialogs:
   Password
   Username & password
   Serial
   InputBox
  The dialogs are imbedded (like InstallOptions dialogs).
  The benefits of using this plugin is that the password or
  serial code is secure (it is not stored in an INI file
  like InstallOptions).

 --------------------------------------------------------------

 Place PassDialog.dll in your NSIS\Plugins folder or
 simply extract all files in the Zip to NSIS\

 See NSIS\Examples\PassDialog\* for examples of use.

 ==============================================================
 The Functions:

  PassDialog::Dialog [dialog_name] [params]
   Pop $Var

   [dialog_name] : password, userpass, serial, inputbox
   $Var          : success, back, cancel, error
   [params]      : Described under General Parameters

  This is the normal way to call the plugin.

  ------------------------------------------------------------

  PassDialog::InitDialog /NOUNLOAD [dialog_name] [params]
   Pop $HWNDVar
  PassDialog::Show
   Pop $Var

   [dialog_name] : password, userpass, serial, inputbox
   $HWNDVar      : HWND handle of the window
   $Var          : success, back, cancel, error
   [params]      : Described under General Parameters

  This method allows you to modify controls on the dialog with
  SendMessage, SetCtlColors etc by using the $HWNDVar between
  the InitDialog and Show calls and also in the Pages' Leave
  function.

 ==============================================================
 Dialogs [dialog_name]:

  InputBox       : A page with a single input box.
  Password       : A page with a single password box.
  UserPass       : A page with username and password boxes.
  Serial         : A page with customisable serial boxes.

 ==============================================================
 General Parameters [params]:

  These parameters apply to all dialog.

  /HEADINGTEXT [text]
   Text to be displayed in the heading label.
   Applies to all dialogs.

  /GROUPTEXT [text]
   Text to be displayed on the group box.
   Applies to all dialogs.

 ==============================================================
 Password and UserPass Parameters [params]:

  These parameters apply to the Password and UserPass dialogs
  only.

  /USERTEXT [label_text] [box_text]
   Text to be displayed in the username field box and label.
   [label_text] : Text to be displayed to the right of the box.
   [box_text]   : Text to be placed in the text box.

  /PASSTEXT [label_text] [box_text]
   Text to be displayed in the password field box and label.
   [label_text] : Text to be displayed to the right of the box.
   [box_text]   : Text to be placed in the text box.

 ==============================================================
 Serial Parameters [params]:

  These parameters apply to the Serial dialog only.

  /BOXTOP [top]
   The top position of all boxes.

  /CENTER
   Text in the field boxes are center aligned.

  ------------------------------------------------------------

  The following perameters must come after all others.

  /BOX [left] [width] [max_length] [box_text]
   Creates a serial field box.
   [left]       : Position of the box from the left.
   [width]      : Width of the box.
   [max_length] : Max number of characters to fit in the box.
   [box_text]   : Text to be displayed in the text initially.

  /BOXDASH [left] [width] [max_length] [box_text]
   The same as using /BOX, except a dash (-) is placed after
   the serial field box.

 ==============================================================
 InputBox Parameters [params]:

  These parameters apply to the InputBox dialog only, and
  must come after all other parameters.

  /BOX [label_text] [box_text] [max_length]
   Creates an input box with label.
   [label_text] : Text to be displayed to the right of the box.
   [box_text]   : Text to be placed in the input text box.
   [max_length] : Max number of characters to fit in the box.

  /BOXRO [label_text] [box_text] [max_length]
   Creates an input box with label which is read only.
   All parameters are the same as /BOX.

 ==============================================================
 Control ID's:

  You can find the control ID defines in
   Contrib\PassDialog\resource.h

 ==============================================================
 Change Log:

  v0.4 (10th August 2015)
   * Project moved to VC++ 2008.
   * Unicode port by JasonFriday13.
   * 64 bit support by JasonFriday13.
   * Plugin moved to NSIS 2.42+ plugin API.
   * Fixed serial box position adding bug.
   * Fixed window colour bug on recent nsis versions.
   * Example script fixes.

  v0.3 (6th May 2006)
   * Project moved to VC++ 6.
   * Support for multiple input boxes on the InputBox page.
   * Swapped [box_text] and [label_text] around for
     /PASSTEXT and /USERTEXT.
   * Added example that uses MD5DLL plugin to encrypt usernames
     and password.

  v0.2
   * Fixed label text on InputBox (wasn't changable).
   * Added /MAXLEN for InputBox dialog.

  v0.1
   * First build.