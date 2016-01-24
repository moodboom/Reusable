#if !defined(AFX_CHEVDROP_H__E462CCF6_009E_11D4_84C8_0080450EA020__INCLUDED_)
#define AFX_CHEVDROP_H__E462CCF6_009E_11D4_84C8_0080450EA020__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChevDrop.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChevDrop window

#define BUTTONSPERROW	4

class CChevDrop : public CWnd
{
	CToolBarCtrl	m_tb;

	// Imagelist for the toolbar	
	CImageList	m_hImageList;

	// This window receives the notifications for the toolbar !
	HWND	m_hMsgReceiver;

	// Helper to create a toolbar
	HWND	CreateToolBar( HWND hwndParent, HWND hToolToReplicate );

	// Cleans all toolbar 
	BOOL	CleanToolBar( );

// Construction
public:
	CChevDrop();

	BOOL	CreatePopup( CWnd* pParent );

	BOOL	ShowPopup(	
						CWnd*	pMsgReceiver,
						CToolBar* pToolBar,
						CRect	rectDisplayed,
						CPoint	ptScreen );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChevDrop)
	protected:
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChevDrop();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChevDrop)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHEVDROP_H__E462CCF6_009E_11D4_84C8_0080450EA020__INCLUDED_)
