#include <windows.h>
#include "../exdll/exdll.h"

HINSTANCE g_hInstance;

HWND g_hwndParent;

struct {
	char name[16];
	DWORD value;
} pr[] = {
	{"above normal", ABOVE_NORMAL_PRIORITY_CLASS},
	{"below normal", BELOW_NORMAL_PRIORITY_CLASS},
	{"high", HIGH_PRIORITY_CLASS},
	{"idle", IDLE_PRIORITY_CLASS},
	{"normal", NORMAL_PRIORITY_CLASS},
	{"realtime", REALTIME_PRIORITY_CLASS},
};

void ShowError() {
	DWORD dwError = GetLastError();
	LPSTR szMessage = 0;
	LPSTR szErrorBuf;

	if (dwError == ERROR_SUCCESS) return;

	/* Get an error description using FormatMessage.*/
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		dwError,
		0,
		(LPSTR)&szMessage,
		0,
		NULL
	);

	szErrorBuf = (LPSTR)GlobalAlloc(GPTR, lstrlen(szMessage)+1000);
	if (!szErrorBuf) {
		MessageBox(NULL, "Unknown exception and memory error!", "Runtime Error!", MB_OK|MB_ICONERROR);
	}
	else {
		/* Create the string.*/
		wsprintf(szErrorBuf, "Error code 0x%08X: %s", dwError, szMessage);

		/* Display the error using MessageBox.*/
		MessageBox(NULL, szErrorBuf, "Runtime Error!", MB_OK|MB_ICONERROR);

		GlobalFree(szErrorBuf);
	}
}

void __declspec(dllexport) ExecWait(HWND hwndParent, int string_size, char *variables, stack_t **stacktop) {
	char priority[128];
	char cmd[1024];

	int i;

	DWORD pri = 0;

	char *dir;
	DWORD d;

	static PROCESS_INFORMATION ProcInfo;
	STARTUPINFO StartUp = {sizeof(StartUp), };

	EXDLL_INIT();

	if (popstring(cmd)) {
		popstring(priority);
		pushstring("error");
		return;
	}
	if (popstring(priority)) {
		pushstring("error");
		return;
	}

	for(i = 0; i < sizeof(pr); i++) {
		if(!lstrcmp(pr[i].name, priority)) {
			pri = pr[i].value;
		}
	}

	if(!pri) {
		pushstring("error");
		return;
	}

	dir = getuservariable(INST_OUTDIR);
	d=GetFileAttributes(dir);
	if (d == INVALID_FILE_ATTRIBUTES || !(d & FILE_ATTRIBUTE_DIRECTORY)) dir = 0;

	if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, pri, NULL, dir, &StartUp, &ProcInfo)) {
		ShowError();
		pushstring("error");
		return;
	}
	
	CloseHandle(ProcInfo.hThread);

	{
		DWORD lExitCode;
		while (WaitForSingleObject(ProcInfo.hProcess, 100) == WAIT_TIMEOUT)
		{
			MSG msg;
			while (PeekMessage(&msg,NULL,WM_PAINT,WM_PAINT,PM_REMOVE))
				DispatchMessage(&msg);
		}
		GetExitCodeProcess(ProcInfo.hProcess, &lExitCode);

		wsprintf(priority, "%d", lExitCode);
		pushstring(priority);
	}

	CloseHandle(ProcInfo.hProcess);
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=hInst;
  return TRUE;
}
