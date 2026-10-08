{
  ExecTimout Plugin for NSIS
  Written by LoRd_MuldeR <mulder2@gmx.de>

  based on: NSIS ExDLL example, by Peter Windridge

  Tested in Delphi 7.0, NSIS v2.37
}

library exdll;

uses
  Windows;

////////////////////////////////////////
// NSIS API
////////////////////////////////////////

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

function PopString(): string;
var
  th: pstack_t;
begin
  if integer(g_stacktop^) <> 0 then begin
    th := g_stacktop^;
    Result := PChar(@th.text);
    g_stacktop^ := th.next;
    GlobalFree(HGLOBAL(th));
  end;
end;

procedure PushString(const str: string='');
var
  th: pstack_t;
begin
  if integer(g_stacktop) <> 0 then begin
    th := pstack_t(GlobalAlloc(GPTR, SizeOf(stack_t) + g_stringsize));
    lstrcpyn(@th.text, PChar(str), g_stringsize);
    th.next := g_stacktop^;
    g_stacktop^ := th;
  end;
end;

function GetUserVariable(const varnum: TVariableList): string;
begin
  if (integer(varnum) >= 0) and (integer(varnum) < integer(__INST_LAST)) then
    Result := g_variables + integer(varnum) * g_stringsize
  else
    Result := '';
end;

procedure SetUserVariable(const varnum: TVariableList; const value: string);
begin
  if (value <> '') and (integer(varnum) >= 0) and (integer(varnum) < integer(__INST_LAST)) then
    lstrcpy(g_variables + integer(varnum) * g_stringsize, PChar(value))
end;

////////////////////////////////////////
// SysUtils
////////////////////////////////////////

procedure CvtInt;
{ IN:
    EAX:  The integer value to be converted to text
    ESI:  Ptr to the right-hand side of the output buffer:  LEA ESI, StrBuf[16]
    ECX:  Base for conversion: 0 for signed decimal, 10 or 16 for unsigned
    EDX:  Precision: zero padded minimum field width
  OUT:
    ESI:  Ptr to start of converted text (not start of buffer)
    ECX:  Length of converted text
}
asm
        OR      CL,CL
        JNZ     @CvtLoop
@C1:    OR      EAX,EAX
        JNS     @C2
        NEG     EAX
        CALL    @C2
        MOV     AL,'-'
        INC     ECX
        DEC     ESI
        MOV     [ESI],AL
        RET
@C2:    MOV     ECX,10

@CvtLoop:
        PUSH    EDX
        PUSH    ESI
@D1:    XOR     EDX,EDX
        DIV     ECX
        DEC     ESI
        ADD     DL,'0'
        CMP     DL,'0'+10
        JB      @D2
        ADD     DL,('A'-'0')-10
@D2:    MOV     [ESI],DL
        OR      EAX,EAX
        JNE     @D1
        POP     ECX
        POP     EDX
        SUB     ECX,ESI
        SUB     EDX,ECX
        JBE     @D5
        ADD     ECX,EDX
        MOV     AL,'0'
        SUB     ESI,EDX
        JMP     @z
@zloop: MOV     [ESI+EDX],AL
@z:     DEC     EDX
        JNZ     @zloop
        MOV     [ESI],AL
@D5:
end;

function StrToIntDef(const S: string; Default: Integer): Integer;
var
  E: Integer;
begin
  Val(S, Result, E);
  if E <> 0 then Result := Default;
end;

function IntToStr(Value: Integer): string;
//  FmtStr(Result, '%d', [Value]);
asm
        PUSH    ESI
        MOV     ESI, ESP
        SUB     ESP, 16
        XOR     ECX, ECX       // base: 0 for signed decimal
        PUSH    EDX            // result ptr
        XOR     EDX, EDX       // zero filled field width: 0 for no leading zeros
        CALL    CvtInt
        MOV     EDX, ESI
        POP     EAX            // result ptr
        CALL    System.@LStrFromPCharLen
        ADD     ESP, 16
        POP     ESI
end;

////////////////////////////////////////
// Plugin Functions
////////////////////////////////////////

function MyCreateProcess(Commandline: String): THandle;
var
  StartupInfo: TStartupInfo;
  ProcessInfo: TProcessInformation;
begin
  ZeroMemory(@StartupInfo, SizeOf(TStartupInfo));
  ZeroMemory(@ProcessInfo, SizeOf(TProcessInformation));

  StartupInfo.cb := SizeOf(TStartupInfo);

  if not CreateProcess(nil, PAnsiChar(Commandline), nil, nil, false, 0, nil, nil, StartupInfo, ProcessInfo) then
  begin
    Result := INVALID_HANDLE_VALUE;
    Exit;
  end;

  CloseHandle(ProcessInfo.hThread);
  Result := ProcessInfo.hProcess;
end;

procedure ExecTimeout(const hwndParent: HWND; const string_size: integer; const variables: PChar; const stacktop: pointer); cdecl;
var
  Commandline: String;
  Timeout: Integer;
  Terminate: Boolean;
  Process: THandle;
  Wait: Cardinal;
  ExitCode: Cardinal;
begin
  // setup global variables
  g_stringsize := string_size;
  g_hwndParent := hwndParent;
  g_stacktop := stacktop;
  g_variables := variables;
  // end global variable setup

  Commandline := PopString;
  Timeout := StrToIntDef(PopString, 0);
  Terminate := StrToIntDef(PopString, 1) <> 0;

  Process := MyCreateProcess(Commandline);

  if Process = INVALID_HANDLE_VALUE then
  begin
    PushString('error');
    Exit;
  end;

  if Timeout > 0 then begin
    Wait := WaitForSingleObject(Process, Timeout);

    if Wait = WAIT_FAILED then
    begin
      CloseHandle(Process);
      PushString('error');
      Exit;
    end;

    if Wait = WAIT_TIMEOUT then
    begin
      if Terminate then
      begin
        TerminateProcess(Process, 1);
      end;
      CloseHandle(Process);
      PushString('timeout');
      Exit;
    end;
  end;

  GetExitCodeProcess(Process, ExitCode);
  CloseHandle(Process);

  PushString(IntToStr(ExitCode));
end;

exports ExecTimeout;

begin
end.
