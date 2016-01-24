// ScrapDlg.h : header file
//

#if !defined(AFX_SCRAPDLG_H__62E26F3E_A5F5_11D5_BCC1_00D0B7E64F2B__INCLUDED_)
#define AFX_SCRAPDLG_H__62E26F3E_A5F5_11D5_BCC1_00D0B7E64F2B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CScrapDlg dialog

class CScrapDlg : public CDialog
{
// Construction
public:
	CScrapDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CScrapDlg)
	enum { IDD = IDD_SCRAP_DIALOG };
	CEdit	m_OutputEdit;
	CString	m_strInput;
	CString	m_strOutput;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScrapDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// ----------------
	// TEST FUNCTIONS
	// ----------------
	// The code for each test should be placed in a function.
	// Add the necessary #include's right before the function.

	void SortedVectorTest();
	void BrowserTest();

	// ----------------


	void ForceRefresh();

	CString ociErr(int rc);
	struct OCIError* m_errhp;

	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CScrapDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnGo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRAPDLG_H__62E26F3E_A5F5_11D5_BCC1_00D0B7E64F2B__INCLUDED_)
