unit frmDebug;
(* NSIS Debugger v0.1
   Please read included readme file
   Written by Saivert
   http://members.tripod.com(files_saivert/
*)

interface

uses
  Windows, Messages, SysUtils, Variants, Graphics, Classes, Controls, Forms,
  Dialogs, ExtCtrls, Grids, ValEdit, HotLabel, NxSFormResizeGrid, StdCtrls,
  ComCtrls, Spin, Commctrl, ShellApi;

var
   ShowNotMsgs: Boolean = False;
   DockToNSIS: Boolean = False;

type
  TDbgForm = class(TForm)
    PageControl1: TPageControl;
    TabVariables: TTabSheet;
    TabStack: TTabSheet;
    TabAbout: TTabSheet;
    GroupBox1: TGroupBox;
    LabelDebugTool: TLabel;
    LabelVersion: TLabel;
    NxSFormResizeGrid1: TNxSFormResizeGrid;
    LabelHomepage: TLabel;
    LabelAuthor: TLabel;
    LabelPleaseVisit: TLabel;
    VarList: TValueListEditor;
    Panel1: TPanel;
    UpdTimer: TTimer;
    TabConfig: TTabSheet;
    StackList: TValueListEditor;
    Panel2: TPanel;
    GroupBox3: TGroupBox;
    GroupBoxUpdInt: TGroupBox;
    LabelUpdInt: TLabel;
    LabelSecs: TLabel;
    AutoUpdCB: TCheckBox;
    UpdVarButton: TButton;
    UpdStackButton: TButton;
    GroupBox4: TGroupBox;
    DispNotCB: TCheckBox;
    DispTimer: TTimer;
    TabDebugLog: TTabSheet;
    LogMemo: TMemo;
    Panel3: TPanel;
    CleareLogBtn: TButton;
    LastEntryLabel: TLabel;
    Image1: TImage;
    SaveButton: TButton;
    PushStackButton: TButton;
    PopStackButton: TButton;
    ClearStackButton: TButton;
    ClearButton: TButton;
    SaveStackButton: TButton;
    KeepButton: TButton;
    RestoreButton: TButton;
    UpdIntEdit: TEdit;
    UpdIntUD: TUpDown;
    DockCB: TCheckBox;
    TermButton: TButton;
    CtrlIdsCB: TCheckBox;
    GroupBox2: TGroupBox;
    OpacityTB: TTrackBar;
    OpacityLabel: TLabel;
    procedure AutoUpdCBClick(Sender: TObject);
    procedure UpdTimerTimer(Sender: TObject);
    procedure UpdVarButtonClick(Sender: TObject);
    procedure UpdStackButtonClick(Sender: TObject);
    procedure OpacityTBChange(Sender: TObject);
    procedure DispNotCBClick(Sender: TObject);
    procedure DispTimerTimer(Sender: TObject);
    procedure CleareLogBtnClick(Sender: TObject);
    procedure SaveButtonClick(Sender: TObject);
    procedure CtrlIdsCBClick(Sender: TObject);
    procedure TermButtonClick(Sender: TObject);
    procedure KeepButtonClick(Sender: TObject);
    procedure LabelHomepageClick(Sender: TObject);
    procedure RestoreButtonClick(Sender: TObject);
    procedure ClearButtonClick(Sender: TObject);
    procedure PushStackButtonClick(Sender: TObject);
    procedure PopStackButtonClick(Sender: TObject);
    procedure ClearStackButtonClick(Sender: TObject);
    procedure PageControl1Change(Sender: TObject);
    procedure UpdIntEditChange(Sender: TObject);
    procedure SaveStackButtonClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure DockCBClick(Sender: TObject);
  private
    tf: TForm;
  public
    procedure MakeVariableList;
    procedure MakeStackList;
    procedure GrabInstallerDlg; {called from subclass}
  end;

var
  DbgForm: TDbgForm;

implementation
{$R *.dfm}
uses nsis;

const
  SHomepage: PChar = 'http://members.tripod.com/files_saivert/';

const
  SKeepBtnTexts: array[Boolean] of ShortString = ('&Keep', '&Restore');
var
  IsKeeping: Boolean = False;
  keptvars: TStringList; {list used to keep variables}

const
  varnames: array[0..Ord(__INST_LAST)-1] of string = (
    '$0', '$1', '$2', '$3', '$4', '$5', '$6', '$7', '$8', '$9',
    '$R0', '$R1', '$R2', '$R3', '$R4', '$R5', '$R6', '$R7', '$R8', '$R9',
    '$CMDLINE', '$INSTDIR', '$OUTDIR', '$EXEDIR', '$LANGUAGE' );


procedure TDbgForm.FormCreate(Sender: TObject);
begin
  Icon.Handle := GetClassLong(g_hwndParent, GCL_HICON);
  Screen.Cursors[crHandPoint] := LoadCursor(0, IDC_HAND);
  GrabInstallerDlg;

  MakeVariableList;
  MakeStackList;
end;
    
procedure TDbgForm.GrabInstallerDlg;
var
  r: TRect;
begin
  {position debugger dialog near the installer's dialog}
  GetWindowRect(g_hwndParent, r);
  Top := r.Top;
  Left := r.Left-Width;
  {if window is outside screen, pull it back in}
  if Left < 0 then Left := 0;
  if Top < 0 then Top := 0;
  if Top+Height > Self.Monitor.Height then
    Top := Self.Monitor.Height-Height;
  if Left+Width > Self.Monitor.Width then
    Left := Self.Monitor.Height-Width;
end;

procedure TDbgForm.MakeVariableList;
var
  x: Integer;
begin
  for x := 0 to Ord(__INST_LAST)-1 do
  begin
    VarList.Values[varnames[x]] := PChar(g_variables+x*g_stringsize);
  end;
end;

const
  CONF_BIGSTRING_MAX = 200;
  CONF_BIGSTRING_SUBST: PChar = '...';

procedure TDbgForm.MakeStackList;
var
	stackWalker: pstack_t;
  i, l: Integer;
  buf: array[0..1024] of Char;
begin
	{fill in stack - based on code in Dumpstate}
	i := 0;
  StackList.Strings.Clear;
  stackWalker := g_stacktop^;
  while Assigned(stackWalker) do
  begin
    lstrcpy(buf, @stackWalker^.text);
		l := lstrlen(buf);

		if l > CONF_BIGSTRING_MAX then
			lstrcpy(buf + CONF_BIGSTRING_MAX, CONF_BIGSTRING_SUBST);

    StackList.InsertRow(IntToStr(i), string(buf), True);
		Inc(i);
    stackWalker := stackWalker^.next;
	end;
end;

procedure TDbgForm.AutoUpdCBClick(Sender: TObject);
begin
  UpdTimer.Enabled := AutoUpdCB.Checked;

  {Disable if irrelevant}
  UpdIntEdit.Enabled := AutoUpdCB.Checked;
  UpdIntUD.Enabled := AutoUpdCB.Checked;
  LabelUpdInt.Enabled := AutoUpdCB.Checked;
  LabelSecs.Enabled := AutoUpdCB.Checked;
end;

procedure TDbgForm.UpdTimerTimer(Sender: TObject);
begin
  MakeVariableList;
  MakeStackList;
end;

procedure TDbgForm.UpdVarButtonClick(Sender: TObject);
begin
  MakeVariableList
end;

procedure TDbgForm.UpdStackButtonClick(Sender: TObject);
begin
  MakeStackList
end;

procedure TDbgForm.OpacityTBChange(Sender: TObject);
begin
  OpacityLabel.Caption := Format('Opacity: %d%%',
    [ Round((OpacityTB.Position * 100) / 255) ]);

  if OpacityTB.Position < 255 then
    SetWindowLong(g_hwndParent, GWL_EXSTYLE, WS_EX_LAYERED)
  else SetWindowLong(g_hwndParent, GWL_EXSTYLE, 0);
  SetLayeredWindowAttributes(g_hwndParent, 0, OpacityTB.Position, LWA_ALPHA);
end;

procedure TDbgForm.DispNotCBClick(Sender: TObject);
begin
  ShowNotMsgs := DispNotCB.Checked
end;

procedure TDbgForm.DispTimerTimer(Sender: TObject);
var
  s: string;
  h, h2: THandle;
  buf: array[0..1024] of Char;
  classbuf: array[0..64] of Char;
  tw: Integer;
  mp: TPoint;
  r: TRect;
begin
  if not Assigned(tf) then
  begin
    DispTimer.Enabled := False;
    Exit;
  end;
  mp := Mouse.CursorPos;

  {hide window when mouse is outside of NSIS installer}
  GetWindowRect(g_hwndParent, r);
  if not PtInRect(r, mp) then
  begin
    ShowWindow(tf.Handle, SW_HIDE);
    Exit;
  end else ShowWindow(tf.Handle, SW_SHOWNOACTIVATE);

  tf.Left := mp.X+32;
  tf.Top := mp.Y+32;
  h := WindowFromPoint(mp);

  if h <> 0 then
  begin
    Windows.ScreenToClient(h, mp);
    h2 := ChildWindowFromPoint(h, mp);
    if h2 = 0 then
      h2 := h;

    GetClassName(h2, classbuf, sizeof(classbuf));
    GetWindowText(h2, buf, sizeof(buf));
  end;

  s := Format('Title=%s'#13#10'Class name=%s'#13#10'Control id=%d',
    [buf, classbuf, GetDlgCtrlId(h)]);
  DrawText(TLabel(tf.Tag).Canvas.Handle, PChar(s), Length(s), r,
    DT_CALCRECT or DT_WORDBREAK);
  tw := (r.Right-r.Left) + 10;
  if tw > 320 then
  begin
    tw := 320;
    { calculate height of text rectangle }
    SetRect(r, 0, 0, 320, 0);
    DrawText(TLabel(tf.Tag).Canvas.Handle, PChar(s), Length(s), r,
      DT_CALCRECT or DT_WORDBREAK);
    tf.Height := (r.Bottom-r.Top) + 10;
  end else
  begin
    tf.Height := (r.Bottom-r.Top) + 10;
  end;
  tf.Width := tw;
  TLabel(tf.Tag).SetBounds(2, 2, tf.Width-2, tf.Height-2);
  TLabel(tf.Tag).Caption := s;
end;

procedure TDbgForm.CleareLogBtnClick(Sender: TObject);
begin
  LogMemo.Clear;
  LastEntryLabel.Caption := Format('Last cleared: %s', [DateTimeToStr(Now)]);
end;

procedure TDbgForm.SaveButtonClick(Sender: TObject);
var
  x: Integer;
begin
  for x := 0 to Ord(__INST_LAST)-1 do
  begin
    StrPLCopy(PChar(g_variables+x*g_stringsize),
      VarList.Strings.ValueFromIndex[x], g_stringsize);
  end;
end;

procedure TDbgForm.CtrlIdsCBClick(Sender: TObject);
var
  l: TLabel;
begin
  if CtrlIdsCB.Checked then
  begin
  { Create a generic form and put a label on it... }
    tf := TForm.Create(DbgForm);
    tf.BorderStyle := bsNone;
    tf.FormStyle := fsStayOnTop;
    tf.Color := clInfoBk;
    tf.Font.Color := clInfoText;
    tf.SetBounds(0, 0, 1, 1); {Avoid bouncing on screen}
    l := TLabel.Create(tf);
    tf.Tag := Integer(l);
    l.Parent := tf;
    l.WordWrap := True;
    l.ParentColor := True;
    l.Show;
    tf.Show;
    SetWindowLong(tf.Handle, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
    SetWindowLong(tf.Handle, GWL_STYLE, WS_BORDER);    
  end else
  begin
    tf.Free;
    tf := nil;
  end;
  DispTimer.Enabled := CtrlIdsCB.Checked;
end;

procedure TDbgForm.TermButtonClick(Sender: TObject);
begin
  if MessageDlg('This will terminate the NSIS installer process,'#13#10+
  'Not giving it a chance to deinitialize anything.'#13#10+
  'Very brutal so to speak. Are you sure you want this?',
  mtConfirmation, [mbYes, mbNo], 0) = mrYes then
    TerminateProcess(GetCurrentProcess, 0);
end;

procedure TDbgForm.KeepButtonClick(Sender: TObject);
var
  x: Integer;
begin
  if not Assigned(keptvars) then keptvars := TStringList.Create;
  keptvars.Clear;
  for x := 0 to Ord(__INST_LAST)-1 do
    keptvars.Add(PChar(g_variables+x*g_stringsize));
  RestoreButton.Enabled := True;
end;

procedure TDbgForm.LabelHomepageClick(Sender: TObject);
begin
  ShellExecute(Handle, 'open', SHomepage, nil, nil, SW_SHOWNORMAL);
end;

procedure TDbgForm.RestoreButtonClick(Sender: TObject);
var
  x: Integer;
begin
  if not Assigned(keptvars) then exit;
  for x := 0 to Ord(__INST_LAST)-1 do
    lstrcpyn(PChar(g_variables+x*g_stringsize),PChar(keptvars[x]),g_stringsize);
  MakeVariableList; {update view}
end;

procedure TDbgForm.ClearButtonClick(Sender: TObject);
var
  x: Integer;
begin
  { put zeroes everywhere }
  for x := 0 to Ord(__INST_LAST)-1 do
    ZeroMemory(PChar(g_variables+x*g_stringsize), g_stringsize);
  MakeVariableList; {update view}
end;

procedure TDbgForm.PushStackButtonClick(Sender: TObject);
var
  s: string;
begin
  s := InputBox('Stack: Push', 'Type string to push:', '');
  if Length(s) > 0 then
    PushString(PChar(s));
  MakeStackList; {update view}
end;

procedure TDbgForm.PopStackButtonClick(Sender: TObject);
var
  buf: array[0..1024] of Char;
begin
  PopString(buf);
  MakeStackList; {update view}
end;

procedure TDbgForm.ClearStackButtonClick(Sender: TObject);
var
  buf: array[0..1024] of Char;
  res: Integer;
begin
  repeat
    res := PopString(buf)
  until res = 1;
  MakeStackList; {update view}
end;

procedure TDbgForm.PageControl1Change(Sender: TObject);
begin
  {ohh... I hate UI programming...}
  if (PageControl1.ActivePage = TabConfig) or
     (PageControl1.ActivePage = TabAbout) then
  begin
    if (ClientWidth < 354) then
      ClientWidth := 354;
    if (ClientHeight < 273) then
      ClientHeight := 273;
    { hardcoded values - works most the time
      (except with weird DPI settings) }
  end;
end;

procedure TDbgForm.UpdIntEditChange(Sender: TObject);
begin
  UpdTimer.Interval := UpdIntUD.Position*1000
end;

procedure TDbgForm.SaveStackButtonClick(Sender: TObject);
var
	stackWalker: pstack_t;
  i: Integer;
begin
	i := 0;
  stackWalker := g_stacktop^;
  while Assigned(stackWalker) do
  begin
    lstrcpyn(@stackWalker^.text,
      PChar(StackList.Strings.ValueFromIndex[i]), g_stringsize);
		Inc(i);
    stackWalker := stackWalker^.next;
	end;
end;

procedure TDbgForm.DockCBClick(Sender: TObject);
begin
  DockToNSIS := DockCB.Checked
end;

end.
