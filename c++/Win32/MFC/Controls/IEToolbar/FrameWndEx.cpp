//-------------------------------------------------------------------//
// CFrameWndEx class
//-------------------------------------------------------------------//
// MDM	4/2/2004 3:12:19 PM
// This class sets up a frame with support for the self-contained SizableReBar.
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#include "stdafx.h"

// MDM Not sure where, but somewhere the following includes fall into
// the VS7-deprecated ".h" stream classes.  Nip that in the bud.
#include <fstream>

#include "FrameWndEx.h"
#include "WindowListDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CFrameWndEx

IMPLEMENT_DYNCREATE( CFrameWndEx, CFrameWnd )

CFrameWndEx::~CFrameWndEx()
{
}

/////////////////////////////////////////////////////////////////////////////
// Overrides

void CFrameWndEx::DelayUpdateFrameMenu( HMENU hMenuAlt )
{
    // This function seems to be called by the framework only two times
    // when in-place activation begins and ends.

    if ( m_bInPlace )
    {
        VERIFY( SetMenu( 0 ) );
    }

    m_bInPlace = !m_bInPlace;

    CFrameWnd::DelayUpdateFrameMenu( hMenuAlt );
}

void CFrameWndEx::OnUpdateFrameMenu( HMENU hMenuAlt )
{
    if ( hMenuAlt == 0 )
    {
        // Attempt to get default menu from document
        CDocument* pDoc = GetActiveDocument();
        if ( pDoc != 0 )
        {
            hMenuAlt = pDoc->GetDefaultMenu();
        }
        // Use default menu stored in frame if none from document
        if ( hMenuAlt == 0 )
        {
            hMenuAlt = m_hMenuDefault;
        }
    }

    // Finally, set the menu
    if ( m_bInPlace )
    {
        VERIFY( ::SetMenu( m_hWnd, hMenuAlt ) );
    }
    else
    {
        VERIFY( m_wndMenuBar.SetMenu( hMenuAlt ) );
    }
}


//-------------------------------------------------------------------//
// PreTranslateMessage()															//
//-------------------------------------------------------------------//
// Route msgs to the parent if we don't handle them.
//-------------------------------------------------------------------//
BOOL CFrameWndEx::PreTranslateMessage(MSG* pMsg) 
{
	// If embedded, let the parent have a crack at it first...
	//
	// WARNING: We had dialogs as children of the frame that
	// were marked as "controls" in the resource editor, and this
	// caused nasty infinite message looping.  Be careful.
	if ( m_bEmbedded )
		if ( GetParent()->PreTranslateMessage(pMsg) )
			return TRUE;

	return inherited::PreTranslateMessage( pMsg );
}


//-------------------------------------------------------------------//
// OnCmdMsg()																			//
//-------------------------------------------------------------------//
// Same as PreTranslateMessage(), see above.
// This handles commands.
//-------------------------------------------------------------------//
#include <ustring.h>
BOOL CFrameWndEx::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// If embedded, let the parent have a crack at it first...
	//
	// WARNING: We had dialogs as children of the frame that
	// were marked as "controls" in the resource editor, and this
	// caused nasty infinite message looping.  Be careful.
	if ( m_bEmbedded )
		if ( GetParent()->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
			return TRUE;
	
	return inherited::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}


	// OLD
   // This is how you do standard control bar handling.
	// 
	// WARNING: Do we need to specify location in the new version?
	// void ShowBar(CControlBar& bar, UINT location);
   /*

   // HIDE
	// This is required so that SHOWING the toolbar later works.
	FloatBar(bar);
	bar.ShowWindow(SW_HIDE);
	bar.GetDockingFrame()->ShowControlBar(&bar, FALSE, TRUE);

	// SHOW
   bar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&bar,location);
	bar.ShowWindow(SW_SHOW);

	// FLOAT
   FloatControlBar(&bar,CPoint(0,0));

	*/


/////////////////////////////////////////////////////////////////////////////
// CFrameWndEx message handlers

BEGIN_MESSAGE_MAP(CFrameWndEx, CFrameWnd)
    //{{AFX_MSG_MAP(CFrameWndEx)
    ON_WM_CREATE()
    ON_WM_SYSCOMMAND()
    ON_WM_MENUCHAR()
    ON_WM_NCACTIVATE()
    ON_WM_SETTINGCHANGE()
    ON_WM_CLOSE()
    ON_WM_INITMENUPOPUP()
    ON_WM_MENUSELECT()
    ON_WM_DRAWITEM()
    ON_WM_MEASUREITEM()
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMDIClientWndEx

IMPLEMENT_DYNCREATE( CMDIClientWndEx, CWnd )

CMDIClientWndEx::CMDIClientWndEx() : m_hWindowMenu( 0 )
{
}

CMDIClientWndEx::~CMDIClientWndEx()
{
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

CMDIFrameWndEx* CMDIClientWndEx::GetParentFrame() const
{
    return STATIC_DOWNCAST( CMDIFrameWndEx, CWnd::GetParentFrame() );
}

/////////////////////////////////////////////////////////////////////////////
// CMDIClientWndEx message handlers

BEGIN_MESSAGE_MAP(CMDIClientWndEx, CWnd)
    //{{AFX_MSG_MAP(CMDIClientWndEx)
    //}}AFX_MSG_MAP
    ON_MESSAGE( WM_MDISETMENU, OnMDISetMenu )
    ON_MESSAGE( WM_MDIREFRESHMENU, OnMDIRefreshMenu )
END_MESSAGE_MAP()

LRESULT CMDIClientWndEx::OnMDISetMenu( WPARAM wParam, LPARAM lParam )
{
    m_hWindowMenu = ( HMENU )lParam;

    CMDIFrameWndEx* pFrame = GetParentFrame();
    if ( !pFrame->m_bInPlace )
    {
        VERIFY( pFrame->m_wndMenuBar.SetMenu( ( HMENU )wParam ) );
        wParam = 0;
    }

    return DefWindowProc( WM_MDISETMENU, wParam, lParam );
}

LRESULT CMDIClientWndEx::OnMDIRefreshMenu( WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = DefWindowProc( WM_MDIREFRESHMENU, wParam, lParam );

    CMenu* pWindowMenu = CMenu::FromHandle( m_hWindowMenu );
    if ( pWindowMenu != 0 )
    {
        CWinAppEx* pApp = CWinAppEx::GetInstance();

		  // MDM	2/28/2005 12:10:10 PM
        // Looks like this is trying to add an icon to each window menu
        // item, but it's trashing the text of the menu items.
        // We remove this, and all looks great, so I won't worry
        // about it for now.  (IS CMenuXP assisting in this task?  not sure...)
        /*
		  UINT nItems = pWindowMenu->GetMenuItemCount();
        for ( UINT nIndex = 0; nIndex < nItems; nIndex++ )
        {
            UINT nID = pWindowMenu->GetMenuItemID( nIndex );
            if ( nID >= AFX_IDM_FIRST_MDICHILD )
            {
                HICON hIcon = ( HICON )::GetClassLong(
                    GetDlgItem( nID )->GetSafeHwnd(), GCL_HICONSM );
                if ( hIcon != 0 )
                {
                    pApp->ReplaceMenuIcon( nID, hIcon );
                }
                else
                {
                    pApp->RemoveMenuIcon( nID );
                }
            }
        }
		  */

        pWindowMenu->RemoveMenu( ID_WINDOW_LIST, MF_BYCOMMAND );
        pWindowMenu->RemoveMenu( AFX_IDM_FIRST_MDICHILD + 9, MF_BYCOMMAND ); // "More Windows..."

        CString strText;
        VERIFY( strText.LoadString( IDS_WINDOW_LIST ) );
        VERIFY( pWindowMenu->AppendMenu( MF_STRING, ID_WINDOW_LIST, strText ) );
    }

    return lResult;
}

/////////////////////////////////////////////////////////////////////////////
// CMDIFrameWndEx

IMPLEMENT_DYNCREATE( CMDIFrameWndEx, CMDIFrameWnd )

CMDIFrameWndEx::CMDIFrameWndEx()
{
}

CMDIFrameWndEx::~CMDIFrameWndEx()
{
   // MDM	2/24/2005
	// Make sure any derived class properly created a client window
	// (see CreateMDIClientWndForSubclassing() ).
	ASSERT( m_pwndMDIClient );
	delete m_pwndMDIClient;
}

/////////////////////////////////////////////////////////////////////////////
// Overrides

void CMDIFrameWndEx::DelayUpdateFrameMenu( HMENU hMenuAlt )
{
    // This function seems to be called by the framework only two times
    // when in-place activation begins and ends.

    if ( m_bInPlace )
    {
        CMenu* pFrameMenu = GetMenu();
        VERIFY( SetMenu( 0 ) );

        if ( pFrameMenu != 0 )
        {
            BOOL bMaximized;
            CMDIChildWnd* pActive = MDIGetActive( &bMaximized );
            if ( ( pActive!= 0 ) && bMaximized )
            {
                if ( pFrameMenu->GetSubMenu( 0 ) == pActive->GetSystemMenu( FALSE ) )
                {
                    VERIFY( pFrameMenu->RemoveMenu( 0, MF_BYPOSITION ) );
                    VERIFY( pFrameMenu->DeleteMenu( SC_MINIMIZE, MF_BYCOMMAND ) );
                    VERIFY( pFrameMenu->DeleteMenu( SC_RESTORE, MF_BYCOMMAND ) );
                    VERIFY( pFrameMenu->DeleteMenu( SC_CLOSE, MF_BYCOMMAND ) );
                    VERIFY( pActive->ModifyStyle( 0, WS_SYSMENU ) );
                }
            }
        }
    }

    m_bInPlace = !m_bInPlace;

    CMDIFrameWnd::DelayUpdateFrameMenu( hMenuAlt );
}

void CMDIFrameWndEx::OnUpdateFrameTitle( BOOL bAddToTitle )
{
    CFrameWndBase< CMDIFrameWnd >::OnUpdateFrameTitle( bAddToTitle );

    // If our MDIChildFrame was created with ( WS_VISIBLE | FWS_ADDTOTITLE )
    // styles, then we have to refresh Window menu in order to reflect
    // MDIChildFrame title changes.  CMDIChildWnd::OnUpdateFrameTitle()
    // calls CMDIFrameWnd::OnUpdateFrameTitle(), so here we go.
    VERIFY( m_pwndMDIClient->PostMessage( WM_MDIREFRESHMENU ) );
}

/////////////////////////////////////////////////////////////////////////////
// CMDIFrameWndEx message handlers

BEGIN_MESSAGE_MAP(CMDIFrameWndEx, CMDIFrameWnd)
    //{{AFX_MSG_MAP(CMDIFrameWndEx)
    ON_WM_CREATE()
    ON_WM_SYSCOMMAND()
    ON_WM_MENUCHAR()
    ON_WM_NCACTIVATE()
    ON_WM_SETTINGCHANGE()
    ON_WM_CLOSE()
    ON_WM_INITMENUPOPUP()
    ON_WM_MENUSELECT()
    ON_WM_DRAWITEM()
    ON_WM_MEASUREITEM()
    ON_COMMAND(ID_WINDOW_LIST, OnWindowList)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CMDIFrameWndEx::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CFrameWndBase< CMDIFrameWnd >::OnCreate( lpCreateStruct ) == -1 )
    {
        return -1;
    }

   // MDM	2/24/2005 4:59:19 PM
   // Updated to allow CMDIClientWndEx-derived client windows.
   // VERIFY( m_wndMDIClient.SubclassWindow( m_hWndMDIClient ) );
   VERIFY( CreateMDIClientWndForSubclassing()->SubclassWindow( m_hWndMDIClient ) );

    return 0;
}

void CMDIFrameWndEx::OnWindowList()
{
    CWindowListDialog dlg( m_pwndMDIClient, this );
    dlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWndEx

IMPLEMENT_DYNCREATE( CMDIChildWndEx, CMDIChildWnd )

CMDIChildWndEx::CMDIChildWndEx()
{
}

CMDIChildWndEx::~CMDIChildWndEx()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWndEx message handlers

BEGIN_MESSAGE_MAP(CMDIChildWndEx, CMDIChildWnd)
    //{{AFX_MSG_MAP(CMDIChildWndEx)
    ON_WM_INITMENUPOPUP()
    ON_WM_DRAWITEM()
    ON_WM_MEASUREITEM()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

