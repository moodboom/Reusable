//-------------------------------------------------------------------//
// MultiColumnSortHeaderCtrl.h : header file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined(MULTI_COLUMN_SORT_HEADER_CTRL_H)
#define MULTI_COLUMN_SORT_HEADER_CTRL_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "..\..\..\..\ustring.h"

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define IDB_ARROW1UP					19000
#define IDB_ARROW1DOWN				19001
#define IDB_ARROW2UP					19002
#define IDB_ARROW2DOWN				19003
#define IDB_ARROW3UP					19004
#define IDB_ARROW3DOWN				19005
// 
// Since this is a reusable class, we don't have access to a resource file.
//
// 1) Make sure that the above defines are "rolled into" your project's
//		resource file.  Select "Resource Set Includes" on the View menu and add 
//		this header file to the "Read-only symbol directives" listbox.
// 2) Make sure your project includes the associated resources in its
//		resource file.
//
// We want to exclude the remainder of the include file when dealing with App Studio.
#ifndef RC_INVOKED
/////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------//
// Globals / Statics
//-------------------------------------------------------------------//
const int cnArrowCount = 6;
//-------------------------------------------------------------------//


/////////////////////////////////////////////////////////////////////////////
// MultiColumnSortHeaderCtrl window

class MultiColumnSortHeaderCtrl : public CHeaderCtrl
{
// Construction
public:
	MultiColumnSortHeaderCtrl();
	virtual ~MultiColumnSortHeaderCtrl();
	
	int  SetSortImage( 
		int	nCol, 
		BOOL	bAsc,
		int	nSortRank = 0
	);
	
	int     m_nSortCol;
	BOOL    m_bSortAsc;

	CBitmap m_bmpArrows [ cnArrowCount ];
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MultiColumnSortHeaderCtrl)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void RemoveAllSortImages();
	void RemoveSortImage( int iItem );
	const int GetLastColumn() const;
	int m_iLastColumn;

	// Generated message map functions
protected:
	//{{AFX_MSG(MultiColumnSortHeaderCtrl)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	
	// MDM
	// I removed this, we now track this notification in the parent list control.
	//	afx_msg void OnEndtrack(NMHDR* pNMHDR, LRESULT* pResult);


	////////////////////////////////////////////////////////////
	// COLUMN DESCRIPTIONS
	//

public:
	ustring GetColumnDescription( int nAbsCol )
	{ 
		return vstrColDesc[ nAbsCol ]; 
	}
	void SetColumnDescription( int nAbsCol, ustring strDesc )
	{ 
		if ( (int)vstrColDesc.size() < nAbsCol + 1 )
			vstrColDesc.resize( nAbsCol + 1 );

		vstrColDesc[ nAbsCol ] = strDesc; 
	}

protected:
	std::vector<ustring> vstrColDesc;
	//
	////////////////////////////////////////////////////////////


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif	// RC_INVOKED
#endif	// !defined(MULTI_COLUMN_SORT_HEADER_CTRL_H)
