//-------------------------------------------------------------------//
// DeferPos.h implementation														//
//
// This class wraps the BeginDeferWindowPos/DeferWindowPos/EndDeferWindowPos
// APIs using a "resource allocation is acquisition" idiom.
//
// There used to be MFC and Win32 versions of most functions.  I commented
// out the Win32 stuff cause I didn't wanna maintain it.  Sorry!
//
// This class is derived from code posted on CodeGuru quite a
// while back.
//-------------------------------------------------------------------//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "DeferPos.h"


/////////////////////////////////////////////////////////////////
// Globals Statics Constants												//
/////////////////////////////////////////////////////////////////
//

//
/////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------//
// CDeferPos()																			//
//-------------------------------------------------------------------//
// Constructor.
//
// This sets up the RAIA idiom by calling BeginDeferWindowPos. The 
// number of windows can be passed as an argument to optimize memory 
// management, although the API will grow the memory if needed at 
// run time.
//
// NOTE: this was never fully implemented, due to the difficulty
// of tracking the intended visibility of the controls...
// If a parent pointer is supplied, we will check children - if 
// they do not fit fully on the screen, they will not be shown.
//-------------------------------------------------------------------//
CDeferPos::CDeferPos(
	// CWnd*	pParent,
	int	nWindows
) {
	// We now need MFC for this class.  See header.
	#ifndef	_MFC_VER
		ASSERT( false );
	#endif

	/*
	// Get and save the parent client window.
	if ( m_pParent )
		m_pParent->GetClientRect( m_rectParent );
	*/
	
	m_hdwp = BeginDeferWindowPos( nWindows );
	// TRACE( _T("Deferring windows resize, hdwp = %d\n"), (int)m_hdwp );
}


//-------------------------------------------------------------------//
// ~CDeferPos()																		//
//-------------------------------------------------------------------//
// Destructor
// This concludes the RAIA idiom by ensuring EndDeferWindowPos is 
// called.
//-------------------------------------------------------------------//
CDeferPos::~CDeferPos()
{
	// TRACE( _T("Resizing deferred windows, hdwp = %d\n"), (int)m_hdwp );
	EndDeferWindowPos( m_hdwp );
}


/*
//-------------------------------------------------------------------//
// MoveWindow()																		//
//-------------------------------------------------------------------//
// MoveWindow
// Emulates a call to ::MoveWindow but the actual call is delayed until
// the CDeferPos object is destroyed.  All delayed window positions are
// then done "at once", which can reduce flicker.
//-------------------------------------------------------------------//
BOOL CDeferPos::MoveWindow(
	HWND hWnd, 
	int x, 
	int y, 
	int nWidth, 
	int nHeight,
	BOOL bRepaint
) {

	UINT uFlags = SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER;
	if (!bRepaint)
		uFlags |= SWP_NOREDRAW;
	return SetWindowPos(hWnd, 0, x, y, nWidth, nHeight, uFlags);
}


//-------------------------------------------------------------------//
// SetWindowPos()																		//
//-------------------------------------------------------------------//
// SetWindowPos
// Emulates a call to ::SetWindowPos but the actual call is delayed until
// the CDeferPos object is destroyed.  All delayed window positions are
// then done "at once", which can reduce flicker.
//-------------------------------------------------------------------//
BOOL CDeferPos::SetWindowPos(
	HWND hWnd, 
	HWND hWndAfter, 
	int x, 
	int y, 
	int nWidth,
	int nHeight, 
	UINT uFlags
) {

	if (m_hdwp != 0)
	{
		if ( m_hParent )
		{
			// Check the child rect - if it is not fully inside parent,
			// hide the child and return.


		}
		
		#ifdef _DEBUG
			
			HDWP dwpCheck = DeferWindowPos(m_hdwp, hWnd, hWndAfter, x, y, nWidth, nHeight, uFlags);

			// Are we going to have problems if this changes?  Maybe, maybe not, not sure...
			ASSERT( m_hdwp == dwpCheck );

			m_hdwp = dwpCheck;

		#else
			
			m_hdwp = DeferWindowPos(m_hdwp, hWnd, hWndAfter, x, y, nWidth, nHeight, uFlags);
		
		#endif
	}
	return m_hdwp != 0;
}
*/


//-------------------------------------------------------------------//
// MoveWindow()																		//
//-------------------------------------------------------------------//
// Emulates a call to ::MoveWindow but the actual call is delayed until
// the CDeferPos object is destroyed.  All delayed window positions are
// then done "at once", which can reduce flicker.
//-------------------------------------------------------------------//
BOOL CDeferPos::MoveWindow(
	CWnd* pWnd, 
	int x, 
	int y, 
	int nWidth, 
	int nHeight,
	BOOL bRepaint
) {
	// return ::MoveWindow(pWnd->GetSafeHwnd(), x, y, nWidth, nHeight, bRepaint);

	UINT uFlags = SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER;
	if (!bRepaint)
		uFlags |= SWP_NOREDRAW;
	return SetWindowPos( pWnd, 0, x, y, nWidth, nHeight, uFlags);

}


//-------------------------------------------------------------------//
// SetWindowPos()																		//
//-------------------------------------------------------------------//
// SetWindowPos
// Emulates a call to ::SetWindowPos but the actual call is delayed until
// the CDeferPos object is destroyed.  All delayed window positions are
// then done "at once", which can reduce flicker.
//-------------------------------------------------------------------//
BOOL CDeferPos::SetWindowPos(
	CWnd* pWnd, 
	CWnd* pWndAfter, 
	int x, 
	int y, 
	int nWidth,
	int nHeight, 
	UINT uFlags
) {

	// return SetWindowPos(pWnd->GetSafeHwnd(), pWndAfter->GetSafeHwnd(), x, y, nWidth,	nHeight, uFlags);

	if (m_hdwp != 0)
	{
		// MDM	12/21/2000 11:03:05 AM
		// This was never finished/perfected, instead, we switched to a "scrollable dialog"
		// approach when under a required minimum size.
		/*
		if ( m_pParent )
		{
			// Check the child rect - if it is not fully inside parent,
			// hide the child.
			if ( 
					!m_rectParent.PtInRect( CPoint( x, y ) )
				|| !m_rectParent.PtInRect( CPoint( x + nWidth, y + nHeight ) )
			) {
				if ( pWnd->IsWindowVisible() )
					pWnd->ShowWindow( SW_HIDE );
				return true;

				// x = m_rectParent.right + 1;
				// y = m_rectParent.bottom + 1;
				// nWidth = 4;
				// nHeight = 4;
			}

		}
		*/
		
		// WHOA!!  Big implications...NOT good.
		// if ( !pWnd->IsWindowVisible() )
		// 	pWnd->ShowWindow( SW_SHOWNA );

		#ifdef _DEBUG
			
			HDWP dwpCheck = DeferWindowPos(m_hdwp, pWnd->GetSafeHwnd(), pWndAfter->GetSafeHwnd(), x, y, nWidth, nHeight, uFlags);

			// Are we going to have problems if this changes?  Maybe, maybe not, not sure...
			ASSERT( m_hdwp == dwpCheck );

			m_hdwp = dwpCheck;

		#else
			
			m_hdwp = DeferWindowPos(m_hdwp, pWnd->GetSafeHwnd(), pWndAfter->GetSafeHwnd(), x, y, nWidth, nHeight, uFlags);
		
		#endif
	}
	return m_hdwp != 0;

}
