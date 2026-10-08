#if !defined(AFX_SUBCLASSWND_H__DEBF5029_8183_4FDB_BD2F_3DF7DAD5C4AB__INCLUDED_)
#define AFX_SUBCLASSWND_H__DEBF5029_8183_4FDB_BD2F_3DF7DAD5C4AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SubclassWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SubclassWnd window

class SubclassWnd : public CWnd
{
// Construction
public:
	SubclassWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SubclassWnd)
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	CWnd * m_pWndToHide;
	void SetHideWnd(CWnd *pWndToHide);
	MSG varMyPostMessage;
	void FinishUp();
	int m_nPostCommand;
	virtual ~SubclassWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(SubclassWnd)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnBack();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUBCLASSWND_H__DEBF5029_8183_4FDB_BD2F_3DF7DAD5C4AB__INCLUDED_)
