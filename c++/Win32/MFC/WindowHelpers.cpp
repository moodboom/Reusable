//-------------------------------------------------------------------//
// WindowHelpers.cpp
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "FontDlg.h"													// Base class, allows custom font.

#include "WindowHelpers.h"


//-------------------------------------------------------------------//
// GetOnScreenRect()																	//
//-------------------------------------------------------------------//
// This updates the given rect, in screen coordinates, so that it
// is completely visible under the current resolution.  If it is
// already on-screen, it is not adjusted.
//-------------------------------------------------------------------//
CRect GetOnScreenRect( CRect rectWnd )
{
	// We handle multi-display coordinates - see "Multiple Display Monitors"
	// docs in MSDN.
	// Doesn't seem like integers will be handling large virtual room very far 
	// into the future:
	//
	//		32k / 1600 pix/screen = 200 screens wide
	//		32k / 3200 pix/screen = 100 screens wide
	//		32k / 6400 pix/screen =  50 screens wide
	//		etc.
	//
	// Unless 64-bit int's kick in in time.  Update this as needed.
																						// NOTE
																						// Virtual flags only available on Windoesnot 98/2000.  
																						// On other Winblows, use primary display.
	int nDisplayX			= ::GetSystemMetrics( SM_XVIRTUALSCREEN	);	// 0 should be fine here.
	int nDisplayWidth		= ::GetSystemMetrics( SM_CXVIRTUALSCREEN );	if ( nDisplayWidth  == 0 ) nDisplayWidth  = ::GetSystemMetrics( SM_CXSCREEN	);
	int nDisplayY			= ::GetSystemMetrics( SM_YVIRTUALSCREEN	);	// 0 should be fine here.
	int nDisplayHeight	= ::GetSystemMetrics( SM_CYVIRTUALSCREEN );	if ( nDisplayHeight == 0 ) nDisplayHeight = ::GetSystemMetrics( SM_CYSCREEN	);

	int nXBorder = ::GetSystemMetrics( SM_CXSMICON ) * 2;
	int nYBorder = ::GetSystemMetrics( SM_CYSMICON ) * 2;

	if ( rectWnd.top > nDisplayY + nDisplayHeight )
		rectWnd.OffsetRect( 0, ( nDisplayY + nDisplayHeight ) - rectWnd.top - nYBorder );
	if ( rectWnd.bottom < nDisplayY )
		rectWnd.OffsetRect( 0, nDisplayY - rectWnd.bottom + nYBorder );

	if ( rectWnd.left > nDisplayX + nDisplayWidth )
		rectWnd.OffsetRect( ( nDisplayX + nDisplayWidth ) - rectWnd.left - nXBorder, 0 );
	if ( rectWnd.right < nDisplayX )
		rectWnd.OffsetRect( nDisplayX - rectWnd.right + nXBorder, 0 );

	return rectWnd;
}


//-------------------------------------------------------------------//
// GetDialogSizeFromTemplate()													//
//-------------------------------------------------------------------//
// Here, we create a quick and dirty generic dlg so we can extract
// the size.  Paul recommended this in MSJ Sept '98 C++ Q&A.
// The silly little temp dialog class is set up here as well.
//-------------------------------------------------------------------//
class CTempDlg : public FontDlg
{
	typedef FontDlg inherited;

public:
	CTempDlg( int nID )
	:
		// Call base class.
		inherited( nID )
	{}

	CRect m_rcDlg;
protected:
	virtual BOOL OnInitDialog();
};
BOOL CTempDlg::OnInitDialog()
{
	// Initialize dialog: save size, then quit.
	GetWindowRect(&m_rcDlg);
	m_rcDlg -= m_rcDlg.TopLeft();
	EndDialog(0);
	return TRUE;
}
CSize GetDialogSizeFromTemplate( 
	UINT	nIDTemplate, 
	CWnd* pParent 
) {
	CTempDlg dlg( nIDTemplate );
	dlg.Create(
		// MAKEINTRESOURCE(nIDTemplate),		// NOT USED for FontDlg
		pParent
	);
	return dlg.m_rcDlg.Size();
}


//-------------------------------------------------------------------//
// GetAnyAvailableTopmostWindow()												//
//-------------------------------------------------------------------//
// Let's work really hard to find a parent.
//-------------------------------------------------------------------//
CWnd* GetAnyAvailableTopmostWindow()
{
	CWnd* pParent = AfxGetMainWnd();
	
	if ( !pParent || !pParent->GetSafeHwnd() )
	{
		pParent = CWnd::GetSafeOwner();
	}
	if ( !pParent || !pParent->GetSafeHwnd() )
	{
		pParent = CWnd::GetActiveWindow();
	}

	// Walk up the parent chain as far as we can...
	CWnd* pNext = pParent;
	while ( pNext && pNext->GetSafeHwnd() )
	{
		pParent = pNext;
		pNext = pNext->GetParent();
	}

	return pParent;

}
