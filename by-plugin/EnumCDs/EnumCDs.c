#include <windows.h>
#include "../ExDLL/exdll.h"

static char *szDrivesBuf;

void __declspec(dllexport) next(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
  EXDLL_INIT();
  {
    static char *p;
    DWORD dwBufSize;

    if (!szDrivesBuf) {
      dwBufSize = GetLogicalDriveStrings(0, 0);
      if (!dwBufSize) {
        pushstring("error");
        return;
      }
      
      szDrivesBuf=(char *) GlobalAlloc(GPTR, dwBufSize);
      if (!szDrivesBuf) {
        pushstring("error");
        return;
      }

      dwBufSize = GetLogicalDriveStrings(dwBufSize, szDrivesBuf);
      if (!dwBufSize) {
        pushstring("error");
        return;
      }
      p = szDrivesBuf;
    }

    while (*p && GetDriveType(p) != DRIVE_CDROM)
      p += lstrlen(p) + 1;

    if (*p) {
      pushstring(p);
      p += lstrlen(p) + 1;
    }
    else
      pushstring("done");
  }
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  if (ul_reason_for_call == DLL_PROCESS_DETACH && szDrivesBuf) GlobalFree(szDrivesBuf);
	return TRUE;
}
