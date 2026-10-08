#include <windows.h>
//#include <stdio.h>
#include <ctype.h>

#include "../exdll.h"
#include "resource.h"

BOOL CALLBACK nsisDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

char g_makensis_path[MAX_PATH];
char nsifilename[MAX_PATH];
char g_options[MAX_PATH];
char g_cmdline[1024];

HINSTANCE g_hInstance;
HWND g_hwndParent;
HWND g_hwnd;
HANDLE g_hThread;


void wnd_printf(const char *str) {
  UINT l;
  char *p;
  char buf[31000];
  char existing_text[32000];
  existing_text[0]=0;
  if (!*str) return;
  l=GetDlgItemText(g_hwnd, IDC_OUTPUTTEXT, existing_text, 32000);
  l+=strlen(str);

  p=existing_text;
  existing_text[31000]=0;
  while (l > 31000 && *p) {

    while (*p != '\r' && *p != '\n' && *p) {
      p++;
      l--;
    }

    while (*p == '\r' || *p == '\n') {
      p++;
      l--;
    }
  }

  lstrcpy(buf,p);
  lstrcpy(existing_text,buf);
  lstrcat(existing_text,str);

  SetDlgItemText(g_hwnd, IDC_OUTPUTTEXT, existing_text);
  SendDlgItemMessage(g_hwnd, IDC_OUTPUTTEXT, EM_LINESCROLL, 0, SendDlgItemMessage(g_hwnd, IDC_OUTPUTTEXT, EM_GETLINECOUNT, 0, 0)); // scroll to the last line of the textbox
}

void ErrorMessage(char *str) { //display detailed error info
  LPVOID msg;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &msg, 0, NULL);
  wnd_printf(str);
  wnd_printf(": ");
  wnd_printf((char*)msg);
  LocalFree(msg);
}

DWORD WINAPI ThreadProc(LPVOID p) { // thread that will start & monitor wwwinamp
  char buf[1024];           //i/o buffer

  unsigned long exit=0;  //process exit code
  unsigned long bread;   //bytes read
  unsigned long avail;   //bytes available

  STARTUPINFO si={sizeof(si),};
  SECURITY_ATTRIBUTES sa={sizeof(sa),};
  SECURITY_DESCRIPTOR sd={0,};               //security information for pipes
  PROCESS_INFORMATION pi={0,};
  HANDLE newstdout=0,read_stdout=0;         //pipe handles

  OSVERSIONINFO osv={sizeof(osv)};
  GetVersionEx(&osv);
  if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT)        //initialize security descriptor (Windows NT)
  {
    InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
    sa.lpSecurityDescriptor = &sd;
  }
  else sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;         //allow inheritable handles

  if (!CreatePipe(&read_stdout,&newstdout,&sa,0))  //create stdout pipe
  {
    ErrorMessage("CreatePipe");
    PostMessage(g_hwnd,WM_USER+1203,1,0);
    pushstring("8");
    return 1;
  }

  GetStartupInfo(&si);      //set startupinfo for the spawned process
  /*
    The dwFlags member tells CreateProcess how to make the process.
    STARTF_USESTDHANDLES validates the hStd* members. STARTF_USESHOWWINDOW
    validates the wShowWindow member.
  */
  si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;
  si.hStdOutput = newstdout;
  si.hStdError = newstdout;     //set the new handles for the child process

  // *******************************************************************
  // If there is a command line in the config file, use it for create process

  //spawn the child process
  if (!CreateProcess(NULL,g_cmdline,NULL,NULL,TRUE,CREATE_NEW_CONSOLE,
      NULL,NULL,&si,&pi))
  {
    ErrorMessage("CreateProcess");
    wnd_printf("\r\nPlease make sure the path to makensis.exe is correct.");
    CloseHandle(newstdout);
    CloseHandle(read_stdout);
    PostMessage(g_hwnd,WM_USER+1203,1,0);
    pushstring("9");
    return 2;
  }

  memset(buf,0,sizeof(buf));
  while (1)      //main program loop
  {
    PeekNamedPipe(read_stdout,buf,1023,&bread,&avail,NULL);
    //check to see if there is any data to read from stdout
    if (bread != 0)
    {
      memset(buf,0,sizeof(buf));
      if (avail > 1023)
      {
        while (bread >= 1023)
        {
          ReadFile(read_stdout,buf,1023,&bread,NULL);  //read the stdout pipe
          wnd_printf(buf);
          memset(buf,0,sizeof(buf));
        }
      }
      else
      {
        ReadFile(read_stdout,buf,1023,&bread,NULL);
        wnd_printf(buf);
      }
    }

    GetExitCodeProcess(pi.hProcess,&exit);      //while the process is running
    if (exit != STILL_ACTIVE)
      break;

    Sleep(100);
  }
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  CloseHandle(newstdout);
  CloseHandle(read_stdout);


  wnd_printf(buf);

  PostMessage(g_hwnd,WM_USER+1203,exit,0);
  buf[0]=0;
  itoa(exit, buf, 10);
  pushstring(buf);

  return 0;
}


void makeEXE()
{
  DWORD id;
  wsprintf(g_cmdline,"\"%s\" %s \"%s\"",g_makensis_path,g_options,nsifilename);

  g_hThread=CreateThread(NULL,0,ThreadProc,0,0,&id);
}

BOOL CALLBACK nsisDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  static HBRUSH g_hbrBackground;
  static HFONT hFont;
  if (uMsg == WM_DESTROY)
  {
    if (g_hbrBackground) DeleteObject(g_hbrBackground);
    g_hbrBackground=0;
    if (hFont) DeleteObject(hFont);
    hFont=0;
  }
  switch (uMsg) {

    case WM_INITDIALOG:
    {
      g_hwnd=hwndDlg;
      SetTimer(hwndDlg, 001, 100, (TIMERPROC)NULL);

      g_hbrBackground = GetSysColorBrush(COLOR_WINDOW); // CreateSolidBrush(RGB(255, 255, 255));
      hFont=CreateFont(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
        OUT_CHARACTER_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,FIXED_PITCH|FF_DONTCARE,"Courier New");

      SendDlgItemMessage(hwndDlg,IDC_OUTPUTTEXT,WM_SETFONT,(WPARAM)hFont,0);
    }
    break;

    case WM_CTLCOLORSTATIC: //WM_CTLCOLOREDIT:
    {
      if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_OUTPUTTEXT)) {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdcStatic, GetSysColor(COLOR_WINDOW));
        //SetBkMode(hdcStatic, OPAQUE); // TRANSPARENT
        return (LONG)g_hbrBackground;
      }
    }
    break;

    case WM_LBUTTONDOWN:
    {
      POINT pt;
      GetCursorPos(&pt);
      PostMessage(hwndDlg, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(pt.x, pt.y));
    }
    break;

    case WM_TIMER:
      if (KillTimer(hwndDlg, 001))
      {
        SetDlgItemText(g_hwnd, IDC_OUTPUTTEXT, "");
        makeEXE();
      }
      else
      {
        // ErrorMessage("KillTimer failed");
        PostMessage(g_hwnd,WM_USER+1203,1,0);
        pushstring("7");
      }
    break;

    case WM_CLOSE:
    {
      if (!g_hThread) EndDialog(hwndDlg,0);
    }
    break;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK)
      {
        if (!g_hThread) EndDialog(hwndDlg, 0);
      }
    break;

    case WM_USER+1203:
    {
      if (g_hThread)
      {
        CloseHandle(g_hThread);
        g_hThread=0;
      }
      EnableWindow(GetDlgItem(hwndDlg, IDOK), 1);

      if (!g_hThread && wParam == 0)
      {
        Sleep(700);
        EndDialog(hwndDlg,wParam);
      }
    }
    break;
  }
  return 0;
}

void __declspec(dllexport) nmwMakensis(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
  g_hwndParent=hwndParent;

  EXDLL_INIT();
  {
    if (popstring(g_makensis_path)) return;
    if (popstring(g_options)) return;
    if (popstring(nsifilename)) return;
    
    if (strlen(g_makensis_path)>0 && strlen(nsifilename)>0) 
    {
      DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DLGNSIS), g_hwndParent, nsisDlgProc);
    }
    else
    {
      pushstring("error");
    }
  }
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=hInst;
  return TRUE;
}


