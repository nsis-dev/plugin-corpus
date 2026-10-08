/**************************************************************
 *                    Dump Log NSIS plugin                    *
 **************************************************************

History
=======
1.1 - 20190820 - Anders
* Complete rewrite for Unicode and 64-bit support
+ Added DumpLogUTF8

1.0 - 20051031 - Shengalts Aleksander aka Instructor (Shengalts@mail.ru)
* Initial version

*/
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

static BOOL WriteOctets(HANDLE hFile, const void*Data, DWORD cb)
{
	DWORD cbio;
	BOOL succ = WriteFile(hFile, Data, cb, &cbio, NULL);
	return succ && cb == cbio;
}

enum { INST_0 = 0 };
typedef struct _stack_t { struct _stack_t *next; TCHAR text[1]; } stack_t;

stack_t **g_ppStackTop;

#define freeitem(pSI) GlobalFree((HGLOBAL)(pSI))
static stack_t* WINAPI popitem()
{
	stack_t *pSI = g_ppStackTop && *g_ppStackTop ? *g_ppStackTop : NULL;
	if (pSI) *g_ppStackTop = pSI->next;
	return pSI;
}

static LPTSTR WINAPI getvarptr(const UINT VarId, int StrCap, LPTSTR Vars) { return Vars + VarId * StrCap; }
static void WINAPI setvar(const UINT VarId, LPCTSTR Value, int StrCap, LPTSTR Vars) { lstrcpy(Vars + VarId * StrCap, Value); }

static INT WINAPI getvarindex(LPCTSTR p)
{
	UINT base;
	if (*p == '.') ++p; // Support System plug-in syntax
	base = *p == 'r' ? INST_0 : INST_0 + 10;
	return ((*p|32) == 'r' && *++p >= '0' && *p <= '9') ? base + (*p - '0') : -1;
}

static void WINAPI DumpLogWorker(UINT Codepage, HWND hWndNSIS, int StrCap, LPTSTR Vars)
{
	stack_t *pPathItem = popitem();
	stack_t *pResultItem = popitem();
	LPTSTR buf1 = GlobalAlloc(GPTR, (StrCap + 2 + !0) * sizeof(*buf1)), buf2 = 0, bufptr;
	INT outvarid = getvarindex(pResultItem->text), logempty = TRUE, cbch = sizeof(*buf1);
	HWND hLog = FindWindowExA(FindWindowExA(hWndNSIS, NULL, "#32770", NULL), NULL, WC_LISTVIEWA, NULL);
	DWORD count = hLog ? (DWORD) SendMessage(hLog, LVM_GETITEMCOUNT, 0, 0) : 0, i, cch, cbbuf2 = 0;
	BOOL needsconversion = cbch > 1 ? Codepage != 1200 : FALSE;

	setvar(outvarid, TEXT("-1"), StrCap, Vars);
	if (buf1 && count)
	{
		HANDLE hFile = CreateFile(pPathItem->text, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD cb;
			LVITEM lvi;
#ifdef UNICODE
			static const unsigned char u8b[] = {0xEF,0xBB,0xBF};
			static const unsigned char u16lb[] = {0xFF,0xFE};
			WriteOctets(hFile, Codepage == 1200 ? u16lb : u8b, Codepage == 1200 ? 2 : 3);
#endif
			for (i = 0, lvi.iSubItem = 0; i < count; ++i)
			{
				lvi.pszText = buf1, lvi.cchTextMax = StrCap + !0;
				cch = (DWORD) SendMessage(hLog, LVM_GETITEMTEXT, i, (LPARAM) &lvi);
				if (logempty || cch)
				{
					bufptr = lvi.pszText;
					bufptr[cch] = '\r', bufptr[++cch] = '\n', bufptr[++cch] = '\0';
					cb = cch * cbch;
#ifdef UNICODE
					if (needsconversion)
					{
						cb = WideCharToMultiByte(Codepage, 0, bufptr, cch, NULL, 0, NULL, NULL);
						if (!cb) break;
						if (cbbuf2 < cb)
						{
							if (cbbuf2) GlobalFree(buf2);
							if (!(buf2 = GlobalAlloc(GPTR, cbbuf2 = cb))) break;
						}
						cb = WideCharToMultiByte(Codepage, 0, bufptr, cch, (LPSTR) buf2, cbbuf2, NULL, NULL);
						if (!cb) break;
						bufptr = buf2;
					}
#endif
					if (!WriteOctets(hFile, bufptr, cb)) break;
				}
			}
			CloseHandle(hFile);
			if (i == count) setvar(outvarid, TEXT("0"), StrCap, Vars);
		}
	}

	if (sizeof(void*) > 4 || buf1) GlobalFree(buf1);
	if (sizeof(void*) > 4 || buf2) GlobalFree(buf2);
	freeitem(pPathItem);
	freeitem(pResultItem);
}

EXTERN_C void __declspec(dllexport) __cdecl DumpLog(HWND hWndNSIS, int StrCap, LPTSTR Vars, stack_t **ppStackTop) // <c:\path\output.log> <.r0..9|.R0..9>
{
	g_ppStackTop = ppStackTop;
	DumpLogWorker(sizeof(*Vars) > 1 ? 1200 : CP_ACP, hWndNSIS, StrCap, Vars);
}

#ifdef UNICODE
EXTERN_C void __declspec(dllexport) __cdecl DumpLogUTF8(HWND hWndNSIS, int StrCap, LPTSTR Vars, stack_t **ppStackTop)
{
	g_ppStackTop = ppStackTop;
	DumpLogWorker(65001, hWndNSIS, StrCap, Vars);
}
#endif

EXTERN_C BOOL APIENTRY _DllMainCRTStartup(HINSTANCE hInst, DWORD Reason, PVOID pCtx)
{
	return TRUE | (BOOL)(UINT) hInst;
}
