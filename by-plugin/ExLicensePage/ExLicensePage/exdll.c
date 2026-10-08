#include <windows.h>
#include "exdll.h"

HINSTANCE g_hInstance;

HWND g_hwndParent;

void __declspec(dllexport) showPage(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
  g_hwndParent=hwndParent;

  EXDLL_INIT();


  // note if you want parameters from the stack, pop them off in order.
  // i.e. if you are called via exdll::myFunction file.dat poop.dat
  // calling popstring() the first time would give you file.dat,
  // and the second time would give you poop.dat. 
  // you should empty the stack of your parameters, and ONLY your
  // parameters.

  // do your stuff here
  {
		char		strLicenseFile[1024];
	
		popstring(strLicenseFile);

  }
}



BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance=hInst;
	return TRUE;
}
