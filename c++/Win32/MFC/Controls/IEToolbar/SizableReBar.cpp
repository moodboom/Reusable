//-------------------------------------------------------------------//
// CSizableReBar class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#include "stdafx.h"

#include <map>                      // For std::map
#include <STLContainers.h>          // For TraceVector

#include "MenuBar.h"
#include "SizableReBar.h"
#include "ToolBarEx.h"
#include "..\..\WinAppEx.h"

#include <afxpriv.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSizableReBar

int CSizableReBar::m_nStateInfoVersion = 1;

LPCTSTR CSizableReBar::m_lpszStateInfoFormat  = _T("wID=%04X,cx=%d,fStyle=%08X");
LPCTSTR CSizableReBar::m_lpszStateInfoVersion = _T("Version");
LPCTSTR CSizableReBar::m_lpszStateInfoLocked  = _T("Locked");
LPCTSTR CSizableReBar::m_lpszStateInfoBand    = _T("BandStateInfo");

IMPLEMENT_DYNAMIC( CSizableReBar, CControlBar )

CSizableReBar::CSizableReBar()
{
    m_bTracking   = false;
    m_cxEdge      = 3;
    m_cyEdge      = 3;
    m_hbmBack     = 0;
    m_bCommonPart = true;
    m_bCustomPart = true;
    m_bLocked     = false;

    SetBorders();
}

CSizableReBar::~CSizableReBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// Operations

bool CSizableReBar::Create( CWnd* pParentWnd, UINT nID, DWORD dwStyle )
{
    ASSERT_VALID( pParentWnd );     // must have a parent
    ASSERT( !( ( dwStyle & CBRS_SIZE_FIXED ) && ( dwStyle & CBRS_SIZE_DYNAMIC ) ) );

    m_dwStyle = ( dwStyle & CBRS_ALL );    // save the control bar styles
    m_dwStyle |= CBRS_HIDE_INPLACE;

    // Register and create the window - skip CControlBar::Create()
    CString strWndClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor( 0, IDC_ARROW ), ::GetSysColorBrush( COLOR_BTNFACE ), 0 );

    dwStyle &= ~CBRS_ALL;       // keep only the generic window styles
    dwStyle |= WS_CLIPCHILDREN; // prevents flashing

    if ( !CWnd::Create( strWndClass, 0, dwStyle, CRect( 0, 0, 0, 0 ), pParentWnd, nID ) )
    {
        return false;
    }

    return true;
}

bool CSizableReBar::AddBar( CWnd* pBar, LPCTSTR pszText, CBitmap* pbmp,
                            DWORD dwStyle, LPCTSTR lpszTitle,
                            bool bAlwaysVisible, bool bHasGripper, int nStartingWidth )
{
    REBARBANDINFO rbbi;
    rbbi.fMask  = RBBIM_STYLE;
    rbbi.fStyle = dwStyle;

    if ( pszText != 0 )
    {
        rbbi.fMask |= RBBIM_TEXT;
        rbbi.lpText = ( LPTSTR )pszText;
    }

    if ( pbmp != 0 )
    {
        rbbi.fMask |= RBBIM_BACKGROUND;
        rbbi.hbmBack = ( HBITMAP )*pbmp;
    }

    return DoAddBar( pBar, &rbbi, lpszTitle, bAlwaysVisible, bHasGripper, nStartingWidth );
}

bool CSizableReBar::AddBar( CWnd* pBar, COLORREF clrFore, COLORREF clrBack, LPCTSTR pszText,
                            DWORD dwStyle, LPCTSTR lpszTitle,
                            bool bAlwaysVisible, bool bHasGripper, int nStartingWidth )
{
    REBARBANDINFO rbbi;
    rbbi.fMask   = RBBIM_STYLE | RBBIM_COLORS;
    rbbi.fStyle  = dwStyle;
    rbbi.clrFore = clrFore;
    rbbi.clrBack = clrBack;

    if ( pszText != 0 )
    {
        rbbi.fMask |= RBBIM_TEXT;
        rbbi.lpText = ( LPTSTR )pszText;
    }

    return DoAddBar( pBar, &rbbi, lpszTitle, bAlwaysVisible, bHasGripper, nStartingWidth );
}

bool CSizableReBar::SetBkImage( CBitmap* pbmp )
{
    m_hbmBack = *pbmp;

    REBARBANDINFO rbbi;
    rbbi.cbSize = sizeof( rbbi );
    rbbi.fMask  = RBBIM_BACKGROUND;

    for ( UINT nBand = 0; nBand < m_rbCtrl.GetBandCount(); nBand++ )
    {
        rbbi.hbmBack = m_hbmBack;
        VERIFY( m_rbCtrl.SetBandInfo( nBand, &rbbi ) );
    }

    return true;
}

void CSizableReBar::EnableContextMenu( bool bCommonPart /*=true*/, bool bCustomPart /*=true*/ )
{
    m_bCommonPart = bCommonPart;
    m_bCustomPart = bCustomPart;
}

bool CSizableReBar::Lock( bool bLock /*=true*/ )
{
    bool bLocked = m_bLocked;
    if ( m_bLocked != bLock )
    {
        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof( rbbi );
        rbbi.fMask  = RBBIM_STYLE | RBBIM_ID;

        for ( UINT nBand = 0; nBand < m_rbCtrl.GetBandCount(); nBand++ )
        {
            VERIFY( m_rbCtrl.GetBandInfo( nBand, &rbbi ) );

            for ( int nIndex = 0; nIndex <= m_aBars.GetUpperBound(); nIndex++ )
            {
                if ( m_aBars[ nIndex ].nID == rbbi.wID )
                {
                    if ( bLock || !m_aBars[ nIndex ].bHasGripper )
                    {
                        rbbi.fStyle &= ~RBBS_GRIPPERALWAYS;
                        rbbi.fStyle |= RBBS_NOGRIPPER;
                    }
                    else
                    {
                        rbbi.fStyle |= RBBS_GRIPPERALWAYS;
                        rbbi.fStyle &= ~RBBS_NOGRIPPER;
                    }
                    break;
                }
            }

            VERIFY( m_rbCtrl.SetBandInfo( nBand, &rbbi ) );
        }

        m_bLocked = bLock;
		  
		  // MDM 
        // GetParentFrame()->DelayRecalcLayout();
		  PostMessage( WM_RECALCPARENT );
    }

    return bLocked;
}

bool CSizableReBar::IsLocked()
{
    return m_bLocked;
}

void CSizableReBar::LoadState( LPCTSTR lpszProfileName )
{
    // This function restores index, width and style from the registry for
    // each band in the rebar.

    int nVersion = AfxGetApp()->GetProfileInt( lpszProfileName, m_lpszStateInfoVersion, -1 );
    if ( nVersion != m_nStateInfoVersion )
    {
        return;
    }

    int nLocked = AfxGetApp()->GetProfileInt( lpszProfileName, m_lpszStateInfoLocked, 0 );
    m_bLocked = ( nLocked != 0 );

    CString strValue = AfxGetApp()->GetProfileString( lpszProfileName, m_lpszStateInfoBand );
    if ( !strValue.IsEmpty() )
    {
        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof( rbbi );
        rbbi.fMask  = RBBIM_STYLE | RBBIM_SIZE | RBBIM_ID;

        for ( UINT nBand = 0; nBand < m_rbCtrl.GetBandCount(); nBand++ )
        {
            CString strBandState;
            VERIFY( AfxExtractSubString( strBandState, strValue, nBand, _T('\n') ) );

            UINT nID, cx, nStyle;
            int nResult = _stscanf( strBandState, m_lpszStateInfoFormat, &nID, &cx, &nStyle );
            ASSERT( nResult == 3 );

            m_rbCtrl.MoveBand( m_rbCtrl.IDToIndex( nID ), nBand );
            VERIFY( m_rbCtrl.GetBandInfo( nBand, &rbbi ) );
            rbbi.cx     = cx;
            rbbi.fStyle = ( rbbi.fStyle & ~( RBBS_HIDDEN | RBBS_BREAK ) ) | nStyle;
            VERIFY( m_rbCtrl.SetBandInfo( nBand, &rbbi ) );
        }
    }
}

void CSizableReBar::SaveState( LPCTSTR lpszProfileName )
{
    // This function saves index, width and style in the registry for each
    // band in the rebar, so that it could be possible to restore all these
    // settings when the user runs the program next time.

    VERIFY( AfxGetApp()->WriteProfileInt( lpszProfileName, m_lpszStateInfoVersion, m_nStateInfoVersion ) );
    VERIFY( AfxGetApp()->WriteProfileInt( lpszProfileName, m_lpszStateInfoLocked, m_bLocked ? 1 : 0 ) );

    CString strValue;

    REBARBANDINFO rbbi;
    rbbi.cbSize = sizeof( rbbi );
    rbbi.fMask  = RBBIM_STYLE | RBBIM_SIZE | RBBIM_ID;

    for ( UINT nBand = 0; nBand < m_rbCtrl.GetBandCount(); nBand++ )
    {
        VERIFY( m_rbCtrl.GetBandInfo( nBand, &rbbi ) );

        CString strBandState;
        strBandState.Format( m_lpszStateInfoFormat, rbbi.wID, rbbi.cx, rbbi.fStyle );
        strValue += ( strValue.IsEmpty() ? _T("") : _T("\n") ) + strBandState;
    }

    VERIFY( AfxGetApp()->WriteProfileString( lpszProfileName,
        m_lpszStateInfoBand, strValue ) );
}

CReBarCtrl& CSizableReBar::GetReBarCtrl()
{
    return m_rbCtrl;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

bool CSizableReBar::DoAddBar( CWnd* pBar, REBARBANDINFO* prbbi, LPCTSTR lpszTitle,
                              bool bAlwaysVisible, bool bHasGripper, int nStartingWidth )
{
    UINT nID = ( UINT )pBar->GetDlgCtrlID();
    ASSERT( nID != 0 );
    ASSERT( m_rbCtrl.IDToIndex( nID ) == -1 );    // ID must be unique!

    // Save bar info for context menu
    BARINFO barinfo;
    barinfo.nID            = nID;
    barinfo.bAlwaysVisible = bAlwaysVisible;
    barinfo.bHasGripper    = bHasGripper;
    _tcsncpy( barinfo.szTitle, lpszTitle ? lpszTitle : _T(""), MAX_PATH );
    m_aBars.Add( barinfo );

    prbbi->cbSize    = sizeof( *prbbi );
    prbbi->fMask    |= RBBIM_ID | RBBIM_BACKGROUND | RBBIM_CHILD | RBBIM_CHILDSIZE;
    prbbi->wID       = nID;
    prbbi->hbmBack   = m_hbmBack;
    prbbi->hwndChild = pBar->m_hWnd;

    BOOL bHorz = ( m_dwStyle & CBRS_ORIENT_HORZ ) != 0;
    CControlBar* pTemp = DYNAMIC_DOWNCAST( CControlBar, pBar );
    if ( pTemp != 0 )
    {
        CSize szBar = pTemp->CalcFixedLayout( FALSE, bHorz );
        prbbi->cxMinChild = bHorz ? szBar.cx : szBar.cy;
        prbbi->cyMinChild = bHorz ? szBar.cy : szBar.cx;
    }
    else
    {
        CRect rcWindow;
        pBar->GetWindowRect( rcWindow );
        prbbi->cxMinChild = bHorz ? rcWindow.Width() : rcWindow.Height();
        prbbi->cyMinChild = bHorz ? rcWindow.Height() : rcWindow.Width();
    }

    if ( m_bLocked || !bHasGripper )
    {
        prbbi->fStyle &= ~RBBS_GRIPPERALWAYS;
        prbbi->fStyle |= RBBS_NOGRIPPER;
    }
    else
    {
        prbbi->fStyle |= RBBS_GRIPPERALWAYS;
        prbbi->fStyle &= ~RBBS_NOGRIPPER;
    }

    if ( pBar->IsKindOf( RUNTIME_CLASS( CMenuBar ) ) ||
         pBar->IsKindOf( RUNTIME_CLASS( CToolBarEx ) ) )
    {
        prbbi->fStyle |= RBBS_USECHEVRON;
    }

    if ( nStartingWidth > 0 )
    {
        prbbi->fMask |= RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE;
		prbbi->cx = nStartingWidth;
		prbbi->cxMinChild = nStartingWidth;
    }

    if ( !m_rbCtrl.InsertBand( ( UINT )-1, prbbi ) )
    {
        return false;
    }

    pBar->SetOwner( this );
    GetParentFrame()->RecalcLayout();
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Overrides

CSize CSizableReBar::CalcFixedLayout( BOOL bStretch, BOOL bHorz )
{
//    ASSERT( bStretch ); // the bar is stretched (is not the child of a dockbar)

    REBARBANDINFO rbbi;
    rbbi.cbSize = sizeof( rbbi );
    rbbi.fMask  = RBBIM_CHILD | RBBIM_STYLE;

    for ( UINT nBand = 0; nBand < m_rbCtrl.GetBandCount(); nBand++ )
    {
        VERIFY( m_rbCtrl.GetBandInfo( nBand, &rbbi ) );

        CControlBar* pBar = DYNAMIC_DOWNCAST( CControlBar, CWnd::FromHandlePermanent( rbbi.hwndChild ) );
        BOOL bWindowVisible = ( pBar != 0 ) ? pBar->IsVisible() : ( ( ::GetWindowLong( rbbi.hwndChild, GWL_STYLE ) & WS_VISIBLE ) != 0 );
        BOOL bBandVisible = !( rbbi.fStyle & RBBS_HIDDEN );
        if ( bWindowVisible != bBandVisible )
        {
            VERIFY( m_rbCtrl.ShowBand( nBand, bWindowVisible ) );
        }
    }

    CRect rcReBar;
    m_rbCtrl.GetWindowRect( rcReBar );

    CSize szCurrent(
        (  bHorz && bStretch ) ? 32767 : rcReBar.Width(),
        ( !bHorz && bStretch ) ? 32767 : rcReBar.Height() );

    if ( m_dwStyle & CBRS_ORIENT_VERT )
    {
        if ( !m_bLocked && ( szCurrent.cx > 0 ) )
        {
            szCurrent.cx += m_cxEdge;
        }
    }

    if ( m_dwStyle & CBRS_ORIENT_HORZ )
    {
        if ( !m_bLocked && ( szCurrent.cy > 0 ) )
        {
            szCurrent.cy += m_cyEdge;
        }
    }

    return szCurrent;
}

CSize CSizableReBar::CalcDynamicLayout( int /*nLength*/, DWORD dwMode )
{
    return CalcFixedLayout( dwMode & LM_STRETCH, dwMode & LM_HORZ );
}

LRESULT CSizableReBar::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
    // Special handling for certain messages (forwarding to owner/parent)
    switch ( message )
    {
        case WM_POPMESSAGESTRING:
        case WM_SETMESSAGESTRING:
            return GetOwner()->SendMessage( message, wParam, lParam );

        default:
            return CControlBar::WindowProc( message, wParam, lParam );
    }
}

BOOL CSizableReBar::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
    if ( wParam != 1 )
    {
        return FALSE;
    }

    REBARBANDINFO rbbi;
    rbbi.cbSize = sizeof( rbbi );
    rbbi.fMask  = RBBIM_CHILD;

    // Let child windows handle certain messages
    NMHDR* pNMHDR = ( NMHDR* )lParam;
    switch ( pNMHDR->code )
    {
        case RBN_HEIGHTCHANGE:
        case RBN_ENDDRAG:
        {
            CFrameWnd* pFrameWnd = GetParentFrame();
            ASSERT( pFrameWnd != 0 );

            if ( !( ( CFrameWndFriend* )pFrameWnd )->m_bInRecalcLayout )
            {
                pFrameWnd->RecalcLayout();
            }
            else
            {
                PostMessage( WM_RECALCPARENT );
            }
            break;
        }
        case RBN_CHILDSIZE:
        {
            NMREBARCHILDSIZE* lprbcs = ( NMREBARCHILDSIZE* )pNMHDR;
            VERIFY( m_rbCtrl.GetBandInfo( lprbcs->uBand, &rbbi ) );

            ::SendMessage( rbbi.hwndChild, WM_REBAR_CHILDSIZE,
                ( WPARAM )&lprbcs->rcBand,
                ( LPARAM )&lprbcs->rcChild );
            break;
        }
        case RBN_CHEVRONPUSHED:
        {
            NMREBARCHEVRON* lpnm = ( NMREBARCHEVRON* )pNMHDR;
            VERIFY( m_rbCtrl.GetBandInfo( lpnm->uBand, &rbbi ) );

            CRect rcChevron( lpnm->rc );
            m_rbCtrl.ClientToScreen( rcChevron );
            ::SendMessage( rbbi.hwndChild, WM_REBAR_CHEVRONPUSHED,
                ( WPARAM )( LPCRECT )rcChevron, lpnm->lParamNM );
            break;
        }
        default:
            // Forward notifications from the re-bar control to owner/parent
            *pResult = GetParent()->SendMessage( WM_NOTIFY, wParam, lParam );
            break;
    }

    return TRUE;
}

void CSizableReBar::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHandler )
{
    UpdateDialogControls( pTarget, bDisableIfNoHandler );
}

/////////////////////////////////////////////////////////////////////////
// CSizableReBar message handlers

BEGIN_MESSAGE_MAP( CSizableReBar, CControlBar )
    //{{AFX_MSG_MAP(CSizableReBar)
    ON_WM_CREATE()
    ON_WM_NCPAINT()
    ON_WM_NCCALCSIZE()
    ON_WM_NCHITTEST()
    ON_WM_CAPTURECHANGED()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_NCLBUTTONDOWN()
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
    ON_MESSAGE_VOID( WM_RECALCPARENT, OnRecalcParent )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////
// CSizableReBar message handlers

int CSizableReBar::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CControlBar::OnCreate( lpCreateStruct ) == -1 )
    {
        return -1;
    }

    DWORD dwOrientStyle = CCS_TOP;
    if ( m_dwStyle & CBRS_ALIGN_BOTTOM )
    {
        dwOrientStyle = CCS_BOTTOM;
    }
    else if ( m_dwStyle & CBRS_ALIGN_LEFT )
    {
        dwOrientStyle = CCS_LEFT;
    }
    else if ( m_dwStyle & CBRS_ALIGN_RIGHT )
    {
        dwOrientStyle = CCS_RIGHT;
    }

    if ( !m_rbCtrl.Create( RBS_BANDBORDERS | RBS_VARHEIGHT | RBS_DBLCLKTOGGLE | CCS_NODIVIDER | CCS_NOPARENTALIGN | dwOrientStyle |
            WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, CRect( 0, 0, 0, 0 ), this, 1 ) )
    {
        return -1;
    }

    VERIFY( m_rbCtrl.ModifyStyleEx( 0, WS_EX_TOOLWINDOW ) );

    return 0;
}

void CSizableReBar::OnNcLButtonDown( UINT nHitTest, CPoint point )
{
    if ( !m_bTracking )
    {
        if ( ( nHitTest >= HTSIZEFIRST ) && ( nHitTest <= HTSIZELAST ) )
        {
            StartTracking( nHitTest, point );
        }
    }
}

void CSizableReBar::OnMouseMove( UINT nFlags, CPoint point )
{
    if ( m_bTracking )
    {
        OnTrackUpdateSize( point );
    }

    CControlBar::OnMouseMove( nFlags, point );
}

void CSizableReBar::OnLButtonUp( UINT nFlags, CPoint point )
{
    if ( m_bTracking )
    {
        StopTracking();
    }

    CControlBar::OnLButtonUp( nFlags, point );
}

void CSizableReBar::OnCaptureChanged( CWnd *pWnd )
{
    if ( m_bTracking && ( pWnd != this ) )
    {
        StopTracking();
    }

    CControlBar::OnCaptureChanged( pWnd );
}

void CSizableReBar::OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp )
{
    CControlBar::OnNcCalcSize( bCalcValidRects, lpncsp );

    if ( !m_bLocked )
    {
        CRect rcWindow( lpncsp->rgrc[ 0 ] );
        CRect rcClient( rcWindow );
        CRect rcEdge;
        if ( GetSizingEdgeRect( rcWindow, rcEdge, GetSizingEdgeHitCode() ) )
        {
            if ( rcClient.SubtractRect( rcWindow, rcEdge ) )
            {
                lpncsp->rgrc[ 0 ] = rcClient;
            }
        }
    }
}

void CSizableReBar::OnNcPaint()
{
    EraseNonClient();
}


// MDM VC8 "fixed" the return value from several macros.
// I'll make it backwards-compatible for now...
#if _MSC_VER >= 1400
	LRESULT CSizableReBar::OnNcHitTest( CPoint point )
#else
	UINT CSizableReBar::OnNcHitTest( CPoint point )
#endif
{
    CRect rcWindow;
    GetWindowRect( rcWindow );

    CRect rcEdge;
    UINT nHitTest = GetSizingEdgeHitCode();
    if ( GetSizingEdgeRect( rcWindow, rcEdge, nHitTest ) && rcEdge.PtInRect( point ) )
    {
        return nHitTest;
    }

    return HTCLIENT;
}

void CSizableReBar::OnSize( UINT nType, int cx, int cy )
{
    CControlBar::OnSize( nType, cx, cy );

    CRect rcClient;
    GetClientRect( rcClient );
    m_rbCtrl.MoveWindow( rcClient );
}

void CSizableReBar::OnContextMenu( CWnd* /*pWnd*/, CPoint point )
{
    if ( !m_bCommonPart && !m_bCustomPart )
    {
        return;
    }

    CMenu menu;
    VERIFY( menu.CreatePopupMenu() );

    int nBand;

    REBARBANDINFO rbbi;
    rbbi.cbSize = sizeof( rbbi );

    // Populate menu by adding titles of the bars that can be shown/hidden
    if ( m_bCommonPart )
    {
        for ( int nIndex = 0; nIndex <= m_aBars.GetUpperBound(); nIndex++ )
        {
            if ( _tcslen( m_aBars[ nIndex ].szTitle ) > 0 )
            {
                nBand = m_rbCtrl.IDToIndex( m_aBars[ nIndex ].nID );
                if ( nBand != -1 )
                {
                    rbbi.fMask = RBBIM_STYLE | RBBIM_CHILD;
                    VERIFY( m_rbCtrl.GetBandInfo( nBand, &rbbi ) );

                    UINT nFlags = MF_BYCOMMAND;
                    if ( m_aBars[ nIndex ].bAlwaysVisible )
                    {
                        nFlags |= MF_GRAYED;
                    }
                    if ( !( rbbi.fStyle & RBBS_HIDDEN ) )
                    {
                        nFlags |= MF_CHECKED;
                    }

                    VERIFY( menu.AppendMenu( nFlags,
                        m_aBars[ nIndex ].nID, m_aBars[ nIndex ].szTitle ) );
                }
            }
        }

        CString strLock;
        VERIFY( strLock.LoadString( IDS_LOCKTOOLBARS ) );

        UINT nFlags = MF_BYCOMMAND;
        if ( m_bLocked )
        {
            nFlags |= MF_CHECKED;
        }

        VERIFY( menu.AppendMenu( MF_SEPARATOR ) );
        VERIFY( menu.AppendMenu( nFlags, ID_LOCKTOOLBARS, strLock ) );
    }

    // Populate menu by adding items specific to the bar that was clicked
    HWND hwndBar = 0;
    if ( m_bCustomPart )
    {
        RBHITTESTINFO rbhti;
        rbhti.pt = point;
        m_rbCtrl.ScreenToClient( &rbhti.pt );

        nBand = m_rbCtrl.HitTest( &rbhti );
        if ( nBand != -1 )
        {
            rbbi.fMask = RBBIM_CHILD;
            VERIFY( m_rbCtrl.GetBandInfo( nBand, &rbbi ) );
            hwndBar = rbbi.hwndChild;
            ::SendMessage( hwndBar, WM_REBAR_CONTEXTMENU, ( WPARAM )&menu, 0 );
        }
    }

    // Track menu
    CWinAppEx::GetInstance()->SetMenuIcons( &menu, true );
    UINT nID = menu.TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
        point.x, point.y, GetParentFrame() );
    nBand = m_rbCtrl.IDToIndex( nID );
    if ( nBand != -1 )  // was it "Show/Hide Bar" command?
    {
        rbbi.fMask = RBBIM_CHILD | RBBIM_STYLE;
        VERIFY( m_rbCtrl.GetBandInfo( nBand, &rbbi ) );
        BOOL bBandVisible = ( rbbi.fStyle & RBBS_HIDDEN ) != 0;
        VERIFY( m_rbCtrl.ShowBand( nBand, bBandVisible ) );

        CControlBar* pBar = DYNAMIC_DOWNCAST( CControlBar, CWnd::FromHandlePermanent( rbbi.hwndChild ) );
        BOOL bWindowVisible = ( pBar != 0 ) ? pBar->IsVisible() : ( ( ::GetWindowLong( rbbi.hwndChild, GWL_STYLE ) & WS_VISIBLE ) != 0 );
        if ( bWindowVisible != bBandVisible )
        {
            VERIFY( ::ShowWindow( rbbi.hwndChild, bBandVisible ? SW_SHOW : SW_HIDE ) );
        }
    }
    else if ( nID == ID_LOCKTOOLBARS )
    {
        Lock( !m_bLocked );
    }
    else if ( hwndBar != 0 )
    {
        // It was bar-specific command, so forward it to the bar
        ::SendMessage( hwndBar, WM_COMMAND, MAKEWPARAM( nID, 0 ), 0 );
    }

    VERIFY( menu.DestroyMenu() );
}

void CSizableReBar::OnRecalcParent()
{
    CFrameWnd* pFrameWnd = GetParentFrame();
    ASSERT( pFrameWnd != 0 );
    pFrameWnd->RecalcLayout();
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

UINT CSizableReBar::GetSizingEdgeHitCode() const
{
    if ( m_dwStyle & CBRS_ALIGN_LEFT )
    {
        return HTRIGHT;
    }
    if ( m_dwStyle & CBRS_ALIGN_TOP )
    {
        return HTBOTTOM;
    }
    if ( m_dwStyle & CBRS_ALIGN_RIGHT )
    {
        return HTLEFT;
    }
    if ( m_dwStyle & CBRS_ALIGN_BOTTOM )
    {
        return HTTOP;
    }

    return HTNOWHERE;
}

bool CSizableReBar::GetSizingEdgeRect( const CRect& rcBar, CRect& rcEdge, UINT nHitTest ) const
{
    rcEdge = rcBar;

    switch ( nHitTest )
    {
        case HTLEFT:
            rcEdge.right = rcEdge.left + m_cxEdge;
            break;
        case HTBOTTOM:
            rcEdge.top = rcEdge.bottom - m_cyEdge;
            break;
        case HTRIGHT:
            rcEdge.left = rcEdge.right - m_cxEdge;
            break;
        case HTTOP:
            rcEdge.bottom = rcEdge.top + m_cyEdge;
            break;
        default:
            return false;
    }

    return true;
}

void CSizableReBar::StartTracking( UINT nHitTest, CPoint pt )
{
    SetCapture();

    // Make sure no updates are pending
    VERIFY( RedrawWindow( 0, 0, RDW_ALLCHILDREN | RDW_UPDATENOW ) );

    CRect rcClient;
    GetClientRect( rcClient );

    m_htEdge    = nHitTest;
    m_szOld     = rcClient.Size();
    m_ptOld     = pt;
    m_bTracking = true;
}

void CSizableReBar::StopTracking()
{
    m_bTracking = false;
    ReleaseCapture();
}

void CSizableReBar::OnTrackUpdateSize( CPoint pt )
{
    ClientToScreen( &pt );

    CSize szDelta = pt - m_ptOld;
    CSize sizeNew = m_szOld;

    switch ( m_htEdge )
    {
        case HTLEFT:
            sizeNew.cx -= szDelta.cx;
            break;
        case HTTOP:
            sizeNew.cy -= szDelta.cy;
            break;
        case HTRIGHT:
            sizeNew.cx += szDelta.cx;
            break;
        case HTBOTTOM:
            sizeNew.cy += szDelta.cy;
            break;
    }

    if ( sizeNew != m_szOld )
    {
        m_szOld = sizeNew;
        m_ptOld = pt;

        // Try to re-size the re-bar control
        CRect rc( CPoint( 0, 0 ), sizeNew );
        m_rbCtrl.SizeToRect( rc );
    }
}


void CSizableReBar::GetRowHeights( std::vector<int>& vectHeights )
{
   std::map<int,int> mapTopHeight;

   for ( int nIndex = 0; nIndex <= m_aBars.GetUpperBound(); nIndex++ )
   {
      int nBand = m_rbCtrl.IDToIndex( m_aBars[ nIndex ].nID );

      // Get band rect.
      CRect rectBand;
      m_rbCtrl.GetRect( nBand, rectBand );

      // The stupidly-designed GetRowHeight() function takes
      // a BAND index as parameter, not ROW index.  M$ shoots, and 
      // bricks, again!
      mapTopHeight[ rectBand.top ] = m_rbCtrl.GetRowHeight( nBand );
   }

   // There should be no need to sort a map!  It is kept sorted by 
   // its predicate function...
   // std::sort( vectHeights.begin(), vectHeights.end() );

   // TraceMap( mapTopHeight );

   // Now pull out the results.
   std::map<int,int>::iterator it;
   for ( it = mapTopHeight.begin(); it != mapTopHeight.end(); it++ )
   {
      TRACE( _T("Top %d, Height %d"), it->first, it->second );
      vectHeights.push_back( it->second );
   }

   // TraceVector( vectHeights );

}