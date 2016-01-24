#if !defined(DATABASE_MANAGER_DLG_H)
	#define DATABASE_MANAGER_DLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DatabaseManagerDlg.h : header file
//

#include "DBMgrListControl.h"	// This is where the DB's are shown.

/////////////////////////////////////////////////////////////////////////////
// DatabaseManagerDlg dialog

class DatabaseManagerDlg : public CDialog
{
// Construction
public:
	DatabaseManagerDlg();

// Dialog Data
	//{{AFX_DATA(DatabaseManagerDlg)
	enum { IDD = IDD_DATABASE_DLG };
	DBMgrListControl	DBList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DatabaseManagerDlg)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	bool bRefreshTrees;

	bool bOldListMsg;

	// This is our manually-attached accelerators table.
	HACCEL hAccelerators;

	// Generated message map functions
	//{{AFX_MSG(DatabaseManagerDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRightClickKey();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // DATABASE_MANAGER_DLG_H
