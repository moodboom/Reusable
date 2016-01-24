// DialogSplitterCtrl.cpp : implementation file
//
//
//	NOTE: YOU WILL NOT SEE ANY SPLITTER CURSORS WITH THE DEFAULT APPWIZARD-GENERATED PROJECT.
//			YOU MUST REMOVE THE FOLLOWING LINE IN THE "Resource Includes" SECTION OF YOUR RESOURCE FILE:
//
//		#define _AFX_NO_SPLITTER_RESOURCES
//
// Select "View|Resource Includes" and remove the line.
//
//

#include "stdafx.h"

#include "DialogSplitterCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DialogSplitterCtrl

DialogSplitterCtrl::DialogSplitterCtrl( bool bVertical )
:

	// Init vars.
	m_bVertical( bVertical )

{
}

DialogSplitterCtrl::~DialogSplitterCtrl()
{
}


//-------------------------------------------------------------------//
// BindWithControl()																	//
//-------------------------------------------------------------------//
BOOL DialogSplitterCtrl::BindWithControl(CWnd *parent, DWORD ctrlId)
{
	// We must have a valid control with which to bind.
	ASSERT( parent->GetDlgItem(ctrlId)->GetSafeHwnd() );
	
	m_Parent = parent;
	
	// Set default borders.
	// Note that we used to try to set the right border to the edge
	// of the parent.  BUT this may be called before the parent
	// is properly sized, and we end up with a bum border.  Set
	// the left border to a safe minimum, and leave the right border
	// for the parent to set.
	m_nLeftBorder = 3;
	m_nRightBorder = -1;
	/*
	CRect rectParent;
	m_Parent->GetClientRect( &rectParent );
	m_nRightBorder = 
		( m_bVertical? rectParent.Width() : rectParent.Height() )
		- m_nLeftBorder;
	*/

	SubclassWindow(m_Parent->GetDlgItem(ctrlId)->GetSafeHwnd());

	// Make sure to get mouse message from the dialog window
	DWORD style = GetStyle();
	::SetWindowLong(GetSafeHwnd(), GWL_STYLE, style | SS_NOTIFY);

	return TRUE;
}


//-------------------------------------------------------------------//
// Unbind()																				//
//-------------------------------------------------------------------//
void DialogSplitterCtrl::Unbind(void)
{
	// DetachAllPanes();
	UnsubclassWindow();
}

/*
void DialogSplitterCtrl::SetMinWidth(int left, int right)
{
    m_minLeft = left;
    m_minRight = right;
}

BOOL DialogSplitterCtrl::AttachAsLeftPane(DWORD ctrlId)
{
    m_leftIds.Add(ctrlId);
    return TRUE;
}

BOOL DialogSplitterCtrl::AttachAsRightPane(DWORD ctrlId)
{
    m_rightIds.Add(ctrlId);
    return TRUE;
}

BOOL DialogSplitterCtrl::DetachAllPanes(void)
{
    m_leftIds.RemoveAll();
    m_rightIds.RemoveAll();
    return TRUE;
}

void DialogSplitterCtrl::RecalcLayout(void)
{
    CWnd *pane;
    RECT rcBar, rcPane;

    GetWindowRect(&rcBar);
    m_Parent->ScreenToClient(&rcBar);

    int i;
    DWORD id;

    for (i=0; i<m_leftIds.GetSize(); i++) {
        id = m_leftIds.GetAt(i);
	    pane = m_Parent->GetDlgItem(id);
        pane->GetWindowRect(&rcPane);
        m_Parent->ScreenToClient(&rcPane);
        rcPane.right = rcBar.left - 1;
        pane->MoveWindow(&rcPane, FALSE);
    }

    for (i=0; i<m_rightIds.GetSize(); i++) {
        id = m_rightIds.GetAt(i);
	    pane = m_Parent->GetDlgItem(id);
        pane->GetWindowRect(&rcPane);
        m_Parent->ScreenToClient(&rcPane);
        rcPane.left = rcBar.right + 1;
        pane->MoveWindow(&rcPane, FALSE);
    }

    m_Parent->Invalidate();
}

//-------------------------------------------------------------------//
// GetMouseClipRect()																//
//-------------------------------------------------------------------//
BOOL DialogSplitterCtrl::GetMouseClipRect(LPRECT rectClip, CPoint point)
{
	RECT rectOrg, rectTarget, rectParent;

	GetWindowRect(&rectOrg);
	m_Parent->GetClientRect(&rectParent);
	m_Parent->ClientToScreen(&rectParent);

	rectTarget = rectOrg;
	rectTarget.left = rectParent.left + m_minLeft;

	RECT rectPane;
	DWORD id;
	int i;
	for (i=0; i<m_leftIds.GetSize(); i++) {
		id = m_leftIds.GetAt(i);
		m_Parent->GetDlgItem(id)->GetWindowRect(&rectPane);
		if (rectTarget.left < rectPane.left + m_minLeft) {
			rectTarget.left = rectPane.left + m_minLeft;
		}
	}

	rectTarget.right = rectParent.right - m_minRight;
	
	for (i=0; i<m_rightIds.GetSize(); i++) {
		id = m_rightIds.GetAt(i);
		m_Parent->GetDlgItem(id)->GetWindowRect(&rectPane);
		if (rectTarget.right > rectPane.right - m_minRight) {
			rectTarget.right = rectPane.right - m_minRight;
		}
	}

	if (rectTarget.left >= rectTarget.right) {
		TRACE("No room to drag the x-splitter bar");
		return FALSE;
	}

	rectClip->left = rectTarget.left + point.x;
	rectClip->right = rectTarget.right - (rectOrg.right - rectOrg.left - point.x) + 1;
	rectClip->top = rectOrg.top;
	rectClip->bottom = rectOrg.bottom;

	return TRUE;
}


*/

BEGIN_MESSAGE_MAP(DialogSplitterCtrl, CWnd)
	//{{AFX_MSG_MAP(DialogSplitterCtrl)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DialogSplitterCtrl message handlers


//-------------------------------------------------------------------//
// OnSetCursor()																		//
//-------------------------------------------------------------------//
// This was taken directly from CSplitterWnd::SetSplitCursor(),
// in WINSPLIT.CPP.
//
//	IMPORTANT NOTE REGARDING RESOURCES:
//
//		Note: If this LoadCursor call fails, it is likely that
//		 _AFX_NO_SPLITTER_RESOURCES is defined in your .RC file.
//		To correct the situation, remove the following line from your
//		 resource script:
//		     #define _AFX_NO_SPLITTER_RESOURCES
//		This should be done using the Resource.Set Includes... command.
//
// NOTE: Apparently our definition of horiz/vert differs from MFC's.
//-------------------------------------------------------------------//

// cache of last needed cursor
AFX_STATIC_DATA HCURSOR _afx_hcurLast = NULL;
AFX_STATIC_DATA HCURSOR _afx_hcurDestroy = NULL;
AFX_STATIC_DATA UINT _afx_idcPrimaryLast = 0; // store the primary IDC

BOOL DialogSplitterCtrl::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	// This USED to do it!
	/*
	if ( m_bVertical )
	   ::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_HSPLITBAR));
	else 
		::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR));
	*/

	UINT idcPrimary;        // app supplied cursor
	LPCTSTR idcSecondary;    // system supplied cursor (MAKEINTRESOURCE)

	// AfxLockGlobals(CRIT_SPLITTERWND);
	if ( m_bVertical )
	{
		idcPrimary = AFX_IDC_HSPLITBAR;
		idcSecondary = IDC_SIZEWE;
	}
	else
	{
		idcPrimary = AFX_IDC_VSPLITBAR;
		idcSecondary = IDC_SIZENS;
	}

	if (idcPrimary != 0)
	{
		HCURSOR hcurToDestroy = NULL;
		if (idcPrimary != _afx_idcPrimaryLast)
		{
			HINSTANCE hInst = AfxFindResourceHandle(
				MAKEINTRESOURCE(idcPrimary), RT_GROUP_CURSOR);

			// load in another cursor
			hcurToDestroy = _afx_hcurDestroy;

			// Note: If this LoadCursor call fails, it is likely that
			//  _AFX_NO_SPLITTER_RESOURCES is defined in your .RC file.
			// To correct the situation, remove the following line from your
			//  resource script:
			//      #define _AFX_NO_SPLITTER_RESOURCES
			// This should be done using the Resource.Set Includes... command.

			if ((_afx_hcurDestroy = _afx_hcurLast =
			   ::LoadCursor(hInst, MAKEINTRESOURCE(idcPrimary))) == NULL)
			{
				// will not look as good
				TRACE0("Warning: Could not find splitter cursor - using system provided alternative.\n");

				ASSERT(_afx_hcurDestroy == NULL);    // will not get destroyed
				_afx_hcurLast = ::LoadCursor(NULL, idcSecondary);
				ASSERT(_afx_hcurLast != NULL);
			}
			_afx_idcPrimaryLast = idcPrimary;
		}
		ASSERT(_afx_hcurLast != NULL);
		::SetCursor(_afx_hcurLast);
		ASSERT(_afx_hcurLast != hcurToDestroy);
		if (hcurToDestroy != NULL)
			::DestroyCursor(hcurToDestroy); // destroy after being set
	}
	// AfxUnlockGlobals(CRIT_SPLITTERWND);


    return TRUE;
}


//-------------------------------------------------------------------//
// OnLButtonDown()																	//
//-------------------------------------------------------------------//
// Here's where we do our stuff.
//
// We send messages to the parent to notify it of results.
// The notification messages are as follows:
//
//		WM_SPLITTER_MOVED					Sent as splitter is dragged
//			wParam								Amount splitter moved since last msg
//			lParam								Splitter ID
//												
//		WM_SPLITTER_FINISHED_MOVE		Sent after user finishes splitter drag
//			wParam								(not used)
//			lParam								Splitter ID
//			
//-------------------------------------------------------------------//
void DialogSplitterCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint point) 
{
	// don't handle if capture already set
	if (::GetCapture() != NULL) return;

	/*
	// don't handle if no room to drag
	RECT rectMouseClip;
	if (!GetMouseClipRect(&rectMouseClip, point)) return;
	::ClipCursor(&rectMouseClip);
	*/

	// set capture to the window which received this message
	SetCapture();
	ASSERT(this == CWnd::GetCapture());

	// No more drag rects.
	/*
	// get DC for drawing
	CDC* pDrawDC;
    pDrawDC = m_Parent->GetDC();
	ASSERT_VALID(pDrawDC);
	*/

	CRect		rectWnd;
	int		curX;
	// int		curY;
	// int		xDiff, yDiff;
	
	// No more drag rects.
	// CRect		rectOrg;
	// CSize		sizeBar;

	GetWindowRect( rectWnd );

	// No more drag rects.
	// sizeBar = CSize(rectOrg.Width(), rectOrg.Height());

	m_Parent->ScreenToClient( rectWnd );

	// DEBUG
	// Our coords are off somewhere...
	// TRACE( _T("Started bar at: %d\t: %d width\n"), rectOrg.left, rectOrg.Width() );
    
	// No more drag rects.
	// pDrawDC->DrawDragRect(&rectOrg, sizeBar, NULL, sizeBar);
   
	// rectCur = rectOld;
	// xDiff = yDiff = 0;

	// get messages until capture lost or cancelled/accepted
	for (;;) 
	{
		MSG msg;
		VERIFY( ::GetMessage( &msg, NULL, 0, 0 ) );

		if (CWnd::GetCapture() != this)
			break;

		// We handle movement/accept messages.
		switch ( msg.message ) 
		{
			case WM_MOUSEMOVE:
				
				// Get the mouse position in SPLITTER CLIENT COORDINATES.
				curX = (int)(short)( m_bVertical? LOWORD( msg.lParam ) : HIWORD( msg.lParam ) );

				// If we have moved, and we are not at our limit...
				if ( 
						curX != 0 
					&& ( 
								m_nLeftBorder == -1
							||	( m_bVertical? rectWnd.left  : rectWnd.top    ) + curX >= m_nLeftBorder
					)
					&& (		m_nRightBorder == -1
							||	( m_bVertical? rectWnd.right : rectWnd.bottom ) + curX <= m_nRightBorder
					)
				) {
				
					// Update the position of the splitter ( in DIALOG CLIENT COORDS )
					// using the splitter position ( in SPLITTER CLIENT COORDS ).
					// In other words, curX is the new position of the splitter
					// relative to the previous position, i.e., the CHANGE in position.
					if ( m_bVertical )
					{
						rectWnd.left += curX;
						rectWnd.right += curX;
					} else
					{
						rectWnd.top += curX;
						rectWnd.bottom += curX;
					}

					// Update the current point, now that we have updated the wnd.
					// point.x = curX;
					
					// MDM	8/5/99 11:15:07 AM
					// TO DO
					// Forget tracking the old rect.
					// Just move the splitter and send a message to the parent to 
					// resize accordingly.
					// ALSO:
					// Forget having to call AttachAs...() and RecalcLayout(), just
					// let the parent know when we are dragged, so it can resize.

					// maybe, if time...
					// Combine two classes into one, use a boolean to differentiate.

					// pDrawDC->DrawDragRect(&rectCur, sizeBar, &rectOld, sizeBar);

					// Move and redraw immediately so we don't leave pixel juice.
					// Or don't redraw?
					// MoveWindow( rectWnd, FALSE );
					// Or don't move.  Expect the parent to handle the move.  That
					// way, it can assure that controls are moved in the proper order
					// to prevent overlapping and subsequent pixel smear.
					// MoveWindow( rectWnd );

					// rectOld = rectCur;
					
					// DEBUG
					// TRACE( _T("Moved bar to:   %d\t: %d width\n"), rectWnd.left, rectWnd.Width() );
					// TRACE( _T("Mouse point:	   %d\t: %d width\n"), curX, curY );
					// TRACE( _T("Orig point:     %d\t: %d width\n"), point.x, point.y );

					// m_Parent->SendMessage( WM_SPLITTER_MOVED, xDiff, GetDlgCtrlID() );

					// ROOT - Send it to parent as well, to cover for cases where
					// the frame holds the controls and not the view.
					if ( FALSE == m_Parent->SendMessage( WM_SPLITTER_MOVED, (UINT) curX, GetDlgCtrlID() ) )
					{
						CWnd *pGrandParent = m_Parent->GetParent();
						if ( pGrandParent )
							pGrandParent->SendMessage( WM_SPLITTER_MOVED, (UINT) curX, GetDlgCtrlID() );
					}
				}
				break;

		// handle cancel messages
		case WM_KEYDOWN:
			if (msg.wParam != VK_ESCAPE)
				break;
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
            goto ExitLoop;

		// just dispatch rest of the messages
		default:
			DispatchMessage(&msg);
			break;
		}
	}

ExitLoop:
	
	// No more drag rects.
	// pDrawDC->DrawDragRect(&rectCur, sizeBar, NULL, sizeBar);
	// m_Parent->ReleaseDC(pDrawDC);

	ReleaseCapture();
	::ClipCursor(NULL);

	// Make sure that we have made the final move.
	// if ( xDiff != 0 )
	// 	m_Parent->SendMessage( WM_SPLITTER_MOVED, xDiff, GetDlgCtrlID() );		

	// move the splitter bar & re-position the attached panes if necessary
	// MoveWindow(rectCur, FALSE);
	// RecalcLayout();

	// Let the parent know that we are done.
	m_Parent->SendMessage( WM_SPLITTER_FINISHED_MOVE, 0, GetDlgCtrlID() );

}


/*

/////////////////////////////////////////////////////////////////////////////
// CySplitterWnd

CySplitterWnd::CySplitterWnd()
{
}

CySplitterWnd::~CySplitterWnd()
{
}

BOOL CySplitterWnd::BindWithControl(CWnd *parent, DWORD ctrlId)
{
    m_Parent = parent;

    SubclassWindow(m_Parent->GetDlgItem(ctrlId)->GetSafeHwnd());
    
    // Make sure to get mouse message from the dialog window
    DWORD style = GetStyle();
    ::SetWindowLong(GetSafeHwnd(), GWL_STYLE, style | SS_NOTIFY);

    return TRUE;
}

void CySplitterWnd::Unbind(void)
{
    DetachAllPanes();
    UnsubclassWindow();
}

void CySplitterWnd::SetMinHeight(int above, int below)
{
    m_minAbove = above;
    m_minBelow = below;
}

BOOL CySplitterWnd::AttachAsAbovePane(DWORD ctrlId)
{
    m_aboveIds.Add(ctrlId);
    return TRUE;
}

BOOL CySplitterWnd::AttachAsBelowPane(DWORD ctrlId)
{
    m_belowIds.Add(ctrlId);
    return TRUE;
}

BOOL CySplitterWnd::DetachAllPanes(void)
{
    m_aboveIds.RemoveAll();
    m_belowIds.RemoveAll();
    return TRUE;
}

void CySplitterWnd::RecalcLayout(void)
{
    CWnd *pane;
    RECT rcBar, rcPane;

    GetWindowRect(&rcBar);
    m_Parent->ScreenToClient(&rcBar);

    int i;
    DWORD id;

    for (i=0; i<m_aboveIds.GetSize(); i++) {
        id = m_aboveIds.GetAt(i);
	    pane = m_Parent->GetDlgItem(id);
        pane->GetWindowRect(&rcPane);
        m_Parent->ScreenToClient(&rcPane);
        rcPane.bottom = rcBar.top - 1;
        pane->MoveWindow(&rcPane, FALSE);
    }

    for (i=0; i<m_belowIds.GetSize(); i++) {
        id = m_belowIds.GetAt(i);
	    pane = m_Parent->GetDlgItem(id);
        pane->GetWindowRect(&rcPane);
        m_Parent->ScreenToClient(&rcPane);
        rcPane.top = rcBar.bottom + 1;
        pane->MoveWindow(&rcPane, FALSE);
    }

    m_Parent->Invalidate();
}

BOOL CySplitterWnd::GetMouseClipRect(LPRECT rectClip, CPoint point)
{
    RECT rectOrg, rectTarget, rectParent, rectPane;
    int i;
    DWORD id;

    GetWindowRect(&rectOrg);
    m_Parent->GetClientRect(&rectParent);
    m_Parent->ClientToScreen(&rectParent);

    rectTarget = rectOrg;
    rectTarget.top = rectParent.top + m_minAbove;
    for (i=0; i<m_aboveIds.GetSize(); i++) {
        id = m_aboveIds.GetAt(i);
        m_Parent->GetDlgItem(id)->GetWindowRect(&rectPane);
        if (rectTarget.top < rectPane.top + m_minAbove) {
            rectTarget.top = rectPane.top + m_minAbove;
        }
    }

    rectTarget.bottom = rectParent.bottom - m_minBelow;
    for (i=0; i<m_belowIds.GetSize(); i++) {
        id = m_belowIds.GetAt(i);
        m_Parent->GetDlgItem(id)->GetWindowRect(&rectPane);
        if (rectTarget.bottom > rectPane.bottom - m_minBelow) {
            rectTarget.bottom = rectPane.bottom - m_minBelow;
        }
    }

    if (rectTarget.top >= rectTarget.bottom) {
        TRACE("No room to drag the y-splitter bar");
        return FALSE;
    }

    rectClip->left = rectOrg.left;
    rectClip->right = rectOrg.right;
    rectClip->top = rectTarget.top + point.y;
    rectClip->bottom = rectTarget.bottom - (rectOrg.bottom - rectOrg.top - point.y) + 1;

    return TRUE;
}


BEGIN_MESSAGE_MAP(CySplitterWnd, CWnd)
	//{{AFX_MSG_MAP(CySplitterWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CySplitterWnd message handlers

BOOL CySplitterWnd::OnSetCursor(CWnd* , UINT , UINT ) 
{
	// TODO: Add your message handler code here and/or call default
    ::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR));
    return TRUE;
	
	//return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CySplitterWnd::OnLButtonDown(UINT , CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	// don't handle if capture already set
	if (::GetCapture() != NULL) return;

    // don't handle if no room to drag
    RECT rectMouseClip;
    if (!GetMouseClipRect(&rectMouseClip, point)) return;
    ::ClipCursor(&rectMouseClip);

	// set capture to the window which received this message
	SetCapture();
	ASSERT(this == CWnd::GetCapture());

    // get DC for drawing
	CDC* pDrawDC;
    pDrawDC = m_Parent->GetDC();
	ASSERT_VALID(pDrawDC);

    int     curX, curY;
	int     xDiff, yDiff;
	CRect   rectOrg, rectCur, rectOld;
    CSize   sizeBar;

    GetWindowRect(rectOrg);
    sizeBar = CSize(rectOrg.Width(), rectOrg.Height());

    m_Parent->ScreenToClient(rectOrg);
    pDrawDC->DrawDragRect(&rectOrg, sizeBar, NULL, sizeBar);
    rectOld = rectCur = rectOrg;
    xDiff = yDiff = 0;

	// get messages until capture lost or cancelled/accepted
	for (;;) {
		MSG msg;
		VERIFY(::GetMessage(&msg, NULL, 0, 0));

		if (CWnd::GetCapture() != this)
			break;

		switch (msg.message) {
		// handle movement/accept messages
		case WM_MOUSEMOVE:
			// handle resize cases (and part of move)
            curX = (int)(short)LOWORD(msg.lParam);
            curY = (int)(short)HIWORD(msg.lParam);

            xDiff = curX - point.x;
            yDiff = curY - point.y;

            rectCur = rectOrg;
            rectCur.top += yDiff;
            rectCur.bottom += yDiff;
            pDrawDC->DrawDragRect(&rectCur, sizeBar, &rectOld, sizeBar);
            rectOld = rectCur;

            break;

		// handle cancel messages
		case WM_KEYDOWN:
			if (msg.wParam != VK_ESCAPE)
				break;
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
            goto ExitLoop;

		// just dispatch rest of the messages
		default:
			DispatchMessage(&msg);
			break;
		}
	}

ExitLoop:
    pDrawDC->DrawDragRect(&rectCur, sizeBar, NULL, sizeBar);

    m_Parent->ReleaseDC(pDrawDC);
    ReleaseCapture();
    ::ClipCursor(NULL);

    if (yDiff == 0) return;

    // move the splitter bar & re-position the attached panes if necessary
    MoveWindow(rectCur, FALSE);
    RecalcLayout();

	m_Parent->SendMessage(WM_SPLITTER_MOVED, yDiff, GetDlgCtrlID());

    //CWnd::OnLButtonDown(nFlags, point);
}
*/