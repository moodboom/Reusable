//-------------------------------------------------------------------//
// FontDlg.h : header file
//
// This class can be used as a base for all your dialogs that 
// should share the same custom font.  
//
// We have our own DoModal() and Create() functions, so that both 
// modal and modeless dlgs are handled.  

// NOTE that CDialog::Create() is not virtual (stupid mofo's), so 
// we make our own version here.  Make sure you call the right one.
//
// NOTE: There is really no need to replace all usage of CDialog
// with FontDialog in your derived class, other than the base class
// in the class declaration and the base constructor call.  This
// class doesn't do much besides override DoModal() and Create().
// It doesn't need to be tied in, e.g., to the message mapping.
// 
// NOTE: there is only ONE static font available for use across 
// all dialogs derived from this one.  Any dialog that is 
// opened AFTER the font settings are changed will open using the 
// new font.
//
//	Copyright © 2002 A better Software.
//-------------------------------------------------------------------//

#if !defined FONT_DLG_H
	#define FONT_DLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxpriv.h>				// For undocumented CDialogTemplate class.

#include "FontHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// FontDlg dialog

class FontDlg : public CDialog
{
	typedef CDialog inherited;

public:

	// Constructors
	// Same as CDialog versions, we just call those.
	FontDlg( LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL ); // : CDialog( lpszTemplateName	, pParentWnd		)	{}
	FontDlg( UINT nIDTemplate, CWnd* pParentWnd = NULL); // : CDialog( nIDTemplate			, pParentWnd	)	{}
	
// Dialog Data
	//{{AFX_DATA(FontDlg)
	//}}AFX_DATA
	// This is dangerous, it doesn't set up the template, and we assume this 
	// is done in both DoModal() and Create().
	// FontDlg() : CDialog()	{}

	// Destructor.
	virtual ~FontDlg();

// Implementation

	// Modal dlgs use this.
	int DoModal();

	// Modeless dlgs use this.
	// NOTE the diff between this and CDialog() version - don't call that one!
	BOOL Create( CWnd* pParentWnd = NULL );

	static void SetCustomFont( SimpleFont &sf );
 	static void SetSystemFont()							{ s_psfCustom = 0;	}

	// MDM	2/16/2005 10:04:46 PM
	// Best not do this... apparently, when OnSize() is called, if
	// it is called within OnInitDialog(), it will trigger another
	// call to OnInitDialog(), and cause infinite recursion...
	/*
	//-------------------------------------------------------------------//
	// ForceResize()																		//
	//-------------------------------------------------------------------//
	// This is used to force a resize, e.g., after moving the splitter
	// window or after changing the current property in small view.
	// It's a helper that may or may not be needed by derived classes.
	//-------------------------------------------------------------------//
	void ForceResize()
	{
		// Get the current window size.  It won't be changing, but we
		// want to call OnSize() to update the positions of the controls.
		CRect rectDlg;
		GetClientRect( rectDlg );
		OnSize( SIZE_RESTORED, rectDlg.Width(), rectDlg.Height() );
		UpdateWindow();
	}
	*/

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FontDlg)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:

	void SetTemplateFont( CDialogTemplate& dlt );

	static bool bUseCustomFont() { return ( s_psfCustom != 0 ); }

	static SimpleFont*	s_psfCustom;

	// Generated message map functions
	//{{AFX_MSG(FontDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
#endif	// FONT_DLG_H
