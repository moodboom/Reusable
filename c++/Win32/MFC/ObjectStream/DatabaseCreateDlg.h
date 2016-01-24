#if !defined(DATABASE_CREATE_DLG_H)
#define DATABASE_CREATE_DLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


// DatabaseCreateDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// DatabaseCreateDlg dialog

class DatabaseCreateDlg : public CDialog
{
// Construction
public:
	
	// standard constructor
	DatabaseCreateDlg(
		CWnd* pParent = NULL
	);   

// Dialog Data
	//{{AFX_DATA(DatabaseCreateDlg)
	enum { IDD = IDD_DATABASE_CREATE };
	CString	strFilename;
	CString	strDatabaseName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DatabaseCreateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DatabaseCreateDlg)
	afx_msg void OnBrowse();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(DATABASE_CREATE_DLG_H)
