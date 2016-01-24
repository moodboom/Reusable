//-------------------------------------------------------------------//
// TransparentDlgBar class
//-------------------------------------------------------------------//
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "..\..\GlobalData.h"

#include "..\..\MemDC.h"

#include "TransparentDlgBar.h"

/////////////////////////////////////////////////////////////////////////////
// TransparentDlgBar

IMPLEMENT_DYNAMIC( TransparentDlgBar, CDialogBar )

TransparentDlgBar::TransparentDlgBar()
{
    m_bTransparent = true;
}

BEGIN_MESSAGE_MAP( TransparentDlgBar, CDialogBar )
    ON_COMMAND( ID_TRANSPARENT, OnTransparent )
    ON_MESSAGE( WM_REBAR_CONTEXTMENU, OnReBarContextMenu )
    ON_WM_ERASEBKGND()
    ON_WM_MOVE()
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TransparentDlgBar message handlers

void TransparentDlgBar::OnTransparent() 
{
    m_bTransparent = !m_bTransparent;
    Invalidate();
}

LRESULT TransparentDlgBar::OnReBarContextMenu( WPARAM wParam, LPARAM /*lParam*/ )
{
    CMenu* pMenu = ( CMenu* )wParam;
    ASSERT_VALID( pMenu );

    VERIFY( pMenu->AppendMenu( MF_STRING | ( m_bTransparent ? MF_CHECKED : MF_UNCHECKED ),
        ID_TRANSPARENT, _T("Transparent") ) );

    return 0L;
}

BOOL TransparentDlgBar::OnEraseBkgnd( CDC* pDC )
{
    if ( m_bTransparent )
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
				
				// RemoveUnneededBackgroundRegions( rgnBackground );

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

			// Previous code
			/*
        CWnd* pParent = GetParent();
        ASSERT_VALID( pParent );

        CPoint pt( 0, 0 );
        MapWindowPoints( pParent, &pt, 1 );
        pt = pDC->OffsetWindowOrg( pt.x, pt.y );
        LRESULT lResult = pParent->SendMessage( WM_ERASEBKGND, ( WPARAM )pDC->m_hDC, 0L );
        pDC->SetWindowOrg( pt.x, pt.y );
        return lResult;
		  */
    }

    return CDialogBar::OnEraseBkgnd( pDC );
}

void TransparentDlgBar::OnMove( int x, int y )
{
    if ( m_bTransparent )
    {
        Invalidate();
    }
    else
    {
        CDialogBar::OnMove( x, y );
    }
}

HBRUSH TransparentDlgBar::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
    HBRUSH hbr = CDialogBar::OnCtlColor( pDC, pWnd, nCtlColor );

    if ( m_bTransparent && ( nCtlColor == CTLCOLOR_STATIC ) )
    {
        pDC->SetBkMode( TRANSPARENT );
        hbr = ( HBRUSH )::GetStockObject( NULL_BRUSH );
    }

    return hbr;
}


