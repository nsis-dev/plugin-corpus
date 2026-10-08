#define _WIN32_IE 0x0600
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include "nsis\pluginapi.h" /* This means NSIS 2.42 or higher is required. */

// Unicode port and x64 tweaks by Jason Ross aka JasonFriday13.

#ifdef UNICODE
#  define ShellNotifyIconStr "Shell_NotifyIconW"
#else
#  define ShellNotifyIconStr "Shell_NotifyIconA"
#endif

typedef HRESULT (CALLBACK *_tDllGetVersionProc)(DLLVERSIONINFO *pdvi);
typedef BOOL (__stdcall *_tShell_NotifyIconProc)(DWORD dwMessage, PNOTIFYICONDATA lpdata);

HINSTANCE g_hInstance;
LONG_PTR oldWndProc = 0;
_tShell_NotifyIconProc Shell_NotifyIconProc;
_tDllGetVersionProc DllGetVersion;
HANDLE hwnd;
int iIconClicked = 0; // 0 = no, 1 = hide, 2 = show
BOOL bCheckIconClick = FALSE, Initialized = FALSE;

int iconPresent, defaultTimeout, optShowTBMinimized, optShowNINormal, optHideNIMinimized;
PNOTIFYICONDATA NID;
TCHAR *ProgressMessage;

void CleanUp();

/*  Our plugin callback so our plugin stays loaded all the time. */
UINT_PTR __cdecl NSISPluginCallback(enum NSPIM Event) 
{
  if (Event == NSPIM_GUIUNLOAD)
    CleanUp();

  return 0;
}

void Initialize(extra_parameters* xp)
{
  if (!Initialized)
  {
    xp->RegisterPluginCallback(g_hInstance, NSISPluginCallback);
    Initialized = TRUE;
  }
}

void PrepareState(int state)
{
    NID->uFlags = NIF_STATE | NIF_MESSAGE;
    NID->dwStateMask = NIS_HIDDEN;
      if (state)
        NID->dwState = (optHideNIMinimized)?(NIS_HIDDEN):(0);
      else
        NID->dwState = (!optShowNINormal)?(NIS_HIDDEN):(0);
}

void WindowTaskButton(int state, BOOL Clicked)    // minimized - (state == 1)
{
    if (state)
    {
        ShowWindow(hwnd, SW_MINIMIZE);
        iIconClicked = 1;
        if (optShowTBMinimized || (!iconPresent)) ShowWindow(hwnd, SW_SHOW);
        else ShowWindow(hwnd, SW_HIDE);
    } else
    {
        iIconClicked = 2;
        ShowWindow(hwnd, SW_SHOW);
        OpenIcon(hwnd);
    }
    PrepareState(state);
    if (iconPresent)
        Shell_NotifyIcon(NIM_MODIFY, NID);
    if (bCheckIconClick && iconPresent && Clicked)
      SendMessage((HWND)hwnd, WM_NOTIFY_OUTER_NEXT, 1, 0);
}

void CleanUp()
{
    // window is destoring, clean up everything
    if (NID)
    {
        WindowTaskButton(IsIconic(hwnd), FALSE);
        NID->uFlags = 0;
        Shell_NotifyIconProc(NIM_DELETE, NID);
        GlobalFree(NID);
        NID = NULL;
    }
    iconPresent = 0;
    
    if (ProgressMessage)
    {
        KillTimer(hwnd, 666);
        GlobalFree(ProgressMessage);
        ProgressMessage = NULL;
    }
    if (oldWndProc)
    {
      SetWindowLongPtr(hwnd, GWLP_WNDPROC, oldWndProc);
      oldWndProc = 0;
    }
    iIconClicked = 0;
    bCheckIconClick = FALSE;
}

LRESULT CALLBACK IconWndProc(
    HWND hwnd,        // handle to window
    UINT uMsg,        // message identifier
    WPARAM wParam,    // first message parameter
    LPARAM lParam)    // second message parameter
{ 
    if ((uMsg == WM_APP + 999) && ((lParam == WM_LBUTTONDOWN) || (lParam == WM_RBUTTONDOWN) || (lParam == WM_LBUTTONDBLCLK)))
    {
       WindowTaskButton(!IsIconic(hwnd), TRUE);
       return TRUE;
    }
    else if ((uMsg == WM_SIZE) && ((wParam == SIZE_MINIMIZED) || (wParam == SIZE_RESTORED)))
    {
       WindowTaskButton(wParam == SIZE_MINIMIZED, FALSE);
    }
    else if ((uMsg == WM_TIMER) && (wParam == 666) && (ProgressMessage) && (iconPresent))
    {
        // our progress timer
        HWND hwPb, childwnd;
        PBRANGE pbr;
        int prc;
        INT_PTR pos;
        childwnd=FindWindowEx(hwnd,NULL,_T("#32770"),NULL);
        hwPb = GetDlgItem(childwnd, 1004);
        
        SendMessage(hwPb, PBM_GETRANGE, 0, (LPARAM)&pbr);
        pos = SendMessage(hwPb, PBM_GETPOS, 0, 0);
        prc = (int)((pos*100)/(pbr.iHigh-pbr.iLow));
        wsprintf(NID->szTip, ProgressMessage, prc);

        NID->uFlags = NIF_STATE | NIF_MESSAGE | NIF_TIP;
        Shell_NotifyIcon(NIM_MODIFY, NID);
    }
    else if (uMsg == WM_DESTROY)
    {
        // cleanup everything
        CleanUp();        
    }
    return CallWindowProc((WNDPROC)oldWndProc, hwnd, uMsg, wParam, lParam);
}

void __declspec(dllexport) Icon(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters* xp)
{
  int i, fnum, op, msg = ((iconPresent)?(NIM_MODIFY):(NIM_ADD)), action;
  TCHAR *flags, *buffer;
  int IconDimensions = 16;

  if (!NID) return; /* Prevents legacy nsis 'remove' code from crashing. */

  EXDLL_INIT();
  Initialize(xp);

  flags = GlobalAlloc(GPTR, sizeof(TCHAR)*g_stringsize);
  buffer = GlobalAlloc(GPTR, sizeof(TCHAR)*g_stringsize);

  hwnd = NID->hWnd = hwndParent;
  PrepareState(IsIconic(hwnd));
  NID->hIcon = NULL;

  // icon flags
  popstring(flags);
  fnum = lstrlen(flags);  
  action = 1;
  for (i = 0; i < fnum; i++)
  {
      op = 0;
      switch (flags[i])
      {
      // messages
      case 'a': case 'A':  msg = NIM_ADD; break;
      case 'r': case 'R':  msg = NIM_DELETE; break;
      case 'm': case 'M':  msg = NIM_MODIFY; break;

      // icons
      case '.':
          IconDimensions = 0; break;
      case 'h': case 'H':
          NID->hIcon = (HICON)popintptr(); break;  
      case 'i': case 'I':  
          op = 1;
      case 's': case 'S':  
          NID->hIcon = (HICON)popintptr();
          NID->hIcon = LoadImage(((!op)?(0):(GetModuleHandle(NULL))), 
              (LPCTSTR)NID->hIcon, IMAGE_ICON, IconDimensions, IconDimensions, LR_SHARED);
          break;
      case 'f': case 'F':
          popstring(buffer);
          NID->hIcon = LoadImage(0, 
              buffer, IMAGE_ICON, IconDimensions, IconDimensions, LR_LOADFROMFILE|LR_SHARED);
          break;

      // click event
      case 'c': case 'C':
          bCheckIconClick = TRUE;
          break;

      // return click event
      case 'd': case 'D':
          wsprintf(buffer, _T("%i"), iIconClicked);
          iIconClicked = 0;
          pushstring(buffer);
          break;

      // tip
      case 't': case 'T':
          popstring(NID->szTip);
          NID->uFlags |= NIF_TIP;
          break;

      // balloon tooltips
      case 'n': case 'N':
          op = NIIF_INFO;
      case 'w': case 'W':
          if (!op) op = NIIF_WARNING;
      case 'e': case 'E':
          if (!op) op = NIIF_ERROR;
      case 'b': case 'B':
          NID->uFlags |= NIF_INFO;
          NID->dwInfoFlags = op;
          popstring(NID->szInfoTitle);
          popstring(NID->szInfo);
          NID->uTimeout = defaultTimeout;
          break;
      
      // options
      case 'o': case 'O':
          defaultTimeout = popint();
          break;

      // not operator
      case '!': action = 1-action; break;

      case 'x': case 'X':
          optShowTBMinimized = action;
          action = 1;
          break;
      case 'y': case 'Y':
          optShowNINormal = action;
          action = 1;
          break;
      case 'z': case 'Z':
          optHideNIMinimized = action;
          action = 1;
          break;

      // progress indication support
      case 'p': case 'P':
          if (action)
          {
            if (ProgressMessage == NULL)
                SetTimer(hwnd, 666, 1000, NULL);
            popstring(ProgressMessage = GlobalAlloc(GPTR, sizeof(TCHAR)*g_stringsize));
          } else
          {
            if (ProgressMessage != NULL)
            {
                KillTimer(hwnd, 666);
                GlobalFree(ProgressMessage);
                ProgressMessage = NULL;
            }
          }
          break;
      }
  }
  // icon work - icon given? 
  if (NID->hIcon != 0) NID->uFlags |= NIF_ICON;

  if (msg != NIM_DELETE) iconPresent = 1;
  else 
      iconPresent = 0;
  Shell_NotifyIconProc(msg, NID);

  // Subclass window
  if ((oldWndProc == 0) && (iconPresent))
  {
      oldWndProc = GetWindowLongPtr(hwnd, GWLP_WNDPROC);
      SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)IconWndProc);
  }

  // May be we should react to changes
  WindowTaskButton(IsIconic(hwnd), FALSE);

  GlobalFree(flags);
  GlobalFree(buffer);
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  if (ul_reason_for_call == DLL_PROCESS_ATTACH)
  {
    DLLVERSIONINFO dvi;
    HMODULE shldll = GetModuleHandle(_T("shell32.dll"));
    DllGetVersion = (_tDllGetVersionProc)GetProcAddress(shldll, "DllGetVersion");
    Shell_NotifyIconProc = (_tShell_NotifyIconProc)GetProcAddress(shldll, ShellNotifyIconStr);
    dvi.cbSize = sizeof(DLLVERSIONINFO);
    DllGetVersion(&dvi);

    g_hInstance=hInst;
    iconPresent = 0;
    defaultTimeout = 10000;
    optShowTBMinimized = 0;
    optShowNINormal = 0;
    optHideNIMinimized = 0;

    ProgressMessage = NULL;

    // prepare NotifyIconData Structure
    NID = GlobalAlloc(GPTR, sizeof(NOTIFYICONDATA));
    NID->uID = 0;
    NID->dwStateMask = 0;
    NID->uFlags = NIF_MESSAGE;
    NID->uCallbackMessage = WM_APP + 999;  
    NID->cbSize = (dvi.dwMajorVersion >= 5)?(sizeof(NOTIFYICONDATA)):(NOTIFYICONDATA_V1_SIZE);

    // inform shell controls we want old messaging style
    NID->uVersion = 0;
    Shell_NotifyIconProc(NIM_SETVERSION, NID);
  }    
  if (ul_reason_for_call == DLL_PROCESS_DETACH)
  {
    // cleanup everything
    CleanUp();        
  }
  return TRUE;
}

#ifdef _DEBUG
void main()
{
}
#endif