/*//AKMAKE:2//{
	__Targets:[{CPU:"AMD64"}], Type:"WinAPI-Library,Charset=Unicode",
	Sources:"%*SELF*%", Libs:"kernel32 user32 advapi32 shell32 uuid sysmsvcrt ntdll",
	"? $_MSC_VER":{CPPFlags:{Add:"/O1s /Oy /Gz /GF /W3 /Gy /LD /Zl"}, LinkFlags:{Add:"/NOLOGO /OPT:REF /OPT:ICF,99 /MERGE:.rdata=.text /ignore:4104 dumpstate.res"}},
	"? $_MSC_VER >= 1300":{CPPFlags:{Add:"/GL"}, LinkFlags:{Add:"/VERBOSE:SAFESEH /LARGEADDRESSAWARE"}},
	"? $_MSC_VER && $_MSC_VER < 1400":{CPPFlags:{Add:"/GX- /Og"}},
	"? $_MSC_VER >= 1400":{CPPFlags:{Add:"/GS- /GR- /EHs-c-"},LinkFlags:{Add:"/MANIFEST:NO"}}, "? $_MSC_VER >= 1400 && !$_WIN64":{LinkFlags:{Add:"/NXCOMPAT"}},
	"? $_MSC_VER && $_MSC_VER < 1600":{LinkFlags:{Add:"/OPT:NOWIN98"}},
	"? 'DEF'":{CPPDef:{Add:['WINVER=0x501', 'WINVERMIN=0x500', 'NO_COMCTL32_MEMFUNCS=1']}}
}//AKMAKE:END//
*/
#include "util.c"
/**
 *
 * dumpstate.c
 * A little debug tool for use as an NSIS extension DLL.
 *
 * Copyright (C) Andrew Francis, 2003. Portions Copyright (C) Nullsoft, Inc.
 * See README.TXT for copyright information relating to modification and
 * redistribution.
 *
 */

// we can't just dump data into the listbox as it gets narky about large strings.
// so define a limit at which we truncate it and append a warning, total length
// > 1024 seems to cause badness
#define CONF_BIGSTRING_MAX 200
#define CONF_BIGSTRING_SUBST TEXT(" [ ... continues ...]")

#include <windows.h>
#include "util.h"
#include "exdll.h"
#include "resource.h"

NSIS_TCHAR *g_clipboardText = NULL;
HINSTANCE g_hInstance;
HWND g_hwndParent;
extra_parameters *g_pXP;
void *g_DlgData;

static const NSIS_TCHAR *g_regNames[] = {
TEXT("$0"),TEXT("$1"),TEXT("$2"),TEXT("$3"),TEXT("$4"),TEXT("$5"),TEXT("$6"),TEXT("$7"),TEXT("$8"),TEXT("$9"),
TEXT("$R0"),TEXT("$R1"),TEXT("$R2"),TEXT("$R3"),TEXT("$R4"),TEXT("$R5"),TEXT("$R6"),TEXT("$R7"),TEXT("$R8"),TEXT("$R9"),
TEXT("$CMDLINE"),TEXT("$INSTDIR"),TEXT("$OUTDIR"),TEXT("$EXEDIR"),TEXT("$LANGUAGE")
};
static const NSIS_TCHAR *getuservarname(int n) { ASS(n < sizeof(g_regNames) / sizeof(NSIS_TCHAR*)); return g_regNames[n]; }

static const NSIS_TCHAR g_flagCount = 14, *g_flagNames[] = {
	TEXT("autoclose"),TEXT("all_user_var"),TEXT("exec_error"),TEXT("abort"),TEXT("exec_reboot"),TEXT("reboot_called"),TEXT("cur_insttype?"),
	TEXT("plugin_api_version"),TEXT("silent"),TEXT("instdir_error"),TEXT("rtl"),TEXT("errlvl"),TEXT("alter_reg_view"),TEXT("status_update") 
};
#define GetFlag(idx) ( ((int*)(g_pXP->exec_flags))[idx] )

static BOOL VerifyStringSize(HWND hOwner) {
	BOOL selfUnicode = sizeof(NSIS_TCHAR) > 1, valid = IsWindow(g_hwndParent) ? selfUnicode == !!IsWindowUnicode(g_hwndParent) : TRUE;
	if (!valid && MessageBox(hOwner, TEXT("String size mismatch?\n\nContinue?"), NULL, MB_YESNO|MB_ICONWARNING) == IDYES) valid = TRUE;
	return valid;
}

static FARPROC GetModuleProcAddress(LPCSTR Mod, LPCSTR FN) { return GetProcAddress(LoadLibraryA(Mod), FN); }
static INT_PTR ReturnDlgResult(HWND hDlg, INT_PTR Res) { return SetWindowLongPtr((hDlg), DWLP_MSGRESULT, (Res)) | TRUE; }

static BOOL MyIsOS(UINT q) {
	const FARPROC fp = GetModuleProcAddress("SHLWAPI", (char*) 437);
	return fp ? ((BOOL(WINAPI*)(UINT))fp)(q) : (BOOL)(UINT_PTR) fp;
}
static BOOL IsWow64() { return sizeof(void*) < 8 && MyIsOS(30); }

static void SetUnicodeSymbolText(HWND hDlg, UINT CtlId, UINT Codpt) {
	if (sizeof(NSIS_TCHAR) > 1 && Codpt <= 0xffff) {
		UINT16 buf[10];
		buf[0] = (UINT16) Codpt, buf[1] = L'\0';
		SendMessage(GetDlgItem(hDlg, CtlId), WM_SETTEXT, 0, (LPARAM) buf);
	}
}

static LPTSTR LVGetItemTextAndLParam(HWND hLV, UINT Item, UINT SubItem, LPTSTR Buf, UINT cchCap, LPARAM*pLP) {
	LVITEM lvi;
	lvi.mask = LVIF_TEXT|LVIF_PARAM;
	lvi.iItem = Item, lvi.iSubItem = SubItem;
	lvi.pszText = Buf, lvi.cchTextMax = cchCap;
	return (ListView_GetItem(hLV, &lvi), *pLP = lvi.lParam, lvi.pszText);
}
static LPTSTR LVGetItemText(HWND hLV, UINT Item, UINT SubItem, LPTSTR Buf, UINT cchCap) {
	LPARAM lp;
	return LVGetItemTextAndLParam(hLV, Item, SubItem, Buf, cchCap, &lp);
}

typedef struct { 
	WNDPROC OrgEditProc;
	HWND hLV;
} LVSUBITEMEDIT;

static LRESULT CALLBACK LVEditSubItemProc(HWND hWnd, UINT Msg, WPARAM wp, LPARAM lp)
{
	LVSUBITEMEDIT *pLVSIE = (LVSUBITEMEDIT*) g_DlgData;
	switch(Msg) {
		case WM_WINDOWPOSCHANGING: {
			WINDOWPOS *pWP = (WINDOWPOS*) lp;
			RECT r;
			GetClientRect(pLVSIE->hLV, &r); // The ListView hides the item label when editing so there is no point trying to only cover the appropriate subitem
			pWP->x = (2), pWP->cx = r.right - ((2) << 1);
		}
		return 0;
	}
	return CallWindowProc(pLVSIE->OrgEditProc, hWnd, Msg, wp, lp);
}

typedef struct {
	const NSIS_TCHAR *name;
	NSIS_TCHAR *buffer;
	BOOL changed;
} editstring_t;

INT_PTR CALLBACK DlgProcEdit(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	editstring_t *e = (editstring_t*) GetWindowLongPtr(hwnd, DWLP_USER);

	switch(msg) {
	case WM_INITDIALOG: {
		HWND hValue = GetDlgItem(hwnd, IDC_EDITVALUE);
		SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR) (e = (editstring_t*) lp));

		SendMessage(GetDlgItem(hwnd, IDC_EDITNAME), WM_SETTEXT, 0, (LPARAM)(e->name));
		SendMessage(hValue, WM_SETTEXT, 0, (LPARAM)(e->buffer));
		SendMessage(hValue, EM_SETSEL, (WPARAM)0, (LPARAM)(-1));
		}
		return TRUE;

	case WM_COMMAND:
		if(LOWORD(wp) == IDOK) {
			// make sure our string isn't too long
			UINT_PTR n = SendMessage(GetDlgItem(hwnd, IDC_EDITVALUE), WM_GETTEXTLENGTH, 0, 0);
			if (n + 1 >= g_stringsize) {
				MessageBoxA(hwnd, "String too long!", NULL, MB_OK|MB_ICONWARNING);
				return TRUE;
			}

			// actually get it back
			if (SendMessage(GetDlgItem(hwnd, IDC_EDITVALUE), WM_GETTEXT, (WPARAM)(n+1), (LPARAM)(e->buffer)) != n) {
				MessageBoxA(hwnd, "couldn't get string", NULL, MB_OK|MB_ICONSTOP);
				return TRUE;
			}

			// mark as changed and return
			e->changed = TRUE;
			return EndDialog(hwnd, 0);
		}
		
		if (LOWORD(wp) == IDCANCEL) {
			e->changed = FALSE;
			return EndDialog(hwnd, 0);
		}
		break;
	}
	return FALSE;
}

BOOL EditString(HWND hOwner, const NSIS_TCHAR *name, NSIS_TCHAR *buffer) {
	editstring_t e;
	e.name = name;
	e.buffer = buffer;
	e.changed = FALSE;
	DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_EDITSTRING), hOwner, DlgProcEdit, (LPARAM) &e);
	return e.changed;
}

static INT_PTR CALLBACK DlgProcFlags(HWND hDlg, UINT Msg, WPARAM wp, LPARAM lp) {
	LVSUBITEMEDIT *pLVSIE = (LVSUBITEMEDIT*) g_DlgData;
	enum { WM_SETITEMFLAGVALUE = WM_APP, COL_VALUE = 1 };

	switch(Msg) {
		case WM_INITDIALOG: {
			HWND hCtl = GetDlgItem(hDlg, IDC_LIST);
			LVCOLUMN lvc;
			UINT i, c, lvs_ex_infotip = 0x00000400, lvs_ex_labeltip = 0x00004000 ,lvs_ex_doublebuffer = 0x00010000;
			g_DlgData = pLVSIE = (LVSUBITEMEDIT*) lp, pLVSIE->hLV = hCtl;

			ListView_SetExtendedListViewStyle(hCtl, LVS_EX_FULLROWSELECT|lvs_ex_infotip|lvs_ex_labeltip|lvs_ex_doublebuffer);
			lvc.mask = LVCF_TEXT|LVCF_SUBITEM|LVCF_WIDTH;
			lvc.cx = 130, lvc.pszText = (LPTSTR) TEXT("Name");
			ListView_InsertColumn(hCtl, lvc.iSubItem = 0, &lvc);
			lvc.cx = 100, lvc.pszText = (LPTSTR) TEXT("Value");
			ListView_InsertColumn(hCtl, lvc.iSubItem = COL_VALUE, &lvc);

			for (i = 0, c = g_flagCount; i < c; ++i) {
				LVITEM lvi;
				lvi.mask = LVIF_TEXT|LVIF_PARAM;
				lvi.iItem = 0x7fff, lvi.iSubItem = 0;
				lvi.lParam = i;
				lvi.pszText = (LPTSTR) g_flagNames[i];
				lvi.iItem = ListView_InsertItem(hCtl, &lvi);
				SendMessage(hDlg, WM_SETITEMFLAGVALUE, lvi.iItem, GetFlag(i));
			}

			ListView_SetColumnWidth(hCtl, 0x0000000, LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(hCtl, COL_VALUE, LVSCW_AUTOSIZE_USEHEADER);
		}
		return TRUE;

		case WM_NOTIFY: if (lp) {
			switch(MAKELONG(wp, ((NMHDR*)lp)->code)) {
				case MAKELONG(IDC_LIST, LVN_BEGINLABELEDIT): {
					NMLVDISPINFO *pLVDI = (NMLVDISPINFO*) lp;
					NSIS_TCHAR buf[100];
					HWND hEd = ListView_GetEditControl(pLVDI->hdr.hwndFrom);
					pLVSIE->OrgEditProc = (WNDPROC) SetWindowLongPtr(hEd, GWLP_WNDPROC, (LONG_PTR) LVEditSubItemProc);
					SendMessage(hEd, WM_SETTEXT, 0, (LPARAM) LVGetItemText(pLVDI->hdr.hwndFrom, pLVDI->item.iItem, COL_VALUE, buf, sizeof(buf) / sizeof(*buf)));
				}
				break;
				case MAKELONG(IDC_LIST, LVN_ENDLABELEDIT): {
					NMLVDISPINFO *pLVDI = (NMLVDISPINFO*) lp;
					if (pLVDI->item.pszText) {
						INT_PTR valid, val = StrToPtr(pLVDI->item.pszText, &valid);
						if (valid)
							SendMessage(hDlg, WM_SETITEMFLAGVALUE, pLVDI->item.iItem, (LPARAM) val);
						else
							MessageBeep(MB_ICONERROR);
					}
				}
				break;
				case MAKELONG(IDC_LIST, LVN_GETINFOTIP): {
					NMLVGETINFOTIP *pLVGIT = (NMLVGETINFOTIP*) lp;
					NSIS_TCHAR buf[100];
					INT_PTR valid, val = StrToPtr(LVGetItemText(pLVGIT->hdr.hwndFrom, pLVGIT->iItem, COL_VALUE, buf, sizeof(buf) / sizeof(*buf)), &valid);
					UINT cchBase = (pLVGIT->dwFlags & LVGIT_UNFOLDED) || pLVGIT->iSubItem ? 0 : lstrlen(pLVGIT->pszText);
					UINT cchMore = wsprintf(buf, TEXT("%s%d%s"), cchBase ? TEXT(" (") : TEXT(""), (int) val, cchBase ? TEXT(")") : TEXT(""));
					if (valid && val && cchBase + cchMore < (UINT) pLVGIT->cchTextMax) // Display the non-zero value in the tip if possible
						lstrcpy(pLVGIT->pszText + cchBase, buf);
				}
				break;
				//case MAKELONG(IDC_LIST, NM_RETURN): goto edititem; // Does not work correctly without special WM_GETDLGCODE handling?
				case MAKELONG(IDC_LIST, NM_DBLCLK): goto edititem;
				case MAKELONG(IDC_LIST, LVN_KEYDOWN): {
					if (((NMLVKEYDOWN*)lp)->wVKey == VK_SPACE || ((NMLVKEYDOWN*)lp)->wVKey == VK_F2) { edititem:
						ListView_EditLabel(pLVSIE->hLV, ListView_GetNextItem(pLVSIE->hLV, -1, LVNI_SELECTED));
						return ReturnDlgResult(hDlg, TRUE);
					}
				}
				break;
			}
		}
		break;

		case WM_COMMAND: {
			switch(wp) {
			case MAKELONG(IDOK, BN_CLICKED): {
				UINT i, c;
				for (i = 0, c = g_flagCount; i < c; ++i) {
					NSIS_TCHAR buf[100];
					LPARAM flagidx;
					INT_PTR valid, val = StrToPtr(LVGetItemTextAndLParam(pLVSIE->hLV, i, COL_VALUE, buf, sizeof(buf) / sizeof(*buf), &flagidx), &valid);
					if (valid) GetFlag(flagidx) = (UINT) val;
				}
			}
			// Fallthough
			case MAKELONG(IDCANCEL, BN_CLICKED):
				return EndDialog(hDlg, wp);
			}
		}
		break;

		case WM_CONTEXTMENU: {
			UINT id = (UINT) GetWindowLongPtr((HWND) wp, GWLP_ID);
			if ((int) lp == -1 && id == IDC_LIST) {
				NMHDR nmh = { (HWND) wp, id, NM_DBLCLK };
				return SendMessage(hDlg, WM_NOTIFY, id, (LPARAM) &nmh);
			}
		}
		break;

		case WM_SETITEMFLAGVALUE: {
			NSIS_TCHAR buf[100];
			LVITEM lvi;
			lvi.mask = LVIF_TEXT;
			lvi.iItem = (UINT) wp, lvi.iSubItem = COL_VALUE;
			wsprintf(lvi.pszText = buf, TEXT("0x%.8X"), (int) lp);
			ListView_SetItem(pLVSIE->hLV, &lvi);
		}
		break;
	}
	return FALSE;
}


static stack_t * MakeNewStackItem() {
	stack_t *newptr = (stack_t*)GlobalAlloc(GPTR,sizeof(stack_t)+(g_stringsize * sizeof(NSIS_TCHAR)));
	if (newptr) lstrcpy(newptr->text, TEXT("<<NEW>>"));
	return newptr;
}

BOOL CopyStateToDialog(HWND hwnd) {
/* This function updates the list boxes with the current contents of variables/stack.
 * It also builds the big text dump of everything, which may be copied to the clipboard.
 */
	NSIS_TCHAR *buf; // current line
	NSIS_TCHAR *clipboardWalker; // points to the end of all currently appended text
	stack_t *stackWalker;
	int i;
	int lineMaxLength = 0, totalLength = 0;

	HWND hwndStack = GetDlgItem(hwnd, IDC_STACK);
	HWND hwndVars  = GetDlgItem(hwnd, IDC_VARS);

	// this doesn't quite belong here, but it's the best place - preserve
	// the state of the listboxes and restore it afterwards. 
	int selectionStack = (int) SendMessage(hwndStack, LB_GETCURSEL, 0, 0);
	int selectionVars  = (int) SendMessage(hwndVars,  LB_GETCURSEL, 0, 0);

	// first we figure out how big stuff will be. allow 24 characters for
	// before ("varname: ") and endline/termination.
	for(stackWalker = *g_stacktop; stackWalker; stackWalker = stackWalker->next) {
		int l = lstrlen(stackWalker->text) + 24;
		totalLength += l;
		if(l > lineMaxLength) lineMaxLength = l;
	}
	for(i = 0; i < __INST_LAST; i++) {
		int l = lstrlen(getuservariable(i)) + 24;
		totalLength += l;
		if(l > lineMaxLength) lineMaxLength = l;
	}
	//dprintf("total length: %d max line: %d\n", totalLength, lineMaxLength);	

	buf = GlobalAlloc(GPTR, lineMaxLength * sizeof(NSIS_TCHAR));
	if(!buf) return FALSE;
	if(g_clipboardText)
		GlobalFree(g_clipboardText);
	g_clipboardText = GlobalAlloc(GPTR, (totalLength + 16) * sizeof(NSIS_TCHAR));
	clipboardWalker = g_clipboardText;

	// clear existing contents
	SendMessage(hwndStack, LB_RESETCONTENT, 0, 0);
	SendMessage(hwndVars, LB_RESETCONTENT, 0, 0);

	// fill in stack
	i = 0;
	for(stackWalker = *g_stacktop; stackWalker; stackWalker = stackWalker->next) {
		int l;
		wsprintf(buf, TEXT("%3d: "), i);
		lstrcat(buf, stackWalker->text);

		l = lstrlen(buf);
		CopyMemory(clipboardWalker, buf, l * sizeof(NSIS_TCHAR));
		clipboardWalker += l;
		*(clipboardWalker++) = '\r', *(clipboardWalker++) = '\n'; 

		if(l > CONF_BIGSTRING_MAX)
			lstrcpy(buf + CONF_BIGSTRING_MAX, CONF_BIGSTRING_SUBST);
		SendMessage(hwndStack, LB_ADDSTRING, 0, (LPARAM)buf);
		
		i++;
	}
	
	*(clipboardWalker++) = '\r'; *(clipboardWalker++) = '\n'; 

	// fill in user vars
	for(i = 0; i < __INST_LAST; i++) {
		int l;
		wsprintf(buf, TEXT("%s: "), getuservarname(i));
		lstrcat(buf, getuservariable(i));

		l = lstrlen(buf);
		CopyMemory(clipboardWalker, buf, l * sizeof(NSIS_TCHAR));
		clipboardWalker += l;
		*(clipboardWalker++) = '\r'; *(clipboardWalker++) = '\n'; 

		if (l > CONF_BIGSTRING_MAX)
			lstrcpy(buf + CONF_BIGSTRING_MAX, CONF_BIGSTRING_SUBST);

		SendMessage(hwndVars, LB_ADDSTRING, 0, (LPARAM)buf);
	}

	// restore state. if the indexes are no longer valid, no matter, it'll
	// just fail gracefully (sendmessage returning LB_ERR)
	SendMessage(hwndStack, LB_SETCURSEL, (WPARAM)selectionStack, 0);
	SendMessage(hwndVars,  LB_SETCURSEL, (WPARAM)selectionVars,  0);

	// we're done
	GlobalFree(buf);
	*clipboardWalker = '\0';
	return TRUE;
}

static void UpdateStackButtons(HWND hDlg) {
	HWND hStack = GetDlgItem(hDlg, IDC_STACK);
	EnableWindow(GetDlgItem(hDlg, IDC_REMOVE), SendMessage(hStack, LB_GETCURSEL, 0, 0) != LB_ERR);
	EnableWindow(GetDlgItem(hDlg, IDC_UP), SendMessage(hStack, LB_GETCOUNT, 0, 0) > 1);
	EnableWindow(GetDlgItem(hDlg, IDC_DOWN), SendMessage(hStack, LB_GETCOUNT, 0, 0) > 1);
}

int DoStackAction(int item, int index) {	
	stack_t **lastptr = g_stacktop;
	int oldIndex = index;

	// a copout.. moving item n up is the same as
	// moving item n-1 down
	if (item == IDC_UP) {
		if (index > 0) {
			if (DoStackAction(IDC_DOWN, index - 1)) {
				return index - 1;
			}
		}
		return LB_ERR;
	}

	while (index > 0 && (*lastptr) && (*lastptr)->next) {
		index--;
		lastptr = &( (*lastptr)->next );
	}

	if (item == IDC_INSERT) {
		stack_t *newptr = MakeNewStackItem();
		newptr->next = *lastptr, *lastptr = newptr;
		return oldIndex;
	}
	else if (item == IDC_REMOVE) {
		stack_t *oldptr = *lastptr;
		*lastptr = (*lastptr)->next;
		GlobalFree(oldptr);
		
		// if we removed the last item, we need to go up one. otherwise
		// stay at the same place
		return *lastptr ? oldIndex : /*oldIndex - 1*/ 0;
	}
	else if (item == IDC_DOWN) {
		stack_t *one, *two, *three;

		one = *lastptr;
		if(one) 
			two = one->next;
		if (one && two) {
			three = two->next;

			*lastptr = two;
			two->next = one;
			one->next = three;
			return oldIndex + 1;
		}
	}
	return LB_ERR;
}

INT_PTR CALLBACK DlgProcState(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch(msg) {
	case WM_INITDIALOG: {
			UINT isWOW64 = IsWow64(), cch;
			NSIS_TCHAR buf[200];
			UINT cc32v = GetModuleProcAddress("COMCTL32", "DSA_Sort") ? 0x60a : GetModuleProcAddress("COMCTL32", "DrawShadowText") ? 0x600 : 0;
			LPCTSTR fmt = sizeof(void*) > 4 ? TEXT("64-bit") : TEXT("%d:%d%hs");
			cch = wsprintf(buf, fmt, sizeof(void*), sizeof(NSIS_TCHAR), isWOW64 ? " WOW64" : "");
			if (cc32v) wsprintf(buf + cch, TEXT(", CC32v%hs"), cc32v > 0x600 ? "6+" : "6");
			SendMessage(GetDlgItem(hwnd, IDC_INFO), WM_SETTEXT, 0, (LPARAM) buf);
#if 0
			if (sizeof(NSIS_TCHAR) > 1 && AtLeastWin8()) { // I have no idea where these symbols are actually supported, playing it safe
				SetUnicodeSymbolText(hwnd, IDC_INSERT, L'+');
				SetUnicodeSymbolText(hwnd, IDC_REMOVE, 0x2212 /*Works on XP*/);
				SetUnicodeSymbolText(hwnd, IDC_UP, 0x25B2 /*Works on 8*/); // 0x2B06, 0x2191
				SetUnicodeSymbolText(hwnd, IDC_DOWN, 0x25BC /*Works on 8*/); // 0x2193
			}
#endif
			CopyStateToDialog(hwnd);
			UpdateStackButtons(hwnd);
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) {
			return EndDialog(hwnd, 0);
		}

		// Flags
		if (wp == IDC_FLAGS) {
			static UINT g_recurse = 0; // A very silly hack to make the button to stay pressed
			SendDlgItemMessage(hwnd, IDC_FLAGS, BM_SETSTATE, TRUE, 0);
			if (!g_recurse++) {
				LVSUBITEMEDIT lvsie;
				void *pOrgDlgData = g_DlgData;
				DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_FLAGS), hwnd, DlgProcFlags, (LPARAM) &lvsie);
				g_DlgData = pOrgDlgData;
				SendDlgItemMessage(hwnd, IDC_FLAGS, BM_SETSTATE, FALSE, 0);
				g_recurse = 0;
			}
			return FALSE;
		}

		// Copy all
		if (LOWORD(wp) == IDC_COPYALL) {
			HANDLE toClipboard;
			void *toClipboardPtr;
			int l;

			ASS(g_clipboardText);
			// GlobalAlloc a global memory region, get a pointer to
			// it and copy our stuff in
			l = (lstrlen(g_clipboardText) + 1) * sizeof(NSIS_TCHAR);
			toClipboard = GlobalAlloc(GMEM_DDESHARE, l);
			if(toClipboard) {
				toClipboardPtr = GlobalLock(toClipboard);
				CopyMemory(toClipboardPtr, g_clipboardText, l);
				GlobalUnlock(toClipboard);

				// open clipboard, empty it, throw our data in, close it again
				if (!OpenClipboard(hwnd)) {
					GlobalFree(toClipboard);
				} else {
					EmptyClipboard();
					SetClipboardData(sizeof(NSIS_TCHAR) > 1 ? CF_UNICODETEXT : CF_TEXT, toClipboard);
					CloseClipboard();
				}
			}
		}

		// Editing
		if (LOWORD(wp) == IDC_VARS && HIWORD(wp) == LBN_DBLCLK) {
			HWND hVars = GetDlgItem(hwnd, IDC_VARS);
			int index = (int) SendMessage(hVars, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				if (EditString(hwnd, getuservarname(index), getuservariable(index))) {
					CopyStateToDialog(hwnd);
					SendMessage(hVars, LB_SETCURSEL, (WPARAM)index, 0);
				}
			}
			return TRUE;
		}

		if(LOWORD(wp) == IDC_STACK && HIWORD(wp) == LBN_DBLCLK) {
			HWND hStack = GetDlgItem(hwnd, IDC_STACK);
			int index = (int) SendMessage(hStack, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				int changeTo = index;
				stack_t *s;
				NSIS_TCHAR name[32];

				wsprintf(name, TEXT("stack[%d]"), index);
				s = *g_stacktop;
				while (s && s->next && index>0) {
					s = s->next;
					index--;
				}
				
				if(s && EditString(hwnd, name, s->text)) {
					CopyStateToDialog(hwnd);
					SendMessage(hStack, LB_SETCURSEL, (WPARAM)changeTo, 0);
								
				}
			}
			return TRUE;
		}

		// silliness with the stack
		if  (LOWORD(wp) == IDC_REMOVE || LOWORD(wp) == IDC_INSERT 
				|| LOWORD(wp) == IDC_UP || LOWORD(wp) == IDC_DOWN) {
			HWND hStack = GetDlgItem(hwnd, IDC_STACK);
			int retry = 0, selection;
stackop_again:
			selection = (int) SendMessage(hStack, LB_GETCURSEL, 0, 0);
			if (selection != LB_ERR) 
				selection = DoStackAction(LOWORD(wp), selection);
			else if (LOWORD(wp) == IDC_INSERT) {
				if (SendMessage(hStack, LB_GETCOUNT, 0, 0) == 0) {
					stack_t *newptr = MakeNewStackItem();
					newptr->next = NULL, *g_stacktop = newptr;
					selection = 0;
				}
				else {
					if (!retry++) {
						SendMessage(hStack, LB_SETCURSEL, 0, 0);
						goto stackop_again;
					}
					else {
						MessageBeep(MB_ICONWARNING);
					}
				}
			}

			if (selection != LB_ERR && selection >= 0) {
				CopyStateToDialog(hwnd);
				SendMessage(hStack, LB_SETCURSEL, (WPARAM)selection, 0);
			}
			UpdateStackButtons(hwnd);
			return TRUE;
		}

		if (wp == MAKELONG(IDC_STACK, LBN_SELCHANGE))
			UpdateStackButtons(hwnd);

		break;

	case WM_VKEYTOITEM: {
			if (LOWORD(wp) == VK_F2) { editlistboxitem:
				SendMessage(hwnd, WM_COMMAND, MAKELONG(GetWindowLongPtr((HWND) lp, GWLP_ID), LBN_DBLCLK), lp); // Trigger edit function
				return -2;
			}
		}
		return -1; // Not DWLP_MSGRESULT, this message is special!

	case WM_CONTEXTMENU: {
			if ((int) lp == -1) {
				lp = wp;
				goto editlistboxitem;
			}
		}
		break;
	}
	return FALSE;
}

EXTERN_C void __declspec(dllexport) dumpinfo(HWND hwndParent, int string_size, NSIS_TCHAR *variables, stack_t **stacktop, extra_parameters*pXP, ...)
{
	LPCSTR fmt = sizeof(void*) > 4 ? "string_size=%u variables=0x%p stacktop=0x%p extra=0x%p\n" : "string_size=%u variables=0x%X stacktop=0x%X extra=0x%X\n";
	dprintf(fmt, string_size, variables, stacktop, pXP);
}

EXTERN_C void __declspec(dllexport) fillregisters(HWND hwndParent, int string_size, NSIS_TCHAR *variables, stack_t **stacktop, ...)
{
	UINT i;
	LPTSTR reg;
	EXDLL_INIT();
	for (i = INST_0; i <= INST_R9; ++i)
		if (!*(reg = getuservariable(i))) // Only overwrite empty variables
			wsprintf(reg, TEXT("<<< %s >>>"), getuservarname(i));
}

EXTERN_C void __declspec(dllexport) debug(HWND hwndParent, int string_size, NSIS_TCHAR *variables, stack_t **stacktop, extra_parameters*pXP, ...)
{
	g_hwndParent = hwndParent, g_pXP = pXP;
	EXDLL_INIT();

	if (sizeof(void*) < 8 && !VerifyStringSize(hwndParent))
		return ;
	DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_STATE), hwndParent, DlgProcState);
	if (g_clipboardText) GlobalFree(g_clipboardText);
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = hInst;
	return TRUE;
}
