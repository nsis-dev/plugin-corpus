#include <windows.h>
#include "..\ExDll\exdll.h"

/**
    ShutdownAllow v0.1 by Afrow UK
    A plugin dll for NSIS that allows Windows to shutdown if the installer is running.

    Last modified: 11th August 06
*/

#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLong
#endif

#ifndef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLong
#endif

#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC GWL_WNDPROC
#endif

#ifndef DWLP_DLGPROC
#define DWLP_DLGPROC DWL_DLGPROC
#endif

// Global variables.
HINSTANCE g_hInstance;
HWND      g_hWndParent;
WNDPROC   g_WndProcOld;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_QUERYENDSESSION)
    return TRUE;
  return CallWindowProc((WNDPROC)g_WndProcOld, hWnd, uMsg, wParam, lParam);
}

extern "C"
void __declspec(dllexport) Allow(HWND hWndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  {
    g_WndProcOld = (WNDPROC)SetWindowLongPtr(hWndParent, GWLP_WNDPROC, (LONG)WindowProc);
  }
}

extern "C"
void __declspec(dllexport) Unload(HWND hWndParent, int string_size, char *variables, stack_t **stacktop, extra_parameters *extra)
{
  g_hWndParent = hWndParent;
  EXDLL_INIT();
  {
    SetWindowLongPtr(hWndParent, GWLP_WNDPROC, (LONG)g_WndProcOld);
  }
}

// Entry point for DLL.
BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance = (HINSTANCE)hInst;
  return TRUE;
}