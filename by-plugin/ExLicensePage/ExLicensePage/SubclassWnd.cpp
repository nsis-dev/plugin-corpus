// SubclassWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ExLicensePage.h"
#include "SubclassWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SubclassWnd

SubclassWnd::SubclassWnd()
{
	m_nPostCommand = -1;
	memset(&varMyPostMessage,0,sizeof(MSG));
	m_pWndToHide = NULL;
}

SubclassWnd::~SubclassWnd()
{
}


BEGIN_MESSAGE_MAP(SubclassWnd, CWnd)
	//{{AFX_MSG_MAP(SubclassWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
//	ON_COMMAND(IDOK,OnOK)
//	ON_COMMAND(IDCANCEL,OnCancel)
//	ON_COMMAND(3,OnBack)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// SubclassWnd message handlers

void SubclassWnd::OnBack()
{
	m_pWndToHide->ShowWindow(SW_HIDE);
}

void SubclassWnd::OnOK()
{
	m_pWndToHide->ShowWindow(SW_HIDE);
}

void SubclassWnd::OnCancel()
{
	m_pWndToHide->ShowWindow(SW_HIDE);
}

void SubclassWnd::FinishUp()
{
	HWND		hWndToPost = m_hWnd;
	UnsubclassWindow();
}

void SubclassWnd::SetHideWnd(CWnd *pWndToHide)
{
	m_pWndToHide = pWndToHide;
}

LRESULT SubclassWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{	
	if(message==WM_COMMAND)
	{
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			OnCancel();
			PostMessage(WM_COMMAND,MAKEWORD(3,0),0);
			break;
		case IDOK:
			OnOK();
			break;
		case 3:
			OnBack();
			break;
		}
	}
	return CWnd::DefWindowProc(message, wParam, lParam);
}
