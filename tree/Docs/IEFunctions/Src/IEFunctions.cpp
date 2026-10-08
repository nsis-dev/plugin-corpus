// IEFunctions.dll
// NSIS pluging with functions for controlling Internet Explorer.
// Author: Remco Lam (kno1@xs4all.nl)
// Thanks to daz for the liposuction (going from a 51K to a 4K filesize)

#include <windows.h>
#include "C:\program files\nsis\contrib\exdll\exdll.h"
#include <Exdisp.h> // voor IWebBrowser2
#include <comdef.h>

#define _NO_ERROR_MESSAGES

// Added utility functions to remove the crt library
#define ARRAYSIZE(a)    (sizeof(a)/sizeof((a)[0]))
int LocalToBSTR(BSTR pWide, LPCSTR pLocal, DWORD dwChars){
	*pWide = 0;
	#ifdef UNICODE
	lstrcpyn(pWide, pLocal, dwChars);
	#else
	MultiByteToWideChar(CP_ACP, 0, pLocal, -1, pWide, dwChars);
	#endif
	return lstrlenW(pWide);
}

// remove the inclusion of the crt - don't want the bloat :o)
void* operator new(size_t size){return(void*)LocalAlloc(LPTR, size);}
void operator delete(void *ptr){if(ptr){LocalFree(ptr);}}


IWebBrowser2* g_pIEBrowser = NULL;

HINSTANCE g_hInstance;

HWND g_hwndParent;

// Runs Internet Explorer through the use of CoCreateInstance (to get a IWebBrowser2 interface pointer)
// pIe has to be NULL
int RunIe( IWebBrowser2 *&pIE ) 
{
   if (pIE == NULL)  // Can only start one instance
   {
	   CoInitialize( NULL );
		
		// Create an instance of Internet Explorer
      HRESULT hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_ALL, 
                                    IID_IWebBrowser2, (void**)&pIE);
      if (SUCCEEDED(hr))
      {
			// Make IE visible
         pIE->put_Visible(VARIANT_TRUE);
			return TRUE;
	  }
#ifndef _NO_ERROR_MESSAGES
	  else
	  {
		  char szMsg[255];
		  // changed this from sprintf
		  wsprintf(szMsg,"CoCreateInstance failed. HRESULT %d",hr);
		  MessageBox(0,szMsg,"DEBUG",MB_OK);
	  }
#endif // _NO_ERROR_MESSAGES
   }
	return FALSE;
}

// in: CLSID-string van een IE-Bar element (toolbar of toolband), en IWebBrowser interface	
void ShowBandObject(IWebBrowser2 *pIE, LPCSTR szBarCLSID, bool bShow = true )
{
	VARIANT	vtBandGUID;
	VARIANT	vtShow;
	VARIANT	vtNotUsed;
	OLECHAR bBarCLSID[2*MAX_PATH];

	LocalToBSTR(bBarCLSID,szBarCLSID,ARRAYSIZE(bBarCLSID));

	vtBandGUID.vt = VT_BSTR;
	vtBandGUID.bstrVal = SysAllocString(bBarCLSID);
	
	vtShow.vt = VT_BOOL;
	vtShow.boolVal = bShow;
	
	vtNotUsed.vt = VT_INT;
	vtNotUsed.intVal = 1;
	
	HRESULT hr = pIE->ShowBrowserBar(&vtBandGUID, &vtShow, &vtNotUsed);
	VariantClear(&vtBandGUID);
}


// Navigate Browser to an URL
int NavigateBrowser(IWebBrowser2 *pIE, LPCSTR szURL)
{
	VARIANT vURL, vtEmpty;
	OLECHAR bstrUrl[2*MAX_PATH];

	LocalToBSTR(bstrUrl,szURL,ARRAYSIZE(bstrUrl));

	vURL.vt = VT_BSTR;
	vURL.bstrVal = SysAllocString(bstrUrl);
	vtEmpty.vt = VT_EMPTY;

	HRESULT hr=pIE->Navigate2(&vURL,&vtEmpty,&vtEmpty,&vtEmpty,&vtEmpty);
	// will autofree the url string allocated earlier
	VariantClear(&vURL);

	if (SUCCEEDED(hr))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

extern "C" void __declspec(dllexport) OpenBrowser(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	g_hwndParent=hwndParent;
	
	EXDLL_INIT();
	
	// do your stuff here
	{
		if (!g_pIEBrowser)
		{
			RunIe(g_pIEBrowser);
		}
#ifndef _NO_ERROR_MESSAGES
		else
		{
			MessageBox(g_hwndParent, "The browser is already opened, close or release the old browser first!", "IEFunctions::OpenBrowser Error", MB_OK|MB_ICONEXCLAMATION);
		}
#endif // _NO_ERROR_MESSAGES
	}
}

extern "C" void __declspec(dllexport) SurfTo(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	g_hwndParent=hwndParent;
	
	EXDLL_INIT();
	
	// do your stuff here
	{
		char *szUrl;
		szUrl = new char[string_size+1];
		//my_memset(szUrl, 0, string_size+1);
		
		popstring(szUrl);
		

		if (g_pIEBrowser)
		{
			NavigateBrowser(g_pIEBrowser, szUrl);
		}
#ifndef _NO_ERROR_MESSAGES
		else
		{
			MessageBox(g_hwndParent, "The browser has not been opened!\r\nCall \"OpenBrowser\" first and do not forget to add the /NOUNLOAD switch.\r\n\r\nRemember to call \"ReleaseBrowser\" or \"CloseBrowser\" afterwards.", "IEFunctions::SurfTo Error", MB_OK|MB_ICONEXCLAMATION);
		}
#endif // _NO_ERROR_MESSAGES
		delete[] szUrl;
	}
}


extern "C" void __declspec(dllexport) ShowBar(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	g_hwndParent=hwndParent;
	
	EXDLL_INIT();
	
	// do your stuff here
	{
		char *szCLSID;
		szCLSID = new char[string_size+1];
		//my_memset(szCLSID, 0, string_size+1);
		
		popstring(szCLSID);
		


		if (g_pIEBrowser)
		{
			ShowBandObject(g_pIEBrowser, szCLSID);
		}
#ifndef _NO_ERROR_MESSAGES
		else
		{
			MessageBox(g_hwndParent, "The browser has not been opened!\r\nCall \"OpenBrowser\" first and do not forget to add the /NOUNLOAD switch.\r\n\r\nRemember to call \"ReleaseBrowser\" or \"CloseBrowser\" afterwards.", "IEFunctions::ShowBar Error", MB_OK|MB_ICONEXCLAMATION);
		}
#endif // _NO_ERROR_MESSAGES
		delete[] szCLSID;
	}
}

extern "C" void __declspec(dllexport) ReleaseBrowser(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	g_hwndParent=hwndParent;
	
	EXDLL_INIT();
	
	// do your stuff here
	{
		if (g_pIEBrowser)
		{
			g_pIEBrowser->Release();
			g_pIEBrowser = NULL;
		}
#ifndef _NO_ERROR_MESSAGES
		else
		{
			MessageBox(g_hwndParent, "The browser has not been opened!\r\nCall \"OpenBrowser\" before releasing the browser and do not forget to add the /NOUNLOAD switch.", "IEFunctions::ReleaseBrowser Error", MB_OK|MB_ICONEXCLAMATION);
		}
#endif // _NO_ERROR_MESSAGES
	}
}

extern "C" void __declspec(dllexport) CloseBrowser(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	g_hwndParent=hwndParent;
	
	EXDLL_INIT();
	
	// do your stuff here
	{
		if (g_pIEBrowser)
		{
			g_pIEBrowser->Quit();
			g_pIEBrowser->Release();
			g_pIEBrowser = NULL;
		}
#ifndef _NO_ERROR_MESSAGES
		else
		{
			MessageBox(g_hwndParent, "The browser has not been opened!\r\nCall \"OpenBrowser\" before closing the browser  and do not forget to add the /NOUNLOAD switch.", "IEFunctions::CloseBrowser Error", MB_OK|MB_ICONEXCLAMATION);
		}
#endif // _NO_ERROR_MESSAGES
	}
}

BOOL WINAPI _DllMainCRTStartup(HMODULE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = (HINSTANCE)hInst;
	// added this
	DisableThreadLibraryCalls(hInst);
    if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		if (g_pIEBrowser)
		{
#ifndef _NO_ERROR_MESSAGES
			MessageBox(g_hwndParent, "The pointer to the browser hasn't been released!\r\n\r\nAfter you're finished with the browser, remember to call CloseBrowser or ReleaseBrowser without the /NOUNLOAD switch. For all other functions, the do use the /NOUNLOAD switch", "IEFunctions Error", MB_OK|MB_ICONEXCLAMATION);
#endif // _NO_ERROR_MESSAGES
			g_pIEBrowser->Release();
			g_pIEBrowser = NULL;
		}
	}
	return TRUE;
}
