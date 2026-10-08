{
        nsRandom NSIS Plugin
        (c) 2003: Leon Zandman (leon@wirwar.com)
}
library nsRandom;

uses
  nsis,
  windows,
  system,
  SysUtils;

function GetRandom(hwndParent: HWND; string_size: integer; variables: PChar; stacktop: pointer):integer; cdecl;
var
  c: PChar;
  buf: array[0..1024] of char;
  val,
  range: integer;
  valr: double;
begin
  Result := 0;

  // set up global variables
  Init(hwndParent,string_size,variables,stacktop);

  try
    // Get range from stack
    PopString(@buf);
    range := StrToInt(buf);

    randomize;

    if (range >= 0) then begin
      val := Random(range);
      PushString(PChar(IntToStr(val)));
    end else begin
      valr := Random;
      PushString(PChar(FloatToStr(valr)));
    end;

    Result:=1;
  except
    PushString('-1');
  end;
end;

exports GetRandom;

begin
end.
