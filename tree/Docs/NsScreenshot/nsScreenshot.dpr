{
        nsScreenshot NSIS Plugin
        (c) 2003: Leon Zandman (leon@wirwar.com)
}
library nsScreenshot;

uses
  nsis,
  graphics,
  types,
  SysUtils;

const
  USER32 = 'user32.dll';

type
  HWND = type LongWord;
  {$EXTERNALSYM HWND}
  HDC = type LongWord;
  {$EXTERNALSYM HDC}
  BOOL = LongBool;
  {$EXTERNALSYM BOOL}

{$EXTERNALSYM GetDesktopWindow}
function GetDesktopWindow: HWND; stdcall; external USER32 name 'GetDesktopWindow';
{$EXTERNALSYM GetWindowDC}
function GetWindowDC(hWnd: HWND): HDC; stdcall; external USER32 name 'GetWindowDC';
{$EXTERNALSYM GetWindowRect}
function GetWindowRect(hWnd: HWND; var lpRect: TRect): BOOL; stdcall; external USER32 name 'GetWindowRect';
{$EXTERNALSYM ReleaseDC}
function ReleaseDC(hWnd: HWND; hDC: HDC): Integer; stdcall; external user32 name 'ReleaseDC';

function GetScreenshot(Filename: string; Hwnd: HWND; var Width: integer; var Height: integer): boolean; forward;
function ScreenShot(Bild: TBitMap; hWnd: HWND): boolean; forward;


function Grab_FullScreen(hwndParent: HWND; string_size: integer; variables: PChar; stacktop: pointer):integer; cdecl;
var
  buf: array[0..1024] of char;
  W,H: integer;
begin
  Result := 0;
  // set up global variables
  Init(hwndParent,string_size,variables,stacktop);

  // Get filename to save to
  PopString(@buf);

  // Get a full-screen screenshot
  if GetScreenShot(buf,GetDesktopWindow,W,H) then begin
    // Everything went just fine...

    // Push image dimensions onto stack
    PushString(PChar(IntToStr(H)));
    PushString(PChar(IntToStr(W)));

    // Push result onto stack
    PushString(PChar('ok'));
    Result := 1;
  end else begin
    // Something went wrong...
    PushString(PChar('error'));
  end;
end;

function Grab(hwndParent: HWND; string_size: integer; variables: PChar; stacktop: pointer):integer; cdecl;
var
  buf: array[0..1024] of char;
  grabWnd: HWND;
  Filename: string;
  W,H: integer;
begin
  Result := 0;
  // set up global variables
  Init(hwndParent,string_size,variables,stacktop);

  try
    // Get filename to save to
    PopString(@buf);
    Filename := buf;

    // Get window handle of window to grab
    PopString(@buf);
    grabWnd := StrToInt(buf);
  except
    PushString(PChar('error'));
    exit;
  end;

  // Get screenshot of parent windows (NSIS)
  if GetScreenShot(Filename,grabWnd,W,H) then begin
    // Everything went just fine...

    // Push image dimensions onto stack
    PushString(PChar(IntToStr(H)));
    PushString(PChar(IntToStr(W)));

    // Push result onto stack
    PushString(PChar('ok'));
    Result := 1;
  end else begin
    // Something went wrong...
    PushString(PChar('error'));
  end;
end;

function GetScreenshot(Filename: string; Hwnd: HWND; var Width: integer; var Height: integer): boolean;
var
  bmp: TBitmap;
begin
  Result := false;

  // Get screenshot
  bmp := TBitmap.Create;
  try
    try
      if ScreenShot(bmp,Hwnd) then begin
        Width  := bmp.Width;
        Height := bmp.Height;
        bmp.SaveToFile(Filename);
        Result := true;
      end;
    except
      // Catch exception and do nothing (function return value remains 'false')
    end;
  finally
    bmp.Free;
  end;
end;

function ScreenShot(Bild: TBitMap; hWnd: HWND): boolean;
var
  c: TCanvas;
  r, t: TRect;
  h: THandle;
begin
  Result := false;

  c := TCanvas.Create;
  c.Handle := GetWindowDC(GetDesktopWindow);

  h := hWnd;
  if h <> 0 then begin
    GetWindowRect(h, t);
    try
      r := Rect(0, 0, t.Right - t.Left, t.Bottom - t.Top);
      Bild.Width  := t.Right - t.Left;
      Bild.Height := t.Bottom - t.Top;
      Bild.Canvas.CopyRect(r, c, t);
    finally
      ReleaseDC(0, c.Handle);
      c.Free;
    end;
    Result := true;
  end;
end;

exports Grab_FullScreen,
        Grab;

begin
end.

