//-------------------------------------------------------------------//
// DialogBar.h interface															//
//-------------------------------------------------------------------//
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef DIALOG_BAR_H
#define DIALOG_BAR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "scbarcf.h"					// Base class - sizing control bar with gripper, caption

#include "..\..\FontDlg.h"			// We contain one of these - it is derived from CDialog and provides a custom font


class DialogBar : public CSizingControlBarCF
{
	typedef CSizingControlBarCF inherited;

	DECLARE_DYNAMIC(DialogBar);

public:
	DialogBar(
		CFrameWnd*	pParent,
		FontDlg*		pDlg,
		UINT			nIDTemplate,
		TCHAR*		pszTitle,
		int			nID,
		DWORD			dwDockStyle		= CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT,
      bool        bUseMaxButton  = false
	);
	virtual ~DialogBar();

	// This will destroy and recreate the contained dialog.
	void RecreateDialog();

protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewBar)
	public:
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CViewBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	// MDM	6/8/2004 11:17:38 PM
	// Testing how to make the client take up NO space...
	afx_msg LRESULT OnSizeMyParent(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	FontDlg*		m_pDlg;
	UINT			m_nIDTemplate;
	CFrameWnd*	m_pParent;

};

#endif	// DIALOG_BAR_H
