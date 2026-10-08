/***************************************************
* FILE NAME: animate.cpp
*
* Copyright 2008 - Present NSIS
*
* PURPOSE:
*    Slide - Roll - Splash screen in NSIS installers
*
* CHANGE HISTORY
*
* Author Date           Modifications
*
* Takhir Bedertdinov
*        July  8, 2008   Primary version
*        July 14, 2008   Animation on window close
*        July 27, 2008   /IFNC option for 'wait'
**************************************************/


#define WINVER 0x0500
#include <windows.h>
#include <olectl.h>
#include <stdio.h>
#include "exdll.h"
#include <tchar.h>

#define COLOR _T("/COLOR=")
#define NOCANCEL _T("/NOCANCEL")
#define IFNC _T("/IFNC")
#define BORDER _T("/BORDER")
#define SFG _T("/SFG")
#define ATIME _T("/ATIME=")
#define FLAGS _T("/FLAGS=")
#define X     _T("/X=")
#define Y     _T("/Y=")
#define TOO_BIG 100000
#define SLEEP_QUANTUM 100

const TCHAR classname[4] = _T("_sp");
HINSTANCE g_hInstance;
HANDLE hThread = NULL;
LPPICTURE pIPicture = NULL;
static WNDCLASS wc = {0,NULL,0,0,NULL,NULL,NULL,NULL,NULL,NULL};
HWND myWnd = NULL;
bool animFinished = false, set_fg = false, nocancel = false, clicked = false, ifnc = false;
int x = TOO_BIG, y = TOO_BIG;
DWORD anim_time = 1000, wstyle = WS_POPUP, bgColor;

// NT 3.5 - 4.0 compatibility patch
typedef BOOL (__stdcall *ANIM_WND)(HWND, DWORD, DWORD dwFlags);
ANIM_WND myAnimateWindow = NULL;
DWORD flags = AW_BLEND;//advsplash compatibility

int myatoi(TCHAR *s)
{
	unsigned int v=0;
	if (*s == _T('0') && (s[1] == _T('x') || s[1] == _T('X')))
	{
		s+=2;
		for (;;)
		{
			int c=*s++;
			if (c >= _T('0') && c <= _T('9')) c-=_T('0');
			else if (c >= _T('a') && c <= _T('f')) c-=_T('a')-10;
			else if (c >= _T('A') && c <= _T('F')) c-=_T('A')-10;
			else break;
			v<<=4;
			v+=c;
		}
	}
	else if (*s == _T('0') && s[1] <= _T('7') && s[1] >= _T('0'))
	{
		s++;
		for (;;)
		{
			int c=*s++;
			if (c >= _T('0') && c <= _T('7')) c-=_T('0');
			else break;
			v<<=3;
			v+=c;
		}
	}
	else
	{
		int sign=0;
		if (*s == _T('-')) { s++; sign++; }
		for (;;)
		{
			int c=*s++ - _T('0');
			if (c < 0 || c > 9) break;
			v*=10;
			v+=c;
		}
		if (sign) return -(int) v;
	}
	return (int)v;
}

/*****************************************************
 * FUNCTION NAME: sf()
 * PURPOSE: 
 *    window proc. 
 * SPECIAL CONSIDERATIONS:
 *    for paint purposes mainly.
 *****************************************************/
void sf(HWND hw)
{
   DWORD ctid = GetCurrentThreadId();
   DWORD ftid = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
   AttachThreadInput(ftid, ctid, TRUE);
   SetForegroundWindow(hw);
   AttachThreadInput(ftid, ctid, FALSE);
}

/*****************************************************
 * FUNCTION NAME: WndProc()
 * PURPOSE: 
 *    window proc. 
 * SPECIAL CONSIDERATIONS:
 *    for paint purposes mainly.
 *****************************************************/
long __stdcall WndProc(HWND hwnd,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT r;
	long w, h;
	
	switch (uMsg)
	{
		
	case WM_PAINT:
		BeginPaint(hwnd,&ps);
	case WM_PRINTCLIENT:
		if(pIPicture != NULL)
		{
			GetClientRect(hwnd,&r);
			pIPicture->get_Width(&w);
			pIPicture->get_Height(&h);
			pIPicture->Render(uMsg == WM_PAINT ? ps.hdc : (HDC)wParam,
				r.left, r.top,	r.right - r.left, r.bottom - r.top, 0, h, w, -h, &r);
		}
		if(uMsg == WM_PAINT)
			EndPaint(hwnd,&ps);
		return 0;
		
		// AnimateWindow() in progress don't like this
	case WM_LBUTTONUP:
		clicked = true;
		if(nocancel || !animFinished)
			break;
	case WM_USER:
		DestroyWindow(hwnd);
		break;
		
	case WM_CLOSE:
		return 0;
		
	}
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

/*****************************************************
 * FUNCTION NAME: animateThread()
 * PURPOSE: 
 *    modal/modeless banner presentation
 * SPECIAL CONSIDERATIONS:
 *    from CreateThread runs window "detached", with NULL
 *    parent, otherwise uses current page as parent
 *****************************************************/
DWORD WINAPI animateThread(LPVOID lpParameter)
{
	HWND hParent = (HWND)lpParameter;
	RECT r;
	MSG msg;
	HBITMAP g_hbm;
	BITMAP bm;
	int bdr;
	
	wc.lpfnWndProc = WndProc;
	wc.hInstance = g_hInstance;
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.lpszClassName = classname;
	wc.hbrBackground = CreateSolidBrush(bgColor);
  if (!RegisterClass(&wc))
		return 1;

// create window and get border size, then increase size
	pIPicture->get_Handle((OLE_HANDLE*)&g_hbm);
	GetObject((HGDIOBJ)g_hbm, sizeof(BITMAP), &bm);
	myWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		classname, _T(""), wstyle, 
		0, 0, bm.bmWidth, bm.bmHeight,
		hParent, NULL, g_hInstance, 0);
	GetClientRect(myWnd, &r);
	bdr = (bm.bmWidth - r.right + r.left) / 2;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
	if(x == TOO_BIG)
		x = r.left+(r.right-r.left-bm.bmWidth)/2;
	else if(x < 0)
		x = r.right - bm.bmWidth + x - bdr * 2;
	if(y == TOO_BIG)
		y = r.top+(r.bottom-r.top-bm.bmHeight)/2;
	else if(y < 0)
		y = r.bottom - bm.bmHeight + y - bdr * 2;
	SetWindowPos(myWnd, 0, x, y, bm.bmWidth + bdr * 2, bm.bmHeight + bdr * 2, SWP_NOZORDER);
	SetWindowLong(myWnd, GWL_STYLE, GetWindowLong(myWnd, GWL_STYLE) & ~WS_BORDER);

	while (IsWindow(myWnd) && GetMessage(&msg,myWnd,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	pIPicture->Release();
	pIPicture = NULL; // do not re-use with /NOUNLOAD
	myWnd = NULL;

	return 0;
}

/*****************************************************
 * FUNCTION NAME: parse_params()
 * PURPOSE: 
 *    plug-in parameters parsing
 * SPECIAL CONSIDERATIONS:
 *    not all params valid in 'wait' call
 *****************************************************/
void parse_params(TCHAR *fn)
{
	TCHAR buf[64];
	while(popstring(fn) == 0 && *fn == _T('/'))
	{
		lstrcpyn(buf, fn, lstrlen(ATIME) + 1);
		if (!lstrcmpi(buf, ATIME))
		{
			anim_time=myatoi(fn + lstrlen(ATIME));
			continue;
		}
		lstrcpyn(buf, fn, lstrlen(FLAGS) + 1);
		if (!lstrcmpi(buf, FLAGS))
		{
			flags=myatoi(fn + lstrlen(FLAGS));
			continue;
		}
		lstrcpyn(buf, fn, lstrlen(X) + 1);
		if (!lstrcmpi(buf, X))
		{
			x=myatoi(fn + lstrlen(X));
			continue;
		}
		lstrcpyn(buf, fn, lstrlen(Y) + 1);
		if (!lstrcmpi(buf, Y))
		{
			y=myatoi(fn + lstrlen(Y));
			continue;
		}
		lstrcpyn(buf, fn, lstrlen(COLOR) + 1);
		if(!lstrcmpi(buf, COLOR))
		{
			bgColor = myatoi(fn + lstrlen(COLOR));
			bgColor = (bgColor & 0x00ff00) |
				(bgColor & 0x0000ff) << 16 | (bgColor & 0xff0000) >> 16;
			continue;
		}
		if (!lstrcmpi(fn, SFG))
		{
			set_fg = true;
			continue;
		}
		if (!lstrcmpi(fn, BORDER))
		{
			wstyle = 0;
			continue;
		}
		if (!lstrcmpi(fn, NOCANCEL))
		{
			nocancel = true;
			continue;
		}
		if (!lstrcmpi(fn, IFNC))
		{
			ifnc = true;
		}
	}
}
/*****************************************************
 * FUNCTION NAME: show()
 * PURPOSE: 
 *    image banner "show" dll entry point
 * SPECIAL CONSIDERATIONS:
 *    
 *****************************************************/
extern "C"
void __declspec(dllexport) show(HWND hwndParent,
                                int string_size,
                                TCHAR *variables,
                                stack_t **stacktop)
{
	TCHAR fn[MAX_PATH];
	WCHAR wcPort[MAX_PATH];
	DWORD dwThreadId;
	HINSTANCE hInstance;
	
	EXDLL_INIT();
	
	bgColor = GetSysColor(COLOR_WINDOW);
	parse_params(fn);

#ifdef _UNICODE
	lstrcpy(wcPort, fn);
#else
   MultiByteToWideChar(CP_ACP,0,fn,-1, wcPort,sizeof(wcPort)/2);
#endif
   OleLoadPicturePath((LPOLESTR)wcPort, 0, 0, 0, IID_IPicture, (void**)&pIPicture);
   if (pIPicture == NULL)
	{
		pushstring(_T("error"));
    return;
	}

// NT 4.0 patch
	if(hInstance = GetModuleHandle(_T("user32.dll")))
		myAnimateWindow = (ANIM_WND)GetProcAddress(hInstance, "AnimateWindow");

// start thread and wait for window initialization
	hThread = CreateThread(0, 0, animateThread, NULL, 0, &dwThreadId);
	if(hThread != NULL)
	{
		while (myWnd == NULL)
			Sleep(SLEEP_QUANTUM);

		if(flags == 0 ||
			myAnimateWindow == NULL ||
			myAnimateWindow(myWnd, anim_time, flags) == 0)
		{
			ShowWindow(myWnd, SW_NORMAL);
		}
		animFinished = true;
	}
	pushstring(_T("show"));
}


/*****************************************************
 * FUNCTION NAME: wait()
 * PURPOSE: 
 *    image banner "wwait" dll entry point
 * SPECIAL CONSIDERATIONS:
 *    parameter - ms to wait
 *****************************************************/
extern "C"
void __declspec(dllexport) wait(HWND hwndParent,
                                int string_size,
                                TCHAR *variables,
                                stack_t **stacktop)
{
	TCHAR s[MAX_PATH];

	animFinished = false;
	parse_params(s);
	if(clicked && ifnc)
		lstrcpy(s, _T("0"));

	if(hThread == NULL)
	{
		pushstring(_T("error"));
	}
	else
	{
		if(WaitForSingleObject(hThread, myatoi(s)) == WAIT_OBJECT_0)
		{
				pushstring(_T("click")); // user closed it with left mouse click
		}
		else // still running
		{
			if(flags != 0 &&
				myAnimateWindow != NULL)
				myAnimateWindow(myWnd, anim_time, flags | AW_HIDE);
			SendMessage(myWnd, WM_USER, 0, 0);
			if(WaitForSingleObject(hThread, SLEEP_QUANTUM) == WAIT_TIMEOUT)
			{
				// closed now, but we remeber user click in /NOCANCEL mode
				pushstring(clicked ? _T("click") : _T("wait"));
			}
			else
			{
				TerminateThread(hThread, (DWORD)-1); // possible situation?
				pushstring(_T("terminate"));
			}
		}
		CloseHandle(hThread);
		hThread = NULL;
	}
	UnregisterClass(wc.lpszClassName, g_hInstance);
	if(hwndParent && set_fg)
		sf(hwndParent);
}

/*****************************************************
 * FUNCTION NAME: hwnd()
 * PURPOSE: 
 *    retrieves target window handle
 * SPECIAL CONSIDERATIONS:
 *    not sure this func is really need
 *****************************************************/
extern "C"
void __declspec(dllexport) hwnd(HWND hwndParent,
                                int string_size,
                                TCHAR *variables,
                                stack_t **stacktop)
{
   TCHAR s[16] = _T("");
   if(IsWindow(myWnd))
      wsprintf(s, _T("%d"), myWnd);
   pushstring(s);
}


/*****************************************************
 * FUNCTION NAME: DllMain()
 * PURPOSE: 
 *    Dll main (initialization) entry point
 * SPECIAL CONSIDERATIONS:
 *    
 *****************************************************/
BOOL WINAPI DllMain(HANDLE hInst,
                    ULONG ul_reason_for_call,
                    LPVOID lpReserved)
{
   g_hInstance = (HINSTANCE)hInst;
   return TRUE;
}


