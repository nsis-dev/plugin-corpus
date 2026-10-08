#include <windows.h>
#include "..\ExDll\pluginapi.h" // requires nsis 2.42 or higher
#include "resource.h"

// Global variables
// [[

HINSTANCE g_hInstance;
HWND      g_hWndParent;

TCHAR szHeadingText[1024];
TCHAR szGroupText[1024];
TCHAR szLabelAText[1024], szLabelBText[1024];
TCHAR szBoxAText[1024], szBoxBText[1024];
BOOL bCenterText = FALSE;
int iBoxCount, iBoxTop;
int iDialog; // 0 = UserPass, 1 = Password, 2 = Serial, 3 = InputBox

HWND hDialog;
HWND hNext;
static LONG_PTR lpWndProcOld;

BOOL g_done = FALSE, g_is_cancel = FALSE, g_is_back = FALSE;
BOOL bInitDialog = FALSE, Initialized = FALSE;

// ]]

// General defines
// [[

#define PARAM_TEXT_HEADING   TEXT("/HEADINGTEXT") // [text]
#define PARAM_TEXT_GROUP     TEXT("/GROUPTEXT")   // [text]
#define PARAM_TEXT_BOXA      TEXT("/USERTEXT")    // [label_text] [box_text]
#define PARAM_TEXT_BOXB      TEXT("/PASSTEXT")    // [label_text] [box_text]

#define PARAM_BOX_MAXLEN     TEXT("/MAXLEN")      // [max_length_in_chars]

#define PARAM_TEXT_CENTER    TEXT("/CENTER")

#define PARAM_SERIAL_BOX     TEXT("/BOX")     // [left] [width] [max_length_in_chars] [box_text]
#define PARAM_SERIAL_BOXDASH TEXT("/BOXDASH") // [left] [width] [max_length_in_chars] [box_text]
#define PARAM_SERIAL_TOP     TEXT("/BOXTOP")  // [top]

#define PARAM_INPUTBOX_BOX   TEXT("/BOX")     // [label_text] [box_text] [max_length_in_chars]
#define PARAM_INPUTBOX_BOXRO TEXT("/BOXRO")   // [label_text] [box_text] [max_length_in_chars]

#define DEFAULT_RECT 1018

#define TEXT_USERPASS_HEADING TEXT("Please enter your username and password before continuing...")
#define TEXT_USERPASS_GROUP   TEXT("Enter Username and Password")

#define TEXT_PASSWORD_HEADING TEXT("Please enter your password before continuing...")
#define TEXT_PASSWORD_GROUP   TEXT("Enter Password")

#define TEXT_SERIAL_HEADING   TEXT("Please enter the serial code before continuing...")
#define TEXT_SERIAL_GROUP     TEXT("Enter Serial Code")
#define TEXT_SERIAL_DASH      TEXT("-")

#define TEXT_INPUTBOX_HEADING TEXT("Please enter your details before continuing...")
#define TEXT_INPUTBOX_GROUP   TEXT("Enter Your Details")

// ]]

// Function declarations
LRESULT CALLBACK ParentWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Function: Callback function so that our dll stays loaded.
UINT_PTR __cdecl NSISPluginCallback(enum NSPIM Event) 
{
  return 0;
}

// Function: Initialize the callback so that our plugin stays loaded.
void Initialize(extra_parameters* xp)
{
  if (!Initialized)
  {
    xp->RegisterPluginCallback(g_hInstance, NSISPluginCallback);
    Initialized = TRUE;
  }
}

// Function: Gets common parameters from stack
void GetParams()
{
  TCHAR szParam[1024];
  while (popstring(szParam) == 0)
  {
    if (lstrcmpi(szParam, PARAM_TEXT_HEADING) == 0)
      popstring(szHeadingText);
    else if (lstrcmpi(szParam, PARAM_TEXT_GROUP) == 0)
      popstring(szGroupText);
    else if (lstrcmpi(szParam, PARAM_TEXT_BOXA) == 0)
    {
      popstring(szLabelAText);
      popstring(szBoxAText);
    }
    else if (lstrcmpi(szParam, PARAM_TEXT_BOXB) == 0)
    {
      popstring(szLabelBText);
      popstring(szBoxBText);
    }
    else if (lstrcmpi(szParam, PARAM_TEXT_CENTER) == 0)
      bCenterText = TRUE;
    else if (lstrcmpi(szParam, PARAM_SERIAL_TOP) == 0)
    {
      popstring(szParam);
      iBoxTop = myatoi(szParam);
    }
    else
    {
      pushstring(szParam);
      break;
    }
  }
}

// Function: Sets common control texts
void SetTexts(HWND hWndDlg)
{
  SetWindowText(GetDlgItem(hWndDlg, IDC_LABELH), szHeadingText);
  SetWindowText(GetDlgItem(hWndDlg, IDC_GROUPBOX), szGroupText);
}

// Function: Sets texts for username or inputbox label & box
void SetUserTexts(HWND hWndDlg)
{
  if (szLabelAText && *szLabelAText)
    SetWindowText(GetDlgItem(hWndDlg, IDC_LABELA), szLabelAText);
  if (szBoxAText && *szBoxAText)
    SetWindowText(GetDlgItem(hWndDlg, IDC_USERNAME), szBoxAText);
}

// Function: Sets texts for password label & box
void SetPassTexts(HWND hWndDlg)
{
  if (szLabelBText && *szLabelBText)
    SetWindowText(GetDlgItem(hWndDlg, IDC_LABELB), szLabelBText);
  if (szBoxBText && *szBoxBText)
    SetWindowText(GetDlgItem(hWndDlg, IDC_PASSWORD), szBoxBText);
}

// Function: Enables next button if all serial boxes are full
void SerialEnableNext()
{
  int i;
  BOOL bEnableNext = TRUE;
  for (i=0; i<iBoxCount; i++)
  {
    if ((int)SendMessage(GetDlgItem(hDialog, ((int)IDC_SERIAL + i)), WM_GETTEXTLENGTH, 0, 0) != (int)SendMessage(GetDlgItem(hDialog, ((int)IDC_SERIAL + i)), EM_GETLIMITTEXT, 0, 0))
      bEnableNext = FALSE;
  }
  if (bEnableNext)
    EnableWindow(hNext, TRUE);
  else
    EnableWindow(hNext, FALSE);
}

// Displays our dialog
void ShowDialog(HWND hDialog)
{
  if (hDialog)
  {
    RECT dialog_r;
    int mainWndWidth, mainWndHeight;

    // Get the sizes of the UI
    GetWindowRect(GetDlgItem(g_hWndParent, DEFAULT_RECT), &dialog_r);
    MapWindowPoints(0, g_hWndParent, (LPPOINT)&dialog_r, 2);

    mainWndWidth = dialog_r.right - dialog_r.left;
    mainWndHeight = dialog_r.bottom - dialog_r.top;

    // Set our window size to fit the UI size
    SetWindowPos(
      hDialog,
      0,
      dialog_r.left,
      dialog_r.top,
      mainWndWidth,
      mainWndHeight,
      SWP_NOZORDER|SWP_NOACTIVATE
    );

    lpWndProcOld = SetWindowLongPtr(g_hWndParent, DWLP_DLGPROC, (LONG_PTR)ParentWndProc);

    // Sets the font of IO window to be the same as the main window
    SendMessage(hDialog, WM_SETFONT, (WPARAM)SendMessage(g_hWndParent, WM_GETFONT, 0, 0), (LPARAM)TRUE);

    // Tell NSIS to remove old inner dialog and pass handle of the new inner dialog
    SendMessage(g_hWndParent, WM_NOTIFY_CUSTOM_READY, (WPARAM)hDialog, 0);
    ShowWindow(hDialog, SW_SHOWNA);

    g_done = FALSE;

    // Loop until the user clicks on a button
    while (!g_done) {
      MSG msg;
      int nResult = GetMessage(&msg, NULL, 0, 0);
      if (!IsDialogMessage(hDialog,&msg) && !IsDialogMessage(g_hWndParent,&msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

    // Return page button result
    pushstring(g_is_cancel ? TEXT("cancel") : g_is_back ? TEXT("back") : TEXT("success"));

    // Set window dialog procedure back to NSIS's
    SetWindowLongPtr(g_hWndParent, DWLP_DLGPROC, lpWndProcOld);
    DestroyWindow(hDialog);

  }
  else
    pushstring(TEXT("error"));
}

// Callback: Handles the parent dialog
LRESULT CALLBACK ParentWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LRESULT bRes;
  if (uMsg == WM_NOTIFY_OUTER_NEXT && wParam == 1)
  {
    HWND hCtl;
    TCHAR szOutput[1024];
    int i;

    switch (iDialog)
    {
      // Output password and/or username to stack
      case 0:
      case 1:
        {
          hCtl = GetDlgItem(hDialog, IDC_PASSWORD);
          GetWindowText(hCtl, szOutput, sizeof(szOutput));
          SetWindowText(hCtl, TEXT(""));
          pushstring(szOutput);

          hCtl = GetDlgItem(hDialog, IDC_USERNAME);
          if (hCtl)
          {
            GetWindowText(hCtl, szOutput, sizeof(szOutput));
            pushstring(szOutput);
          }
        }
        break;
      // Output serial box values to stack
      case 2:
        {
          for (i=iBoxCount-1; i>=0; i--)
          {
            hCtl = GetDlgItem(hDialog, ((int)IDC_SERIAL + i));
            GetWindowText(hCtl, szOutput, sizeof(szOutput));
            SetWindowText(hCtl, TEXT(""));
            pushstring(szOutput);
          }
        }
        break;
      // Output input box values to stack
      case 3:
        {
          for (i=iBoxCount-1; i>=0; i--)
          {
            hCtl = GetDlgItem(hDialog, ((int)IDC_INPUTBOX + (i*2)));
            GetWindowText(hCtl, szOutput, sizeof(szOutput));
            pushstring(szOutput);
          }
        }
    }
  }

  bRes = CallWindowProc((WNDPROC)lpWndProcOld, hWnd, uMsg, wParam, lParam);
  if (uMsg == WM_NOTIFY_OUTER_NEXT && !bRes)
  {
    if (wParam == -1)
      g_is_back = TRUE;
    else if (wParam == NOTIFY_BYE_BYE)
      g_is_cancel = TRUE;
    g_done = TRUE;
    PostMessage(hDialog, WM_CLOSE, 0, 0);
  }
  return bRes;
}

// Callback: Handles the UserPass dialog
LRESULT CALLBACK dlgUserPass(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_INITDIALOG:
    SetTexts(hWndDlg);
    SetUserTexts(hWndDlg);
    SetPassTexts(hWndDlg);
  break;
  case WM_CTLCOLORSTATIC:
  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORDLG:
  case WM_CTLCOLORBTN:
  case WM_CTLCOLORLISTBOX:
    // let the NSIS window handle colors, it knows best
    return SendMessage(g_hWndParent, uMsg, wParam, lParam);
  }
  return FALSE;
}

// Callback: Handles the UserPass dialog
LRESULT CALLBACK dlgPass(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_INITDIALOG:
    SetTexts(hWndDlg);
    SetPassTexts(hWndDlg);
  break;
  case WM_CTLCOLORSTATIC:
  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORDLG:
  case WM_CTLCOLORBTN:
  case WM_CTLCOLORLISTBOX:
    // let the NSIS window handle colors, it knows best
    return SendMessage(g_hWndParent, uMsg, wParam, lParam);
  }
  return FALSE;
}

// Callback: Handles the Serial dialog
LRESULT CALLBACK dlgSerial(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static TCHAR szParam[1024];
  HWND hBox, hDash;
  int iDash = 1, i;


  switch (uMsg)
  {
  case WM_INITDIALOG:
    SetTexts(hWndDlg);

    iBoxTop = 70;
    iBoxCount = 0;

    while (popstring(szParam) == 0)
    {
      if ((lstrcmpi(szParam, PARAM_SERIAL_BOX) == 0) || ((iDash = lstrcmpi(szParam, PARAM_SERIAL_BOXDASH)) == 0))
      {
        int x, w, iMaxLen;
        RECT r;
        DWORD dwStyle;

        GetClientRect(GetDlgItem(hWndDlg, IDC_LABELA), &r);

        // Position of box from left
        popstring(szParam);
          x = myatoi(szParam);

        // Width of box
        popstring(szParam);
          w = myatoi(szParam);

        // Number of chars to fit in box
        popstring(szParam);
          iMaxLen = myatoi(szParam);

        // Text to be in box
        popstring(szParam);

        // Create style
        dwStyle = ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP;
        if (bCenterText)
          dwStyle |= ES_CENTER;
        else
          dwStyle |= ES_LEFT;

        // Create the serial box
        hBox = CreateWindow(
          TEXT("EDIT"),
          szParam,
          dwStyle,
          x + 18,
          iBoxTop,
          w,
          18,
          hWndDlg,
          (HMENU)((int)IDC_SERIAL + iBoxCount),
          g_hInstance,
          NULL
        );

        // If BOXDASH, place a dash (-) after the serial box
        if (iDash == 0)
          hDash = CreateWindow(
            TEXT("STATIC"),
            TEXT_SERIAL_DASH,
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            x + 18 + w,
            iBoxTop,
            10,
            18,
            hWndDlg,
            NULL,
            g_hInstance,
            NULL
          );
        iDash = 1;

        // Set text limit
        if (iMaxLen > 0)
          SendMessage(hBox, EM_SETLIMITTEXT, iMaxLen, 0);

        // Sets the font of box to be the same as the main window
        SendMessage(hBox, WM_SETFONT, (WPARAM)SendMessage(g_hWndParent, WM_GETFONT, 0, 0), (LPARAM)TRUE);

        iBoxCount++;

      }
    }
    SetFocus(GetDlgItem(hWndDlg, IDC_SERIAL));
  break;
  case WM_COMMAND:
    if (HIWORD(wParam) == EN_CHANGE)
    {
      SerialEnableNext();
      for (i=0; i<iBoxCount; i++)
      {
        if (LOWORD(wParam) == (int)IDC_SERIAL + i)
        {

          // If the box is empty, go back to the other box
          TCHAR s[1024];
          GetWindowText(GetDlgItem(hWndDlg, LOWORD(wParam)), s, sizeof(s));
          if (!s || !*s)
          {
            if (hBox = GetDlgItem(hWndDlg, LOWORD(wParam) - 1))
              SetFocus(hBox);
          }

          // Otherwise, go to the next box if this box is full
          else
          {
            if (SendMessage(GetDlgItem(hWndDlg, LOWORD(wParam)), WM_GETTEXTLENGTH, 0, 0) >= SendMessage(GetDlgItem(hWndDlg, LOWORD(wParam)), EM_GETLIMITTEXT, 0, 0))
              if (hBox = GetDlgItem(hWndDlg, LOWORD(wParam) + 1))
                SetFocus(hBox);
          }
          break;
        }
      }
    }
    else if (HIWORD(wParam) == EN_MAXTEXT)
    {
      // Again, if this box is full, go to the next box
      if (hBox = GetDlgItem(hWndDlg, LOWORD(wParam) + 1))
        SetFocus(hBox);
      break;
    }
  break;
  case WM_CTLCOLORSTATIC:
  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORDLG:
  case WM_CTLCOLORBTN:
  case WM_CTLCOLORLISTBOX:
    // let the NSIS window handle colors, it knows best
    return SendMessage(g_hWndParent, uMsg, wParam, lParam);
  }
  return FALSE;
}

// Callback: Handles the InputBox dialog
LRESULT CALLBACK dlgInputBox(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static TCHAR szParam[1024];
  int iMaxLen, iRO = 1;
  HWND hBox, hLabel;
  DWORD dwStyle;
  RECT r;
  HFONT hFont;

  switch (uMsg)
  {
  case WM_INITDIALOG:

    SetTexts(hWndDlg);

    iBoxTop = 70;
    iBoxCount = 0;

    while (popstring(szParam) == 0)
    {
      if ((lstrcmpi(szParam, PARAM_INPUTBOX_BOX) == 0) || ((iRO = lstrcmpi(szParam, PARAM_INPUTBOX_BOXRO)) == 0))
      {
        // Box label text
        popstring(szLabelAText);
        // Box text
        popstring(szBoxAText);
        // Number of chars to fit in box
        popstring(szParam);
        iMaxLen = myatoi(szParam);

        GetClientRect(hWndDlg, &r);

        // Create the static label
        hLabel = CreateWindow(
          TEXT("STATIC"),
          szLabelAText,
          SS_LEFT | WS_CHILD | WS_VISIBLE,
          18,
          iBoxTop,
          128,
          18,
          hWndDlg,
          (HMENU)((int)IDC_LABELA + (iBoxCount*2)),
          g_hInstance,
          NULL
        );

        dwStyle = ES_LEFT | ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP;
        if (iRO == 0)
          dwStyle |= ES_READONLY;

        // Create the input box
        hBox = CreateWindow(
          TEXT("EDIT"),
          szBoxAText,
          dwStyle,
          132,
          iBoxTop,
          r.right-r.left-158,
          18,
          hWndDlg,
          (HMENU)((int)IDC_INPUTBOX + (iBoxCount*2)),
          g_hInstance,
          NULL
        );

        // Set text limit
        if (iMaxLen > 0)
          SendMessage(hBox, EM_SETLIMITTEXT, iMaxLen, 0);

        // Set fonts of label and input box to that of parent window
        hFont = (HFONT)SendMessage(g_hWndParent, WM_GETFONT, 0, 0);
        SendMessage(hBox,   WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
        SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

        iBoxTop += 24;
        iBoxCount += 1;

      }
    }

    if (iMaxLen)
      SendMessage(GetDlgItem(hWndDlg, IDC_INPUTBOX), EM_SETLIMITTEXT, iMaxLen, 0);

  break;
  case WM_CTLCOLORSTATIC:
  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORDLG:
  case WM_CTLCOLORBTN:
  case WM_CTLCOLORLISTBOX:
    // let the NSIS window handle colors, it knows best
    return SendMessage(g_hWndParent, uMsg, wParam, lParam);
  }
  return FALSE;
}

void CreateDialogs()
{
  TCHAR szParam[1024];
  popstring(szParam);

  // Set default values for texts.
  lstrcpy(szHeadingText, TEXT(""));
  lstrcpy(szGroupText, TEXT(""));
  lstrcpy(szLabelAText, TEXT("Username:"));
  lstrcpy(szLabelBText, TEXT("Password:"));
  lstrcpy(szBoxAText, TEXT(""));
  lstrcpy(szBoxBText, TEXT(""));

  GetParams();

  g_done = TRUE;

  // Get Next button handle in case we need it
  hNext = GetDlgItem(g_hWndParent, IDOK);

  // Which page to use?
  if (lstrcmpi(szParam, TEXT("userpass")) == 0)
  {
    if (!*szHeadingText)
      lstrcpy(szHeadingText, TEXT_USERPASS_HEADING);
    if (!*szGroupText)
      lstrcpy(szGroupText, TEXT_USERPASS_GROUP);

    iDialog = 0;
    hDialog = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG1), g_hWndParent, (DLGPROC)dlgUserPass);
  }
  else if (lstrcmpi(szParam, TEXT("password")) == 0)
  {
    if (!szHeadingText)
      lstrcpy(szHeadingText, TEXT_PASSWORD_HEADING);
    if (!szGroupText)
      lstrcpy(szGroupText, TEXT_PASSWORD_GROUP);

    iDialog = 1;
    hDialog = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG2), g_hWndParent, (DLGPROC)dlgPass);
  }
  else if (lstrcmpi(szParam, TEXT("serial")) == 0)
  {
    if (!szHeadingText)
      lstrcpy(szHeadingText, TEXT_SERIAL_HEADING);
    if (!szGroupText)
      lstrcpy(szGroupText, TEXT_SERIAL_GROUP);

    EnableWindow(hNext, FALSE);
    iDialog = 2;
    hDialog = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG3), g_hWndParent, (DLGPROC)dlgSerial);
  }
  else
  {
    if (!szHeadingText)
      lstrcpy(szHeadingText, TEXT_INPUTBOX_HEADING);
    if (!szGroupText)
      lstrcpy(szGroupText, TEXT_INPUTBOX_GROUP);

    iDialog = 3;
    hDialog = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG3), g_hWndParent, (DLGPROC)dlgInputBox);
  }
}

// NSIS Function: Displays dialog with username and password boxes
void __declspec(dllexport) InitDialog(HWND hWndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  Initialize(extra);
  {
    if (!bInitDialog)
    {
      TCHAR szHWND[32];

      CreateDialogs();

      // Return page HWND
      wsprintf(szHWND, TEXT("%d"), hDialog);
      pushstring(szHWND);

      bInitDialog = TRUE;

    }
    else
      pushstring(TEXT("error"));
  }
}

// NSIS Function: Displays dialog with password box
void __declspec(dllexport) Show(HWND hWndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  Initialize(extra);
  {
    if (bInitDialog)
    {
      bInitDialog = FALSE;
      ShowDialog(hDialog);
    }
    else
      pushstring(TEXT("error"));
  }
}

// NSIS Function: Displays dialog with password box
void __declspec(dllexport) Dialog(HWND hWndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  Initialize(extra);
  {
    if (!bInitDialog)
    {
      CreateDialogs();
      ShowDialog(hDialog);
    }
    else
      pushstring(TEXT("error"));
  }
}

// Entry point for DLL
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=hInst;
  return TRUE;
}