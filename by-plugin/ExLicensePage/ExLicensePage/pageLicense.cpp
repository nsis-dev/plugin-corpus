#include "stdafx.h"
#include "exdll.h"
#include "LicensePage.h"
#include "SubclassWnd.h"

HINSTANCE	g_hInstance;
HWND		g_hwndParent;

void __declspec(dllexport) showPage(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	g_hwndParent=hwndParent;
	g_hInstance = AfxGetInstanceHandle();

	EXDLL_INIT();

	char		strLicenseFile[1024];

	popstring(strLicenseFile);

	HWND			hWndOld = FindWindowEx(hwndParent,NULL,"#32770",NULL); // find window to replace

	if(hWndOld)
		::ShowWindow(hWndOld,SW_HIDE);

	SubclassWnd		wndSubclass;

	wndSubclass.SubclassWindow(g_hwndParent);
	
	LicensePage		vPage(CWnd::FromHandlePermanent(hwndParent));
	int				nCount=0;

	wndSubclass.SetHideWnd(&vPage);
	
	vPage.SetFile(strLicenseFile);
	vPage.Create(MAKEINTRESOURCE(IDD_LICENSEPAGE),CWnd::FromHandlePermanent(hwndParent));

//	void				*lpOurProc=(void*)GetWindowLong(vPage.GetSafeHwnd(),GWL_WNDPROC);
//	void				*lpWndProcOld=(void*)SetWindowLong(g_hwndParent,GWL_WNDPROC,(long)lpOurProc);

	while (vPage.IsWindowVisible()) {
		MSG msg;
		int nResult = GetMessage(&msg, NULL, 0, 0);
		if (!vPage.IsDialogMessage(&msg) && 
			!IsDialogMessage(g_hwndParent,&msg) && 
			!TranslateMessage(&msg))
				DispatchMessage(&msg);
	}
	wndSubclass.FinishUp();
 // SetWindowLong(g_hwndParent,GWL_WNDPROC,(long)lpWndProcOld);
}