#include <windows.h>
#include "..\ExDll\exdll.h"
#include "resource.h"

/**
    PopupListBox v0.1 by Afrow UK
    A plugin dll for NSIS that shows a small popup dialog with a listbox control.

    Last modified: 2nd February 06
*/

// This is the parameter to set the heading text.
#define PARAM_HEADINGTEXT "/HEADINGTEXT"
// Set dialog caption.
#define PARAM_CAPTION     "/CAPTION"
// Stop popping items from stack.
#define PARAM_ENDSTACK    "/END"
// Apply the WS_EX_TOOLWINDOW style.
#define PARAM_TOOLWINDOW  "/TOOLWINDOW"
// Can only select one list box item.
#define PARAM_MULTISELECT "/MULTISELECT"

// Default heading text.
#define HEADINGTEXT_LABEL "Please select from the list box..."

// Nothing selected return value.
#define SUCCESS           "SUCCESS"
#define CANCEL            "CANCEL"

#define MAX_STRLEN 1024

// Global variables.
HINSTANCE g_hInstance;
HWND g_hWndParent;
BOOL g_bMultiSel;

void *WINAPI MALLOC(int len) { return (void*)GlobalAlloc(GPTR,len); }
void WINAPI FREE(void *d) { if (d) GlobalFree((HGLOBAL)d); }

// Handles our popup dialog.
static LRESULT CALLBACK DlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hList;
  BOOL bHeadingTextSet = FALSE, bCaptionSet = FALSE;
	static char szData[MAX_STRLEN];
  int iSelIndex;
  LONG lStyle;

	switch (uMsg)
	{
	case WM_INITDIALOG:

		hList = GetDlgItem(hWndDlg, IDC_LISTBOX);

		while (popstring(szData) == 0)
		{
      // Stop popping items from stack.
      if (lstrcmpi(szData, (LPCTSTR)PARAM_ENDSTACK) == 0)
        break;
      // Set heading label text.
      else if (lstrcmpi(szData, (LPCTSTR)PARAM_HEADINGTEXT) == 0)
      {
        popstring(szData);
        SetWindowText(GetDlgItem(hWndDlg, IDC_HEADINGTEXT), (LPCTSTR)szData);
        bHeadingTextSet = TRUE;
      // Apple the WS_EX_TOOLWINDOW style.
      }
      else if (lstrcmpi(szData, (LPCTSTR)PARAM_TOOLWINDOW) == 0)
      {
        lStyle = GetWindowLongPtr(hWndDlg, GWL_EXSTYLE);
        lStyle |= WS_EX_TOOLWINDOW;
        SetWindowLongPtr(hWndDlg, GWL_EXSTYLE, lStyle);
      }
      // Set dialog caption,
      else if (lstrcmpi(szData, (LPCTSTR)PARAM_CAPTION) == 0)
      {
        popstring(szData);
        SetWindowText(hWndDlg, szData);
        bCaptionSet = TRUE;
      }
      // Otherwise add the item to list box.
      else
			  SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)szData);
		}

    if (!bHeadingTextSet)
      SetWindowText(GetDlgItem(hWndDlg, IDC_HEADINGTEXT), (LPCTSTR)HEADINGTEXT_LABEL);

    if (!bCaptionSet)
    {
      GetWindowText(g_hWndParent, szData, sizeof(szData));
      SetWindowText(hWndDlg, szData);
    }

    if (g_bMultiSel)
      SendMessage(hList, LB_SETSEL, TRUE, 0);
    else
      SendMessage(hList, LB_SETCURSEL, 0, 0);

	break;
  // Command button clicked.
	case WM_COMMAND:

		hList = GetDlgItem(hWndDlg, IDC_LISTBOX);

    // Clicked OK button.
		if (LOWORD(wParam) == IDOK)
    {
      // Multiple items selected.
      if (g_bMultiSel)
      {
        int *iArrSelIndexes;
        iSelIndex = SendMessage(hList, LB_GETSELCOUNT, 0, 0);

        if (iSelIndex != 0)
        {
          iArrSelIndexes = (int*)MALLOC(iSelIndex);
          // Get array of selected item indexes.
          SendMessage(hList, LB_GETSELITEMS, (WPARAM)iSelIndex, (LPARAM)iArrSelIndexes);
          pushstring(PARAM_ENDSTACK);
          for (int i=0; i<iSelIndex; i++)
          {
            // Push each item onto stack.
            SendMessage(hList, LB_GETTEXT, (WPARAM)iArrSelIndexes[i], (LPARAM)szData);
            pushstring(szData);
          }
          pushstring(SUCCESS);
          FREE(iArrSelIndexes);
        }
        else
          pushstring(CANCEL);
      }
      else
      {
        iSelIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
        iSelIndex = SendMessage(hList, LB_GETTEXT, (WPARAM)iSelIndex, (LPARAM)szData);
        // No item selected.
        if (iSelIndex == LB_ERR)
          pushstring(CANCEL);
        // Return selected item.
        else
        {
          pushstring(szData);
          pushstring(SUCCESS);
        }
      }

			EndDialog(hWndDlg, 0);
    }
    // Clicked Cancel button.
    else if (LOWORD(wParam) == IDCANCEL)
    {
      pushstring(CANCEL);
      EndDialog(hWndDlg, 0);
    }

	break;
  // Clicked X button.
	case WM_CLOSE:
    pushstring(CANCEL);
	  EndDialog(hWndDlg, 0);
	}

	return FALSE;
}

// NSIS Function: Displays dialog.
extern "C" void __declspec(dllexport) MultiSelect(HWND hWndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  {
    g_bMultiSel = TRUE;
		DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_MULTISELECT), hWndParent, (DLGPROC)DlgProc);
	}
}

// NSIS Function: Displays dialog.
extern "C" void __declspec(dllexport) SingleSelect(HWND hWndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  {
    g_bMultiSel = FALSE;
		DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_SINGLESELECT), hWndParent, (DLGPROC)DlgProc);
	}
}

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance = hInst;
	return TRUE;
}
