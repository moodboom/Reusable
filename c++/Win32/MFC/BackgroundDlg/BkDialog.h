#ifndef _BKDIALOG_H_
#define _BKDIALOG_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "..\FontDlg.h"													// Base class, allows custom font.

#include "..\MemDC.h"

#include "BackgroundUtil.h"


/////////////////////////////////////////////////////////////////////////////
// CBkDialog dialog

class CBkDialog : public FontDlg, public CBackgroundUtil
{
	typedef FontDlg inherited;

// Construction
public:
	CDC* m_pDC;
	~CBkDialog();
	CBkDialog(LPCTSTR pszResourceID, CWnd* pParent = NULL);
	CBkDialog(UINT uResourceID, CWnd* pParent = NULL);
	CBkDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBkDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBkDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Constructor helper, called in all actual constructors.
	void Constructor();

	// Call this in the derived class' OnPaint().
	void PaintBackground( CDC* pDC );

	virtual void InvalidateTransparency();

	virtual void RemoveUnneededBackgroundRegions( CRgn& rgnBackground ) {}

	// Generated message map functions
	//{{AFX_MSG(CBkDialog)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	// Make this sucker virtual.  Damnit!
	virtual afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()

	HBRUSH		m_brushHollow;

};
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
