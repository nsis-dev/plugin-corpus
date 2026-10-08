#include <windows.h>
#include "exdll.h"

// URL control from http://catch22.net/tuts/urlctrl.asp
#include "URLCtrl.h"

int popint();

void __declspec(dllexport) LVLink(HWND hwndParent, int string_size, 
                                char *variables, stack_t **stacktop,
                                extra_parameters *extra)
{
  EXDLL_INIT();

  {
    char szURL[1024];
    HWND hwLV;
	UINT item;
	UINT subitem;

    hwLV = (HWND) popint();
	item = (UINT) popint();
	subitem = (UINT) popint();
    
    if (popstring(szURL))
      return;

    if (hwLV == NULL)
      return;
    urlctrl_lv_set(hwLV , item, subitem, szURL, NULL, NULL, UCF_KBD | UCF_FIT);
  }
}

void __declspec(dllexport) Link(HWND hwndParent, int string_size, 
                                char *variables, stack_t **stacktop,
                                extra_parameters *extra)
{
  EXDLL_INIT();

  {
    char szURL[1024];
    HWND hwLabel;

    hwLabel = (HWND) popint();
    
    if (popstring(szURL))
      return;

    if (hwLabel == NULL)
      return;

    urlctrl_set(hwLabel , szURL, NULL, NULL, UCF_KBD | UCF_FIT);
  }
}

void __declspec(dllexport) Unload(HWND hwndParent, int string_size, 
                                  char *variables, stack_t **stacktop,
                                  extra_parameters *extra)
{
}

unsigned int myatoi(char *s)
{
  unsigned int v=0;

  for (;;)
  {
    unsigned int c=*s++;
    if (c >= '0' && c <= '9') c-='0';
    else break;
    v*=10;
    v+=c;
  }
  return v;
}

int popint()
{
  char buf[1024];
  if (popstring(buf))
    return 0;

  return myatoi(buf);
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
