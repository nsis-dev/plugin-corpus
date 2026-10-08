#include <windows.h>
#include "..\ExDLL\exdll.h"

#define FORCE_SWITCH  "/FORCE"
#define NOSAFE_SWITCH "/NOSAFE"
#define SHUTDOWN_ERROR "ShutDown plug-in: Failed, was unable to close NSIS successfully."

HINSTANCE g_hInstance;
HWND g_hwndParent;
HWND childwnd;
void *lpWndProcOld;

unsigned int my_atoi(char *s);
void PerformShutDown(UINT uFlags, BOOL bPrivilege);
static BOOL CALLBACK ParentWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void __declspec(dllexport) LogOff(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hwndParent=hwndParent;
  EXDLL_INIT();
  {
	PerformShutDown(EWX_LOGOFF, FALSE);
  }
}

void __declspec(dllexport) PowerOff(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hwndParent=hwndParent;
  EXDLL_INIT();
  {
	PerformShutDown(EWX_POWEROFF, TRUE);
  }
}

void __declspec(dllexport) Reboot(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hwndParent=hwndParent;
  EXDLL_INIT();
  {
	PerformShutDown(EWX_REBOOT, TRUE);
  }
}

void __declspec(dllexport) ShutDown(HWND hwndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hwndParent=hwndParent;
  EXDLL_INIT();
  {
	PerformShutDown(EWX_SHUTDOWN, TRUE);
  }
}

void PerformShutDown(UINT uFlags, BOOL bPrivilege)
{

	char szParam[8];
	int iForce;
	int iNoSafe;
	popstring(szParam);

	// Has /FORCE Windows switch been passed to plugin?
	iForce = lstrcmpi(szParam, FORCE_SWITCH);
	// Has /NOSAFE Windows switch been passed to plugin?
	iNoSafe = lstrcmpi(szParam, NOSAFE_SWITCH);

	// If no switches, push the value back onto the stack
	if (iForce != 0 || iNoSafe != 0) {
		pushstring(szParam);
	}

	// Get privileges for shutting down Windows
	if (bPrivilege == TRUE)
	{

		HANDLE hToken;
		TOKEN_PRIVILEGES tkp;

		// Get a token for this process.
		OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

		// Get the LUID for the shutdown privilege.
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

		tkp.PrivilegeCount = 1; // one privilege to set
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		// Get the shutdown privilege for this process.
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

	}

	// Attempt to close NSIS safely. If this fails, we shouldn't continue closing Windows
	// unless /FORCE has been specified
	if(DestroyWindow(g_hwndParent) == TRUE || iNoSafe == 0)
	{
		// Perform the shutdown (safe)!
		ExitWindowsEx(uFlags, 0);
	}
	else if (iForce == 0)
	{
		// Perform the shutdown (terminate)!
		ExitWindowsEx(uFlags | EWX_FORCE, 0);
	}
	else
	{
		// Error message for failure
		pushstring(SHUTDOWN_ERROR);
	}

}

static BOOL CALLBACK ParentWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return TRUE;
}

unsigned int my_atoi(char *s) {
  unsigned int v=0;
  if (*s == '0' && (s[1] == 'x' || s[1] == 'X')) {
    s+=2;
    for (;;) {
      int c=*s++;
      if (c >= '0' && c <= '9') c-='0';
      else if (c >= 'a' && c <= 'f') c-='a'-10;
      else if (c >= 'A' && c <= 'F') c-='A'-10;
      else break;
      v<<=4;
      v+=c;
    }
  }
  else if (*s == '0' && s[1] <= '7' && s[1] >= '0') {
    s++;
    for (;;) {
      int c=*s++;
      if (c >= '0' && c <= '7') c-='0';
      else break;
      v<<=3;
      v+=c;
    }
  }
  else {
    for (;;) {
      int c=*s++ - '0';
      if (c < 0 || c > 9) break;
      v*=10;
      v+=c;
    }
  }
  return (int)v;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=hInst;
	return TRUE;
}
