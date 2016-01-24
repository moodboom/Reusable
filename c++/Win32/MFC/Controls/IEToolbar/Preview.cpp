//-------------------------------------------------------------------//
// CPreviewToolBar class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#include "stdafx.h"
// #include "Resource.h"
#include "Preview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewToolBar

IMPLEMENT_DYNCREATE( CPreviewToolBar, CToolBarEx )

/////////////////////////////////////////////////////////////////////////////
// Overrides

void CPreviewToolBar::Init()
{
    static TBBUTTONEX tbButtons[] =
    {
        { {  0,           AFX_ID_PREVIEW_PRINT,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }, true },
        { {  1,           AFX_ID_PREVIEW_PREV,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }, true },
        { {  2,           AFX_ID_PREVIEW_NEXT,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }, true },
        { {  4,           AFX_ID_PREVIEW_NUMPAGE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }, true },
        { {  5,           AFX_ID_PREVIEW_ZOOMIN,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }, true },
        { {  6,           AFX_ID_PREVIEW_ZOOMOUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }, true },
        { {  I_IMAGENONE, AFX_ID_PREVIEW_CLOSE,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }, true },
    };

    SetBitmaps( IDB_PREVIEWSMALL_COLD, IDB_PREVIEWSMALL_HOT, ( UINT )-1,
        ( UINT )-1, ( UINT )-1, ( UINT )-1, ioSmallIcons );
    SetButtons( sizeof( tbButtons ) / sizeof( tbButtons[ 0 ] ), tbButtons, toTextOnRight );
}

bool CPreviewToolBar::IsTextOptionAvailable( ETextOptions eTextOptions ) const
{
    switch ( eTextOptions )
    {
        case toTextOnRight:
            return true;
        default:
            return false;
    }
}

bool CPreviewToolBar::IsIconOptionAvailable( EIconOptions eIconOptions ) const
{
    switch ( eIconOptions )
    {
        case ioSmallIcons:
            return true;
        default:
            return false;
    }
}

bool CPreviewToolBar::HasButtonText( UINT nID )
{
    switch ( nID )
    {
        case AFX_ID_PREVIEW_PRINT:
        case AFX_ID_PREVIEW_CLOSE:
            return true;

        default:
            return false;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CPreviewViewEx

IMPLEMENT_DYNCREATE( CPreviewViewEx, CPreviewView )

CPreviewViewEx::CPreviewViewEx()
{
    m_pReBar          = 0;
    m_bTempReBar      = false;
    m_pPreviewToolBar = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Overrides

void CPreviewViewEx::OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView )
{
    CView::OnActivateView( bActivate, pActivateView, pDeactiveView );
}

/////////////////////////////////////////////////////////////////////////
// CPreviewViewEx message handlers

BEGIN_MESSAGE_MAP( CPreviewViewEx, CPreviewView )
    //{{AFX_MSG_MAP(CPreviewViewEx)
    ON_COMMAND( AFX_ID_PREVIEW_CLOSE, OnPreviewClose )
    ON_COMMAND( AFX_ID_PREVIEW_PRINT, OnPreviewPrint )
    ON_UPDATE_COMMAND_UI( AFX_ID_PREVIEW_NUMPAGE, OnUpdateNumPageChange )
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPreviewViewEx::OnPreviewClose()
{
    ASSERT( m_pReBar != 0 );
    ASSERT( m_pPreviewToolBar != 0 );

    m_pReBar->Lock( m_bLocked );
    m_pReBar->EnableContextMenu( true );

    CReBarCtrl& rbCtrl = m_pReBar->GetReBarCtrl();
    VERIFY( rbCtrl.DeleteBand( rbCtrl.IDToIndex( m_pPreviewToolBar->GetDlgCtrlID() ) ) );

    m_pPreviewToolBar->DestroyWindow();
    m_pPreviewToolBar = 0;

    if ( m_bTempReBar )
    {
        m_pReBar->DestroyWindow();
        m_pReBar = 0;
    }

    m_pPreviewInfo->m_nCurPage = m_nCurrentPage;
    ( ( CViewFriend* )m_pOrigView )->OnEndPrintPreview( m_pPreviewDC,
        m_pPreviewInfo, CPoint( 0, 0 ), this );
}

void CPreviewViewEx::OnPreviewPrint()
{
    OnPreviewClose();               // force close of Preview

    CWnd* pMainWnd = AfxGetThread()->m_pMainWnd;
    ASSERT_VALID( pMainWnd );
    pMainWnd->SendMessage( WM_COMMAND, ID_FILE_PRINT );
}

void CPreviewViewEx::OnUpdateNumPageChange( CCmdUI* pCmdUI )
{
    bool bZoomed = ( m_nZoomState == ZOOM_OUT );
    UINT nPages = bZoomed ? m_nPages : m_nZoomOutPages;

    // Update icon
    CPreviewToolBar* pToolBar = ( CPreviewToolBar* )pCmdUI->m_pOther;
    ASSERT( pToolBar != 0 );
    ASSERT_KINDOF( CPreviewToolBar, pToolBar );

    int nImage = ( nPages == 1 ) ? 4 : 3;
    if ( pToolBar->SendMessage( TB_GETBITMAP, ( WPARAM )pCmdUI->m_nID ) != nImage )
    {
        pToolBar->SendMessage( TB_CHANGEBITMAP, ( WPARAM )pCmdUI->m_nID,
            MAKELPARAM( nImage, 0 ) );
    }

    // Enable it only if valid to display another page and not zoomed
    pCmdUI->Enable( bZoomed && ( m_nMaxPages != 1 ) &&
        ( m_pPreviewInfo->GetMaxPage() > 1 || m_nPages > 1 ) );
}
