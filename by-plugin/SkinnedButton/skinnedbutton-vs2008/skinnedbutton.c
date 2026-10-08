/* skinnedbutton.c : skinned button plugin
 * Version 1.1
 * Modified by SuperPat
 * homepage: http://www.ultramodernui.com4.ws
 *
 * based on wansis, a Plug-in written by Saivert that skins NSIS like Winamp.
 * wansis homepage: http://saivert.inthegray.com
 */

#define WINVER 0x0500

#include <windows.h>
#include <windef.h>
#include <richedit.h>
#include <commctrl.h>

//Global Variable used in wa_dlg.h
HINSTANCE g_hInstance;

//Private #includes
#include "exdll.h"
#include "wa_dlg.h"

//Private #defines
#define WANSIS_FRAME_CLASS "skinnedbutton_FrameWnd"

//Variables
HWND g_hwndParent;
WNDPROC oldProc;
LONG g_oldnsiswndstyle;
HWND g_oldnsisparentwnd;
HBITMAP g_hbmb;

char szColor[8];
COLORREF hColor;
unsigned int wRed;
unsigned int wGreen;
unsigned int wBlue;
char szRed[3];
char szGreen[3];
char szBlue[3];

//Functions
BOOL CALLBACK EnumChildProc(HWND, LPARAM);
void internal_skin(int);
char* GetLastErrorStr(void);
unsigned int myatoi(char*);
void FixMainControls();

//Implementation

// makes a smaller DLL file
BOOL WINAPI _DllMainCRTStartup(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return 1;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = hInst;
	return TRUE;
}

// THis is really a window procedure
LRESULT CALLBACK ChildDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc;
	LRESULT a, dlgresult;

	proc = (WNDPROC)GetWindowLong(hwnd, DWL_USER);
	dlgresult = CallWindowProc(proc, hwnd, uMsg, wParam, lParam);

	if (a = WADlg_handleDialogMsgs(hwnd, uMsg, wParam, lParam))
	{
		SetWindowLong(hwnd, DWL_MSGRESULT, a);
		return a;
	}
	
	/* fix for buttons unskinning itself after click under control */
	if (uMsg == WM_COMMAND) {
		FixMainControls();
	}

    return dlgresult;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT a;
	if (a = WADlg_handleDialogMsgs(hwnd, uMsg, wParam, lParam))
       return a;

	if (uMsg == WM_NOTIFY_OUTER_NEXT)
    {
		HWND dlg;

		dlg = FindWindowEx(hwnd, 0, "#32770", NULL);
		SetWindowLong(dlg, GWL_WNDPROC, GetWindowLong(dlg, DWL_USER));

		PostMessage(hwnd, WM_USER+0x9, 0, 0);
	}
    else if (uMsg == WM_USER+0x9)
    {
		HWND dlg;
		LONG oldChildDlgProc;

		EnumChildWindows(hwnd, EnumChildProc, 0);

		if (!wParam)
		{
			dlg = FindWindowEx(hwnd, 0, "#32770", NULL);
			if (dlg)
			{
				EnumChildWindows(dlg, EnumChildProc, 0);
				oldChildDlgProc = SetWindowLong(dlg, GWL_WNDPROC, (LONG) ChildDlgProc);
				SetWindowLong(dlg, DWL_USER, (LONG) oldChildDlgProc);
			}
		}

		InvalidateRect(hwnd, NULL, TRUE);		
	}

	return CallWindowProc(oldProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HMENU pm;
	static HWND tt; //Tooltip control

	switch (uMsg)
	{
    	case WM_DESTROY:
    		DestroyWindow(tt);
    
    		SetWindowLong(g_hwndParent, GWL_WNDPROC, (LONG) oldProc);
    		SetWindowLong(g_hwndParent, GWL_STYLE, g_oldnsiswndstyle);
    		SetParent(g_hwndParent, g_oldnsisparentwnd);
    
    		break;
    		
    	case WM_CLOSE:
    		SendMessage(g_hwndParent, WM_SYSCOMMAND, SC_CLOSE, 0);
    		return 0;
    
    	case WM_PAINT:
		{
			PAINTSTRUCT ps;

			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);

			return 0;
		}
	}
	return CallWindowProc(DefWindowProc, hwnd, uMsg, wParam, lParam);
}

// reskin the button when we clic on "Brownce" in the directory page
void FixMainControls()
{
	LONG style;
#define MakeButtonOwnerdraw(hwnd) style = GetWindowLong(hwnd, GWL_STYLE);\
	SetWindowLong(hwnd, GWL_STYLE, (style & ~BS_PUSHBUTTON) | BS_OWNERDRAW);

	MakeButtonOwnerdraw(GetDlgItem(g_hwndParent, 3));
	MakeButtonOwnerdraw(GetDlgItem(g_hwndParent, 2));
	MakeButtonOwnerdraw(GetDlgItem(g_hwndParent, 1));
	MakeButtonOwnerdraw(GetDlgItem(g_hwndParent, 1027));  // Show Detail button

#undef MakeButtonOwnerdraw

}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	char classname[256];

	GetClassName(hwnd, classname, 255);

	if (!lstrcmpi(classname, "BUTTON"))
	{
		LONG style;

		style = GetWindowLong(hwnd, GWL_STYLE);

		if ( ((style & BS_GROUPBOX) == 0) || (GetParent(hwnd) == g_hwndParent) )
			SetWindowLong(hwnd, GWL_STYLE, (style & ~BS_PUSHBUTTON) | BS_OWNERDRAW);
	}
	UpdateWindow(hwnd);
	return TRUE;
}



void __declspec(dllexport) skinit(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	internal_skin(0);
}

void __declspec(dllexport) setskin(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	internal_skin(1);
}

void internal_skin(int isUpdating)
{
	char fnb[MAX_PATH];
	char *szLastErr;

	popstring(fnb); //get parameter, BITMAP image

	if (GetFileAttributes(fnb) == 0xFFFFFFFF)
	{
		szLastErr = GetLastErrorStr();
		pushstring(szLastErr);
		LocalFree((HLOCAL)szLastErr);// Free the buffer.
		return;
	}

	if (isUpdating)
	{
		WADlg_close();
		if (g_hbmb)
               DeleteObject(g_hbmb);
	}

	g_hbmb=LoadImage(NULL,fnb,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);

	if (!g_hbmb) 
	{
		szLastErr = GetLastErrorStr();
		pushstring(szLastErr);
		LocalFree((HLOCAL)szLastErr);// Free the buffer.
		return;
	}
	
   	// Modification Slappy, 2010	
	popstring(szColor);
	hColor = RGB(0, 0, 0);

	// Correct color format: 0xRRGGBB	
	if(strlen(szColor) == 8)
	{
		
		szRed[0] = szColor[2];
		szRed[1] = szColor[3];
		
		szGreen[0] = szColor[4];
		szGreen[1] = szColor[5];
		
		szBlue[0] = szColor[6];
		szBlue[1] = szColor[7];
		szRed[2] = szGreen[2] = szBlue[2] = 0;

		sscanf(szRed,"%x", &wRed);
		sscanf(szGreen,"%x", &wGreen);
		sscanf(szBlue,"%x", &wBlue);
		hColor = RGB(wRed, wGreen, wBlue);
	}	
  
	WADlg_init(g_hbmb, hColor);

	if (!isUpdating)
	{
		SetWindowLong(g_hwndParent, GWL_EXSTYLE, WS_EX_CONTROLPARENT);

		oldProc = (WNDPROC) SetWindowLong(g_hwndParent, GWL_WNDPROC, (LONG) WndProc);
		PostMessage(g_hwndParent, WM_USER+0x9, 0, 0);
		
	    ShowWindow(g_hwndParent, SW_SHOW);
	}

	pushstring("success");
}

void __declspec(dllexport) unskinit(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	// Free graphics stuff
	WADlg_close();
	if (g_hbmb)
           DeleteObject(g_hbmb);
}

// U T I L I T Y   F U N C T I O N S

char* GetLastErrorStr(void)
{
	LPVOID lpMsgBuf;
 
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	return (char*)lpMsgBuf;
}


unsigned int myatoi(char *s)
{
  unsigned int v=0;

  for (;;)
  {
    unsigned int c=*s++;
    if (c >= '0' && c <= '9') c-='0';
    else break;
    v*=10;
    v+=c;
  }
  return v;
}
