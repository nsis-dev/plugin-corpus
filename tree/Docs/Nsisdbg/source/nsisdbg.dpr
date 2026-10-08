library nsisdbg;
(* NSIS Debugger v0.1
   Please read included readme file
   Written by Saivert
   http://members.tripod.com(files_saivert/
*)
uses
  SysUtils,
  nsis,
  windows,
  messages,
  frmDebug in 'frmDebug.pas' {DbgForm};

var
  tid: Cardinal;
  oldnsisproc: TFNWndProc;
  debugwnd: THandle;
  StartHidden: Boolean;
  HasInit: Boolean;

const
  CmdShowDbg = WM_USER+101;

function uithread(Parameter: Pointer): Integer; stdcall;
var
  m: TMsg;
begin
  result := 0;
  DbgForm := TDbgForm.Create(nil);
  if not StartHidden then DbgForm.Show;
  debugwnd := DbgForm.Handle;

  while GetMessage(m, 0, 0, 0) do
  begin
    if not IsWindow(DbgForm.Handle) then break;
    if not IsDialogMessage(DbgForm.Handle, m) then
    begin
      TranslateMessage(m);
      DispatchMessage(m);
    end;
  end;

  debugwnd := 0;
  DbgForm.Destroy;
  EndThread(0);
end;

procedure PutLog(text: string);
var s: string;
begin
  if Assigned(DbgForm) then
  begin
    s := Format('<%s> %s', [DateTimeToStr(Now), text]);
    DbgForm.LogMemo.Lines.Add(s);
  end;
end;

function nsissubclass(wnd: HWND; msg: UINT; wParam: WPARAM; lParam: LPARAM): LRESULT; stdcall;
begin
  if msg = WM_SYSCOMMAND then
  begin
    if (wParam and $0FFF) = CmdShowDbg then
    begin
      PutLog('Debugger System menu item clicked');
      { Restore and show window }
      if Assigned(DbgForm) then DbgForm.Show;
      ShowWindow(debugwnd, SW_RESTORE);
      SetForegroundWindow(debugwnd);
    end;
  end else if msg = WM_NOTIFY_OUTER_NEXT then
  begin
    PutLog('Got WM_Notify_Outer_Next');
    if ShowNotMsgs then
      MessageBox(wnd, 'Notification: WM_Notify_Outer_Next', 'NSIS Debugger', 64);
  end else if msg = WM_NOTIFY_CUSTOM_READY then
  begin
    PutLog('Got WM_Notify_Custom_Ready');
    if ShowNotMsgs then
      MessageBox(wnd, 'Notification: WM_Notify_Custom_Ready', 'NSIS Debugger', 64);
  end else if msg = WM_MOVE then
  begin
    if Assigned(DbgForm) and DockToNSIS then
      DbgForm.GrabInstallerDlg;
  end;

  result := CallWindowProc(oldnsisproc, wnd, msg, wParam, lParam);
end;

{ Initializes debugger.
  Must use /NOUNLOAD option on call line in NSIS script, like:
  nsisdbg::init /NOUNLOAD ["hidden"]
  - or -
  InitPluginsDir
  [Push "hidden"]
  CallInstDll "$PLUGINSDIR\nsisdbg.dll" "init" /NOUNLOAD

  "hidden" or Push "hidden" is optional, and if pushed on stack
  the debugger dialog will be hidden on startup. }
function init(hwndParent: HWND; string_size: integer; variables: PChar; stacktop: pointer):integer; cdecl;
var
  sysmenu: THandle;
  buf: array[0..1024] of char;
  val: Integer;
begin
  if HasInit then
  begin
  {When init is called in the ".onInit" script callback the g_hwndparent
   is set to nil because the installer dialog hasn't been created yet.
   You can the call init a second time in e.g. ".onGUIInit" to fill the
   remaining g_hwndparent variable.
   If you don't care you can always just call init in ".onGUIEnd".}
    PopString(buf);
    TryStrToInt(buf, val);
    g_hwndParent := val;
    result := 1;
    exit;
  end;
  // set up global variables
  nsis.Init(hwndParent,string_size,variables,stacktop);

  oldnsisproc := TFNWndProc(
    SetWindowLong(hwndParent, GWL_WNDPROC, Integer(@nsissubclass)) );

  sysmenu := GetSystemMenu(hwndParent, False);
  InsertMenu(sysmenu, SC_CLOSE, MF_BYCOMMAND, CmdShowDbg, 'Show debugger');
  debugwnd := 0;
  PopString(buf);

  StartHidden := lstrcmpi(buf, 'hidden') = 0;
  BeginThread(nil, 0, @uithread, nil, 0, tid);

  { Wait for the form to be created in uithread }
  while not Assigned(DbgForm) do
    Sleep(25);

  PushString('OK');
  HasInit := True;
  Result:=1;
end;

{ Adds string pushed on stack to log. Prepends a timestamp. }
function sendtolog(hwndParent: HWND; string_size: integer; variables: PChar; stacktop: pointer):integer; cdecl;
var
  buf: array[0..1024] of char;
  s: string;
begin
  // set up global variables (not needed, init does this for us)
  //nsis.Init(hwndParent,string_size,variables,stacktop);

  PopString(buf);
  if Assigned(DbgForm) then
  begin
    s := Format('<%s> %s', [DateTimeToStr(Now), buf]);
    DbgForm.LogMemo.Lines.Add(s);
    DbgForm.LastEntryLabel.Caption := Format('Last entry: %s', [DateTimeToStr(Now)]);
  end;

  result := 1;
end;

{ setoption
  Takes two strings on stack.
  First is option, second is state. }
function setoption(hwndParent: HWND; string_size: integer; variables: PChar; stacktop: pointer):integer; cdecl;
var
  state: array[0..1024] of char;
  opt: array[0..1024] of char;
  val: Integer;
  success: Boolean;
begin
  // set up global variables (not needed, init does this for us)
  //nsis.Init(hwndParent,string_size,variables,stacktop);
  success := True;

  if not Assigned(DbgForm) then
  begin
    result := 0;
    exit;
  end;

  PopString(opt);
  PopString(state);

  if lstrcmpi(opt, 'showctrlids') = 0 then
  begin
    success := TryStrToInt(state, val);
    if success then
      DbgForm.CtrlIdsCB.Checked := val > 0;
  end else if lstrcmpi(opt, 'updateinterval') = 0 then
  begin
    with DbgForm do
    begin
      success := TryStrToInt(state, val);
      if success then
      begin
        UpdTimer.Interval := val*1000;
        UpdIntUD.Position := val;
        UpdTimer.Enabled := UpdTimer.Interval > 0;
        AutoUpdCB.Checked := UpdTimer.Interval > 0;
      end;
    end;
  end else if lstrcmpi(opt, 'notifymsgs') = 0 then
  begin
    success := TryStrToInt(state, val);
    if success then
      DbgForm.DispNotCB.Checked := val > 0;
  end else if lstrcmpi(opt, 'opacity') = 0 then
  begin
    success := TryStrToInt(state, val);
    if success then
      DbgForm.OpacityTB.Position := val;
  end else if lstrcmpi(opt, 'docktonsis') = 0 then
  begin
    success := TryStrToInt(state, val);
    if success then
    begin
      DockToNSIS := val > 0;
      DbgForm.DockCB.Checked := val > 0;
    end;
  end else
  begin
    PushString('invalid option');
  end;
  if not success then PushString('invalid state');
  Result:=1;
end;

{ shutdown
  Call without /NOUNLOAD to fully deinitialize the debeugger.
  NSIS Debugger removes itself from the system menu, and
  restores the window procedure for installer dialog. }
function shutdown(hwndParent: HWND; string_size: integer; variables: PChar; stacktop: pointer):integer; cdecl;
begin
  // set up global variables (not needed, init does this for us)
  //nsis.Init(hwndParent,string_size,variables,stacktop);

  if Assigned(DbgForm) then
    DbgForm.CtrlIdsCB.Checked := False;
  DeleteMenu(GetSystemMenu(g_hwndParent, False), CmdShowDbg, MF_BYCOMMAND);
  SetWindowLong(hwndParent, GWL_WNDPROC, Integer(oldnsisproc));

  Result:=1;
end;

exports
  init,
  sendtolog,
  setoption,
  shutdown;

begin
end.
