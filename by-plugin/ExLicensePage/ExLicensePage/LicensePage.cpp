// LicensePage.cpp : implementation file
//

#include "stdafx.h"
#include "ExLicensePage.h"
#include "LicensePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// LicensePage dialog


LicensePage::LicensePage(CWnd* pParent /*=NULL*/)
	: CDialog(LicensePage::IDD, pParent)
{
	//{{AFX_DATA_INIT(LicensePage)
	//}}AFX_DATA_INIT
}


void LicensePage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LicensePage)
	DDX_Control(pDX, IDC_FOOTER, m_cFooter);
	DDX_Control(pDX, IDC_HEADER, m_cHeader);
	DDX_Control(pDX, IDC_OUTPUT, m_cOutput);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(LicensePage, CDialog)
	//{{AFX_MSG_MAP(LicensePage)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LicensePage message handlers

void LicensePage::SetFile(CString sFile)
{
	m_strFile = sFile;
}

BOOL LicensePage::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd		*pOldWnd = CWnd::FromHandle(FindWindowEx(GetParent()->GetSafeHwnd(),NULL,"#32770",NULL));
	CRect		rRect;

	if(pOldWnd){
		pOldWnd->GetClientRect(rRect);
		pOldWnd->ClientToScreen(rRect);
		GetParent()->ScreenToClient(rRect);
	}
	
	MoveWindow(rRect,true);

	CWnd		*pwndCancelButton = GetParent()->GetDlgItem(IDCANCEL);
	CWnd		*pwndNextButton = GetParent()->GetDlgItem(IDOK);
	CWnd		*pwndBackButton = GetParent()->GetDlgItem(3);
	
	pwndNextButton->SetWindowText("I Agree");

	CString			sLicenseFileContents;

	try
	{
		CString		sHelpFile = AfxGetApp()->m_pszHelpFilePath;

		sHelpFile = sHelpFile.Left(sHelpFile.ReverseFind('\\')+1);
		CFile		vFile(sHelpFile+m_strFile,CFile::modeRead);

		DWORD		dwSize = vFile.GetLength();

		char		*pcTmp = sLicenseFileContents.GetBuffer(dwSize+10);

		if(pcTmp){
			vFile.Read(pcTmp, dwSize);
			sLicenseFileContents.ReleaseBuffer();
		}
	}
	catch(CFileException vException)
	{
	}

	m_cOutput.SetWindowText(sLicenseFileContents);
	m_cOutput.SetSel(0,0,false);
	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void LicensePage::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
}

void LicensePage::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	
	if(nType == SIZE_RESTORED)
	{
		CRect		rRect;

		if(m_cFooter.m_hWnd){
			m_cFooter.GetClientRect(rRect);
			rRect.OffsetRect(0,cy-15);
			m_cFooter.MoveWindow(rRect);
		}

		if(m_cOutput.m_hWnd){
			m_cOutput.GetClientRect(rRect);
			rRect.top = 20;
			rRect.bottom = cy-20;
			rRect.right = cx;
			m_cOutput.MoveWindow(rRect);
		}
	}
}
