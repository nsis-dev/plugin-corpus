/**
 * Invoke Shell Verb NSIS plug-in
 *
 * Version: 1.1
 * Date   : 08 November 2018
 * Author : Robert Strong
 *
 * New in version 1.1:
 * The DoIt command now accepts either a string name of the verb to invoke or
 * the numeric ID of a string resource in shell32.dll containing the name of
 * the verb (as in version 1.0).
 *
 * Copyright (c) 2011 Robert Strong
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

// Uncomment to force Unicode support
//#define UNICODE
//#define _UNICODE

#include <windows.h>
#include <shldisp.h>
#include "..\ExDll\exdll.h"

#define OUT_SUCCESS            TEXT("success")
#define OUT_ERR_CALL_FAILED    TEXT("method failed")
#define OUT_ERR_INVALID_RESID  TEXT("invalid resource")
#define OUT_ERR_INVALID_DIR    TEXT("invalid directory")
#define OUT_ERR_INVALID_FILE   TEXT("invalid file")
#define OUT_ERR_VERB_NOT_FOUND TEXT("verb not found")

extern "C"
void __declspec(dllexport) DoIt(HWND hWndParent, int string_size, 
                                TCHAR *variables, stack_t **stacktop,
                                extra_parameters *extra)
{
  EXDLL_INIT();
  {
    TCHAR szDirPath[MAX_PATH];
    TCHAR szFileName[MAX_PATH];
    TCHAR szResID[MAX_PATH];

    if (popstring(szDirPath) == 0 && popstring(szFileName) == 0 &&
        popstring(szResID) == 0)
    {
      HRESULT hr;
      HINSTANCE hInst;
      WCHAR wszVerbName[100];
      VARIANT v;
      BSTR name;
      BOOL bFoundVerb = FALSE;
      TCHAR* resIDEnd = NULL;
      int iResID = 0;
      int rv = 0;
      IShellDispatch *pSD = NULL;
      Folder *pFolder = NULL;
      FolderItem *pItem = NULL;
      FolderItemVerbs *pVerbs = NULL;
      FolderItemVerb *pVerb = NULL;

#ifdef _UNICODE
      iResID = wcstol(szResID, &resIDEnd, 10);
#else
      iResID = strtol(szResID, &resIDEnd, 10);
#endif

      // If our szResID parameter isn't a number, then it's just
      // the name of a verb that we should invoke directly.
      // If it was a number, then we need to look up the verb
      // name from shell32's localized string resources.
      if (resIDEnd && (*resIDEnd == TEXT('\0'))) {
        hInst = LoadLibraryW(L"shell32.dll");
        rv = LoadStringW(hInst,
                         iResID,
                         wszVerbName,
                         sizeof(wszVerbName)/sizeof(wszVerbName[0])); 
        FreeLibrary(hInst);
        if (!rv)
        {
          pushstring(OUT_ERR_INVALID_RESID);
          return;
        }
      }

      CoInitialize(NULL);

      hr = CoCreateInstance(CLSID_Shell,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IShellDispatch,
                            (void **)&pSD);
      if (FAILED(hr) || pSD ==  NULL)
      {
        pushstring(OUT_ERR_CALL_FAILED);
        CoFreeUnusedLibraries();
        CoUninitialize();
        return;
      }

      v.vt = VT_BSTR;
#ifdef _UNICODE
      v.bstrVal = szDirPath;
#else
      WCHAR wszDirPath[MAX_PATH];
      MultiByteToWideChar(CP_ACP, 0, szDirPath, lstrlen(szDirPath) + 1,
                          wszDirPath, sizeof(wszDirPath)/sizeof(wszDirPath[0]));
      v.bstrVal = wszDirPath;
#endif

      hr = pSD->NameSpace(v, &pFolder);
      if (FAILED(hr) || pFolder == NULL)
      {
        pushstring(OUT_ERR_INVALID_DIR);
        pSD->Release();
        CoFreeUnusedLibraries();
        CoUninitialize();
        return;
      }

#ifdef _UNICODE
      hr = pFolder->ParseName(szFileName, &pItem);
#else
      WCHAR wszFileName[MAX_PATH];
      MultiByteToWideChar(CP_ACP, 0, szFileName, lstrlen(szFileName) + 1,
                          wszFileName, sizeof(wszFileName)/sizeof(wszFileName[0]));
      hr = pFolder->ParseName(wszFileName, &pItem);
#endif
      if (FAILED(hr) || pItem == NULL)
      {
        pushstring(OUT_ERR_INVALID_FILE);
        pSD->Release();
        CoFreeUnusedLibraries();
        CoUninitialize();
        return;
      }

      // If we were given our verb by name, just invoke it directly on the item,
      // since not all valid verbs show up in the FolderItemVerbs list.
      if (iResID == 0) {
        VARIANT vVerbName;
        VariantInit(&vVerbName);
        vVerbName.vt = VT_BSTR;
#ifdef _UNICODE
        vVerbName.bstrVal = SysAllocString(szResID);
#else
        WCHAR wszVerbName[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, szResID, lstrlen(szResID) + 1,
                            wszVerbName, sizeof(wszVerbName)/sizeof(wszVerbName[0]));
        vVerbName.bstrVal = SysAllocString(wszVerbName);
#endif

        hr = pItem->InvokeVerb(vVerbName);
        if (FAILED(hr)) {
          pushstring(OUT_ERR_VERB_NOT_FOUND);
        } else {
          pushstring(OUT_SUCCESS);
        }

        VariantClear(&vVerbName);
        pItem->Release();
        pSD->Release();
        CoFreeUnusedLibraries();
        CoUninitialize();
        return;
      }

      hr = pItem->Verbs(&pVerbs);
      if (FAILED(hr) || pVerbs == NULL)
      {
        pushstring(OUT_ERR_CALL_FAILED);
        pItem->Release();
        pSD->Release();
        CoFreeUnusedLibraries();
        CoUninitialize();
        return;
      }

      long cnt;
      hr = pVerbs->get_Count(&cnt);
      if (FAILED(hr))
      {
        pushstring(OUT_ERR_CALL_FAILED);
        pItem->Release();
        pSD->Release();
        CoFreeUnusedLibraries();
        CoUninitialize();
        return;
      }

      int i;
      v.vt = VT_I4;
      for (i = 0; i < cnt; ++i)
      {
        v.lVal = i;
        hr = pVerbs->Item(v, &pVerb);
        if (FAILED(hr) || pVerb == NULL)
        {
          continue;
        }

        hr = pVerb->get_Name(&name);
        if (FAILED(hr))
        {
          pVerb->Release();
          continue;
        }

        if (!wcscmp((WCHAR *)name, wszVerbName))
        {
          bFoundVerb = TRUE;
          hr = pVerb->DoIt();
          if (FAILED(hr))
          {
            pushstring(OUT_ERR_CALL_FAILED);
          }
        }

        SysFreeString(name);
        pVerb->Release();
        if (bFoundVerb)
        {
          break;
        }
      }

      if (bFoundVerb)
      {
        pushstring(OUT_SUCCESS);
      } else {
        pushstring(OUT_ERR_VERB_NOT_FOUND);
      }

      pVerbs->Release();
      pItem->Release();
      pFolder->Release();
      pSD->Release();

      CoFreeUnusedLibraries();
      CoUninitialize();
    }
  }
}

BOOL WINAPI DllMain(HANDLE hInstNew,
                    ULONG ul_reason_for_call,
                    LPVOID lpReserved)
{
    return TRUE;
}
