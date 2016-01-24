#if !defined(FONT_PROPERTY_SHEET_H)
#define FONT_PROPERTY_SHEET_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FontPropertySheet.h : header file
//

#include "FontHelpers.h"


/////////////////////////////////////////////////////////////////////////////
// FontPropertySheet

class FontPropertySheet : public CPropertySheet
{
	typedef CPropertySheet inherited;
	DECLARE_DYNAMIC(FontPropertySheet)

// Construction
public:
	FontPropertySheet( );
	FontPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	FontPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

	virtual ~FontPropertySheet();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FontPropertySheet)
	public:
	virtual BOOL OnInitDialog();
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:

	static void SetCustomFont( SimpleFont &sf );
 	static void SetSystemFont()							{ s_psfCustom = 0;	}


	// ----------------------------------------------------
   // MDM	2/28/2006 12:56:59 PM
   // You may override these functions in derived classes
   // to take sheet-level action on change and/or apply.
	// ----------------------------------------------------
	virtual void OnChange()  {}
   virtual void OnApply()
   {
   	SetDirty();
   }
	// ----------------------------------------------------


	bool bDirty() { return m_bDirty; }
	void SetDirty( bool bDirty = true ) { m_bDirty = bDirty; }

	// Generated message map functions
protected:
	//{{AFX_MSG(FontPropertySheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	afx_msg LONG OnResizePage (UINT, LONG);
	DECLARE_MESSAGE_MAP()

	RECT m_rctPage;

	static bool bUseCustomFont() { return ( s_psfCustom != 0 ); }
	static SimpleFont*	s_psfCustom;
	static CFont			s_cfCustom;

protected:

	bool m_bDirty;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(FONT_PROPERTY_SHEET_H)
