// ExLicensePage.h : main header file for the EXLICENSEPAGE DLL
//

#if !defined(AFX_EXLICENSEPAGE_H__7997A202_5889_41F3_B932_54C8BC60A08A__INCLUDED_)
#define AFX_EXLICENSEPAGE_H__7997A202_5889_41F3_B932_54C8BC60A08A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CExLicensePageApp
// See ExLicensePage.cpp for the implementation of this class
//

class CExLicensePageApp : public CWinApp
{
public:
	CExLicensePageApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExLicensePageApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CExLicensePageApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXLICENSEPAGE_H__7997A202_5889_41F3_B932_54C8BC60A08A__INCLUDED_)
