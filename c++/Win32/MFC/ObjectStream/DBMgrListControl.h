#ifndef DB_MGR_LIST_CONTROL_H
	#define DB_MGR_LIST_CONTROL_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// DBMgrListControl.h : header file
//


#include "..\MergeMenu.h"		// includes one

class EMDatabase;					// NewDatabaseFromFile() returns one.


/////////////////////////////////////////////////////////////////////////////
// DBMgrListControl window

class DBMgrListControl : public CListCtrl
{
// Construction
public:
	DBMgrListControl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DBMgrListControl)
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual ~DBMgrListControl();

	bool bRefreshTrees;

	void Fill();

	void InsertDBIntoList( 
		int nDBIndex
	);

	void Gather( bool bIncludePublished = false );

	// [Un]selects all items.
	void SelectAll();
	void UnselectAll();

protected:

	/////////////////////
	// Right Click Menu
	//
	
	void PrepareMenu(
		CMenu*	pPopupMenu
	);

	void DisplayRightClickMenu(
		CPoint		point
	);

	//
	///////////////////

	
	EMDatabase* NewDatabaseFromFile(
		UINT	FileDlgTitle,
		bool	bIncludePublished = true
	);


	// Generated message map functions
public:
	afx_msg void OnRightClickKey();
protected:
	//{{AFX_MSG(DBMgrListControl)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetDefault();
	afx_msg void OnPublish();
	afx_msg void OnUnpublish();
	afx_msg void OnDisconnect();
	afx_msg void OnProperties();
	afx_msg void OnConnect();
	afx_msg void OnDBCreate();
	afx_msg void OnDefrag();
	afx_msg void OnDBDefrag();
	afx_msg void OnRepair();
	afx_msg void OnAppend();
	afx_msg void OnClearCache();
	afx_msg void OnDBClearCache();
	afx_msg void OnUpdateAssociations();
	afx_msg void OnVerify();
	afx_msg void OnKeyLog();
	afx_msg void OnGather();
	afx_msg void OnGatherIncludingPublished();
	//}}AFX_MSG

	afx_msg void OnVerifyByType(UINT nID);
	afx_msg void OnVerifyFromType(UINT nID);
	afx_msg void OnGatherTo(UINT nID);
	afx_msg void OnCopyAndGatherTo(UINT nID);
	afx_msg void OnCopyTo(UINT nID);

	// Limit access to in-house functions.
	// Coordinate this with PrepareMenu().
	#ifdef ACCELERATOR_INHOUSE
		afx_msg void OnRefLog();
		afx_msg void OnDBRefLog();
		afx_msg void OnBtreeLog();
		afx_msg void OnDBBtreeLog();
		afx_msg void OnVerifyPrivateOwnership();
	#endif

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // DB_MGR_LIST_CONTROL_H
