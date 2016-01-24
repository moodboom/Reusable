// ChildDlgTabCtrl.cpp : implementation file


#include "stdafx.h"

#include "..\DeferPos.h"			// This class gives us rock-solid OnSize() redraws.

#include "ChildDlgTabCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ChildDlgTabCtrl


//-------------------------------------------------------------------//
// ChildDlgTabCtrl()																	//
//-------------------------------------------------------------------//
ChildDlgTabCtrl::ChildDlgTabCtrl()
{
}


//-------------------------------------------------------------------//
// ~ChildDlgTabCtrl()																//
//-------------------------------------------------------------------//
ChildDlgTabCtrl::~ChildDlgTabCtrl()
{
	for ( int nA = 0; nA < TabDlgs.size(); nA++ )
		delete TabDlgs.at( nA );
}


//-------------------------------------------------------------------//
// AddDialogTab()																		//
//-------------------------------------------------------------------//
// Make sure these dlgs are fully created before calling Init().
// Also, make sure that you used this tab ctrl as the parent of
// the dialogs!
//-------------------------------------------------------------------//
void ChildDlgTabCtrl::AddDialogTab( 
	CDialog* pNewDlg,
	LPCTSTR	szName
) {
	// Did you make the tab ctrl the parent?
	// Otherwise, sizing will be all screwed up.
	ASSERT( pNewDlg->GetParent() == this );
	
	// The majority of our flicker elimination comes from setting
	// WS_CLIPCHILDREN for the dialog.  MAKE SURE THIS IS SET!
	// See MSJ May '97 C++ Q&A for more info - thanks again, Paul!
	ASSERT( pNewDlg->GetStyle() & WS_CLIPCHILDREN );

	InsertItem( TabDlgs.size(), szName );
	TabDlgs.push_back( pNewDlg );
}


//-------------------------------------------------------------------//
// Init()																				//
//-------------------------------------------------------------------//
// Here, we initialize the tab control frame.  Make sure all the 
// child dialogs have been created at this point.
//-------------------------------------------------------------------//
void ChildDlgTabCtrl::Init()
{
	m_tabCurrent = 0;

	// You did add some dlg pages, right?
	ASSERT( TabDlgs.size() > 0 );

	// Show the first, hide the remainder.
	CRect tabRect, itemRect;
	int nX, nY, nXc, nYc;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);

	nX=itemRect.left;
	nY=itemRect.bottom+1;
	nXc=tabRect.right-itemRect.left-1;
	nYc=tabRect.bottom-nY-1;

	TabDlgs[0]->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
	DlgPtrVector::iterator itDlgs;
	for ( itDlgs = TabDlgs.begin() + 1; itDlgs < TabDlgs.end(); itDlgs++ )
		(*itDlgs)->SetWindowPos(&wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);

}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ChildDlgTabCtrl, CTabCtrlEx)
	//{{AFX_MSG_MAP(ChildDlgTabCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ChildDlgTabCtrl message handlers

void ChildDlgTabCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CTabCtrlEx::OnLButtonDown(nFlags, point);

	if ( m_tabCurrent != GetCurFocus() )
	{
		TabDlgs[m_tabCurrent]->ShowWindow(SW_HIDE);
		m_tabCurrent = GetCurFocus();
		TabDlgs[m_tabCurrent]->ShowWindow(SW_SHOW);
		TabDlgs[m_tabCurrent]->SetFocus();
	}
}


//-------------------------------------------------------------------//
// OnSize()																				//
//-------------------------------------------------------------------//
// Resize the dlgs as well.
//-------------------------------------------------------------------//
void ChildDlgTabCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CTabCtrlEx::OnSize(nType, cx, cy);

	// If we don't have our controls yet, 
	// we have nothing to do here.
	if ( 
			TabDlgs.size() == 0 
		||	TabDlgs[0]->GetSafeHwnd() == NULL 
	)
		return;		

	// DEBUG
	// Force a breakpoint.
	// _ASSERT( false );
	
	// Show the current, hide the remainder.
	CRect tabRect, itemRect, rectControl;
	int nX, nY, nXc, nYc;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);

	nX=itemRect.left;
	nY=itemRect.bottom+1;
	nXc=tabRect.right-itemRect.left-1;
	nYc=tabRect.bottom-nY-1;

	// Use CDeferPos to redraw in one windows refresh cycle.
	{
		CDeferPos dp( this, TabDlgs.size() );
		
		for ( int nA = 0; nA < TabDlgs.size(); nA++ )
		{
			// Get the control's rect in the parent's client coordinates.
			TabDlgs[nA]->GetWindowRect( rectControl );
			ScreenToClient( rectControl );

			// If changed, move.
			if ( rectControl != CRect( CPoint( nX, nY ), CSize( nXc, nYc ) ) )
				dp.MoveWindow( TabDlgs[nA], nX, nY, nXc, nYc, TRUE );

			// All controls that are to be moved should always be visible.
			// Make sure the control is showing, now that we have moved it.
			TabDlgs[nA]->ShowWindow( ( nA == m_tabCurrent ) ? SW_SHOW : SW_HIDE );

		}
	}
		
}
