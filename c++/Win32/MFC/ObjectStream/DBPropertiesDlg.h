#if !defined( DB_PROPERTIES_DLG_H )
	#define DB_PROPERTIES_DLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DBPropertiesDlg.h : header file
//

class EMDatabase;

/////////////////////////////////////////////////////////////////////////////
// DBPropertiesDlg dialog

class DBPropertiesDlg : public CDialog
{
// Construction
public:

	DBPropertiesDlg(
		EMDatabase*	pNewDatabase,
		CWnd*			pParent			= NULL
	);

// Dialog Data
	//{{AFX_DATA(DBPropertiesDlg)
	enum { IDD = IDD_DATABASE_PROPERTIES };
	CListCtrl	DBAssocList;
	CString	strFilename;
	CString	strName;
	CString	strStatus;
	CString	strVersion;
	CString	strAuthor;
	CString	strGroupOrCompacted;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DBPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	EMDatabase*	pDatabase;

	void FillAssocList();

	// Generated message map functions
	//{{AFX_MSG(DBPropertiesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnGetAssoc();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined( DB_PROPERTIES_DLG_H )
