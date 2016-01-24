// BkDialog.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "BkDialog.h"

#include "..\PragmaMessages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CBkDialog::CBkDialog(CWnd* pParent /*=NULL*/)
:
	// Call base class.
	inherited( (UINT)0 )
{
	//{{AFX_DATA_INIT(CBkDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Constructor();
}


CBkDialog::CBkDialog(UINT uResourceID, CWnd* pParent)
	: inherited(uResourceID, pParent)
{
	Constructor();
}


CBkDialog::CBkDialog(LPCTSTR pszResourceID, CWnd* pParent)
	: inherited(pszResourceID, pParent)
{
	Constructor();
}


//-------------------------------------------------------------------//
// Constructor()																		//
//-------------------------------------------------------------------//
// Call this from each actual constructor.
//-------------------------------------------------------------------//
void CBkDialog::Constructor()
{
	// Create a hollow brush used in making static controls transparent
	m_brushHollow = (HBRUSH) GetStockObject (HOLLOW_BRUSH);

	// #pragma _DEBUGCODE( "Trying a specific color here..." )
	// m_brushHollow = (HBRUSH) GetStockObject( LTGRAY_BRUSH );
}


CBkDialog::~CBkDialog()
{
	
}


void CBkDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBkDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBkDialog, inherited)
	//{{AFX_MSG_MAP(CBkDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CBkDialog::OnEraseBkgnd(CDC* pDC) 
{
	BOOL bResult;

	// We want to bracket the use of MemDC to
	// handle cleanup after it goes out of scope.
	{
		CMemDC MemDC( pDC );

		CRect rc;
		GetClientRect(rc);
		m_pDC = pDC;

		// Here, we need to call a virtual function to create
		// the region that needs a background draw (ala InvalidateTransparency(),
		// but using CRgn's instead of invalidating.
		CRgn rgnBackground;
		VERIFY( rgnBackground.CreateRectRgn( 0, 0, rc.Width(), rc.Height() ) );
		
		RemoveUnneededBackgroundRegions( rgnBackground );

		/*
		#pragma _DEBUGCODE("Here's some test code that paints the region red.")
		CBrush brA, brB, brC;
		VERIFY(brA.CreateSolidBrush( RGB(255, 0, 0) ));  
		VERIFY(MemDC.FillRgn( &rgnBackground, &brA));      // rgnA Red Filled
		return TRUE;
		*/

		// Set up the region as a filter.
		MemDC.SelectClipRgn( &rgnBackground );

		// Then, call TileBitmap.
		bResult = TileBitmap( &MemDC, rc);

		if ( bResult != TRUE )
			bResult = inherited::OnEraseBkgnd(pDC);
	}

	// Reset the clipping area
	pDC->SelectClipRgn( NULL );

	return bResult;

} // End of OnEraseBkgnd


//-------------------------------------------------------------------//
// PaintBackground()																	//
//-------------------------------------------------------------------//
// Thought this would be good to use in a derived-class OnPaint().
// Turns out we don't do that very often with Dialogs... we'll
// just use OnEraseBkgnd() for now... don't use this unless you
// resynch it with OnEraseBkgnd()...
//-------------------------------------------------------------------//
void CBkDialog::PaintBackground( CDC* pDC )
{
	CRect rc;
	GetClientRect(rc);
	CMemDC MemDC( pDC );
	TileBitmap( &MemDC, rc );
}


HBRUSH CBkDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// MDM	10/5/2004 4:33:52 AM
	// OK time to figure out what's going on...
	// SongPropertiesBar creates a generic dlg, i thought...
	// but we end up here.. here's where we need to fix our
	// control-not-using-background-brush problems...





	
	// DEBUG
	// To turn off dialog-controlled control transparency...
	// for if we are using controls that do this internally...
	// see CEditTrans/CStaticTrans.
	return inherited::OnCtlColor(pDC, pWnd, nCtlColor);

	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here

	// Make static controls transparent
	if (
			nCtlColor == CTLCOLOR_STATIC 
	//	||	nCtlColor == CTLCOLOR_EDIT 
	//	||	nCtlColor == CTLCOLOR_MSGBOX 
	) {
		
		// Make sure that it's not a slider control
		char lpszClassName[256];
		GetClassName (pWnd->m_hWnd, lpszClassName, 255);
		if (strcmp (lpszClassName, TRACKBAR_CLASS) == 0)
			return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

		pDC->SetBkMode (TRANSPARENT);

		// if ( nCtlColor == CTLCOLOR_EDIT )
		// {
		// 	pDC->SetBkColor( 
		// }

		return m_brushHollow;
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}


void CBkDialog::OnSize(UINT nType, int cx, int cy) 
{
	// MDM	9/1/2002 12:31:02 PM
	// When we get a resize, we need to invalidate
	// so that transparent controls redraw their backgrounds.
	InvalidateTransparency();

	// Skip base class or it may end up calling OnInitDialog(!).
	// inherited::OnSize(nType, cx, cy);

}


void CBkDialog::InvalidateTransparency()
{
	// First, invalidate everything.
	Invalidate();

	// Now, validate non-transparent controls.
	CRect rectControl;

	CWnd* pWnd = GetWindow(GW_CHILD);
	while ( pWnd != NULL )
	{
		// Test child type.
		// We want all types except non-read-only edit controls.
		#pragma _TODO("I can't get static controls to be skipped here!")
		char lpszClassName[256];
		GetClassName (pWnd->m_hWnd, lpszClassName, 255);
		if (
			true
			// strcmp (lpszClassName, TRACKBAR_CLASS) == 0
		//		strcmp (lpszClassName, STATUSCLASSNAME) == 0
		//	||	dynamic_cast<CStatic*>( pWnd ) != 0 
		//	( dynamic_cast<CEdit*>( pWnd ) != 0 && ( dynamic_cast<CEdit*>( pWnd )->GetStyle() & ES_READONLY ) != 0 )
		) {
		} else
		{
			pWnd->GetWindowRect( &rectControl );
			ScreenToClient( &rectControl );
			ValidateRect( &rectControl );
		}
		
		pWnd = pWnd->GetWindow( GW_HWNDNEXT );
	}

}
