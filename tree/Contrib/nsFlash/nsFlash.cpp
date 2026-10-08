/*
	nsFlash NSIS plug-in by Stuart Welch <afrowuk@afrowsoft.co.uk>
	v1.0.0.1 - 7th January 2013
*/

#include <windows.h>
#include <Shlwapi.h>
#include "nsFlash.h"
#include "resource.h"

// You can import directly from Flash.ocx or just use the interface I have defined in Flash.h
//#define IMPORT_FLASH_TYPELIB
#ifdef IMPORT_FLASH_TYPELIB
	#pragma warning(disable: 4192)
	#import "C:\Windows\System32\Macromed\Flash\Flash.ocx" rename_namespace("Flash")
#else
	#include "Flash.h"
#endif

#ifdef UNICODE
#include "nsis_unicode\pluginapi.h"
#else
#include "nsis_ansi\pluginapi.h"
#endif

HMODULE g_hInstance;
Flash::IShockwaveFlash* g_pFlash = NULL;
HWND g_hWndActiveX;
PTCHAR g_pszSWFPath;

PAtlAxWinInit AtlAxWinInit = NULL;
PAtlAxAttachControl AtlAxAttachControl = NULL;

static UINT_PTR PluginCallback(enum NSPIM msg)
{
	if (msg == NSPIM_GUIUNLOAD && g_pFlash)
	{
		DestroyWindow(g_hWndActiveX);
		g_pFlash->Release();
		g_pFlash = NULL;
	}
	return 0;
}

static BOOL AtlInit()
{
	if (AtlAxWinInit && AtlAxAttachControl)
		return TRUE;

	HINSTANCE hDLL = LoadLibrary(TEXT("atl.dll"));
	if (hDLL)
	{
		AtlAxWinInit = (PAtlAxWinInit)GetProcAddress(hDLL, "AtlAxWinInit");
		AtlAxAttachControl = (PAtlAxAttachControl)GetProcAddress(hDLL, "AtlAxAttachControl");
		if (AtlAxWinInit && AtlAxAttachControl)
			return AtlAxWinInit();
	}
						
	return FALSE;
}

static HRESULT FlashLoadMove(const PTCHAR pszSWFPath)
{
	HRESULT res = E_OUTOFMEMORY;

#ifdef UNICODE
	BSTR bsSWFPath = SysAllocString(pszSWFPath);
	if (bsSWFPath)
	{
		res = g_pFlash->raw_LoadMovie(0, bsSWFPath);
		SysFreeString(bsSWFPath);
	}
#else
	int len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszSWFPath, -1, NULL, 0);
	if (len > 0)
	{
		BSTR bsSWFPath = SysAllocStringLen(NULL, len);
		if (bsSWFPath)
		{
			if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszSWFPath, -1, bsSWFPath, len) > 0)
				res = g_pFlash->raw_LoadMovie(0, bsSWFPath);
			SysFreeString(bsSWFPath);
		}
	}
#endif

	return res;
}

static BOOL FlashInit(const HWND hRect, const PTCHAR pszSWFPath, const BOOL bReplaceRect)
{
	RECT rc;
	GetClientRect(hRect, &rc);

	HWND hParent;
	if (bReplaceRect)
	{
		hParent = GetParent(hRect);
		GetWindowRect(hRect, &rc);
		MapWindowPoints(NULL, hParent, (LPPOINT)&rc, 2);
		DestroyWindow(hRect);
	}
	else
	{
		hParent = hRect;
	}

	g_hWndActiveX = CreateWindowEx(0, TEXT("AtlAxWin"), NULL, WS_CHILD | WS_VISIBLE, rc.left, rc.top, rc.right, rc.bottom, hParent, NULL, g_hInstance, NULL);
	if (g_hWndActiveX)
	{
		if (SUCCEEDED(CoCreateInstance(__uuidof(Flash::ShockwaveFlash), NULL, CLSCTX_ALL, __uuidof(Flash::IShockwaveFlash), (void**)&g_pFlash)))
		{
			if (SUCCEEDED(AtlAxAttachControl(g_pFlash, g_hWndActiveX, NULL)) &&
				SUCCEEDED(FlashLoadMove(pszSWFPath)))
			{
				g_pFlash->put_Menu(VARIANT_FALSE);
				SetFocus(g_hWndActiveX);
				return TRUE;
			}

			g_pFlash->Release();
			g_pFlash = NULL;
		}
	}

	return FALSE;
}

NSISFUNC(Load)
{
	DLL_INIT();
	{
		BOOL bOK = FALSE;

		if (g_pFlash == NULL)
		{
			PTCHAR pszArg = (PTCHAR)GlobalAlloc(GPTR, sizeof(TCHAR) * string_size);
			if (pszArg)
			{
				if (popstring(pszArg) == 0)
				{
					BOOL bReplaceRect = lstrcmpi(pszArg, TEXT("/replace")) == 0;
					if (!bReplaceRect)
					{
						pushstring(pszArg);
					}
					
					if (popstring(pszArg) == 0)
					{
						HWND hWndRect = (HWND)myatoi(pszArg);
						if (popstring(pszArg) == 0 && hWndRect > 0)
						{
							if (AtlInit() && FlashInit(hWndRect, pszArg, bReplaceRect))
							{
								bOK = TRUE;
							}
						}
					}
				}
			
				GlobalFree(pszArg);
			}
		}

		if (!bOK)
		{
			extra->exec_flags->exec_error = 1;
		}
	}
}

NSISFUNC(Destroy)
{
	DLL_INIT();
	{
		if (g_pFlash)
		{
			DestroyWindow(g_hWndActiveX);
			g_pFlash->Release();
			g_pFlash = NULL;
		}
		else
		{
			extra->exec_flags->exec_error = 1;
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		return FlashInit(hWnd, g_pszSWFPath, FALSE) ? 0 : -1;

	case WM_SIZE:
		MoveWindow(g_hWndActiveX, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;

	case WM_GETMINMAXINFO:
		{
			PMINMAXINFO mmi = (PMINMAXINFO)lParam;
			mmi->ptMinTrackSize.x = NSISSWFWND_WIDTH_MIN;
			mmi->ptMinTrackSize.y = NSISSWFWND_HEIGHT_MIN;
		}
		return 0;

	case WM_PAINT:
	case WM_ERASEBKGND:
		ValidateRect(hWnd, NULL);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		DestroyWindow(g_hWndActiveX);
		DestroyWindow(hWnd);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

NSISFUNC(Window)
{
	DLL_INIT();
	{
		BOOL bOK = FALSE;

		if (g_pFlash == NULL)
		{
			g_pszSWFPath = (PTCHAR)GlobalAlloc(GPTR, sizeof(TCHAR) * string_size);
			if (g_pszSWFPath)
			{
				if (popstring(g_pszSWFPath) == 0)
				{
					int width = NSISSWFWND_WIDTH, height = NSISSWFWND_HEIGHT;

					if (lstrcmpi(g_pszSWFPath, TEXT("/size")) == 0 && popstring(g_pszSWFPath) == 0)
					{
						width = myatoi(g_pszSWFPath);
						if (width < NSISSWFWND_WIDTH_MIN)
							width = NSISSWFWND_WIDTH_MIN;

						PTCHAR t = g_pszSWFPath;
						for (int i = 0; i < string_size && *t != NULL; i++, t++)
						{
							if (*t == TEXT('x') || *t == TEXT('X'))
							{
								height = myatoi(++t);
								if (height < NSISSWFWND_HEIGHT_MIN)
									height = NSISSWFWND_HEIGHT_MIN;
								break;
							}
						}
					}
					else
					{
						pushstring(g_pszSWFPath);
					}

					if (popstring(g_pszSWFPath) == 0)
					{
						PTCHAR pszCaption = (PTCHAR)GlobalAlloc(GPTR, sizeof(TCHAR) * string_size);
						if (pszCaption)
						{
							if (popstring(pszCaption) == 0)
							{
								if (AtlInit())
								{
									PWNDCLASSEX pwc = (PWNDCLASSEX)GlobalAlloc(GPTR, sizeof(WNDCLASSEX));
									if (pwc)
									{
										pwc->cbSize = sizeof(WNDCLASSEX);
										pwc->lpfnWndProc = WndProc;
										pwc->hInstance = g_hInstance;
										pwc->hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(103));
										pwc->lpszClassName = TEXT("NSISSWFWND");

										if (RegisterClassEx(pwc))
										{
											HWND hWnd = CreateWindowEx(0, pwc->lpszClassName, pszCaption, WS_OVERLAPPEDWINDOW, (GetSystemMetrics(SM_CXSCREEN) - width) / 2, (GetSystemMetrics(SM_CYSCREEN) - height) / 2, width, height, HWND_DESKTOP, NULL, g_hInstance, NULL);
											if (hWnd)
											{
												ShowWindow(hWnd, SW_SHOWNORMAL);
     
												MSG msg;
												while(GetMessage(&msg, NULL, 0, 0))
												{
													TranslateMessage(&msg);
													DispatchMessage(&msg);
												}

												g_pFlash->Release();
												g_pFlash = NULL;
											}
										}

										GlobalFree(pwc);
									}
								}
							}

							GlobalFree(pszCaption);
						}
					}
				}

				GlobalFree(g_pszSWFPath);
			}
		}

		if (!bOK)
		{
			extra->exec_flags->exec_error = 1;
		}
	}
}

NSISFUNC(IsInstalled)
{
	DLL_INIT();
	{
		Flash::IShockwaveFlash* pFlash = NULL;
		if (SUCCEEDED(CoCreateInstance(__uuidof(Flash::ShockwaveFlash), NULL, CLSCTX_ALL, __uuidof(Flash::IShockwaveFlash), (void**)&pFlash)))
		{
			pFlash->Release();
			pushstring(TEXT("yes"));
		}
		else
		{
			pushstring(TEXT("no"));
		}
	}
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = (HMODULE)hInst;
	return TRUE;
}