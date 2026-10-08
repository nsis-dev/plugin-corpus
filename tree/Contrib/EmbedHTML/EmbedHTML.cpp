/*
	EmbedHTML NSIS plug-in by Stuart Welch <afrowuk@afrowsoft.co.uk>
	v1.0.0.1 - 19th October 2014
	v1.0.0.2 - 16th May 2015
	v1.0.0.3 - 2nd November 2015
*/

#pragma warning(disable: 4005)

#include <windows.h>
#include "pluginapi.h"
#include "EmbedHTML.h"
#include "EmbedHTMLBrowser.h"

HMODULE g_hInstance;
HWND g_hWndParent;
CEmbedHTMLBrowser* g_pWebBrowser;
BOOL g_bOleInitialized;
DLGPROC ChildDlgProcOld;
WNDPROC ParentWndProcOld;

static void DestroyBrowser()
{
	if (g_pWebBrowser)
	{
		g_pWebBrowser->Destroy();
		g_pWebBrowser = NULL;
	}
}

static UINT_PTR PluginCallback(enum NSPIM msg)
{
	if (msg == NSPIM_GUIUNLOAD)
	{
		DestroyBrowser();
	}
	else if (msg == NSPIM_UNLOAD)
	{
		if (g_bOleInitialized)
			OleUninitialize();
	}

	return 0;
}

static LRESULT CALLBACK ParentWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		if (g_pWebBrowser)
		{
			InvalidateRect(g_pWebBrowser->GetControlWindow(), NULL, TRUE);
		}
		break;
	}

	return CallWindowProc(ParentWndProcOld, hWnd, uMsg, wParam, lParam);
}

static INT_PTR CALLBACK ChildDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		SetWindowLongPtr(g_hWndParent, GWLP_WNDPROC, (LONG)ParentWndProcOld);
		DestroyBrowser();
		break;
	}

	return CallWindowProc((WNDPROC)ChildDlgProcOld, hWndDlg, uMsg, wParam, lParam);
}

NSISFUNC(Load)
{
	DLL_INIT();
	{
		BOOL bOK = FALSE;

		if (g_pWebBrowser == NULL)
		{
			PTCHAR pszArg = (PTCHAR)GlobalAlloc(GPTR, sizeof(TCHAR) * string_size);
			if (pszArg)
			{
				BOOL bReplaceRect = FALSE, bNoMaxEmulation = FALSE;

				while (popstring(pszArg) == 0)
				{
					if (lstrcmpi(pszArg, TEXT("/replace")) == 0)
					{
						bReplaceRect = TRUE;
					}
					else if (lstrcmpi(pszArg, TEXT("/nomaxem")) == 0)
					{
						bNoMaxEmulation = TRUE;
					}
					else
					{
						pushstring(pszArg);
						break;
					}
				}

				if (popstring(pszArg) == 0)
				{
					HWND hWndRect = (HWND)myatoi(pszArg);
					if (popstring(pszArg) == 0 && hWndRect > 0 && IsWindow(hWndRect))
					{
						RECT rc;
						GetClientRect(hWndRect, &rc);

						HWND hParent;
						if (bReplaceRect && (hParent = GetParent(hWndRect)))
						{
							GetWindowRect(hWndRect, &rc);
							MapWindowPoints(NULL, hParent, (LPPOINT)&rc, 2);
							DestroyWindow(hWndRect);
						}
						else
						{
							hParent = hWndRect;
						}
							
						if (SUCCEEDED(OleInitialize(NULL)))
							g_bOleInitialized = TRUE;

						g_pWebBrowser = new CEmbedHTMLBrowser(hParent, rc, pszArg, !bNoMaxEmulation);
						g_hWndParent = hWndParent;

						ParentWndProcOld = (WNDPROC)SetWindowLongPtr(hWndParent, GWLP_WNDPROC, (LONG)ParentWndProc);
						if (hParent != hWndParent)
							ChildDlgProcOld = (DLGPROC)SetWindowLongPtr(hParent, DWLP_DLGPROC, (LONG)ChildDlgProc);

						bOK = TRUE;
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

NSISFUNC(GetIEVersion)
{
	EXDLL_INIT();
	{
		pushint(CEmbedHTMLBrowser::GetInternetExplorerVersion());
	}
}

NSISFUNC(GetURL)
{
	EXDLL_INIT();
	{
		BOOL bSuccess = FALSE;

		if (g_pWebBrowser)
		{
			PTCHAR pszUrl = (PTCHAR)GlobalAlloc(GPTR, sizeof(TCHAR) * string_size);
			if (pszUrl)
			{
				int cbUrl = string_size - 1;
				g_pWebBrowser->GetURL(pszUrl, &cbUrl);
				pushstring(pszUrl);
				bSuccess = TRUE;
				GlobalFree(pszUrl);
			}
		}

		if (!bSuccess)
			pushstring(TEXT(""));
	}
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = (HMODULE)hInst;
	return TRUE;
}