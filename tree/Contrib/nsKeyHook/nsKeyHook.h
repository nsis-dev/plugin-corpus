#ifndef __NsKeyHook_H__
#define __NsKeyHook_H__

typedef struct _KEY_HOOK {
  WNDPROC WndProcOld;
  int iMsg;
  int iKey;
  int iAsyncKeys;
  int iFuncAddress;
  _KEY_HOOK* pNext;
} KEY_HOOK, *PKEY_HOOK;

#define NSISFUNC(name) extern "C" void __declspec(dllexport) name(HWND hWndParent, int string_size, TCHAR* variables, stack_t** stacktop, extra_parameters* extra)

#define PROP_KEY_HOOK_DATA TEXT("_StuKeyHookData_")

#define MemAlloc(n) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, n)
#define MemFree(p) HeapFree(GetProcessHeap(), 0, p)
#define MemFreeEx(p) if (p) HeapFree(GetProcessHeap(), 0, p)

#define LEFT_CTRL      1
#define LEFT_ALT       2
#define LEFT_SHIFT     4
#define RIGHT_CTRL     8
#define RIGHT_ALT      16
#define RIGHT_SHIFT    32
#define ANY_CTRL       64
#define ANY_ALT        128
#define ANY_SHIFT      256

#define HOOK_WM_CHAR    1
#define HOOK_WM_KEYUP   2
#define HOOK_WM_KEYDOWN 4
#define HOOK_WM_PASTE   8
#define HOOK_POST       1024

#endif