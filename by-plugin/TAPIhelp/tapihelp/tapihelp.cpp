#include "stdafx.h"

#define EXDLL_INIT()		{	\
    g_stringsize=string_size;	\
    g_stacktop=stacktop;		\
    g_variables=variables;	}

#define WM_NOTIFY_OUTER_NEXT (WM_USER+0x8)
#define WM_NOTIFY_CUSTOM_READY (WM_USER+0xd)
#define NOTIFY_BYE_BYE 'x'

typedef struct _stack_t {
	struct _stack_t *next;
	char text[1];			// this should be the length of string_size
} stack_t;

static unsigned int g_stringsize = 0;
static stack_t **g_stacktop = NULL;
static char *g_variables = NULL;

enum
{
INST_0,         // $0
INST_1,         // $1
INST_2,         // $2
INST_3,         // $3
INST_4,         // $4
INST_5,         // $5
INST_6,         // $6
INST_7,         // $7
INST_8,         // $8
INST_9,         // $9
INST_R0,        // $R0
INST_R1,        // $R1
INST_R2,        // $R2
INST_R3,        // $R3
INST_R4,        // $R4
INST_R5,        // $R5
INST_R6,        // $R6
INST_R7,        // $R7
INST_R8,        // $R8
INST_R9,        // $R9
INST_CMDLINE,   // $CMDLINE
INST_INSTDIR,   // $INSTDIR
INST_OUTDIR,    // $OUTDIR
INST_EXEDIR,    // $EXEDIR
INST_LANG,      // $LANGUAGE
__INST_LAST
};

/*static int __stdcall popstring (char *str)
{
	stack_t* th;
	if (!g_stacktop || !*g_stacktop)
		return 1;
	th=(*g_stacktop);
	lstrcpy (str, th->text);
	*g_stacktop = th->next;
	GlobalFree ((HGLOBAL) th);
	return 0;
}*/

static void __stdcall pushstring (const char *str)
{
	stack_t* th;
	if (!g_stacktop)
		return;
	th = (stack_t*) GlobalAlloc (GPTR, sizeof (stack_t) + g_stringsize);
	lstrcpyn (th->text,str,g_stringsize);
	th->next=*g_stacktop;
	*g_stacktop=th;
}

/*static char* __stdcall getuservariable (int varnum)
{
	if (varnum < 0 || varnum >= __INST_LAST)
		return NULL;
	else
		return g_variables + varnum * g_stringsize;
}*/

/*static void __stdcall setuservariable (int varnum, char *var)
{
	if (var != NULL && varnum >= 0 && varnum < __INST_LAST) 
		lstrcpy (g_variables + varnum * g_stringsize, var);
}*/

BOOL APIENTRY DllMain (HANDLE /*hModule*/, DWORD /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    return TRUE;
}

extern "C" {

void __declspec(dllexport) getmodemid (HWND /*hwndParent*/, int string_size, char *variables, stack_t **stacktop)
{
	EXDLL_INIT()

	char tmp [16] = { "" };
	HLINEAPP hLineApp = NULL;
	DWORD num = 0;
	DWORD ver = 0x00020000;
	char params_buf [256];
	ZeroMemory (params_buf, sizeof (params_buf));
	LINEINITIALIZEEXPARAMS* params = (LINEINITIALIZEEXPARAMS*) params_buf;
	params->dwTotalSize = sizeof (params_buf);
	params->dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;
	LONG ret = lineInitializeEx (&hLineApp, NULL, NULL, NULL, &num, &ver, params);
	if (ret == 0) {
		while (num--) {
			char caps_buf [2048];
			ZeroMemory (caps_buf, sizeof (caps_buf));
			LINEDEVCAPS* caps = (LINEDEVCAPS*) caps_buf;
			caps->dwTotalSize = sizeof (caps_buf);
			ret = lineGetDevCaps (hLineApp, num, ver, 0, caps);
			if (ret == 0) {
				if (caps->dwMediaModes & LINEMEDIAMODE_DATAMODEM) {
					wsprintf (tmp, "%u", num);
					break;
				}
			}
		}
	}
	if (hLineApp)
		lineShutdown (hLineApp);

	pushstring (tmp);
}

void __declspec(dllexport) getmodemname (HWND /*hwndParent*/, int string_size, char *variables, stack_t **stacktop)
{
	EXDLL_INIT()

	bool res = true;
	HLINEAPP hLineApp = NULL;
	DWORD num = 0;
	DWORD ver = 0x00020000;
	char params_buf [256];
	ZeroMemory (params_buf, sizeof (params_buf));
	LINEINITIALIZEEXPARAMS* params = (LINEINITIALIZEEXPARAMS*) params_buf;
	params->dwTotalSize = sizeof (params_buf);
	params->dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;
	LONG ret = lineInitializeEx (&hLineApp, NULL, NULL, NULL, &num, &ver, params);
	if (ret == 0) {
		while (num--) {
			char caps_buf [2048];
			ZeroMemory (caps_buf, sizeof (caps_buf));
			LINEDEVCAPS* caps = (LINEDEVCAPS*) caps_buf;
			caps->dwTotalSize = sizeof (caps_buf);
			ret = lineGetDevCaps (hLineApp, num, ver, 0, caps);
			if (ret == 0) {
				if (caps->dwMediaModes & LINEMEDIAMODE_DATAMODEM) {
					pushstring (caps_buf + caps->dwLineNameOffset);
					res = true;
					break;
				}
			}
		}
	}
	if (hLineApp)
		lineShutdown (hLineApp);
	
	if (!res)
		pushstring ("");
}

}
