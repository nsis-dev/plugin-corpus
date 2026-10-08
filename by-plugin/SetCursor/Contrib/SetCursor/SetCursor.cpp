#include <windows.h>
#include "..\ExDll\exdll.h"

/**
    SetCursor v0.1 by Afrow UK
    A plugin dll for NSIS that sets the mouse cursor.

    Last modified: 12th April 06
*/


// Global variables.
HINSTANCE g_hInstance;
HWND g_hWndParent;

unsigned int my_atoi(char *s);

// Set mouse cursor.
extern "C" void __declspec(dllexport) System(HWND hWndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  {
    char szParam[32];
    HCURSOR cursor;
    POINT p;

    popstring(szParam);

    if (lstrcmpi(szParam, TEXT("APPSTARTING")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_APPSTARTING, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if ((lstrcmpi(szParam, TEXT("ARROW")) == 0) ||
        (lstrcmpi(szParam, TEXT("NORMAL")) == 0))
      cursor = (HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("CROSS")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_CROSS, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("HAND")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_HAND, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("HELP")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_HELP, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("IBEAM")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_IBEAM, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("ICON")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_ICON, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("NO")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_NO, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("SIZE")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_SIZE, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("SIZEALL")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_SIZEALL, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("SIZENESW")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_SIZENESW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("SIZENS")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_SIZENS, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("SIZENWSE")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_SIZENWSE, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("SIZEWE")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_SIZEWE, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("UPARROW")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_UPARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
    if (lstrcmpi(szParam, TEXT("WAIT")) == 0)
      cursor = (HCURSOR)LoadImage(NULL, IDC_WAIT, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    else
      cursor = (HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

    if (!cursor)
      pushstring("error loading cursor");
    else
    {
      SetClassLongPtr(hWndParent, GCL_HCURSOR, (LONG)cursor);
      GetCursorPos(&p);
      SetCursorPos(p.x, p.y);
    }

  }
}

// Set mouse cursor from file.
extern "C" void __declspec(dllexport) File(HWND hWndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  {
    char szParam[MAX_PATH];
    HCURSOR cursor;
    POINT p;

    popstring(szParam);
    cursor = (HCURSOR)LoadImage(NULL, szParam, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);

    if (!cursor)
      pushstring("error loading cursor");
    else
    {
      SetClassLongPtr(g_hWndParent, GCL_HCURSOR, (LONG)cursor);
      GetCursorPos(&p);
      SetCursorPos(p.x, p.y);
    }
  }
}

// Set mouse cursor position.
extern "C" void __declspec(dllexport) Position(HWND hWndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  {
    char szParam[32];
    int x, y;

    popstring(szParam);
    x = my_atoi(szParam);
    popstring(szParam);
    y = my_atoi(szParam);

    if (!SetCursorPos(x, y))
      pushstring("error setting cursor position");
  }
}

// Set mouse cursor visibility.
extern "C" void __declspec(dllexport) Visibility(HWND hWndParent, int string_size, 
                                      char *variables, stack_t **stacktop,
                                      extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  {
    char szParam[32];
    popstring(szParam);

    if ((lstrcmpi(szParam, TEXT("true")) == 0) ||
        (lstrcmpi(szParam, TEXT("1")) == 0))
      wsprintf(szParam, "%i", ShowCursor(TRUE));
    else
      wsprintf(szParam, "%i", ShowCursor(FALSE));

    pushstring(szParam);
  }
}

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance = hInst;
	return TRUE;
}

// Converts char to int
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

