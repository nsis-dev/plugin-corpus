#include <windows.h>
#include "..\ExDLL\exdll.h"

#define BUTTON_TEXT_FONTSIZE  14
#define BUTTON_TEXT_SHOWPARAM TEXT("/SHOWTEXT")
#define BUTTON_TEXT_HIDEPARAM TEXT("/HIDETEXT")
#define BUTTON_TEXT_SHOW      TEXT("Show Details")
#define BUTTON_TEXT_HIDE      TEXT("Hide Details")

#define BUTTON_RIGHT_PARAM    TEXT("/RIGHT")

#define IDC_SHOWBUTTON  1027
#define IDC_NEXTBUTTON  1
#define IDC_MODERNUI    1037
#define IDC_PRGSWND     1016
#define IDC_PRGSBAR     1004

#define BUTTON_WIDTH    84
#define BUTTON_HEIGHT   20

#define PROGRESS_HEIGHT 20

#define ICON_SPACE      32
#define CONTROL_SPACE   4

#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLong
#endif

#ifndef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLong
#endif

#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC GWL_WNDPROC
#endif

#ifndef DWLP_DLGPROC
#define DWLP_DLGPROC DWL_DLGPROC
#endif

HINSTANCE	g_hInstance;
UINT		uMsgCreate;
BOOL		bButtonRight;
HWND		g_hwndParent, hPrgsWnd, hButton, childwnd;
WNDPROC lpWndProcOld;
DLGPROC lpDlgProcOld;
char		szButtonShowText[1024], szButtonHideText[1024], szParam[1024];

LRESULT CALLBACK wndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == uMsgCreate)
	{
		RECT wndRect, ctsRect;
		HWND hPrgsBar;
		int iButtonLeft, iPrgsBarLeft, iPrgsBarWidth;

		hPrgsBar = GetDlgItem(childwnd, IDC_PRGSBAR);

		// Find out if we're using NSIS Modern UI or default
		// Set control sizes accordingly
		GetClientRect(childwnd, &wndRect);
		if (bButtonRight == TRUE)
		{
			if (GetDlgItem(g_hwndParent, IDC_MODERNUI))
				iPrgsBarLeft = CONTROL_SPACE;
			else
				iPrgsBarLeft = ICON_SPACE + CONTROL_SPACE;
			iButtonLeft   = wndRect.right - CONTROL_SPACE - BUTTON_WIDTH;
			iPrgsBarWidth = iButtonLeft - iPrgsBarLeft - CONTROL_SPACE;
		}
		else
		{
			if (GetDlgItem(g_hwndParent, IDC_MODERNUI))
				iButtonLeft = CONTROL_SPACE;
			else
				iButtonLeft = ICON_SPACE + CONTROL_SPACE;
			iPrgsBarLeft  = iButtonLeft + BUTTON_WIDTH + CONTROL_SPACE;
			iPrgsBarWidth = wndRect.right - iPrgsBarLeft - CONTROL_SPACE;
		}

		GetClientRect(hPrgsBar, &ctsRect);

		// Setup show/hide button (uses existing button control)
		hButton  = GetDlgItem(childwnd, IDC_SHOWBUTTON);
		hPrgsWnd = GetDlgItem(childwnd, IDC_PRGSWND);
		SetWindowPos(hButton, NULL, iButtonLeft, ctsRect.bottom, BUTTON_WIDTH, BUTTON_HEIGHT, (UINT)NULL);
		if (!IsWindowVisible(hPrgsWnd))
			SetWindowText(hButton, szButtonShowText);
		else
			SetWindowText(hButton, szButtonHideText);

		// Show the new button
		ShowWindow(hButton, SW_SHOWNA);

		// Resize progress bar
		SetWindowPos(hPrgsBar, NULL, iPrgsBarLeft, ctsRect.bottom, iPrgsBarWidth, PROGRESS_HEIGHT, (UINT)NULL);
		
		return TRUE;
	}
	else // Otherwise, call other window procedure
		return CallWindowProc((WNDPROC)lpWndProcOld,hwndDlg,uMsg,wParam,lParam);
}

BOOL CALLBACK dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_COMMAND)
	{
		if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == IDC_SHOWBUTTON )
			{
				// Hide details window
				if (IsWindowVisible(hPrgsWnd))
				{
					ShowWindow(hPrgsWnd, SW_HIDE);
					SetWindowText(hButton, (char *)szButtonShowText);
				}
				else // Show details window
				{
					ShowWindow(hPrgsWnd, SW_SHOW);
					SetWindowText(hButton, (char *)szButtonHideText);
				}
			}
		}
		return TRUE;
	}
	else // Otherwise, call other window procedure
		return CallWindowProc((WNDPROC)lpDlgProcOld,hwndDlg,uMsg,wParam,lParam);
}

extern "C"
void __declspec(dllexport) Set(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hwndParent=hwndParent;
  EXDLL_INIT();
  {

	childwnd     = FindWindowEx(hwndParent, NULL, "#32770", NULL);
	bButtonRight = FALSE;

	// Check for /SHOWTEXT and /HIDETEXT param
	while (popstring(szParam) == 0)
	{
		if (lstrcmpi(szParam, BUTTON_RIGHT_PARAM) == 0) {
			bButtonRight = TRUE;
		} else if (lstrcmpi(szParam, BUTTON_TEXT_HIDEPARAM) == 0) {
			popstring(szButtonHideText);
		} else if (lstrcmpi(szParam, BUTTON_TEXT_SHOWPARAM) == 0) {
			popstring(szButtonShowText);
		} else {
			pushstring(szParam);
			break;
		}
	}

	// If no params sent, use defaults
	if (!*szButtonShowText)
		lstrcpy(szButtonShowText, BUTTON_TEXT_SHOW);
	if (!*szButtonHideText)
		lstrcpy(szButtonHideText, BUTTON_TEXT_HIDE);

	// Apply new window and dialog procedures
	uMsgCreate = RegisterWindowMessage("toggleinstfiles create");
	lpWndProcOld = (WNDPROC)SetWindowLongPtr(childwnd, GWLP_WNDPROC, (LONG)wndProc);
	SendMessage(childwnd, uMsgCreate, TRUE, (LPARAM)childwnd);
	lpDlgProcOld = (DLGPROC)SetWindowLongPtr(childwnd, DWLP_DLGPROC, (LONG)dlgProc);

  }
}

extern "C"
void __declspec(dllexport) Unload(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  EXDLL_INIT();
  {
	  // Apply old window and dialog procedures
		SetWindowLongPtr(childwnd, GWLP_WNDPROC,  (LONG)lpWndProcOld);
		SetWindowLongPtr(childwnd, DWLP_DLGPROC, (LONG)lpDlgProcOld);
		SendMessage(childwnd, uMsgCreate, FALSE, (LPARAM)childwnd);
  }
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=(HINSTANCE)hInst;
	return TRUE;
}