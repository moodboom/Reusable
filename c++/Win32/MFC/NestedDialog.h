#if !defined(NESTED_DIALOG_H)
#define NESTED_DIALOG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NestedDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//

#define IDC_CURSOR_HAND                 23000

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


#include "ScrollDialog.h"					// Base class.


/////////////////////////////////////////////////////////////////////////////
// NestedDialog dialog

class NestedDialog : public CScrollDialog
{
	typedef CScrollDialog inherited;

// Construction
public:
	NestedDialog(
		UINT	DialogID		= 0,
		CWnd* pParent		= NULL
	);

// Dialog Data
	//{{AFX_DATA(NestedDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NestedDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CDialog* GetTopmostDlg();

	// Generated message map functions
	//{{AFX_MSG(NestedDialog)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif	// RC_INVOKED
#endif	// !defined(NESTED_DIALOG_H)
