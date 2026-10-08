#include <windows.h>
#include <stdio.h>
#include <shlobj.h>
#include "..\exdll\exdll.h"
#include "resource.h"

#pragma warning( disable : 4311 )
#pragma warning( disable : 4312 )

extern "C"
{

#define NSISDIREX_KILL (WM_APP + 0)

HINSTANCE g_hInstance;

HWND hwParent;
HWND hwChild;
HWND hwNSISDirEx;
HWND hwIconText;
HWND hwDirText;
HWND hwRequiredText;
HWND hwAvailableText;
HWND hwIcon;
HWND hwDirEdit;
HWND hwBrowseButton;
HWND hwNextButton;

void* lpWndProcOld;

char dir_text[128 + 1];
char dir[MAX_PATH];
char icon_text[128 + 1];
unsigned int required;
unsigned int available;
int g_done;

BOOL CALLBACK dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ParentWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void __declspec(dllexport) Init(HWND hwndParent, int string_size, char* variables, stack_t** stacktop)
{
	char buf[64];
	int visible;

	buf[0] = 0;
	dir_text[0] = 0;
	dir[0] = 0;
	icon_text[0] = 0;

	hwParent = hwndParent;
	
	EXDLL_INIT();
	hwChild = FindWindowEx(hwndParent, NULL, "#32770", NULL); // find window to replace
	if(hwChild == NULL)
	{
		// find the main child window
		hwChild = GetDlgItem(hwndParent, 1018);
		if(hwChild == NULL)
		{
			pushstring("error finding child hwnd");
			return;
		}
	}

	hwNextButton = GetDlgItem(hwndParent,IDOK);

	// grab all the text for the windows
	popstring(icon_text);
	popstring(dir_text);
	popstring(dir);
	popstring(buf);
	required = atoi(buf);

	visible = IsWindowVisible(hwChild);
	// hide the child window
	if(visible == TRUE)
	{
		ShowWindow(hwChild, SW_HIDE);
	}

	hwNSISDirEx = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG), hwndParent, dlgProc);
	if(hwNSISDirEx == NULL)
	{
		pushstring("error creating dialog");
	}
	else
	{
		g_done = 0;
		// change the window proc to our window proc
		lpWndProcOld = (void*)SetWindowLong(hwndParent, GWL_WNDPROC, (long) ParentWndProc);
		// main message loop
		while(g_done == 0)
		{
			MSG msg;
			int nResult = GetMessage(&msg, NULL, 0, 0);
			if(IsDialogMessage(hwNSISDirEx,&msg) == FALSE && IsDialogMessage(hwndParent,&msg) == FALSE && TranslateMessage(&msg) == FALSE)
			{
				DispatchMessage(&msg);
			}
		}
		// nuke the window we made
		DestroyWindow(hwNSISDirEx);
		// reset teh window proc
		SetWindowLong(hwndParent, GWL_WNDPROC, (long) lpWndProcOld);
		// if the child was visible before show it again
		if(visible == TRUE)
		{
			ShowWindow(hwChild, SW_SHOW);
		}
	}
}

void ScreenToClientRect(RECT* rect)
{
	POINT point;

	point.x = rect->left;
	point.y = rect->top;
	ScreenToClient(hwParent, &point);
	rect->left = point.x;
	rect->top = point.y;

	point.x = rect->right;
	point.y = rect->bottom;
	ScreenToClient(hwParent, &point);
	rect->right = point.x;
	rect->bottom = point.y;
}

void FormatSize(char* str, unsigned int size)
{
	char sh=20;
	char c='G';
	char s=0;
	if(size < 1024)
	{
		sh=0;
		c='K';
	}
	else if(size < 1024*1024)
	{
		sh=10;
		c='M';
	}
	else if(GetVersion() & 0x80000000)
	{
		s='+';//only display the + on GB shown on win9x.
	}
	wsprintf(str + strlen(str),"%d.%d%cB%c", size >> sh,((size * 10) >> sh)%10, c, s);
}

static int CALLBACK WINAPI BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED)
	{
		char pathname[MAX_PATH] = "";
		SendMessage(hwDirEdit, WM_GETTEXT, MAX_PATH, (LPARAM)pathname);
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)1, (LPARAM)pathname);
	}
	return 0;
}

BOOL CALLBACK dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buf[MAX_PATH];

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			RECT dialog_r, temp_r;
			HFONT hFont = (HFONT)SendMessage(hwParent, WM_GETFONT, 0, 0);
			int yoffset = 0;
			int xoffset = 0;
			int tempheight = 0;
			HICON icon;

			GetWindowRect(hwChild, &dialog_r);
			ScreenToClientRect(&dialog_r);
			SetWindowPos(hwndDlg, 0, dialog_r.left, dialog_r.top, dialog_r.right - dialog_r.left, dialog_r.bottom - dialog_r.top, SWP_NOZORDER | SWP_NOACTIVATE);

			// get a handle to all the windows
			hwIconText = GetDlgItem(hwndDlg, IDC_ICON_TEXT);
			hwDirText = GetDlgItem(hwndDlg, IDC_DIR_TEXT);
			hwRequiredText = GetDlgItem(hwndDlg, IDC_REQUIRED_TEXT);
			hwAvailableText = GetDlgItem(hwndDlg, IDC_AVAILABLE_TEXT);
			hwIcon = GetDlgItem(hwndDlg, IDC_NSISICON);
			hwDirEdit = GetDlgItem(hwndDlg, IDC_DIR_EDIT);
			hwBrowseButton = GetDlgItem(hwndDlg, IDC_BROWSE_BUTTON);

			// reset the font on all the windows
			SendMessage(hwNSISDirEx, WM_SETFONT, (WPARAM) hFont, TRUE);
			SendMessage(hwIconText, WM_SETFONT, (WPARAM) hFont, TRUE);
			SendMessage(hwDirText, WM_SETFONT, (WPARAM) hFont, TRUE);
			SendMessage(hwRequiredText, WM_SETFONT, (WPARAM) hFont, TRUE);
			SendMessage(hwAvailableText, WM_SETFONT, (WPARAM) hFont, TRUE);
			SendMessage(hwIcon, WM_SETFONT, (WPARAM) hFont, TRUE);
			SendMessage(hwDirEdit, WM_SETFONT, (WPARAM) hFont, TRUE);
			SendMessage(hwBrowseButton, WM_SETFONT, (WPARAM) hFont, TRUE);

			// setup the icon
			icon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(103));
			if(icon != NULL)
			{
				SendMessage(hwIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);
				SetWindowPos(hwIcon, NULL, 0, 0, 32, 32, SWP_NOACTIVATE);
			}
			else
			{
				SetWindowPos(hwIcon, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW);
			}

			// start setting the positions of all the windows
			// setup the icon window
			GetWindowRect(hwIcon, &temp_r);
			temp_r.right += 5;
			temp_r.bottom += 5;
			ScreenToClientRect(&temp_r);
			xoffset = temp_r.right;
			yoffset = temp_r.top;

			// setup the icon text
			SetWindowText(hwIconText, icon_text);
			SetWindowPos(hwIconText, NULL, xoffset, yoffset, dialog_r.right - temp_r.left, temp_r.bottom, SWP_NOACTIVATE);
			yoffset += temp_r.bottom + 50;

			// setup the directory text
			GetWindowRect(hwDirText, &temp_r);
			SetWindowText(hwDirText, dir_text);
			SetWindowPos(hwDirText, NULL, xoffset, yoffset, dialog_r.right - xoffset, temp_r.bottom - temp_r.top + 5, SWP_NOACTIVATE);
			yoffset += (temp_r.bottom - temp_r.top + 10);

			// setup the directory edit
			GetWindowRect(hwDirEdit, &temp_r);
			SetWindowText(hwDirEdit, dir);
			SetWindowPos(hwDirEdit, NULL, xoffset, yoffset, dialog_r.right - dialog_r.left - xoffset - 75, temp_r.bottom - temp_r.top + 5, SWP_NOACTIVATE);
			xoffset += (dialog_r.right - dialog_r.left - xoffset - 75);
			tempheight = temp_r.bottom - temp_r.top + 5;

			// setup the browse button
			GetWindowRect(hwBrowseButton, &temp_r);
			SetWindowPos(hwBrowseButton, NULL, xoffset, yoffset, temp_r.right - temp_r.left, tempheight, SWP_NOACTIVATE);

			// setup the required space text
			GetWindowRect(hwRequiredText, &temp_r);
			yoffset = dialog_r.bottom - (((temp_r.bottom - temp_r.top) * 2) + 35);
			SetWindowPos(hwRequiredText, NULL, 0, yoffset, dialog_r.right - dialog_r.left, temp_r.bottom - temp_r.top + 5, SWP_NOACTIVATE);

			strcpy(buf, "Space Required: ");
			FormatSize(buf, required);
			SetWindowText(hwRequiredText, buf);
			
			// setup the available space text
			yoffset += (temp_r.bottom - temp_r.top) + 15;
			SetWindowPos(hwAvailableText, NULL, 0, yoffset, dialog_r.right - dialog_r.left, temp_r.bottom - temp_r.top + 5, SWP_NOACTIVATE);

			ShowWindow(hwndDlg, SW_SHOWNA);
			SetFocus(GetDlgItem(hwParent, IDOK));
		}
			break;// WM_INITDIALOG
		case WM_COMMAND:
		{
			DWORD lw = LOWORD(wParam);
			DWORD hw = HIWORD(wParam);
			// directory has changed
			if(lw == IDC_DIR_EDIT && hw == EN_CHANGE)
			{
				char buf[MAX_PATH];
				DWORD sectors_per_cluster;
				DWORD bytes_per_sector;
				DWORD number_of_free_clusters;
				DWORD total_number_of_clusters;
				buf[0] = 0;
				SendMessage(hwDirEdit, WM_GETTEXT, MAX_PATH, (LPARAM)buf);
				buf[3] = 0;
				// check to see if there is enough space on the drive
				if(GetDiskFreeSpace(buf, &sectors_per_cluster, &bytes_per_sector, &number_of_free_clusters, &total_number_of_clusters) == TRUE)
				{
					available = MulDiv(bytes_per_sector * sectors_per_cluster, number_of_free_clusters, 1 << 10);

					// enable the next button depending on whether or not we have enough space
					if(available < required)
					{
						EnableWindow(hwNextButton, FALSE);
					}
					else
					{
						EnableWindow(hwNextButton, TRUE);
					}

					strcpy(buf, "Space Available: ");
					FormatSize(buf, available);
				}
				else
				{
					buf[0] = 0;
					EnableWindow(hwNextButton, FALSE);
				}
				SetWindowText(hwAvailableText, buf);
			}
			else if(lw == IDC_BROWSE_BUTTON)
			{
				char name[MAX_PATH] = "";
				char str[256] = "Browse for folder to install to";
				BROWSEINFO bi={0,};
				ITEMIDLIST* idlist;

				// open up the windows directory browser
				SendMessage(hwDirEdit, WM_GETTEXT, MAX_PATH, (LPARAM)name);
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = name;
				bi.lpfn=BrowseCallbackProc;
				bi.lParam=(LPARAM)hwndDlg;
				bi.lpszTitle=str;
				bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

				idlist = SHBrowseForFolder(&bi);
				if(idlist != NULL)
				{
					IMalloc* m;
					SHGetPathFromIDList(idlist, name);
					SHGetMalloc(&m);
					if(m != NULL)
					{
						m->Free(idlist);
						m->Release();
					}
					SendMessage(hwDirEdit, WM_SETTEXT, 0, (LPARAM)name);
				}
			}
		}
			break;// WM_COMMAND
		case NSISDIREX_KILL:
			g_done = 1;
			if(wParam == NOTIFY_BYE_BYE)
			{
				pushstring("cancel");
			}
			else
			{
				// we hit next
				buf[0] = 0;
				GetWindowText(hwDirEdit, buf, MAX_PATH);
				// push the final directory string out
				pushstring(buf);
				// show that we succeded
				pushstring("success");
			}
			break;// NSISDIREX_KILL
		default:
			break;// default
	}

	return 0;
}

LRESULT CALLBACK ParentWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// the next button was hit, let's bail
	if(uMsg == WM_NOTIFY_OUTER_NEXT)
	{
		PostMessage(hwNSISDirEx, NSISDIREX_KILL, wParam, 0);
	}
	return CallWindowProc((long (__stdcall *)(struct HWND__ *, unsigned int, unsigned int, long))lpWndProcOld, hwndDlg, uMsg, wParam, lParam);
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = (HINSTANCE)hInst;
	return TRUE;
}

}// extern "C"
