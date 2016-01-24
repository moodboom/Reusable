#if !defined(DAO_MULTI_MASTER_LIST_H)
#define DAO_MULTI_MASTER_LIST_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DaoMultiMasterList.h : header file
//

#include <afxdao.h>												// For Dao access (no pun intended)

#include "..\ListDataControls\ListDataMasterList.h"	// Base class


/////////////////////////////////////////////////////////////////////////////
// DaoMultiMasterList window

class DaoMultiMasterList : public ListDataMasterList
{
	typedef ListDataMasterList inherited;

// Construction
public:
	DaoMultiMasterList(
		CString	strUniqueRegistryName,
		bool		bUseExternalData			= true,
		UINT		NoItemMenuID				= 0,
		UINT		OneItemMenuID				= 0,
		UINT		MultiItemMenuID			= 0
	);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DaoMultiMasterList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~DaoMultiMasterList();
	
	virtual void Initialize();

	// These are our (shiny new) abstracted functions for database access.
	virtual bool SeekRecord( long ID )
	{
		// Get the corresponding record set.
		COleVariant IDCriterion( ID, VT_I4 );
		return (
			m_pSet->Seek(
				_T("="),
				&IDCriterion
			) != FALSE
		);
	}

	virtual void EditCurrentRecord( int nItem )
	{
		m_pSet->Edit();
	}

	virtual void SaveCurrentRecord( int nItem )
	{
		m_pSet->Update();
	}

	// Our database set.
	// It should be kept open and indexed on its ID index.
	CDaoRecordset* m_pSet;

	// Generated message map functions
protected:
	//{{AFX_MSG(DaoMultiMasterList)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(DAO_MULTI_MASTER_LIST_H)
