#if !defined(AFX_LICENSEPAGE_H__CFE245FE_D0E0_449C_A285_00B6414A2796__INCLUDED_)
#define AFX_LICENSEPAGE_H__CFE245FE_D0E0_449C_A285_00B6414A2796__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LicensePage.h : header file
//

#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
// LicensePage dialog

class LicensePage : public CDialog
{
// Construction
public:
	CString m_strFile;
	void SetFile(CString sFile);
	LicensePage(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(LicensePage)
	enum { IDD = IDD_LICENSEPAGE };
	CStatic	m_cFooter;
	CStatic	m_cHeader;
	CEdit	m_cOutput;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LicensePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(LicensePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LICENSEPAGE_H__CFE245FE_D0E0_449C_A285_00B6414A2796__INCLUDED_)
