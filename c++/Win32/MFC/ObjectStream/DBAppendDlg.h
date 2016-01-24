#if !defined(AFX_DBAPPENDDLG_H__1A016EE8_55B8_11D1_9BA2_00A0C90A1CE1__INCLUDED_)
#define AFX_DBAPPENDDLG_H__1A016EE8_55B8_11D1_9BA2_00A0C90A1CE1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DBAppendDlg.h : header file
//

class EMDatabase;

/////////////////////////////////////////////////////////////////////////////
// DBAppendDlg dialog

class DBAppendDlg : public CDialog
{
// Construction
public:
	DBAppendDlg(
		EMDatabase*	pNewTargetDatabase,
		CWnd*			pParent						= NULL
	);

// Dialog Data
	//{{AFX_DATA(DBAppendDlg)
	enum { IDD = IDD_DATABASE_APPEND };
	CListCtrl	FromList;
	CString	strDBFilename;
	CString	strDBName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DBAppendDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	EMDatabase*			pTargetDatabase;

	CTypedPtrArray<CPtrArray, EMDatabase*> SourceDatabases;

	// Generated message map functions
	//{{AFX_MSG(DBAppendDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAdd();
	afx_msg void OnDel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DBAPPENDDLG_H__1A016EE8_55B8_11D1_9BA2_00A0C90A1CE1__INCLUDED_)
