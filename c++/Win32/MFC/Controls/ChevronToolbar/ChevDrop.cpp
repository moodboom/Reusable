// ChevDrop.cpp : implementation file
//

#include "stdafx.h"

#include "ChevDrop.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChevDrop

CChevDrop::CChevDrop()
{
	m_hMsgReceiver= NULL;

}

CChevDrop::~CChevDrop()
{
}


BEGIN_MESSAGE_MAP(CChevDrop, CWnd)
	//{{AFX_MSG_MAP(CChevDrop)
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CChevDrop::CreatePopup( CWnd* pParent )
{
	if ( !m_hWnd )
	{
		// Register for our popup window
		LPCTSTR lpszClassName = AfxRegisterWndClass( CS_HREDRAW|CS_VREDRAW );
		return CWnd::CreateEx(	WS_EX_TOOLWINDOW, 
								lpszClassName, 
								"", 
								WS_POPUP|WS_DLGFRAME,
								CRect(0,0,0,0), 
								pParent, 
								NULL );
	}

	return TRUE;
}

// helper to create a new toolbar
HWND CChevDrop::CreateToolBar( HWND hwndParent, HWND hToolToReplicate ) 
{ 
	HWND hwndTB; 

	// Make sure common control lib is loaded
	InitCommonControls(); 
	hwndTB = CreateWindowEx(0, 
							TOOLBARCLASSNAME, 
							(LPSTR) NULL, 
							WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | 
							CCS_NODIVIDER | CBRS_NOALIGN, 
							0, 0, 0, 0, 
							hwndParent, 
							(HMENU) AFX_IDW_TOOLBAR+40, 
							AfxGetInstanceHandle (), 
							NULL); 

	// Doc says, required for backward compatibility
	::SendMessage( hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 

	// Our toolbar may have dropdown buttons, so got to set the extended style
	::SendMessage( hwndTB, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS );

	// Attach to a object.. just for the convenience
	m_tb.Attach ( hwndTB );

	// Get ImageList from the toolbar
	HIMAGELIST	hHot = (HIMAGELIST)::SendMessage( hToolToReplicate, TB_GETIMAGELIST, 0, 0 );

	// Create a duplicate of the imagelist
	HIMAGELIST	hImageList = ImageList_Duplicate( hHot );

	// Set the imagelist for our new toolbar
	::SendMessage( hwndTB, TB_SETIMAGELIST, 0, (LPARAM)hImageList );

	// Attach it to an MFC object so that it automagically deletes the handle 
	// when it goes out of scope, toooooooo lazy to delete it 
	m_hImageList.Attach ( hImageList );

	return hwndTB; 
} 

BOOL CChevDrop::CleanToolBar( )
{

	BOOL	bReturn = TRUE;

	if ( m_tb.GetSafeHwnd ( ) )
	{
		HWND hTool = m_tb.Detach ( );
		bReturn = ::DestroyWindow ( hTool ) || bReturn ;
	}

	if ( m_hImageList.GetSafeHandle ( ) )
	{
		bReturn = m_hImageList.DeleteImageList ( ) || bReturn ;
	}

	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////
// CChevDrop message handlers

BOOL CChevDrop::ShowPopup(	
							CWnd*	pMsgReceiver,
							CToolBar* pToolBar,
							CRect	rectDisplayed,
							CPoint	ptScreen )
{
	// Somebody should be there to receive the message from toolbar
	ASSERT(pMsgReceiver != NULL);
	// The receiving handle should not be empty
	ASSERT(pMsgReceiver->GetSafeHwnd ()!=NULL);
	// The source toolbar should exist
	ASSERT(pToolBar != NULL);
	// It is really a toolbar ??
	ASSERT(pToolBar->IsKindOf(RUNTIME_CLASS(CToolBar)));


	// Clean if any previous objects have been attached
	CleanToolBar( );

	// Create a tool bar with the popup as parent
	CreateToolBar( GetSafeHwnd (), pToolBar->GetSafeHwnd() );

	// Store the window which receives notifications from the toolbar
	// We have to redirect a few later
	m_hMsgReceiver = pMsgReceiver->GetSafeHwnd ();

	
	// This flag indicates if atleast one has been added to the menu
	// POPUP Menu is shown only if atleast one item has to be shown
	BOOL	bAtleastOne=FALSE;

	// Run along all the buttons, find hidden ones and add them to new toolbar
	int iCount, iButtonCount = pToolBar->GetToolBarCtrl().GetButtonCount();
	for ( iCount = 0 ; iCount < iButtonCount ; iCount++ )
	{
		TBBUTTON tbinfo;
		pToolBar->GetToolBarCtrl().GetButton ( iCount, &tbinfo );
			
		// If the button is a separator then we can also add a separator to the
		// popup menu
		if (  tbinfo.fsStyle & TBSTYLE_SEP )
		{
			// It wouldnt be nice if there is a separator as the first item in the menu
			if ( bAtleastOne )
			{
				// add to our toolbar

				// NOTE: Adding separators changes the way in which the toolbar
				// shows the popup. The program uses CToolBarCtl::SetRows( ) to 
				// wrap buttons, the behaviour of this method is different when
				// separators and grouping is there, so uncomment the following line
				// to see how it works 

//				m_tb.AddButtons ( 1, &tbinfo );
			}
		}
		else
		{
			// Get the button rectangle
			CRect rectButton;
			pToolBar->GetItemRect ( iCount, &rectButton );

			// Check the intersection of the button and the band
			CRect interRect;
			interRect.IntersectRect ( &rectButton, &rectDisplayed );

			// if the intersection is not the same as button then
			// the button is not completely visible, so add to menu
			if ( interRect != rectButton )
			{

				// Yeah buttons seem to be hidden now
				m_tb.AddButtons ( 1, &tbinfo );

				// Yeah, have added one, so can show the menu
				bAtleastOne=TRUE;
			}
		}
	}

	// Show the window only if atleast one item has been added
	if ( !bAtleastOne )
	{
		return FALSE;
	}

	SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOMOVE);

	// Better call this after modifying buttons.. dunno what will go wrong if i dont
	// use this
	m_tb.AutoSize ( );

	// Calculate the number of approx rows required
	int iRows = max( (m_tb.GetButtonCount ( )+BUTTONSPERROW-1) / BUTTONSPERROW, 1 );

	CRect recWindow;
	// Set rows calculated. recWindow is used in resizing the parent popup window
	m_tb.SetRows ( iRows, TRUE, &recWindow );

	// NOTE: the number of rows need not be the same as iRows... Please check
	// doc for CToolBarCtrl::SetRows( )

	//TRACE3( "Rows %d Width %d Height %d\n", iRows, recWindow.Width(), recWindow.Height() );
	
	CSize szBar;
	m_tb.GetMaxSize ( &szBar );

	// BUGBUG: When the toolbar had one button, recWindow had 0 width, so have to 
	// get width from GetMaxSize( ). This may be wrong when the button
	// had TBSTYLE_DROPDOWN style...
	int iWidth = recWindow.Width() == 0 ? szBar.cx : recWindow.Width();

	// Get the top and bottom spacing for the toolbar
	DWORD dwPad = ::SendMessage( m_tb.GetSafeHwnd(), TB_GETPADDING, 0, 0 );
	CRect rectWindow(	ptScreen.x, 
						ptScreen.y, 
						ptScreen.x+iWidth+LOWORD(dwPad) ,
						ptScreen.y+recWindow.Height()+HIWORD(dwPad)
						);

	// Have to adjust to screen pos
	int cxScreen = ::GetSystemMetrics ( SM_CXSCREEN );
	int cyScreen = ::GetSystemMetrics ( SM_CYSCREEN );

	// Move little left to show the complete toolbar
	if ( rectWindow.right > cxScreen )
	{
		int diff = rectWindow.right - cxScreen;
		rectWindow.left -= diff;
		rectWindow.right -= diff;
	}
	// Move little up to show the complete toolbar
	if ( rectWindow.bottom > cyScreen )
	{
		int diff = rectWindow.bottom - cyScreen;
		rectWindow.top -= diff;
		rectWindow.bottom -= diff;
	}

	// Should we check for top and left positions ????
	// Move the parent popup window and show
	MoveWindow( &rectWindow, TRUE );

	// Move toolbar to the top corner and show
	m_tb.MoveWindow (	//hWndToolbar, 
					rectWindow.left, 
					rectWindow.top,
					rectWindow.Width(),
					rectWindow.Height(),
					TRUE );

	ShowWindow( SW_SHOW );
	m_tb.ShowWindow ( SW_SHOW );


// ****************	Uncomment the following to get a weird scroll effect :-)) ********

/*
#define SPI_GETMENUANIMATION                0x1002
	// Sliding effect
	BOOL	bSlide;
	SystemParametersInfo( SPI_GETMENUANIMATION, 0, &bSlide, 0 );
	if ( bSlide )
	{
		CDC			memDC;
		CClientDC	wndDC(this);
		CWindowDC	screenDC( NULL );

		memDC.CreateCompatibleDC ( &screenDC );

		CBitmap		Bit, *pOld;
		Bit.CreateCompatibleBitmap ( &screenDC, 
									 rectWindow.Width (), 
									 rectWindow.Height() );
		pOld = memDC.SelectObject ( &Bit );

		GetWindowRect( &rectWindow );
		memDC.PatBlt (	0,0,
						rectWindow.Width (), 
						rectWindow.Height(), BLACKNESS );

		SendMessage( WM_PRINT, 
					(WPARAM)memDC.GetSafeHdc(), 
					(LPARAM)PRF_CHILDREN|PRF_CLIENT|PRF_OWNED|PRF_ERASEBKGND );
		SendMessage( WM_PRINT, 
					(WPARAM)memDC.GetSafeHdc(), 
					(LPARAM)PRF_NONCLIENT);

		
		int iSteps = rectWindow.Height();
		for ( ; iSteps > 5 ; iSteps -= 5 )
		{
			screenDC.BitBlt(	rectWindow.left,
								rectWindow.top,
								rectWindow.Width(),
								rectWindow.Height( ) - iSteps,

								&memDC,
								0, iSteps, SRCCOPY );
			Sleep( 3 );
		}

		screenDC.BitBlt(	rectWindow.left,
							rectWindow.top,
							rectWindow.Width(),
							rectWindow.Height( ),

							&memDC,
							0, 0, SRCCOPY );

		memDC.SelectObject ( pOld );
	}
*/
	// all went ok !
	return TRUE;
}


void CChevDrop::OnKillFocus(CWnd* pNewWnd) 
{
	TRACE0( "KILLFOCUS\n" );

	SetWindowPos( NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOACTIVATE );
}

void CChevDrop::PostNcDestroy() 
{

	CleanToolBar( );

	CWnd::PostNcDestroy();

	TRACE0( "NC destroy\n" );

	// Commit suicide :-(
	//delete this;
}

BOOL CChevDrop::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	BOOL bReturn;

	// Pass on the message to the Window that needs it
	bReturn = ::SendMessage ( m_hMsgReceiver, WM_COMMAND, wParam, lParam );

	return bReturn;
}

BOOL CChevDrop::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{

	// Notifications are sent by the toolbar control.
	// A typical message is TBN_DROPDOWN which should be passed to the window
	// to handle the dropdown.. which may show another menu :-)

	BOOL bReturn = ::SendMessage( m_hMsgReceiver, WM_NOTIFY, wParam, lParam );
	return bReturn;
}

void CChevDrop::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ( nChar == VK_ESCAPE )
	{
		// If Escape key was pressed, close popup window
		SetWindowPos( NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOACTIVATE );
	}
	else
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

