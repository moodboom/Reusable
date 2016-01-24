// ScrollDialog.cpp : implementation file
//

#include "stdafx.h"

#include <ios>						// Use STL-compatible header!

#include "..\PragmaMessages.h"

#include "ScrollDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGripper (included in ScrollDialog.*)
/////////////////////////////////////////////////////////////////////////////

// CGripper message map 
BEGIN_MESSAGE_MAP(CGripper, CScrollBar)
	//{{AFX_MSG_MAP(CGripper)
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CGripper message handlers

// MDM VC8 "fixed" the return value from several macros.
// I'll make it backwards-compatible for now...
#if _MSC_VER >= 1400
	LRESULT CGripper::OnNcHitTest(CPoint point) 
#else
	UINT CGripper::OnNcHitTest(CPoint point) 
#endif
{
	UINT ht = CScrollBar::OnNcHitTest(point);
	if(ht==HTCLIENT)
	{
		ht = HTBOTTOMRIGHT;
	}
	return ht;
}
/////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------//
// CScrollDialog()																	//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
CScrollDialog::CScrollDialog(
	UINT	nIDD,
	CWnd*	pParent,
	bool	bAllowUserResize,
	bool	bAllowDragScroll
) : 

	// Call base class.
	CBkDialog( nIDD, pParent ),

	// Init vars.
	m_bAllowUserResize( bAllowUserResize	),
	m_bAllowDragScroll( bAllowDragScroll	),
	m_bDragging			( false					)

{
	//{{AFX_DATA_INIT(CScrollDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_bInitialized = FALSE;
}


//-------------------------------------------------------------------//
// DoDataExchange()																	//
//-------------------------------------------------------------------//
void CScrollDialog::DoDataExchange(CDataExchange* pDX)
{
	inherited::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScrollDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


// BEGIN_MESSAGE_MAP(CScrollDialog, CDialog)
BEGIN_MESSAGE_MAP(CScrollDialog, inherited)
	//{{AFX_MSG_MAP(CScrollDialog)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScrollDialog message handlers

BOOL CScrollDialog::OnInitDialog() 
{
	inherited::OnInitDialog();

    // Set Initial Scroll Positions
    m_nHscrollPos = 0;
    m_nVscrollPos = 0;

    // Get the initial dimensions of the dialog
    GetClientRect(&m_ClientRect);
    m_bInitialized = TRUE;

	if ( m_bAllowUserResize )
	{
		// Create a resize gripper
		CRect initRect;
		GetClientRect(initRect);
		initRect.left = initRect.right - GetSystemMetrics(SM_CXHSCROLL);
		initRect.top  = initRect.bottom - GetSystemMetrics(SM_CYVSCROLL);

		DWORD dwStyle = WS_CHILD | SBS_SIZEBOX | 
						  SBS_SIZEBOXBOTTOMRIGHTALIGN | 
						  SBS_SIZEGRIP | 
						  WS_VISIBLE;

		m_Grip.Create(dwStyle,initRect, this, AFX_IDW_SIZE_BOX);

		// Modify our window style to ensure that we have a resizeable dialog
		ModifyStyle(
			WS_BORDER | DS_MODALFRAME,
			WS_THICKFRAME | WS_OVERLAPPEDWINDOW,
			SWP_FRAMECHANGED
		);
	}

	// DEBUG
	/*
	ModifyStyle(
		WS_BORDER | DS_MODALFRAME,
		WS_THICKFRAME | WS_OVERLAPPEDWINDOW,
		SWP_FRAMECHANGED
	);
	*/

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CScrollDialog::ResetScrollbars()
{
	/*
    // Reset our window scrolling information
	CRect tempRect;
	GetClientRect(&tempRect);
    ScrollWindow(m_nHscrollPos*tempRect.Height(), 0, NULL, NULL);
    ScrollWindow(0, m_nVscrollPos*tempRect.Width(), NULL, NULL);
    m_nHscrollPos = 0;
    m_nVscrollPos = 0;
    SetScrollPos(SB_HORZ, m_nHscrollPos, TRUE);
    SetScrollPos(SB_VERT, m_nVscrollPos, TRUE);
	*/
}


//-------------------------------------------------------------------//
// SetupScrollbars()																	//
//-------------------------------------------------------------------//
void CScrollDialog::SetupScrollbars()
{
	// The current client rect		= the page size.
	// The provided client size	= the max size.
	CRect tempRect;
	GetClientRect(&tempRect);

	WINDOWPLACEMENT wp;
	GetWindowPlacement(&wp);
	BOOL bMaximized = (wp.showCmd == SW_MAXIMIZE);

	if ( m_bAllowUserResize )
	{
		// Do not show the sizing gripper if not needed.
		if (
				( 
						tempRect.Height() > m_ClientRect.Height()
					|| tempRect.Width() > m_ClientRect.Width()
				) 
			|| bMaximized
		)
			m_Grip.ShowWindow( SW_HIDE );
		else
		{
			m_Grip.ShowWindow( SW_SHOW );
			CRect initRect;
			initRect = tempRect;
			initRect.left = initRect.right - GetSystemMetrics(SM_CXHSCROLL);
			initRect.top  = initRect.bottom - GetSystemMetrics(SM_CYVSCROLL);
			m_Grip.MoveWindow( initRect );
		}
	}

	// MDM	7/19/01 7:06:09 AM
	// Set the thumb size too.
	SCROLLINFO info;
   info.cbSize = sizeof(SCROLLINFO);     
   info.fMask = SIF_PAGE;
   info.nPage = tempRect.Height();
	SetScrollInfo( SB_VERT, &info, FALSE );
   info.nPage = tempRect.Width();
	SetScrollInfo( SB_HORZ, &info, FALSE );

	// Set the vertical and horizontal scrolling info
	int nHscrollMax = __max( 0, m_ClientRect.Width() );
	m_nHscrollPos = __min( m_nHscrollPos, m_ClientRect.Width() );
	SetScrollRange(SB_HORZ, 0, m_ClientRect.Width(), FALSE);
	SetScrollPos(SB_HORZ, m_nHscrollPos, TRUE);
	int nVscrollMax = __max(0, m_ClientRect.Height() );
	m_nVscrollPos = __min(m_nVscrollPos, m_ClientRect.Height() );
	SetScrollRange(SB_VERT, 0, m_ClientRect.Height(), FALSE);
	SetScrollPos(SB_VERT, m_nVscrollPos, TRUE);

	// Setup the gripper
	if ( !bMaximized && m_bAllowUserResize )
	{
		CWnd *resizeWnd = GetDlgItem(AFX_IDW_SIZE_BOX);
		if (resizeWnd) 
		{
			CRect rect, rect2;
			resizeWnd->GetWindowRect(&rect);
			GetClientRect(rect2);
			rect2.left = rect2.right - rect.Width();
			rect2.top  = rect2.bottom - rect.Height();
			resizeWnd->MoveWindow(&rect2);
		}
	}

	// OLD
	/*
	// Calculate how many scrolling increments for the client area
	m_nHorzInc = (m_ClientRect.Width()  - tempRect.Width())/m_nYPage;
	m_nVertInc = (m_ClientRect.Height() - tempRect.Height())/m_nXPage;

	if ( m_bAllowUserResize )
	{
		// Do not show the sizing gripper if not needed.
		if (
				(m_nHorzInc > 0) 
			|| (m_nVertInc > 0 ) 
			|| bMaximized
		)
			m_Grip.ShowWindow( SW_HIDE );
		else
		{
			m_Grip.ShowWindow( SW_SHOW );
			CRect initRect;
			initRect = tempRect;
			initRect.left = initRect.right - GetSystemMetrics(SM_CXHSCROLL);
			initRect.top  = initRect.bottom - GetSystemMetrics(SM_CYVSCROLL);
			m_Grip.MoveWindow( initRect );
		}
	}

	// Set the vertical and horizontal scrolling info
	m_nHscrollMax = __max(0, m_nHorzInc);
	m_nHscrollPos = __min(m_nHscrollPos, m_nHscrollMax);
	SetScrollRange(SB_HORZ, 0, m_nHscrollMax, FALSE);
	SetScrollPos(SB_HORZ, m_nHscrollPos, TRUE);
	m_nVscrollMax = __max(0, m_nVertInc);
	m_nVscrollPos = __min(m_nVscrollPos, m_nVscrollMax);
	SetScrollRange(SB_VERT, 0, m_nVscrollMax, FALSE);
	SetScrollPos(SB_VERT, m_nVscrollPos, FALSE);

	// Setup the gripper
	if ( !bMaximized && m_bAllowUserResize )
	{
		CWnd *resizeWnd = GetDlgItem(AFX_IDW_SIZE_BOX);
		if (resizeWnd) 
		{
			CRect rect, rect2;
			resizeWnd->GetWindowRect(&rect);
			GetClientRect(rect2);
			rect2.left = rect2.right - rect.Width();
			rect2.top  = rect2.bottom - rect.Height();
			resizeWnd->MoveWindow(&rect2);
		}
	}
	*/
}


//-------------------------------------------------------------------//
// OnVScroll()																			//
//-------------------------------------------------------------------//
void CScrollDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( pScrollBar == 0 )
	{
		// The current client rect		= the page size.
		// The provided client size	= the max size.
		CRect tempRect;
		GetClientRect(&tempRect);

		// Handle vertical scrollbar messages
		int nInc;
		switch (nSBCode)
		{
			/*
			// These can be tweaked to better fit the implementation
			case SB_TOP:        nInc = -m_nVscrollPos;               break;
			case SB_BOTTOM:     nInc = m_nVscrollMax-m_nVscrollPos;  break;
			case SB_LINEUP:     nInc = -1;                           break;
			case SB_LINEDOWN:   nInc = 1;                            break;
			case SB_PAGEUP:     nInc = __min(-1, -m_nVertInc);       break;
			case SB_PAGEDOWN:   nInc = __max(1, m_nVertInc);         break;
			case SB_THUMBTRACK: nInc = nPos - m_nVscrollPos;         break;
			default:            nInc = 0;
			*/

			case SB_TOP:        nInc = -m_nVscrollPos;								break;
			case SB_BOTTOM:     nInc = m_ClientRect.Height()-m_nVscrollPos;	break;
			case SB_LINEUP:     nInc = -10;												break;
			case SB_LINEDOWN:   nInc = 10;												break;
			case SB_PAGEUP:     nInc = __min(-1, -tempRect.Height() );			break;
			case SB_PAGEDOWN:   nInc = __max(1, tempRect.Height() );				break;
			case SB_THUMBTRACK: nInc = nPos - m_nVscrollPos;						break;
			default:            nInc = 0;
		}

		#pragma _TODO("Fix this so we can't scroll the dlg off the top of the screen")
		nInc = __max( -m_nVscrollPos, __min( nInc, m_ClientRect.Height() - m_nVscrollPos) );

		if (nInc)
		{
			// We are about to scroll, time to invalidate
			// so the backgrounds get updated.
			InvalidateTransparency();
			
			m_nVscrollPos += nInc;
			// int iMove = -tempRect.Height() * nInc;
			int iMove = -nInc;
			ScrollWindow(0, iMove, NULL, NULL);
			SetScrollPos(SB_VERT, m_nVscrollPos, TRUE);

		}

	}

	inherited::OnVScroll(nSBCode, nPos, pScrollBar);

}


void CScrollDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( pScrollBar == 0 )
	{
		// The current client rect		= the page size.
		// The provided client size	= the max size.
		CRect tempRect;
		GetClientRect(&tempRect);

		// Handle horizontal scrollbar messages
		int nInc;
		switch (nSBCode)
		{
			/*
			// These can be tweaked to better fit the implementation
			case SB_TOP:        nInc = -m_nHscrollPos;               break;
			case SB_BOTTOM:     nInc = m_nHscrollMax-m_nHscrollPos;  break;
			case SB_LINEUP:     nInc = -1;                           break;
			case SB_LINEDOWN:   nInc = 1;                            break;
			case SB_PAGEUP:     nInc = -m_nYPage;                    break;
			case SB_PAGEDOWN:   nInc = m_nYPage;                     break;
			case SB_THUMBTRACK: nInc = nPos - m_nHscrollPos;         break;
			default:            nInc = 0;
			*/

			case SB_TOP:        nInc = -m_nHscrollPos;								break;
			case SB_BOTTOM:     nInc = m_ClientRect.Width() - m_nHscrollPos;	break;
			case SB_LINEUP:     nInc = -10;												break;
			case SB_LINEDOWN:   nInc = 10;												break;
			case SB_PAGEUP:     nInc = -tempRect.Width();							break;
			case SB_PAGEDOWN:   nInc = tempRect.Width();								break;
			case SB_THUMBTRACK: nInc = nPos - m_nHscrollPos;						break;
			default:            nInc = 0;
		}

		nInc = __max( -m_nHscrollPos, __min( nInc, m_ClientRect.Width() - m_nHscrollPos ) );

		if (nInc)
		{
			// We are about to scroll, time to invalidate
			// so the backgrounds get updated.
			InvalidateTransparency();
			
			m_nHscrollPos += nInc;
			int iMove = -tempRect.Width() * nInc;
			ScrollWindow(iMove, 0, NULL, NULL);
			SetScrollPos(SB_HORZ, m_nHscrollPos, TRUE);
		}
	}

	inherited::OnHScroll(nSBCode, nPos, pScrollBar);
}


//-------------------------------------------------------------------//
// OnSize()																				//
//-------------------------------------------------------------------//
void CScrollDialog::OnSize(UINT nType, int cx, int cy) 
{
	inherited::OnSize(nType, cx, cy);
	if (m_bInitialized)
	{
		ResetScrollbars();
		SetupScrollbars();					
    }
}


//-------------------------------------------------------------------//
// OnDestroy()																			//
//-------------------------------------------------------------------//
// Fix from CodeGuru comments.
//-------------------------------------------------------------------//
void CScrollDialog::OnDestroy() 
{
	inherited::OnDestroy();
	m_bInitialized = FALSE;	
}


//-------------------------------------------------------------------//
// OnLButtonDown()																	//
//-------------------------------------------------------------------//
// For "drag-scrolling".
//-------------------------------------------------------------------//
void CScrollDialog::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ( m_bAllowDragScroll )
	{
		// Let's do the drag.
		SetCapture();
		m_bDragging = true;
		
		// MDM	3/15/2003 1:09:23 AM
		// We are going to start tracking the start point
		// and subsequent drag points.  We adjust the start
		// point so that it includes the "scroll position".
		// Then it can be passed directly to OnVScroll()...
		m_ptDragFrom = point + CPoint( m_nHscrollPos, m_nVscrollPos );
		
		::SetCursor(AfxGetApp()->LoadCursor (IDC_CURSOR_HAND));
	}

	inherited::OnLButtonDown(nFlags, point);

}


//-------------------------------------------------------------------//
// OnLButtonUp()																		//
//-------------------------------------------------------------------//
// For "drag-scrolling".
//-------------------------------------------------------------------//
void CScrollDialog::OnLButtonUp(UINT nFlags, CPoint point) 
{
	/// IF WE WHERE DRAGGING THEN RELEASE THE MOUSE CAPTURE
	if( m_bDragging )
	{
		ReleaseCapture();
		m_bDragging = false;
	}
	
	inherited::OnLButtonUp(nFlags, point);
}


//-------------------------------------------------------------------//
// OnMouseMove()																		//
//-------------------------------------------------------------------//
// For "drag-scrolling".
//-------------------------------------------------------------------//
void CScrollDialog::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ( m_bDragging && nFlags & MK_LBUTTON )
	{
		// MDM	3/15/2003 12:00:42 AM
		// We want to scroll if needed.		
		OnVScroll( SB_THUMBTRACK, m_ptDragFrom.y - point.y, 0 );
	}	
	
	inherited::OnMouseMove(nFlags, point);
}


