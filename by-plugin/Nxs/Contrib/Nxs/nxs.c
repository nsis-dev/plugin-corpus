/***************************************************
* FILE NAME: nxs.c
*
* PURPOSE:
*    NSIS plug-in, displays banner with Cancel button
*
* CHANGE HISTORY
*
* $LOG$
*
* Author              Date          Modifications
* saivert             2005-03-09    Original
*    http://nsis.sourceforge.net/archive/profile.php?userid=690
* Takhir Bedertdinov  2005-04-01    cross-dialog fixes
*    Moscow, Russia, ineum@narod.ru
*  June 03, 2011 - UNICODE confiruation added
**************************************************/

//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "ExDll.h"
#include "resource.h"

#ifndef PBM_SETMARQUEE
#define PBM_SETMARQUEE  (WM_USER + 10)
#define PBS_MARQUEE  0x08
#endif

HINSTANCE hInstance;
HWND hwBanner = NULL;
HWND hwndParent = NULL;
HANDLE hThread = NULL;
BOOL bFailed;
BOOL bClickedCancel;
TCHAR buf[1024];
BOOL marquee = 0;

int my_atoi(TCHAR *s)
{
  unsigned int v=0;
  int sign=1; // sign of positive
  TCHAR m=10; // base of 0
  TCHAR t=_T('9'); // cap top of numbers at 9

  if (*s == _T('-'))
  {
    s++;  //skip over -
    sign=-1; // sign flip
  }

  if (*s == _T('0'))
  {
    s++; // skip over 0
    if (s[0] >= _T('0') && s[0] <= _T('7'))
    {
      m=8; // base of 8
      t=_T('7'); // cap top at 7
    }
    if ((s[0] & ~0x20) == _T('X'))
    {
      m=16; // base of 16
      s++; // advance over 'x'
    }
  }

  for (;;)
  {
    int c=*s++;
    if (c >= _T('0') && c <= t) c-=_T('0');
    else if (m==16 && (c & ~0x20) >= _T('A') && (c & ~0x20) <= _T('F')) c = (c & 7) + 9;
    else break;
    v*=m;
    v+=c;
  }
  return ((int)v)*sign;
}

void updateDlg(HWND hDlg)
{
   TCHAR bb[1024];
   HWND hBar = GetDlgItem(hDlg, IDC_PROGRESS);
	while (!popstring(buf) && lstrcmpi(buf, _T("/end")) != 0)
   {
      if (lstrcmpi(buf, _T("/sub")) == 0) {
				popstring(buf);
            GetDlgItemText(hDlg, IDC_SUBTEXT, bb, sizeof(bb));
            if(lstrcmpi(bb, buf) != 0)
				   SetDlgItemText(hDlg, IDC_SUBTEXT, buf);
			} else if (lstrcmpi(buf, _T("/top")) == 0) {
				popstring(buf);
				SetDlgItemText(hDlg, IDC_MAINTEXT, buf);
			} else if (lstrcmpi(buf, _T("/h")) == 0) {
				int hidden;
            DWORD gwl = GetWindowLong(hDlg, GWL_EXSTYLE);

				popstring(buf);
				hidden = my_atoi(buf);
				if (hidden)
				   SetWindowLong(hDlg, GWL_EXSTYLE, gwl | WS_EX_TOOLWINDOW);
				else
					SetWindowLong(hDlg, GWL_EXSTYLE, gwl & ~WS_EX_TOOLWINDOW);
			} else if (lstrcmpi(buf, _T("/pos")) == 0) {
				int progress;

				popstring(buf);
				progress=my_atoi(buf);
				SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETPOS, progress, 0);
			} else if (lstrcmpi(buf, _T("/max")) == 0) {
				int rangemax;

				popstring(buf);
				rangemax=my_atoi(buf);
				SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, rangemax));
			} else if (lstrcmpi(buf, _T("/can")) == 0) {
				int enable;

				popstring(buf);
				enable = my_atoi(buf);
				EnableWindow(GetDlgItem(hDlg, IDCANCEL), enable);
			} else if (lstrcmpi(buf, _T("/marquee")) == 0) {
				popstring(buf);
				marquee = my_atoi(buf);
				if(marquee)
				   SetWindowLong(hBar,GWL_STYLE, GetWindowLong(hBar,GWL_STYLE) | PBS_MARQUEE); 
				else
				   SetWindowLong(hBar,GWL_STYLE, GetWindowLong(hBar,GWL_STYLE) & ~PBS_MARQUEE); 
				SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETMARQUEE, (WPARAM)marquee, (LPARAM)marquee );
			} else {
   				SetWindowText(hDlg, buf);
			}
	}
}

BOOL CALLBACK BannerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	if (uMsg == WM_INITDIALOG)
   {
		SendDlgItemMessage(hwndDlg, IDC_ICON1, STM_SETICON, (WPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(103)), 0);
		updateDlg(hwndDlg);
      return TRUE;
	}
   else if (uMsg == WM_COMMAND && LOWORD(wParam)==IDCANCEL && HIWORD(wParam)==BN_CLICKED) {
		++bClickedCancel;
	} else if (uMsg == WM_CLOSE) {

		DestroyWindow(hwndDlg);
	}
	return 0;
}

DWORD WINAPI BannerThread(LPVOID lpParameter) {
	MSG msg;

// has not parent, good for .onInit and cross-dialog, but
// may be problems with foreground and minimize states
	hwBanner = CreateDialog(
		hInstance,
		MAKEINTRESOURCE(IDD_MSI),
		NULL,
		BannerProc
		);
	if (hwBanner == NULL) {
		bFailed = TRUE;
		return 0;
	}
//   ShowWindow(hwBanner, SW_NORMAL);

	while (IsWindow(hwBanner) &&
      GetMessage(&msg, NULL, 0, 0))
   {
      if(!IsDialogMessage(hwBanner, &msg))
      {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
      }

	}

	hwBanner = NULL;

	return 0;
}

void __declspec(dllexport) Show(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop)
{
   DWORD dwThreadId;
	DWORD dwMainThreadId = GetCurrentThreadId();
	EXDLL_INIT();
		
		hwBanner = NULL;
		bClickedCancel = FALSE;
		
		bFailed = FALSE;
		
		hThread = CreateThread(0, 0, BannerThread, NULL, 0, &dwThreadId);
		
		// wait for the window to initalize and for the stack operations to finish
		while (hThread &&
         (hwBanner == NULL || !IsWindowVisible(hwBanner)) &&
         !bFailed) {
			Sleep(10);
		}

		CloseHandle(hThread);

		if (AttachThreadInput(dwMainThreadId, dwThreadId, TRUE)) {
			// Activates and displays a window
			ShowWindow(hwBanner, SW_SHOW);
			AttachThreadInput(dwMainThreadId, dwThreadId, FALSE);
		}
		else
			ShowWindow(hwBanner, SW_SHOW);

}


void __declspec(dllexport) Update(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop)
{
   WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
   EXDLL_INIT();
   updateDlg(hwBanner);
   if(hwndParent != NULL && hwndParent == GetForegroundWindow() && IsWindow(hwBanner))
   {
      AttachThreadInput(GetWindowThreadProcessId(hwndParent,NULL),
                        GetCurrentThreadId(),TRUE);
      SetForegroundWindow(hwBanner);
      SetWindowPos(hwndParent, hwBanner, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE);
      AttachThreadInput(GetWindowThreadProcessId(hwndParent,NULL),
                        GetCurrentThreadId(),FALSE);
   }
}


void __declspec(dllexport) getWindow(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop)
{
	wsprintf(buf, _T("%u"), hwBanner);
	pushstring(buf);
}


void __declspec(dllexport) HasUserAborted(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop)
{
	EXDLL_INIT();

	{
		if (bClickedCancel)
			pushstring(_T("1"));
		else
			pushstring(_T("0"));
	}
}

void __declspec(dllexport) Destroy(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop)
{
	if(hwBanner != NULL && IsWindow(hwBanner))
   {
      PostMessage(hwBanner, WM_CLOSE, 0, 0);
	   // Wait for the thread to finish
	   while (hwBanner)
		   Sleep(25);
   }
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved) {
	hInstance = hInst;
	if (hwBanner && ul_reason_for_call == DLL_PROCESS_DETACH) {
		Destroy(0, 0, 0, 0);
	}
	return TRUE;
}
