//-------------------------------------------------------------------//
// CMenuBar class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#include "stdafx.h"
#include "MenuBar.h"
#include "SizableReBar.h"
#include "..\..\WinAppEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMenuBarButton

CMenuBarButton::CMenuBarButton()
{
    m_bHidden = true;
    m_bPushed = false;

    SetMDIChild( 0 );
    SetMenuBarRect( 0 );
    SetTheme( 0 );
}

CMenuBarButton::~CMenuBarButton()
{
}

bool CMenuBarButton::HitTest( CPoint pt ) const
{
    return ( IsVisible() && GetButtonRect().PtInRect( pt ) );
}

void CMenuBarButton::SetMDIChild( HWND hWndMDIChild )
{
    m_hWndMDIChild = hWndMDIChild;
}

void CMenuBarButton::SetMenuBarRect( LPCRECT lpRect )
{
    if ( lpRect != 0 )
    {
        m_rcMenuBar = *lpRect;
    }
    else
    {
        m_rcMenuBar.SetRectEmpty();
    }
}

void CMenuBarButton::SetTheme( HTHEME hTheme )
{
    m_hTheme = hTheme;
}

DWORD CMenuBarButton::GetMDIChildStyle() const
{
    return ( DWORD )::GetWindowLong( m_hWndMDIChild, GWL_STYLE );
}

bool CMenuBarButton::IsEnabled() const
{
    return true;
}

bool CMenuBarButton::IsVisible() const
{
    return ( !m_bHidden && ::IsWindow( m_hWndMDIChild ) && ( GetMDIChildStyle() & WS_SYSMENU ) );
}

bool CMenuBarButton::IsPushed() const
{
    return m_bPushed;
}

bool CMenuBarButton::HideButton( bool bHide )
{
    bool bWasHidden = m_bHidden;
    m_bHidden = bHide;
    return bWasHidden;
}

bool CMenuBarButton::PushButton( bool bPush )
{
    bool bWasPushed = m_bPushed;
    m_bPushed = bPush;
    return bWasPushed;
}

CSize CMenuBarButton::GetButtonSize()
{
    // Sys-menu icon and caption buttons are all inside
    // a rectangle of the following size:
    const NONCLIENTMETRICS& info = CWinAppEx::GetInstance()->GetNonClientMetrics();
    return CSize(
        info.iMenuWidth,     //::GetSystemMetrics( SM_CXMENUSIZE ),
        info.iMenuHeight );  //::GetSystemMetrics( SM_CYMENUSIZE ) );
}

#ifndef TMSCHEMA_H

#define WP_MDIMINBUTTON         16
#define WP_MDICLOSEBUTTON       20
#define WP_MDIRESTOREBUTTON     22

#define MINBS_NORMAL            1
#define MINBS_HOT               2
#define MINBS_PUSHED            3
#define MINBS_DISABLED          4

#define CBS_NORMAL              1
#define CBS_HOT                 2
#define CBS_PUSHED              3
#define CBS_DISABLED            4

#define RBS_NORMAL              1
#define RBS_HOT                 2
#define RBS_PUSHED              3
#define RBS_DISABLED            4

#endif  // !TMSCHEMA_H

void CMenuBarButton::DrawButton( CDC* pDC, bool bFrameActive )
{
    if ( IsVisible() )
    {
        CRect rc = GetButtonRect();
        if ( m_hTheme != 0 )
        {
            UINT nPartID;
            UINT nState;
            switch ( GetSysCommandID() )
            {
                case SC_CLOSE:
                    nPartID = WP_MDICLOSEBUTTON;
                    if ( bFrameActive && IsEnabled() )
                    {
                        nState  = IsPushed() ? CBS_PUSHED : CBS_NORMAL;
                    }
                    else
                    {
                        nState = CBS_DISABLED;
                    }
                    break;
                case SC_MINIMIZE:
                    nPartID = WP_MDIMINBUTTON;
                    if ( bFrameActive && IsEnabled() )
                    {
                        nState  = IsPushed() ? MINBS_PUSHED : MINBS_NORMAL;
                    }
                    else
                    {
                        nState = MINBS_DISABLED;
                    }
                    break;
                case SC_RESTORE:
                    nPartID = WP_MDIRESTOREBUTTON;
                    if ( bFrameActive && IsEnabled() )
                    {
                        nState  = IsPushed() ? RBS_PUSHED : RBS_NORMAL;
                    }
                    else
                    {
                        nState = RBS_DISABLED;
                    }
                    break;
                default:
                    ASSERT( false );
                    return;
            }

            HRESULT hr = ::DrawThemeBackgroundXP( m_hTheme, *pDC, nPartID, nState, rc, 0 );
            ASSERT( SUCCEEDED( hr ) );
        }
        else
        {
            UINT nState;
            switch ( GetSysCommandID() )
            {
                case SC_CLOSE:
                    nState = DFCS_CAPTIONCLOSE;
                    break;
                case SC_MINIMIZE:
                    nState = DFCS_CAPTIONMIN;
                    break;
                case SC_RESTORE:
                    nState = DFCS_CAPTIONRESTORE;
                    break;
                default:
                    ASSERT( false );
                    return;
            }

            if ( IsPushed() )
            {
                nState |= DFCS_PUSHED;
            }
            if ( !IsEnabled() )
            {
                nState |= DFCS_INACTIVE;
            }
            VERIFY( pDC->DrawFrameControl( rc, DFC_CAPTION, nState ) );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CMenuBarButtonMin

CRect CMenuBarButtonMin::GetButtonRect() const
{
    CPoint ptTopLeft;
    CSize szButton = GetButtonSize();

    if ( m_hTheme != 0 )
    {
        ptTopLeft.x = m_rcMenuBar.right - szButton.cx * 3;
        ptTopLeft.y = m_rcMenuBar.top + ( m_rcMenuBar.Height() - szButton.cy ) / 2;
    }
    else
    {
        // Minimize box has 2 pixel border on all sides but right
        szButton.cx -= 2;
        szButton.cy -= 4;

        ptTopLeft.x = m_rcMenuBar.right - ( szButton.cx + 2 ) * 3 + 2;
        ptTopLeft.y = m_rcMenuBar.top + ( m_rcMenuBar.Height() - szButton.cy ) / 2;
    }

    return CRect( ptTopLeft, szButton );
}

UINT CMenuBarButtonMin::GetSysCommandID() const
{
    return ::IsIconic( m_hWndMDIChild ) ? SC_RESTORE : SC_MINIMIZE;
}

bool CMenuBarButtonMin::IsEnabled() const
{
    return ( ( GetMDIChildStyle() & WS_MINIMIZEBOX ) != 0 );
}

bool CMenuBarButtonMin::IsVisible() const
{
    return CMenuBarButton::IsVisible() &&
        ( ( GetMDIChildStyle() & ( WS_MINIMIZEBOX | WS_MAXIMIZEBOX ) ) != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CMenuBarButtonMax

CRect CMenuBarButtonMax::GetButtonRect() const
{
    CPoint ptTopLeft;
    CSize szButton = GetButtonSize();

    if ( m_hTheme != 0 )
    {
        ptTopLeft.x = m_rcMenuBar.right - szButton.cx * 2;
        ptTopLeft.y = m_rcMenuBar.top + ( m_rcMenuBar.Height() - szButton.cy ) / 2;
    }
    else
    {
        // Max box has a 2 pixel border on all sides but left, which is zero
        szButton.cx -= 2;
        szButton.cy -= 4;

        ptTopLeft.x = m_rcMenuBar.right - ( szButton.cx + 2 ) * 2;
        ptTopLeft.y = m_rcMenuBar.top + ( m_rcMenuBar.Height() - szButton.cy ) / 2;
    }

    return CRect( ptTopLeft, szButton );
}

UINT CMenuBarButtonMax::GetSysCommandID() const
{
    return /*::IsZoomed( m_hWndMDIChild ) ?*/ SC_RESTORE /*: SC_MAXIMIZE*/;
}

bool CMenuBarButtonMax::IsEnabled() const
{
    return ( ( GetMDIChildStyle() & WS_MAXIMIZEBOX ) != 0 );
}

bool CMenuBarButtonMax::IsVisible() const
{
    return CMenuBarButton::IsVisible() &&
        ( ( GetMDIChildStyle() & ( WS_MINIMIZEBOX | WS_MAXIMIZEBOX ) ) != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CMenuBarButtonClose

CRect CMenuBarButtonClose::GetButtonRect() const
{
    CPoint ptTopLeft;
    CSize szButton = GetButtonSize();

    if ( m_hTheme != 0 )
    {
        ptTopLeft.x = m_rcMenuBar.right - szButton.cx;
        ptTopLeft.y = m_rcMenuBar.top + ( m_rcMenuBar.Height() - szButton.cy ) / 2;
    }
    else
    {
        // Close box has a 2 pixel border on all sides but left, which is zero
        szButton.cx -= 2;
        szButton.cy -= 4;

        ptTopLeft.x = m_rcMenuBar.right - ( szButton.cx + 2 );
        ptTopLeft.y = m_rcMenuBar.top + ( m_rcMenuBar.Height() - szButton.cy ) / 2;
    }

    return CRect( ptTopLeft, szButton );
}

UINT CMenuBarButtonClose::GetSysCommandID() const
{
    return SC_CLOSE;
}

/////////////////////////////////////////////////////////////////////////////
// CMenuBar

// Static member variables
HHOOK     CMenuBar::m_hMsgHook = 0;
CMenuBar* CMenuBar::m_pMenuBar = 0;

IMPLEMENT_DYNAMIC( CMenuBar, CToolBar )

CMenuBar::CMenuBar()
{
    m_hWndMDIChild   = 0;
    m_hWndOldFocus   = 0;
    m_hMenu          = 0;
    m_hMenuTracking  = 0;
    m_bItemTracking  = false;
    m_bItemDropped   = false;
    m_bButtonCapture = false;
    m_bFrameActive   = true;
    m_hTheme         = 0;

    // Create menu bar buttons
    m_aMenuBarButtons.Add( new CMenuBarButtonMin   );
    m_aMenuBarButtons.Add( new CMenuBarButtonMax   );
    m_aMenuBarButtons.Add( new CMenuBarButtonClose );
}

CMenuBar::~CMenuBar()
{
    if ( m_fontMenu.GetSafeHandle() != 0 )
    {
        VERIFY( m_fontMenu.DeleteObject() );
    }

    for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
    {
        delete m_aMenuBarButtons.GetAt( nIndex );
    }

    m_aMenuBarButtons.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// Operations

bool CMenuBar::SetMenu( HMENU hMenu )
{
    // Delete old buttons
    while ( m_nCount > 0 )
    {
        VERIFY( GetToolBarCtrl().DeleteButton( --m_nCount ) );
    }

    CMenu* pMenu = CMenu::FromHandle( hMenu );
    if ( pMenu != 0 )
    {
        // Allocate space for buttons
        UINT nItems = pMenu->GetMenuItemCount();
        VERIFY( SetButtons( 0, nItems + 1 ) );

        // Add sys-menu button which is the leftmost
        SetButtonInfo( 0, IDBUTTON_SYSMENU,
            TBBS_BUTTON | TBBS_DROPDOWN | TBBS_NOPREFIX, 0 );

        // Add all other buttons
        for ( UINT nIndex = 0; nIndex < nItems; nIndex++ )
        {
            UINT nID    = pMenu->GetMenuItemID( nIndex );
            UINT nStyle = TBBS_BUTTON | TBBS_AUTOSIZE | TBBS_DROPDOWN;

            switch ( nID )
            {
                case -1:
                    nID = IDBUTTON_SYSMENU + 1 + nIndex;
                    break;
                case 0:
                    nStyle = TBBS_SEPARATOR;
                    break;
                default:
                    nStyle ^= TBBS_DROPDOWN;

							// MDM	10/14/2004 2:01:21 PM
							// We hit this when we add a submenu that is tied
							// directly to a command (it's not a popup).  
							// NOTE THAT IT SEEMS TO WORK FINE despite this comment:
							// ASSERT( false );    // not supported yet

                    break;
            }

            CString strMenu;
            pMenu->GetMenuString( nIndex, strMenu, MF_BYPOSITION );

            SetButtonInfo( nIndex + 1, nID, nStyle, 0 /*-2=I_IMAGENONE*/ );
            VERIFY( SetButtonText( nIndex + 1, strMenu ) );
        }
    }

    UpdateMenuBar();

    m_hMenu = hMenu;
    return true;
}

HMENU CMenuBar::GetMenu() const
{
    return m_hMenu;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

CReBarCtrl& CMenuBar::GetParentReBarCtrl() const
{
    return *STATIC_DOWNCAST( CReBarCtrl, GetParent() );
}

int CMenuBar::GetParentBandIndex() const
{
    int nBand = GetParentReBarCtrl().IDToIndex( ( UINT )GetDlgCtrlID() );
    ASSERT( nBand != -1 );
    return nBand;
}

void CMenuBar::SetButtonWidth( UINT nID, int nWidth )
{
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof( tbbi );
    tbbi.dwMask = TBIF_SIZE;
    tbbi.cx     = ( WORD )nWidth;

    VERIFY( GetToolBarCtrl().SetButtonInfo( nID, &tbbi ) );
}

void CMenuBar::UpdateMenuBar()
{
    // Set new font
    if ( m_fontMenu.m_hObject != 0 )
    {
        VERIFY( m_fontMenu.DeleteObject() );
    }

    VERIFY( m_fontMenu.CreateFontIndirect( &CWinAppEx::GetInstance()->GetNonClientMetrics().lfMenuFont ) );
    SetFont( &m_fontMenu, TRUE );

    CToolBarCtrl& tbCtrl = GetToolBarCtrl();
    tbCtrl.SetBitmapSize( CSize( 0, 0 ) );
    tbCtrl.AutoSize();

    // Calc row height
    int cyMenu    = ::GetSystemMetrics( SM_CYMENU );
    int cyPadding = HIWORD( SendMessage( TB_GETPADDING ) );
    int cyButton  = HIWORD( tbCtrl.GetButtonSize() );

    if ( cyMenu > cyButton )
    {
        cyButton = cyMenu;
        VERIFY( tbCtrl.SetBitmapSize( CSize( 0, cyButton - cyPadding ) ) );
    }

    // Adjust appearance of sys-menu button
    bool bSysMenu = ( m_hWndMDIChild != 0 );
    int cxButton = bSysMenu ? CMenuBarButton::GetButtonSize().cx : 0;
    if ( GetCount() > 0 )
    {
        VERIFY( tbCtrl.HideButton( IDBUTTON_SYSMENU, !bSysMenu ) );
        if ( bSysMenu )
        {
            // Adjust sys-menu button width
            SetButtonWidth( IDBUTTON_SYSMENU, cxButton );
        }
    }

    // Calc minimal and ideal width of the menu bar
    int cxIdeal = cxButton * 3;
    for ( int nIndex = 0; nIndex < GetCount(); nIndex++ )
    {
        CRect rcItem;
        if ( tbCtrl.GetItemRect( nIndex, rcItem ) )
        {
            cxIdeal += rcItem.Width();
        }
    }

    // Our parent must be a re-bar control by design.
    // So, minimal and ideal width as well as minimal height of
    // the parent band should also be adjusted accordingly.
    REBARBANDINFO rbbi;
    rbbi.cbSize     = sizeof( rbbi );
    rbbi.fMask      = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;
    rbbi.cxMinChild = cxButton;
    rbbi.cyMinChild = cyButton;
    rbbi.cxIdeal    = cxIdeal;
    VERIFY( GetParentReBarCtrl().SetBandInfo( GetParentBandIndex(), &rbbi ) );

    CRect rcMenuBar;
    GetClientRect( rcMenuBar );
    RepositionSysButtons( rcMenuBar );
}

void CMenuBar::RepositionSysButtons( CRect rcMenuBar )
{
    CRect rcItem;
    GetToolBarCtrl().GetItemRect( GetCount() - 1, rcItem );

    int cxAvailable = ( rcMenuBar.right - rcItem.right );
    int cxNeeded    = ( CMenuBarButton::GetButtonSize().cx * 3 );
    if ( cxAvailable < cxNeeded )
    {
        // Without this sys-buttons would overlap menu items
        rcMenuBar.right = rcItem.right + cxNeeded;
    }

    for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
    {
        CMenuBarButton* pButton = m_aMenuBarButtons.GetAt( nIndex );
        pButton->SetMenuBarRect( rcMenuBar );
        pButton->HideButton( m_hWndMDIChild == 0 );
    }

    Invalidate();
}

void CMenuBar::EnterTrackingMode( int nItem )
{
    if ( !m_bItemTracking )
    {
        m_bItemTracking = true;
        m_hWndOldFocus  = 0;

        // Gain focus
        if ( GetFocus() != this )
        {
            m_hWndOldFocus = SetFocus()->GetSafeHwnd();
        }

        // Capture mouse
        if ( GetCapture() != this )
        {
            SetCapture();
            SendMessage( WM_SETCURSOR, ( WPARAM )m_hWnd, MAKELPARAM( HTCLIENT, 0 ) );
        }

        GetToolBarCtrl().SetHotItem( nItem );
    }
}

void CMenuBar::TrackChevronMenu( CRect& rcChevron, int nItem )
{
    ExitTrackingMode();

    CMenu* pMenu = CMenu::FromHandle( m_hMenu );
    if ( pMenu != 0 )
    {
        CWinAppEx* pApp = CWinAppEx::GetInstance();

        // Create new popup menu
        CMenu menu;
        VERIFY( menu.CreatePopupMenu() );

        TCHAR szBuffer[ MAX_PATH ];
        MENUITEMINFO mii;
        mii.cbSize = sizeof( mii );

        // Populate popup menu by adding clipped items
        UINT nItems = pMenu->GetMenuItemCount();
        for ( UINT nIndex = 0; nIndex < nItems; nIndex++ )
        {
            if ( IsItemClipped( nIndex + 1 ) )
            {
                mii.fMask      = MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
                mii.dwTypeData = szBuffer;
                mii.cch        = sizeof( szBuffer ) / sizeof( szBuffer[ 0 ] );
                VERIFY( pMenu->GetMenuItemInfo( nIndex, &mii, TRUE ) );

                UINT nMenuItems = menu.GetMenuItemCount();
                if ( ( nMenuItems > 0 ) || !( mii.fType & MFT_SEPARATOR ) )
                {
                    VERIFY( ::InsertMenuItem( menu.m_hMenu, nMenuItems, TRUE, &mii ) );
                }
            }
        }

        // Populate popup menu by adding menu items for clipped sys-buttons
        if ( m_hWndMDIChild != 0 )
        {
            CMenu* pSysMenu = CMenu::FromHandle( ::GetSystemMenu( m_hWndMDIChild, FALSE ) );
            ASSERT( pSysMenu != 0 );

            if ( menu.GetMenuItemCount() > 0 )
            {
                VERIFY( menu.AppendMenu( MF_SEPARATOR ) );
            }

            CRect rcClient;
            GetClientRect( rcClient );

            for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
            {
                CMenuBarButton* pButton = m_aMenuBarButtons.GetAt( nIndex );
                if ( pButton->GetButtonRect().right > rcClient.right )
                {
                    UINT nID = pButton->GetSysCommandID();
                    if ( pApp->IsWin50() )
                    {
                        MENUITEMINFO_WIN50 mii;
                        mii.cbSize     = sizeof( mii );
                        mii.fMask      = MIIM_ID | MIIM_STATE | MIIM_FTYPE | MIIM_STRING | MIIM_BITMAP;
                        mii.dwTypeData = szBuffer;
                        mii.cch        = sizeof( szBuffer ) / sizeof( szBuffer[ 0 ] );
                        VERIFY( pSysMenu->GetMenuItemInfo( nID, &mii ) );

                        mii.fState    &= ~MFS_DEFAULT;
                        VERIFY( ::InsertMenuItem( menu.m_hMenu, menu.GetMenuItemCount(), TRUE, &mii ) );
                    }
                    else
                    {
                        mii.fMask      = MIIM_STATE | MIIM_TYPE;
                        mii.dwTypeData = szBuffer;
                        mii.cch        = sizeof( szBuffer ) / sizeof( szBuffer[ 0 ] );
                        VERIFY( pSysMenu->GetMenuItemInfo( nID, &mii ) );

                        mii.fState    &= ~MFS_DEFAULT;
                        VERIFY( menu.AppendMenu( mii.fState | MF_STRING, nID, szBuffer ) );
                    }
                }
            }
        }

        // Expand sub-menu item, if requested
        if ( nItem >= 0 )
        {
            CString strText;
            GetButtonText( nItem, strText );

            for ( int nPos = 0; strText[ nPos ]; nPos++ )
            {
                if ( strText[ nPos ] == _T('&') )
                {
                    nPos++;

                    if ( strText[ nPos ] && ( strText[ nPos ] != _T('&') ) )
                    {
                        // Simulate hot key press
                        ::keybd_event( strText[ nPos ], 0, 0, 0 );
                        ::keybd_event( strText[ nPos ], 0, KEYEVENTF_KEYUP, 0 );
                        break;
                    }
                }
            }
        }

        // Now, just track menu
        TPMPARAMS tpm;
        tpm.cbSize    = sizeof( tpm );
        tpm.rcExclude = rcChevron;

        CWinAppEx::TrackPopupMenuEx( menu.m_hMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL | ( pApp->IsWin50() ? TPM_VERPOSANIMATION : 0 ),
            rcChevron.left, rcChevron.bottom, GetParentFrame(), &tpm );

        // Clean-up
        while ( menu.RemoveMenu( 0, MF_BYPOSITION ) );
        VERIFY( menu.DestroyMenu() );
    }
}

void CMenuBar::TrackPopupMenu()
{
    ASSERT( m_hMenu != 0 );

    CWinAppEx* pApp = CWinAppEx::GetInstance();
    UINT nMenuAnimation = TPM_VERPOSANIMATION;
    for ( int nItem = m_nItem; m_bContinue; nItem = m_nItem )
    {
        // Get popup menu to be tracked
        m_hMenuTracking = ( nItem == 0 ) ?
            ::GetSystemMenu( m_hWndMDIChild, FALSE ) :
            ::GetSubMenu( m_hMenu, nItem - 1 );
        if ( m_hMenuTracking == 0 )
        {
            ASSERT( false );
            break;
        }

        CToolBarCtrl& tbCtrl = GetToolBarCtrl();
        tbCtrl.PressButton( GetItemID( nItem ), TRUE );

        if ( m_bSelectFirst )
        {
            // Select first menu item
            ::keybd_event( VK_DOWN, 0, 0, 0 );
            ::keybd_event( VK_DOWN, 0, KEYEVENTF_KEYUP, 0 );
        }

        CRect rc;
        tbCtrl.GetItemRect( nItem, rc );
        ClientToScreen( rc );

        TPMPARAMS tpm;
        tpm.cbSize    = sizeof( tpm );
        tpm.rcExclude = rc;

        m_bPrimaryMenu = false;
        m_bSubmenuItem = false;
        m_bContinue    = false;

        // Install message hook
        m_bItemDropped = true;
        m_pMenuBar     = this;
        m_hMsgHook     = ::SetWindowsHookEx( WH_MSGFILTER, MessageProc, 0, ::GetCurrentThreadId() );
        ASSERT( m_hMsgHook != 0 );

        CWinAppEx::TrackPopupMenuEx( m_hMenuTracking,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL | ( pApp->IsWin50() ? nMenuAnimation : 0 ),
            rc.left, rc.bottom, GetParentFrame(), &tpm );

        // Uninstall message hook
        VERIFY( ::UnhookWindowsHookEx( m_hMsgHook ) );
        m_hMsgHook      = 0;
        m_pMenuBar      = 0;
        m_hMenuTracking = 0;
        m_bItemDropped  = false;

        tbCtrl.PressButton( GetItemID( nItem ), FALSE );

        if ( pApp->IsWin2K() )
        {
            // W2K: use animation only for the first menu
            nMenuAnimation = TPM_NOANIMATION;
        }
    }
}

void CMenuBar::ContinueTracking( bool bSelectFirst )
{
    GetParentFrame()->PostMessage( WM_CANCELMODE );   // close currently tracked menu

    m_bSelectFirst = bSelectFirst;
    m_bContinue    = true;
}

void CMenuBar::ExitTrackingMode()
{
    if ( m_bItemTracking )
    {
        GetToolBarCtrl().SetHotItem( -1 );

        // Restore focus
        if ( ( GetFocus() == this ) && ::IsWindow( m_hWndOldFocus ) )
        {
            ::SetFocus( m_hWndOldFocus );
        }

        // Release capture
        if ( GetCapture() == this )
        {
            VERIFY( ReleaseCapture() );
        }

        m_bItemTracking = false;
        m_hWndOldFocus  = 0;
    }
}

void CMenuBar::ShowChevronMenu( int nItem )
{
    if ( m_bItemDropped )
    {
        GetParentFrame()->PostMessage( WM_CANCELMODE );   // close currently tracked menu
    }

    GetParentReBarCtrl().PostMessage( RB_PUSHCHEVRON, GetParentBandIndex(), nItem + 1 );
}

bool CMenuBar::IsItemClipped( int nItem ) const
{
    CRect rcClient;
    GetClientRect( rcClient );

    CRect rcItem;
    GetToolBarCtrl().GetItemRect( nItem, rcItem );

    return ( rcItem.right > rcClient.right );
}

bool CMenuBar::IsOverChevron( CPoint pt ) const
{
    CPoint ptScreen( pt );
    ClientToScreen( &ptScreen );

    CReBarCtrl& rbCtrl = GetParentReBarCtrl();

    // Is mouse over parent re-bar control?
    if ( WindowFromPoint( ptScreen ) == &rbCtrl )
    {
        MapWindowPoints( &rbCtrl, &pt, 1 );

        RBHITTESTINFO rbht;
        rbht.pt = pt;

        // Is mouse over chevron of the parent band?
        return ( ( rbCtrl.HitTest( &rbht ) != -1 ) &&
            ( rbht.flags == RBHT_CHEVRON ) &&
            ( rbht.iBand == GetParentBandIndex() ) );
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// Overrides

void CMenuBar::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler )
{
    CMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST( CMDIFrameWnd, GetParentFrame() );
    if ( pMDIFrame != 0 )
    {
        // Check active MDI child
        BOOL bMaximized;
        CMDIChildWnd* pActive = pMDIFrame->MDIGetActive( &bMaximized );

        bool bSysMenu = ( pActive != 0 ) && ( pActive->GetStyle() & WS_SYSMENU ) && bMaximized;
        HWND hWndMDIChild = bSysMenu ? pActive->GetSafeHwnd() : 0;

        // Is it new MDI child?
        if ( hWndMDIChild != m_hWndMDIChild )
        {
            m_hWndMDIChild = hWndMDIChild;

            for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
            {
                m_aMenuBarButtons[ nIndex ]->SetMDIChild( m_hWndMDIChild );
            }

            UpdateMenuBar();
        }
    }

    CWinAppEx* pApp = CWinAppEx::GetInstance();
    if ( pApp->IsWin2K() && !m_bItemTracking )
    {
        BOOL bAltDown = ( ::GetKeyState( VK_MENU ) & 0x8000 );
        BOOL bF10Down = ( ::GetKeyState( VK_F10  ) & 0x8000 );
        DWORD dwDTFlags = ( pApp->GetMenuUnderlines() || bAltDown || bF10Down ) ? 0 : DT_HIDEPREFIX;
        if ( GetToolBarCtrl().SetDrawTextFlags( DT_HIDEPREFIX, dwDTFlags ) != dwDTFlags )
        {
            Invalidate();
        }
    }

    // MDM Codeguru comments "fix" (needed?)
	 // CToolBar::OnUpdateCmdUI( pTarget, bDisableIfNoHndler );
    CToolBar::OnUpdateCmdUI( pTarget, FALSE );
}

int CMenuBar::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const
{
    UINT nTipID = ( UINT )-1;

    if ( CWinAppEx::GetInstance()->IsWin50() ) // W98/W2K specific
    {
        // If mouse is over any of sys-buttons, show corresponding tooltip
        for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
        {
            CMenuBarButton* pButton = m_aMenuBarButtons.GetAt( nIndex );
            if ( pButton->HitTest( point ) )
            {
                switch ( pButton->GetSysCommandID() )
                {
                    case SC_CLOSE:
                        nTipID = IDS_TOOLTIP_CLOSE;
                        break;
                    case SC_MINIMIZE:
                        nTipID = IDS_TOOLTIP_MINIMIZE;
                        break;
//                    case SC_MAXIMIZE:
//                        nTipID = IDS_TOOLTIP_MAXIMIZE;
//                        break;
                    case SC_RESTORE:
                        nTipID = IDS_TOOLTIP_RESTORE;
                        break;
                }

                if ( ( nTipID != ( UINT )-1 ) && ( pTI != 0 ) )
                {
                    pTI->uFlags   = TTF_TRANSPARENT;
                    pTI->rect     = pButton->GetButtonRect();
                    pTI->hwnd     = m_hWnd;
                    pTI->uId      = nTipID;
                    pTI->lpszText = LPSTR_TEXTCALLBACK;
                }

                break;
            }
        }
    }

    return nTipID;  // CToolBar::OnToolHitTest( point, pTI );
}

/////////////////////////////////////////////////////////////////////////
// CMenuBar message handlers

BEGIN_MESSAGE_MAP(CMenuBar, CToolBar)
    //{{AFX_MSG_MAP(CMenuBar)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_CAPTURECHANGED()
    ON_WM_KEYDOWN()
    ON_WM_SYSKEYDOWN()
    ON_WM_KILLFOCUS()
    ON_WM_GETDLGCODE()
    ON_WM_SETTINGCHANGE()
    //}}AFX_MSG_MAP

    ON_MESSAGE( WM_MB_SHOWPOPUPMENU, OnShowPopupMenu )
    ON_MESSAGE( WM_REBAR_CHILDSIZE, OnReBarChildSize )
    ON_MESSAGE( WM_REBAR_CHEVRONPUSHED, OnReBarChevronPushed )
    ON_MESSAGE( WM_THEMECHANGED, OnThemeChanged )

    ON_UPDATE_COMMAND_UI_RANGE( IDBUTTON_FIRST, IDBUTTON_LAST, OnUpdateMenuButton )

    // Toolbar control notifications
    ON_NOTIFY_REFLECT( TBN_DROPDOWN, OnDropDown )
    ON_NOTIFY_REFLECT( TBN_HOTITEMCHANGE, OnHotItemChange )
    ON_NOTIFY_REFLECT( NM_CUSTOMDRAW, OnCustomDraw )
END_MESSAGE_MAP()

int CMenuBar::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CToolBar::OnCreate( lpCreateStruct ) == -1 )
    {
        return -1;
    }

    if ( CWinAppEx::GetInstance()->IsWinXP() )
    {
        OpenTheme();

        HRESULT hr = ::SetWindowThemeXP( m_hWnd, L"", L"" );
        ASSERT( SUCCEEDED( hr ) );
    }

    GetToolBarCtrl().SetImageList( 0 );

    return 0;
}

void CMenuBar::OnDestroy()
{
    CToolBar::OnDestroy();

    CloseTheme();
}

bool CMenuBar::OnButtonDown( UINT nFlags, CPoint pt, bool bLeft )
{
    if ( m_bItemTracking && !m_bItemDropped )
    {
        ClientToScreen( &pt );
        CWnd* pWnd = WindowFromPoint( pt );
        if ( pWnd != this )
        {
            // It's a good idea to allow user exit tracking mode
            // by clicking mouse anywhere outside the menu bar.
            ExitTrackingMode();

            // It would be also nice to forward this event to
            // the window the user has just clicked.
            if ( pWnd != 0 )
            {
                LPARAM nPosition = MAKELPARAM( pt.x, pt.y );
                WPARAM nHitTest  = pWnd->SendMessage( WM_NCHITTEST, 0, nPosition );

                // Is over client area of the window?
                if ( nHitTest == HTCLIENT )
                {
                    pWnd->ScreenToClient( &pt );
                    LPARAM nPosition = MAKELPARAM( pt.x, pt.y );
                    pWnd->PostMessage( bLeft ? WM_LBUTTONDOWN : WM_RBUTTONDOWN,
                        nFlags, nPosition );
                }
                else
                {
                    pWnd->PostMessage( bLeft ? WM_NCLBUTTONDOWN : WM_NCRBUTTONDOWN,
                        nHitTest, nPosition );
                }
            }

            return false;
        }
    }

    return true;
}

void CMenuBar::OnRButtonDown( UINT nFlags, CPoint point )
{
    if ( OnButtonDown( nFlags, point, false ) )
    {
        CToolBar::OnRButtonDown( nFlags, point );
    }
}

void CMenuBar::OnLButtonDown( UINT nFlags, CPoint point )
{
    if ( OnButtonDown( nFlags, point, true ) )
    {
        // Was it over any of sys-buttons?
        for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
        {
            CMenuBarButton* pButton = m_aMenuBarButtons.GetAt( nIndex );
            if ( pButton->IsEnabled() && pButton->HitTest( point ) )
            {
                ExitTrackingMode(); // just in case

                pButton->PushButton( true );
                RedrawWindow( pButton->GetButtonRect(), 0,
                    RDW_INVALIDATE | RDW_ERASE );   // visual feedback

                SetCapture();
                m_bButtonCapture = true;
                return;
            }
        }

        CToolBar::OnLButtonDown( nFlags, point );
    }
}

void CMenuBar::OnLButtonUp( UINT nFlags, CPoint point )
{
    if ( m_bButtonCapture )
    {
        // Was it over any of sys-buttons?
        for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
        {
            CMenuBarButton* pButton = m_aMenuBarButtons.GetAt( nIndex );
            if ( pButton->IsEnabled() && pButton->HitTest( point ) )
            {
                pButton->PushButton( false );
                RedrawWindow( pButton->GetButtonRect(), 0,
                    RDW_INVALIDATE | RDW_ERASE );   // visual feedback

                // Send corresponding sys-command
                ::PostMessage( m_hWndMDIChild, WM_SYSCOMMAND, pButton->GetSysCommandID(), 0 );
                break;
            }
        }

        VERIFY( ReleaseCapture() );
        m_bButtonCapture = false;
        return;
    }

    CToolBar::OnLButtonUp( nFlags, point );
}

void CMenuBar::OnMouseMove( UINT nFlags, CPoint point )
{
    if ( m_bItemTracking )
    {
        if ( m_bItemDropped && ( m_ptMouseLast == point ) )
        {
            return; // mouse has not actually moved
        }

        if ( IsOverChevron( point ) )
        {
            ShowChevronMenu( -1 );
            return;
        }

        CPoint ptScreen( point );
        ClientToScreen( &ptScreen );

        if ( WindowFromPoint( ptScreen ) != this )
        {
            return;
        }
    }

    m_ptMouseLast = point;

    if ( !m_bItemTracking && m_bButtonCapture )
    {
        // Are we over any of sys-buttons?
        for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
        {
            CMenuBarButton* pButton = m_aMenuBarButtons.GetAt( nIndex );
            bool bPush = pButton->IsEnabled() && pButton->HitTest( point );
            if ( bPush != pButton->PushButton( bPush ) )
            {
                RedrawWindow( pButton->GetButtonRect(), 0,
                    RDW_INVALIDATE | RDW_ERASE );   // visual feedback
            }
        }

        return;
    }

    CToolBar::OnMouseMove( nFlags, point );
}

void CMenuBar::OnLButtonDblClk( UINT nFlags, CPoint point )
{
    CRect rcSysMenu;
    GetItemRect( 0, rcSysMenu );

    // If we are over sys-menu button, send Close command
    if ( rcSysMenu.PtInRect( point ) )
    {
        ::PostMessage( m_hWndMDIChild, WM_SYSCOMMAND, SC_CLOSE, 0 );
        return;
    }

    CToolBar::OnLButtonDblClk( nFlags, point );
}

void CMenuBar::OnCaptureChanged( CWnd* pWnd )
{
    if ( m_bButtonCapture && ( pWnd != this ) )
    {
        for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
        {
            CMenuBarButton* pButton = m_aMenuBarButtons.GetAt( nIndex );
            if ( pButton->PushButton( false ) )
            {
                RedrawWindow( pButton->GetButtonRect(), 0,
                    RDW_INVALIDATE | RDW_ERASE );
            }
        }

        m_bButtonCapture = false;
    }

    CToolBar::OnCaptureChanged( pWnd );
}

void CMenuBar::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if ( m_bItemTracking )
    {
        switch ( nChar )
        {
            case VK_SPACE:
                ExitTrackingMode();
                GetParentFrame()->PostMessage( WM_SYSCOMMAND, SC_KEYMENU, 32 );
                return;
            case VK_ESCAPE:
                ExitTrackingMode();
                return;
            default:
                break;
        }
    }

    CToolBar::OnKeyDown( nChar, nRepCnt, nFlags );
}

void CMenuBar::OnSysKeyDown( UINT /*nChar*/, UINT /*nRepCnt*/, UINT /*nFlags*/ )
{
    ExitTrackingMode();

//    CToolBar::OnSysKeyDown( nChar, nRepCnt, nFlags );
}

void CMenuBar::OnKillFocus( CWnd* pNewWnd )
{
    CToolBar::OnKillFocus( pNewWnd );

    if ( m_bItemTracking )
    {
        ExitTrackingMode();
    }
}

UINT CMenuBar::OnGetDlgCode() 
{
    return ( CToolBar::OnGetDlgCode() | ( m_bItemTracking ? DLGC_WANTALLKEYS : 0 ) );
}

void CMenuBar::OnSettingChange( UINT uFlags, LPCTSTR lpszSection )
{
    CToolBar::OnSettingChange( uFlags, lpszSection );

    UpdateMenuBar();
}

LRESULT CMenuBar::OnShowPopupMenu( WPARAM wParam, LPARAM lParam )
{
    int  nItem         = ( int )wParam;
    bool bSelectFirst  = ( LOWORD( lParam ) != 0 );
    bool bCheckClipped = ( HIWORD( lParam ) != 0 );

    // If item is clipped, show chevron menu
    if ( bCheckClipped && IsItemClipped( nItem ) )
    {
        ShowChevronMenu( nItem );
    }
    // otherwise, track "normal" popup menu
    else
    {
        EnterTrackingMode( nItem );

			// From CodeProject comment.
			// http://www.codeproject.com/docking/sizablerebar.asp?df=100&forumid=2134&fr=26&select=788470#xx788470xx
			//zoc 2004-04-10 check if the same button clicked
			if (m_nItem == nItem)
			{
				ExitTrackingMode();
				return 0L;
			}
			//zoc 2004-04-10

        m_nItem        = nItem;
        m_bSelectFirst = bSelectFirst;
        m_bContinue    = true;
        m_bEscape      = false;

        TrackPopupMenu();

        // If menu was closed by pressing Esc, stay in tracking mode
        if ( m_bEscape )
        {
            SetCapture();
            SendMessage( WM_SETCURSOR, ( WPARAM )m_hWnd, MAKELPARAM( HTCLIENT, 0 ) );
        }
        // otherwise, exit tracking mode
        else
        {
            ExitTrackingMode();
        }
    }

    return 0L;
}

LRESULT CMenuBar::OnReBarChildSize( WPARAM /*wParam*/, LPARAM lParam )
{
//  CRect rcBand ( ( LPCRECT )wParam );
    CRect rcChild( ( LPCRECT )lParam );

    rcChild.OffsetRect( -rcChild.TopLeft() );
    RepositionSysButtons( rcChild );

    return 0L;
}

LRESULT CMenuBar::OnReBarChevronPushed( WPARAM wParam, LPARAM lParam )
{
    CRect rcChevron( ( LPCRECT )wParam );
    int nItem = ( int )lParam - 1;

    TrackChevronMenu( rcChevron, nItem );

    return 0L;
}

LRESULT CMenuBar::OnThemeChanged( WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
    CloseTheme();
    OpenTheme();

    return Default();
}

void CMenuBar::OnUpdateMenuButton( CCmdUI* pCmdUI )
{
    // Do not disable menu buttons
    pCmdUI->Enable();
}

void CMenuBar::OnDropDown( NMHDR* pNMHDR, LRESULT* pResult )
{
    NMTOOLBAR* pNMToolBar = ( NMTOOLBAR* )pNMHDR;

    if ( !m_bItemDropped )
    {
        int  nItem        = CommandToIndex( pNMToolBar->iItem );
        bool bLButtonDown = ( ::GetKeyState( VK_LBUTTON ) < 0 );

        PostMessage( WM_MB_SHOWPOPUPMENU, nItem,
            MAKELPARAM( bLButtonDown ? FALSE : TRUE, bLButtonDown ? FALSE : TRUE ) );
    }

    *pResult = TBDDRET_DEFAULT;
}

void CMenuBar::OnHotItemChange( NMHDR* pNMHDR, LRESULT* pResult )
{
    NMTBHOTITEM* pNMTBHotItem = ( NMTBHOTITEM* )pNMHDR;

    if ( m_bItemTracking)
    {
        // Is it really new hot item?

			// This update is based on Nicoli's CodeProject comment:
			//
			//		http://www.codeproject.com/docking/sizablerebar.asp?df=100&forumid=2134&fr=51&select=86170#xx86170xx
			//
			// But it's pretty old, I think he must have integrated fixes for this.
			// The rest of the fix (updating CMenuBar::OnUpdateCommandUI) looks old/minimal...
			// Revisit as needed...
			/*
			int nItem = 
					( ( pNMTBHotItem->dwFlags & HICF_LEAVING ) || !GetToolBarCtrl().IsButtonEnabled( pNMTBHotItem->idNew ) ) 
				?	-1 
				:	CommandToIndex( ( UINT )pNMTBHotItem->idNew );
			*/

			int nItem = ( pNMTBHotItem->dwFlags & HICF_LEAVING ) ? -1 : CommandToIndex( ( UINT )pNMTBHotItem->idNew );

        if ( m_nItem != nItem )
        {
            // Was it changed not by SetHotItem()?
            if ( pNMTBHotItem->dwFlags & ( HICF_ACCELERATOR | HICF_ARROWKEYS | HICF_DUPACCEL | HICF_MOUSE ) )
            {
                // There should be always hot item in tracking mode
                if ( nItem == -1)
                {
                    *pResult = 1;
                    return;
                }

                // If item is clipped, show chevron menu
                if ( IsItemClipped( nItem ) )
                {
                    ShowChevronMenu( nItem );
                }
                // If we are tracking popup menu, track new one, associated with hot item
                else if ( m_bItemDropped )
                {
                    ContinueTracking( !( pNMTBHotItem->dwFlags & HICF_MOUSE ) );
                }
            }

            m_nItem = nItem;    // remember last hot item
        }
    }

    *pResult = 0;
}

void CMenuBar::OnCustomDraw( NMHDR* pNMHDR, LRESULT* pResult )
{
    LPNMTBCUSTOMDRAW lpNMCustomDraw = ( LPNMTBCUSTOMDRAW )pNMHDR;
    NMCUSTOMDRAW& nmcd = lpNMCustomDraw->nmcd;

    *pResult = CDRF_DODEFAULT;  // by default

    switch ( lpNMCustomDraw->nmcd.dwDrawStage )
    {
        case CDDS_PREERASE:
            *pResult = CDRF_NOTIFYPOSTERASE;
            break;

        case CDDS_POSTERASE:
        {
            CDC* pDC = CDC::FromHandle( nmcd.hdc );
            for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
            {
                m_aMenuBarButtons[ nIndex ]->DrawButton( pDC, m_bFrameActive );
            }
            break;
        }
        case CDDS_PREPAINT:
            *pResult = CDRF_NOTIFYITEMDRAW;
            break;

        case CDDS_ITEMPREPAINT:
        {
            UINT uItemState = nmcd.uItemState;

            if ( ( UINT )nmcd.dwItemSpec == IDBUTTON_SYSMENU )  // draw sys-menu button
            {
                HICON hIcon = ( HICON )::GetClassLong( m_hWndMDIChild, GCL_HICONSM );
                if ( hIcon != 0 )
                {
                    // Icon has 2 pixel border on left, 1 pixel on top/bottom, 0 right
                    CSize sz( CMenuBarButton::GetButtonSize() );
                    sz.cx -= 2;
                    sz.cy -= 2;

                    CRect rc( &nmcd.rc );
                    rc.left  += 2;
                    rc.top   += ( rc.Height() - sz.cy ) / 2;
                    rc.right  = rc.left + sz.cx;
                    rc.bottom = rc.top  + sz.cy;

                    VERIFY( ::DrawIconEx( nmcd.hdc, rc.left, rc.top, hIcon,
                        rc.Width(), rc.Height(), 0, 0, DI_NORMAL ) );
                }

                *pResult = CDRF_SKIPDEFAULT;
            }
            else
            {
                CWinAppEx* pApp = CWinAppEx::GetInstance();
                if ( pApp->IsWin50() )
                {
                    if ( pApp->IsWinXP() && pApp->GetFlatMenu() )
                    {
                        *pResult |= TBCDRF_NOEDGES | TBCDRF_NOOFFSET;
                        if ( !( uItemState & CDIS_DISABLED ) && ( uItemState & ( CDIS_HOT | CDIS_SELECTED ) ) )
                        {
                            lpNMCustomDraw->clrHighlightHotTrack = ::GetSysColor( COLOR_MENUHILIGHT );
                            lpNMCustomDraw->clrText = ::GetSysColor( m_bFrameActive ? COLOR_HIGHLIGHTTEXT : COLOR_GRAYTEXT );
                            *pResult |= TBCDRF_HILITEHOTTRACK;
                        }
                        else if ( ( uItemState & CDIS_DISABLED ) || !m_bFrameActive )
                        {
                            lpNMCustomDraw->clrText = ::GetSysColor( COLOR_GRAYTEXT );
                        }
                    }
                    else if ( !m_bFrameActive )
                    {
                        lpNMCustomDraw->clrText = ::GetSysColor( COLOR_GRAYTEXT );
                    }
                }
            }

            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

void CMenuBar::HookMessageProc( UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDBLCLK:
        {
            CPoint pt( LOWORD( lParam ), HIWORD( lParam ) );
            ScreenToClient( &pt );
            SendMessage( message, wParam, MAKELPARAM( pt.x, pt.y ) );
            break;
        }
        case WM_KEYDOWN:
        {
            UINT nChar = ( UINT )wParam;
            bool bForward = false;  // by default

            switch ( nChar )
            {
                case VK_LEFT:
                    bForward = m_bPrimaryMenu;
                    break;
                case VK_RIGHT:
                    bForward = !m_bSubmenuItem;
                    break;
                case VK_ESCAPE:
                    m_bEscape = m_bPrimaryMenu;
                    break;
            }

            // Should we forward this message to the menu bar?
            if ( bForward )
            {
                SendMessage( message, wParam, lParam );
            }
            break;
        }
        default:
            break;
    }
}

LRESULT CALLBACK CMenuBar::MessageProc( int code, WPARAM wParam, LPARAM lParam )
{
    ASSERT( m_pMenuBar != 0 );

    if ( code == MSGF_MENU )
    {
        MSG* pMsg = ( MSG* )lParam;
        m_pMenuBar->HookMessageProc( pMsg->message, pMsg->wParam, pMsg->lParam );
    }

    return ::CallNextHookEx( m_hMsgHook, code, wParam, lParam );
}

bool CMenuBar::FrameOnSysCommand( UINT nID, LPARAM lParam )
{
    if ( ::IsWindow( m_hWnd ) && IsVisible() && ( ( nID & 0xFFF0 ) == SC_KEYMENU ) && ( lParam == 0 ) )
    {
        if ( IsItemClipped( 1 ) )
        {
            ShowChevronMenu( -1 );
        }
        else
        {
            EnterTrackingMode( 1 );
        }

        return true;
    }

    return false;
}

bool CMenuBar::FrameOnMenuChar( UINT nChar, UINT nFlags, CMenu* /*pMenu*/ )
{
    if ( ::IsWindow( m_hWnd ) && IsVisible() && ( nFlags & MF_SYSMENU ) )
    {
        UINT nID;
        if ( GetToolBarCtrl().MapAccelerator( ( TCHAR )nChar, &nID ) )
        {
            PostMessage( WM_MB_SHOWPOPUPMENU, CommandToIndex( nID ), MAKELPARAM( TRUE, TRUE ) );
            return true;
        }
    }

    return false;
}

void CMenuBar::FrameOnNcActivate( BOOL bActive )
{
    if ( ::IsWindow( m_hWnd ) )
    {
        m_bFrameActive = ( bActive == TRUE );

        Invalidate();
        UpdateWindow();
    }
}

void CMenuBar::FrameOnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL /*bSysMenu*/ )
{
    if ( ::IsWindow( m_hWnd ) && ( m_hMenuTracking != 0 ) )
    {
        HMENU hMenu = pPopupMenu->GetSafeHmenu();

        // We should know if it is primary menu and whether or not selected
        // item is submenu item to handle Esc/Left/Right keys correctly
        m_bPrimaryMenu = ( hMenu == m_hMenuTracking );
        m_bSubmenuItem = ( ::GetSubMenu( hMenu, nIndex ) != 0 );
    }
}

void CMenuBar::FrameOnMenuSelect( UINT /*nItemID*/, UINT nFlags, HMENU hSysMenu )
{
    if ( ::IsWindow( m_hWnd ) && ( m_hMenuTracking != 0 ) )
    {
        // We should know if it is primary menu and whether or not selected
        // item is submenu item to handle Esc/Left/Right keys correctly
        m_bPrimaryMenu = ( hSysMenu == m_hMenuTracking );
        m_bSubmenuItem = ( nFlags & MF_POPUP ) != 0;//( ::GetSubMenu( hSysMenu, nItemID ) != 0 );
    }
}

void CMenuBar::OpenTheme()
{
    m_hTheme = ::IsThemeActiveXP() ? ::OpenThemeDataXP( AfxGetMainWnd()->GetSafeHwnd(), L"Window" ) : 0;
    for ( int nIndex = 0; nIndex <= m_aMenuBarButtons.GetUpperBound(); nIndex++ )
    {
        m_aMenuBarButtons[ nIndex ]->SetTheme( m_hTheme );
    }
}

void CMenuBar::CloseTheme()
{
    if ( m_hTheme != 0 )
    {
        HRESULT hr = ::CloseThemeDataXP( m_hTheme );
        ASSERT( SUCCEEDED( hr ) );

        m_hTheme = 0;
    }
}
