//-------------------------------------------------------------------//
// ListDataMasterList.h : header file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined(LIST_DATA_MASTER_LIST_H)
#define LIST_DATA_MASTER_LIST_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxtempl.h>							// For CTypedPtrArray

#include "..\OLE\OleListCtrl.h"				// Base class

#include "ListDataControlBase.h"				// We maintain a list of these.


/////////////////////////////////////////////////////////////////////////////
// ListDataMasterList window

class ListDataMasterList : public OleListCtrl
{
	typedef OleListCtrl inherited;

// Construction
public:
	ListDataMasterList(
		CString	strUniqueRegistryName,
		bool		bUseExternalData					= false,
		UINT		NoItemMenuID						= 0,
		UINT		OneItemMenuID						= 0,
		UINT		MultiItemMenuID					= 0
	);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ListDataMasterList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ListDataMasterList();
	
	virtual void Initialize();

	void AddControl(
		ListDataControlBase*	pNewControl
	);

	void SavePendingChanges( 
		ListDataControlBase* pControl,
		bool						bIncludeDBUpdate = true
	);

	// This totally reloads our controls based on the 
	// current selection.
	void ReloadControls();

	// This allows us to dynamically remove a control.
	void RemoveControl( int nIndex );
	
	// This is used to get feedback on indexes of controls.
	int GetControlCount() { return DataControls.GetSize(); }

	// REPLACED, see abstracted functions below...
	// Our database set.
	// It should be kept open and indexed on its ID index.
	// CDaoRecordset* m_pSet;

	// Our controls.
	CTypedPtrArray< CPtrArray, ListDataControlBase*>	DataControls;

protected:

	void CheckControlForChanges( 
		CWnd* pControl 
	);

	int m_nControlCount;

	// These provide access to the database ID from the item's
	// associated data.  The base class uses the item's lParam
	// directly.  Derived classes that wish to store additional
	// data via lParam need to override these and include
	// a place for the database ID.
	virtual long		GetItemID( int nItem							);
	virtual long		GetItemID( LPARAM lParam					);
	// virtual void	SetItemID( int nItem			,	long ID	);
	// virtual void	SetItemID( LPARAM lParam	,	long ID	);

	// Derived classes can take action here.
	// Note that the set is open for editing during this call,
	// and any changes made will be subsequently automatically saved.
	virtual void SetEditHasOccurred() {}

	// This determines if this list is actively the master
	// of its controls.  It may be false in multi-master
	// situations.
	bool m_bActive;

	// These are our (shiny new) abstracted functions for database access.
	virtual bool SeekRecord( long ID ) = 0;
	virtual void EditCurrentRecord( int nItem ) = 0;
	virtual void SaveCurrentRecord( int nItem ) = 0;

	LongVector m_lvPreviousSelection;
	bool m_bProcessingPrevSelection;

	// Generated message map functions
protected:
	//{{AFX_MSG(ListDataMasterList)
	afx_msg void OnOdcachehint(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	// We moved this out here to force it to be virtual.
	// WATCH THE SPELLING, we went with the Class Wizard default.
	// Note that this is a ON_NOTIFY_EX message, so that it gets passed
	// through to the parent as needed.
	afx_msg virtual BOOL OnItemchanged( NMHDR* pNMHDR, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(LIST_DATA_MASTER_LIST_H)
