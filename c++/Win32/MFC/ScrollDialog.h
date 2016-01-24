#if !defined(SCROLL_DIALOG_H)
#define SCROLL_DIALOG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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


#include "BackgroundDlg\BkDialog.h"

// ScrollDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGripper
// This class is for the resizing gripper control
class CGripper : public CScrollBar
{
// Construction
public:
	CGripper(){};
// Implementation
public:
	virtual ~CGripper(){};
// Generated message map functions
protected:
	//{{AFX_MSG(CGripper)

	// MDM VC8 "fixed" the return value from several macros.
	// I'll make it backwards-compatible for now...
	#if _MSC_VER >= 1400
		afx_msg LRESULT OnNcHitTest(CPoint point);
	#else
		afx_msg UINT OnNcHitTest(CPoint point);
	#endif

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CScrollDialog dialog

class CScrollDialog : public CBkDialog
{
	typedef CBkDialog inherited;

// Construction
public:
	
	// standard constructor
	CScrollDialog(
		UINT	nIDD, 
		CWnd*	pParent = NULL,
		bool	bAllowUserResize = false,
		bool	bAllowDragScroll = true
	);

	void SetClientRect( CRect& rectNew )	{ m_ClientRect = rectNew; }

// Dialog Data
	//{{AFX_DATA(CScrollDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScrollDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetupScrollbars();
	void ResetScrollbars();
   
	// Keep the scrolling info around for when a scroll occurs.
	// int m_nHorzInc,		m_nVertInc;
	// int m_nVscrollMax,	m_nHscrollMax;
	// int m_nXPage,			m_nYPage;
	int m_nVscrollPos,	m_nHscrollPos;

	CRect m_ClientRect;
	BOOL m_bInitialized;
	CGripper m_Grip;

	bool m_bAllowUserResize;

	// Generated message map functions
	//{{AFX_MSG(CScrollDialog)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// MDM	3/14/2003 11:45:27 PM
	// For dragging.
	bool			m_bAllowDragScroll;
	bool			m_bDragging;
	CPoint		m_ptDragFrom;
	int         posY;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif	// RC_INVOKED
#endif	// !defined(SCROLL_DIALOG_H)
