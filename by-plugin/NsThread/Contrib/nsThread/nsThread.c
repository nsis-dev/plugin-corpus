/*

nsThread NSIS plug-in

Author:  Stuart 'Afrow UK' Welch
Company: Afrow Soft Ltd.
Website: http://www.afrowsoft.co.uk
E-mail:  afrowuk@afrowsoft.co.uk
Date:    30th March 2013
Version: 1.0.0.0

A small NSIS plug-in for creating threads.

*/

#include <windows.h>
#include "nsThread.h"

#ifdef UNICODE
#include "nsis_unicode\pluginapi.h"
#else
#include "nsis_ansi\pluginapi.h"
#endif

HANDLE g_hInstance;
extra_parameters* g_pExtraParameters;

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance = hInst;
  return TRUE;
}

static UINT_PTR PluginCallback(enum NSPIM msg)
{
  return 0;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
  Sleep(750);
  g_pExtraParameters->ExecuteCodeSegment((int)lpParameter, 0);
  return 0;
}

NSISFUNC(Create)
{
  BOOL fSuccess = FALSE;
  TCHAR* pszArg;

  DLL_INIT();
  g_pExtraParameters->RegisterPluginCallback((HMODULE)g_hInstance, PluginCallback);

  pszArg = (TCHAR*)LocalAlloc(LPTR, sizeof(TCHAR) * string_size);
  if (pszArg)
  {
    if (popstring(pszArg) == 0)
    {
      int iFunc = myatoi(pszArg);
      if (iFunc > 0)
      {
        DWORD dwThreadId;
        if (fSuccess = (CreateThread(NULL, 0, ThreadProc, (LPVOID)(iFunc - 1), 0, &dwThreadId) != NULL))
        {
          wsprintf(pszArg, TEXT("%lu"), dwThreadId);
          pushstring(pszArg);
        }
      }
    }

    LocalFree(pszArg);
  }

  if (!fSuccess)
  {
    pushstring(TEXT("error"));
    extra->exec_flags->exec_error = 1;
  }
}

NSISFUNC(Join)
{
  BOOL fSuccess = FALSE;
  TCHAR* pszArg;

  EXDLL_INIT();

  pszArg = (TCHAR*)LocalAlloc(LPTR, sizeof(TCHAR) * string_size);
  if (pszArg)
  {
    if (fSuccess = (popstring(pszArg) == 0))
    {
      DWORD dwTimeout = INFINITE;

      CharLower(pszArg);
      if (pszArg[0] == TEXT('/') &&
          pszArg[1] == TEXT('t') &&
          pszArg[2] == TEXT('i') &&
          pszArg[3] == TEXT('m') &&
          pszArg[4] == TEXT('e') &&
          pszArg[5] == TEXT('o') &&
          pszArg[6] == TEXT('u') &&
          pszArg[7] == TEXT('t') &&
          pszArg[8] == TEXT('='))
      {
        dwTimeout = myatoi(pszArg + 9);
        fSuccess = popstring(pszArg) == 0;
      }

      if (fSuccess)
      {
        HANDLE hThread = OpenThread(READ_CONTROL | SYNCHRONIZE, FALSE, myatoi(pszArg));
        if (fSuccess = (hThread != NULL))
        {
          pushstring(WaitForSingleObject(hThread, dwTimeout) == WAIT_TIMEOUT ? TEXT("yes") : TEXT("no"));
          CloseHandle(hThread);
        }
      }
    }

    LocalFree(pszArg);
  }

  if (!fSuccess)
  {
    pushstring(TEXT("error"));
    extra->exec_flags->exec_error = 1;
  }
}