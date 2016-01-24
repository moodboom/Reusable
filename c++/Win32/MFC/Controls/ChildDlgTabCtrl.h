// ChildDlgTabCtrl.h : header file
//

#if !defined( CHILD_DLG_TAB_CTRL_H )
#define CHILD_DLG_TAB_CTRL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
using namespace std;													// REQD for STL classes!

#include "TabCtrlEx.h"


///////////////////////////////////////////////////////
// Globals, constants, statics
///////////////////////////////////////////////////////
//

typedef vector< CDialog* > DlgPtrVector;

//
///////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// ChildDlgTabCtrl window

class ChildDlgTabCtrl : public CTabCtrlEx
{

// Construction
public:
	ChildDlgTabCtrl();
	virtual ~ChildDlgTabCtrl();

// Attributes
public:

// Operations
public:
	
	// First, call this to add all your dlg pages.
	void AddDialogTab( 
		CDialog* pNewDlg,
		LPCTSTR	szName
	);
	
	// Then initially draw it with this.
	// Override this in derived classes if you wish;
	// you may call AddDialogTab() within it before
	// calling this base class version.
	virtual void Init();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ChildDlgTabCtrl)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(ChildDlgTabCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int				m_tabCurrent;
	DlgPtrVector	TabDlgs;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined( CHILD_DLG_TAB_CTRL_H )
