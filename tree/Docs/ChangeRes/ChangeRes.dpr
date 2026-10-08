{
  NSIS ChangeRes plugin - Daniel Newton danielc4@slingshot.co.nz

  based on:
    NSIS ExDLL example
    (C) 2001 - Peter Windridge
      Fixed and formatted by Alexander Tereschenko
      http://futuris.plastiqueweb.com/


  Example of use:
  ChangeRes::ChangeResolution 800 600 32 60

  takes 4 parameters
    - Width (640, 800, 1024, 1280, etc...)
    - Height (480, 600, 768, 1024, etc...)
    - ColorDepth (8, 16, 32, etc...)
    - Frequency (60, 70, 72, 75, 85, etc...)
}

library ChangeRes;

uses
  Windows;

type
  VarConstants = (
    INST_0,
    INST_1,       // $1
    INST_2,       // $2
    INST_3,       // $3
    INST_4,       // $4
    INST_5,       // $5
    INST_6,       // $6
    INST_7,       // $7
    INST_8,       // $8
    INST_9,       // $9
    INST_R0,      // $R0
    INST_R1,      // $R1
    INST_R2,      // $R2
    INST_R3,      // $R3
    INST_R4,      // $R4
    INST_R5,      // $R5
    INST_R6,      // $R6
    INST_R7,      // $R7
    INST_R8,      // $R8
    INST_R9,      // $R9
    INST_CMDLINE, // $CMDLINE
    INST_INSTDIR, // $INSTDIR
    INST_OUTDIR,  // $OUTDIR
    INST_EXEDIR,  // $EXEDIR
    INST_LANG,    // $LANGUAGE
    __INST_LAST
    );
  TVariableList = INST_0..__INST_LAST;
  pstack_t = ^stack_t;
  stack_t = record
    next: pstack_t;
    text: PChar;
  end;

var
  g_stringsize: integer;
  g_stacktop: ^pstack_t;
  g_variables: PChar;
  g_hwndParent: HWND;

function PopString(str: PChar):integer;
var
  th: pstack_t;
begin
  if integer(g_stacktop^) = 0 then
    begin
    Result:=1;
    Exit;
    end;
  th:=g_stacktop^;
  lstrcpy(str,@th.text);
  g_stacktop^ := th.next;
  GlobalFree(HGLOBAL(th));
  Result:=0;
end;

function PushString(str: PChar):integer;
var
  th: pstack_t;
begin
  if integer(g_stacktop) = 0 then
    begin
    Result:=1;
    Exit;
    end;
  th:=pstack_t(GlobalAlloc(GPTR,sizeof(stack_t)+g_stringsize));
  lstrcpyn(@th.text,str,g_stringsize);
  th.next:=g_stacktop^;
  g_stacktop^:=th;
  Result:=0;
end;

function GetUserVariable(varnum: TVariableList):PChar;
begin
  if (integer(varnum) < 0) or (integer(varnum) >= integer(__INST_LAST)) then
    begin
    Result:='';
    Exit;
    end;
  Result:=g_variables+integer(varnum)*g_stringsize;
end;

function GetDisplaySettings(var DeviceMode: TDeviceMode; Width, Height, ColorDepth, Frequency: Cardinal): Boolean;
var
   x: integer;
   dm: TDeviceMode;
begin
     x := 0;
     Result := False;
     while EnumDisplaySettings(nil, x, dm) do begin
        if (dm.dmPelsWidth = Width) and (dm.dmPelsHeight = Height) and
           (dm.dmBitsPerPel = ColorDepth) and (dm.dmDisplayFrequency = Frequency) then begin
          DeviceMode := dm;
          Result := True;
          Exit;
        end;
        Inc(x);
     end;
end;

function SetScreenResolution(Width, Height, ColorDepth, Frequency: Cardinal): Longint;
var
  DeviceMode: TDeviceMode;
begin
  GetDisplaySettings(DeviceMode, Width, Height, ColorDepth, Frequency);
  DeviceMode.dmSize := SizeOf(TDeviceMode);
  Result := ChangeDisplaySettings(DeviceMode, CDS_UPDATEREGISTRY or CDS_GLOBAL);
end;

function ChangeResolution(hwndParent: HWND; string_size: integer; variables: PChar; stacktop: pointer):Integer; cdecl;
var
  buf: array[0..1024] of char;
  sWidth, sHeight, sColorDepth, sFrequency: string;
  Width, Height, ColorDepth, Frequency: Cardinal;
  Code: Integer;
begin
  Result:=1;

  // set up global variables
  g_stringsize:=string_size;                                                                         
  g_hwndParent:=hwndParent;
  g_stringsize:=string_size;
  g_stacktop:=stacktop;
  g_variables:=variables;

  // get variables from stack
  PopString(@buf);
  sWidth := buf;
  PopString(@buf);
  sHeight := buf;
  PopString(@buf);
  sColorDepth := buf;
  PopString(@buf);
  sFrequency := buf;
  // check if 4 parameters
  if (sFrequency = sColorDepth) or (sFrequency = sHeight) or (sFrequency = sWidth) or
     (sColorDepth = sHeight) or (sColorDepth = sWidth) or
     (sHeight = sWidth)
  then begin
    PushString(PChar('ERROR: Not enough parameters.'));
    Exit;
  end;
  // check that vars are integers
  Val(sFrequency, Frequency, Code);
  if Code > 0 then begin
    PushString(PChar('ERROR: "' + sFrequency + '"Not an integer.'));
    Exit;
  end;
  Val(sColorDepth, ColorDepth, Code);
  if Code > 0 then begin
    PushString(PChar('ERROR: "' + sColorDepth + '"Not an integer.'));
    Exit;
  end;
  Val(sHeight, Height, Code);
  if Code > 0 then begin
    PushString(PChar('ERROR: "' + sHeight + '"Not an integer.'));
    Exit;
  end;
  Val(sWidth, Width, Code);
  if Code > 0 then begin
    PushString(PChar('ERROR: "' + sWidth + '"Not an integer.'));
    Exit;
  end;
  // change res
  Code := SetScreenResolution(Width, Height, ColorDepth, Frequency);
  if Code = DISP_CHANGE_SUCCESSFUL then
    PushString('DISP_CHANGE_SUCCESSFUL')
  else if Code = DISP_CHANGE_RESTART then
    PushString('DISP_CHANGE_RESTART')
  else if Code = DISP_CHANGE_FAILED then
    PushString('DISP_CHANGE_FAILED')
  else if Code = DISP_CHANGE_BADMODE then
    PushString('DISP_CHANGE_BADMODE')
  else if Code = DISP_CHANGE_NOTUPDATED then
    PushString('DISP_CHANGE_NOTUPDATED')
  else if Code = DISP_CHANGE_BADFLAGS then
    PushString('DISP_CHANGE_BADFLAGS')
  else if Code = DISP_CHANGE_BADPARAM then
    PushString('DISP_CHANGE_BADPARAM')
  else
    PushString('UNKNOWN ERROR');
end;

exports ChangeResolution;

begin
end.
