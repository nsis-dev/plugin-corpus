/*

nsKeyHook NSIS plug-in

Author:  Stuart 'Afrow UK' Welch
Company: Afrow Soft Ltd.
Website: http://www.afrowsoft.co.uk
E-mail:  afrowuk@afrowsoft.co.uk
Date:    5th May 2012
Version: 1.0.0.0

A small plug-in to hook keyboard events for controls.

*/

#include <windows.h>
#include "nsKeyHook.h"

#ifdef UNICODE
#include "nsis_unicode\pluginapi.h"
#else
#include "nsis_ansi\pluginapi.h"
#endif

HANDLE g_hInstance;
extra_parameters* g_pExtraParameters = NULL;

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance = hInst;
  return TRUE;
}

static UINT_PTR PluginCallback(enum NSPIM msg)
{
  return 0;
}

#define _CheckAsyncKeyStateEither(a, b, c) \
    if ((iAsyncKeys & a) && !(GetAsyncKeyState(b) & 0x8000) && !(GetAsyncKeyState(c) & 0x8000)) \
      return FALSE

#define _CheckAsyncKeyState(a, b) \
    if ((iAsyncKeys & a) && !(GetAsyncKeyState(b) & 0x8000)) \
      return FALSE

BOOL CheckAsyncKeyState(int iAsyncKeys)
{
  if (iAsyncKeys)
  {
    _CheckAsyncKeyStateEither(ANY_SHIFT, VK_LSHIFT, VK_RSHIFT);
    _CheckAsyncKeyState(LEFT_SHIFT, VK_LSHIFT);
    _CheckAsyncKeyState(RIGHT_SHIFT, VK_RSHIFT);
    _CheckAsyncKeyStateEither(ANY_CTRL, VK_LCONTROL, VK_RCONTROL);
    _CheckAsyncKeyState(LEFT_CTRL, VK_LCONTROL);
    _CheckAsyncKeyState(RIGHT_CTRL, VK_RCONTROL);
    _CheckAsyncKeyStateEither(ANY_ALT, VK_LMENU, VK_RMENU);
    _CheckAsyncKeyState(LEFT_ALT, VK_LMENU);
    _CheckAsyncKeyState(RIGHT_ALT, VK_RMENU);
  }
  return TRUE;
}

BOOL HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, PKEY_HOOK pKeyHook, int iMsg, BOOL* bResult)
{
  BOOL fHandled = FALSE;

  while (pKeyHook)
  {
    if ((pKeyHook->iMsg & iMsg) && (pKeyHook->iKey == 0 || (wParam == pKeyHook->iKey && CheckAsyncKeyState(pKeyHook->iAsyncKeys))))
    {
      if (pKeyHook->iMsg & HOOK_POST)
      {
        *bResult = CallWindowProc((WNDPROC)pKeyHook->WndProcOld, hWnd, uMsg, wParam, lParam);
        fHandled = TRUE;
      }

      if (pKeyHook->iKey == 0 && iMsg != HOOK_WM_PASTE)
      {
        PTCHAR pKey = (PTCHAR)MemAlloc(sizeof(TCHAR) * g_stringsize);
        if (pKey)
        {
          wsprintf(pKey, TEXT("%d"), (int)wParam);
          pushstring(pKey);
        }
      }

      g_pExtraParameters->ExecuteCodeSegment(pKeyHook->iFuncAddress - 1, hWnd);
      
      if (iMsg == HOOK_WM_CHAR || iMsg == HOOK_WM_PASTE)
      {
        PTCHAR pKey = (PTCHAR)MemAlloc(sizeof(TCHAR) * g_stringsize);
        if (pKey)
        {
          if (popstring(pKey) == 0 && myatoi(pKey) == 0)
            fHandled = TRUE;
          MemFree(pKey);
        }
      }
      else
      {
        fHandled = TRUE;
      }
    }
    pKeyHook = pKeyHook->pNext;
  }

  return fHandled;
}

BOOL CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  PKEY_HOOK pKeyHook = (PKEY_HOOK)GetProp(hWnd, PROP_KEY_HOOK_DATA);
  WNDPROC WndProcOld = pKeyHook->WndProcOld;
  BOOL bResult = FALSE;

  switch (uMsg)
  {
  case WM_CHAR:

    if (HandleMessage(hWnd, uMsg, wParam, lParam, pKeyHook, HOOK_WM_CHAR, &bResult))
      return bResult;
    break;

  case WM_KEYUP:
    
    if (HandleMessage(hWnd, uMsg, wParam, lParam, pKeyHook, HOOK_WM_KEYUP, &bResult))
      return bResult;
    break;

  case WM_KEYDOWN:
    
    if (HandleMessage(hWnd, uMsg, wParam, lParam, pKeyHook, HOOK_WM_KEYDOWN, &bResult))
      return bResult;
    break;

  case HOOK_WM_PASTE:

    if (HandleMessage(hWnd, uMsg, wParam, lParam, pKeyHook, HOOK_WM_PASTE, &bResult))
      return bResult;
    break;

  case WM_DESTROY:
    
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)pKeyHook->WndProcOld);
    RemoveProp(hWnd, PROP_KEY_HOOK_DATA);
    WndProcOld = pKeyHook->WndProcOld;

    while (pKeyHook)
    {
      PKEY_HOOK pKeyHookTemp = pKeyHook->pNext;
      MemFree(pKeyHook);
      pKeyHook = pKeyHookTemp;
    }
    break;
  }

  return CallWindowProc((WNDPROC)WndProcOld, hWnd, uMsg, wParam, lParam);
}

NSISFUNC(Add)
{
  BOOL fSuccess = FALSE;
  PTCHAR pszHWnd, pszMsg, pszKey, pszAsyncKeys, pszFuncAddr;
  PKEY_HOOK pKeyHook;

  EXDLL_INIT();
  g_pExtraParameters = extra;
  extra->RegisterPluginCallback((HMODULE)g_hInstance, PluginCallback);

  pszHWnd = (PTCHAR)MemAlloc(sizeof(TCHAR) * string_size);
  pszMsg = (PTCHAR)MemAlloc(sizeof(TCHAR) * string_size);
  pszKey = (PTCHAR)MemAlloc(sizeof(TCHAR) * string_size);
  pszAsyncKeys = (PTCHAR)MemAlloc(sizeof(TCHAR) * string_size);
  pszFuncAddr = (PTCHAR)MemAlloc(sizeof(TCHAR) * string_size);
  pKeyHook = (PKEY_HOOK)MemAlloc(sizeof(KEY_HOOK));

  if (pszHWnd && pszMsg && pszKey && pszAsyncKeys && pszFuncAddr && pKeyHook)
  {
    if (popstring(pszHWnd) == 0 && popstring(pszMsg) == 0 && popstring(pszKey) == 0 && popstring(pszAsyncKeys) == 0 && popstring(pszFuncAddr) == 0)
    {
      HWND hWnd = (HWND)myatoi(pszHWnd);
      pKeyHook->iMsg = myatoi_or(pszMsg);
      pKeyHook->iKey = myatoi(pszKey);
      pKeyHook->iAsyncKeys = myatoi_or(pszAsyncKeys);
      pKeyHook->iFuncAddress = myatoi(pszFuncAddr);

      if (IsWindow(hWnd) && pKeyHook->iMsg > 0 && pKeyHook->iFuncAddress > 0)
      {
        PKEY_HOOK pKeyHookExisting = (PKEY_HOOK)GetProp(hWnd, PROP_KEY_HOOK_DATA);
        if (pKeyHookExisting)
        {
          while (pKeyHookExisting->pNext)
            pKeyHookExisting = pKeyHookExisting->pNext;
          pKeyHookExisting->pNext = pKeyHook;
        }
        else
        {
          pKeyHook->WndProcOld = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG)WndProc);
          SetProp(hWnd, PROP_KEY_HOOK_DATA, pKeyHook);
        }
        fSuccess = TRUE;
      }
    }
  }
    
  MemFreeEx(pszHWnd);
  MemFreeEx(pszMsg);
  MemFreeEx(pszKey);
  MemFreeEx(pszAsyncKeys);
  MemFreeEx(pszFuncAddr);

  if (!fSuccess)
  {
    MemFree(pKeyHook);
    extra->exec_flags->exec_error = 1;
  }
}