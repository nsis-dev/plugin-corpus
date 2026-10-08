/*
	Copyright (C) 2001 Robert Rainwater

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source distribution.
*/

#include <windows.h>
#include <time.h>


enum {
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
	__INST_LAST
};


typedef struct _stack_t {
	struct _stack_t *next;
	char text[1];
} stack_t;


HINSTANCE g_hInstance;
HWND g_hwndParent;
int g_stringsize;
char *g_variables;


char *getuservariable(int varnum);
void setuservariable(int varnum, char *var);


BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved) {
	g_hInstance=hInst;
	return TRUE;
}


// Returns OS dwMajorVersion and dwMinorVersion into $0 and $1
void __declspec(dllexport) osversion(HWND hwndParent, int string_size, char *variables, stack_t **stacktop) {
	g_hwndParent=hwndParent;
	g_stringsize=string_size;
	g_variables=variables;
	{
		OSVERSIONINFO info;
		char buf[sizeof(unsigned long)*8+1];

		ZeroMemory(&info, sizeof(OSVERSIONINFO));
		info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((OSVERSIONINFO *) &info);

		// Major Version
		ultoa(info.dwMajorVersion,buf,10);
		setuservariable(INST_0,buf);
		
		// Minor Version
		ultoa(info.dwMinorVersion,buf,10);
		setuservariable(INST_1,buf);
  	}
}

// Returns OS platform (win31,win9x,winnt,unknown)
void __declspec(dllexport) osplatform(HWND hwndParent, int string_size, char *variables, stack_t **stacktop) {
	g_hwndParent=hwndParent;
	g_stringsize=string_size;
	g_variables=variables;
	{
		OSVERSIONINFO info;

		ZeroMemory(&info, sizeof(OSVERSIONINFO));
		info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((OSVERSIONINFO *) &info);

		if (info.dwPlatformId==VER_PLATFORM_WIN32s)
			setuservariable(INST_0,"win31");
		else if(info.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
			setuservariable(INST_0,"win9x");
		else if (info.dwPlatformId==VER_PLATFORM_WIN32_NT)
			setuservariable(INST_0,"winnt");
		else setuservariable(INST_0,"unknown");
  	}
}


char *getuservariable(int varnum) {
	if (varnum < 0 || varnum >= __INST_LAST) return NULL;
	return g_variables+varnum*g_stringsize;
}


void setuservariable(int varnum, char *var) {
	if (var != NULL && varnum >= 0 && varnum < __INST_LAST) {
		lstrcpy (g_variables + varnum*g_stringsize, var);
	}
}