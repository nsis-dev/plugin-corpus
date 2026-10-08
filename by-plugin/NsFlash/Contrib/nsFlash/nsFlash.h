#ifndef __NSFLASH_H__
#define __NSFLASH_H__

#define NSISFUNC(name) extern "C" void __declspec(dllexport) name(HWND hWndParent, int string_size, TCHAR* variables, stack_t** stacktop, extra_parameters* extra)
#define DLL_INIT() EXDLL_INIT(); extra->RegisterPluginCallback(g_hInstance, PluginCallback);

typedef BOOL (WINAPI* PAtlAxWinInit)();
typedef HRESULT (WINAPI* PAtlAxAttachControl)(IUnknown*, HWND, IUnknown**);

#define NSISSWFWND_WIDTH 800
#define NSISSWFWND_HEIGHT 600
#define NSISSWFWND_WIDTH_MIN 400
#define NSISSWFWND_HEIGHT_MIN 300

#endif