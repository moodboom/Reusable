//-------------------------------------------------------------------//
// CWindowListDialog class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "WindowListDialog.h"

#include <afxpriv.h>    // WM_KICKIDLE, ON_MESSAGE_VOID()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWindowListDialog dialog

CWindowListDialog::CWindowListDialog( CWnd* pMDIClientWnd, CWnd* pParent /*=0*/ )
    : inherited( CWindowListDialog::IDD, pParent )
{
    //{{AFX_DATA_INIT(CWindowListDialog)
    //}}AFX_DATA_INIT

    m_pMDIClientWnd = pMDIClientWnd;
}

void CWindowListDialog::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
    //{{AFX_DATA_MAP(CWindowListDialog)
    DDX_Control(pDX, IDC_BN_MINIMIZE, m_bnMinimize);
    DDX_Control(pDX, IDC_BN_TILE_VERT, m_bnTileVert);
    DDX_Control(pDX, IDC_BN_TILE_HORZ, m_bnTileHorz);
    DDX_Control(pDX, IDC_BN_CASCADE, m_bnCascade);
    DDX_Control(pDX, IDC_BN_CLOSE_WIN, m_bnCloseWin);
    DDX_Control(pDX, IDC_BN_ACTIVATE, m_bnActivate);
    DDX_Control(pDX, IDC_LST_WINDOWS, m_lstWindows);
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

void CWindowListDialog::ReloadWindowList()
{
    VERIFY( m_lstWindows.DeleteAllItems() );
    for ( int i = 0; i < m_imageList.GetImageCount(); i++ )
    {
        m_imageList.Remove( i );
    }

    HWND hWndActive = ( HWND )m_pMDIClientWnd->SendMessage( WM_MDIGETACTIVE );
    CWnd* pWnd;
    for ( int nIndex = 0; ( pWnd = m_pMDIClientWnd->GetDlgItem( AFX_IDM_FIRST_MDICHILD + nIndex ) ) != 0; nIndex++ )
    {
        if ( pWnd->IsKindOf( RUNTIME_CLASS( CMDIChildWnd ) ) )
        {
            HICON hIcon = ( HICON )::GetClassLong( pWnd->m_hWnd, GCL_HICONSM );
            int nImage = ( hIcon != 0 ) ? m_imageList.Add( hIcon ) : -1;

            CString strTitle;
            pWnd->GetWindowText( strTitle );

            int nItem = m_lstWindows.InsertItem( nIndex, strTitle, nImage );
            ASSERT( nItem != -1 );
            VERIFY( m_lstWindows.SetItemData( nItem, ( DWORD )pWnd->m_hWnd ) );

            if ( hWndActive == pWnd->m_hWnd )
            {
                VERIFY( m_lstWindows.SetItemState( nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED ) );
            }
        }
    }
}

void CWindowListDialog::Activate( int nItem )
{
    if ( nItem != -1 )
    {
        HWND hWnd = ( HWND )m_lstWindows.GetItemData( nItem );
        ASSERT( ::IsWindow( hWnd ) );

        m_pMDIClientWnd->SendMessage( ::IsIconic( hWnd ) ? WM_MDIRESTORE : WM_MDIACTIVATE,
            ( WPARAM )hWnd );

        PostMessage( WM_CLOSE );
    }
}

void CWindowListDialog::ArrangeWindows( UINT nMsg, WPARAM wParam /*=0*/, LPARAM lParam /*=0*/ )
{
    for ( int nItem = 0; nItem < m_lstWindows.GetItemCount(); nItem++ )
    {
        HWND hWnd = ( HWND )m_lstWindows.GetItemData( nItem );
        ASSERT( ::IsWindow( hWnd ) );

        if ( m_lstWindows.GetItemState( nItem, LVIS_SELECTED ) & LVIS_SELECTED )
        {
            if ( ::IsIconic( hWnd ) )
            {
                m_pMDIClientWnd->SendMessage( WM_MDIRESTORE, ( WPARAM )hWnd );
            }
        }
        else if ( !::IsIconic( hWnd ) )
        {
            VERIFY( ::ShowWindow( hWnd, SW_MINIMIZE ) );
        }
    }

    VERIFY( m_pMDIClientWnd->SendMessage( nMsg, wParam, lParam ) );
}

/////////////////////////////////////////////////////////////////////////////
// CWindowListDialog message handlers

BEGIN_MESSAGE_MAP(CWindowListDialog, CDialog)
    //{{AFX_MSG_MAP(CWindowListDialog)
    ON_BN_CLICKED(IDC_BN_ACTIVATE, OnActivate)
    ON_BN_CLICKED(IDC_BN_CLOSE_WIN, OnCloseWin)
    ON_BN_CLICKED(IDC_BN_CASCADE, OnCascade)
    ON_BN_CLICKED(IDC_BN_TILE_HORZ, OnTileHorz)
    ON_BN_CLICKED(IDC_BN_TILE_VERT, OnTileVert)
    ON_BN_CLICKED(IDC_BN_MINIMIZE, OnMinimize)
    ON_NOTIFY(NM_DBLCLK, IDC_LST_WINDOWS, OnDblClkWindows)
    //}}AFX_MSG_MAP
    ON_MESSAGE_VOID( WM_KICKIDLE, OnKickIdle )
END_MESSAGE_MAP()

BOOL CWindowListDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    VERIFY( m_imageList.Create( 16, 16, ILC_COLOR | ILC_MASK, 1, 0 ) );
    m_lstWindows.SetImageList( &m_imageList, LVSIL_SMALL );

    ReloadWindowList();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CWindowListDialog::OnActivate()
{
    int nItem = m_lstWindows.GetNextItem( -1, LVNI_SELECTED );
    ASSERT( nItem != -1 );

    Activate( nItem );
}

void CWindowListDialog::OnCloseWin()
{
    for ( POSITION pos = m_lstWindows.GetFirstSelectedItemPosition(); pos != 0; )
    {
        int nItem = m_lstWindows.GetNextSelectedItem( pos );
        HWND hWnd = ( HWND )m_lstWindows.GetItemData( nItem );
        ASSERT( ::IsWindow( hWnd ) );
        ::SendMessage( hWnd, WM_CLOSE, 0, 0 );
    }

    ReloadWindowList();
}

void CWindowListDialog::OnCascade()
{
    ArrangeWindows( WM_MDICASCADE );
}

void CWindowListDialog::OnTileHorz()
{
    ArrangeWindows( WM_MDITILE, ( WPARAM )MDITILE_HORIZONTAL );
}

void CWindowListDialog::OnTileVert()
{
    ArrangeWindows( WM_MDITILE, ( WPARAM )MDITILE_VERTICAL );
}

void CWindowListDialog::OnMinimize()
{
    for ( POSITION pos = m_lstWindows.GetFirstSelectedItemPosition(); pos != 0; )
    {
        int nItem = m_lstWindows.GetNextSelectedItem( pos );
        HWND hWnd = ( HWND )m_lstWindows.GetItemData( nItem );
        ASSERT( ::IsWindow( hWnd ) );
        VERIFY( ::ShowWindow( hWnd, SW_MINIMIZE ) );
    }
}

void CWindowListDialog::OnDblClkWindows( NMHDR* pNMHDR, LRESULT* pResult )
{
    NMITEMACTIVATE* lpnmitem = ( NMITEMACTIVATE* )pNMHDR;
    Activate( lpnmitem->iItem );

    *pResult = 0;
}

void CWindowListDialog::OnKickIdle()
{
    int nSelectedItems = ( int )m_lstWindows.GetSelectedCount();

    m_bnCascade.EnableWindow( nSelectedItems > 1 );
    m_bnTileVert.EnableWindow( nSelectedItems > 1 );
    m_bnTileHorz.EnableWindow( nSelectedItems > 1 );
    m_bnMinimize.EnableWindow( nSelectedItems > 0 );
    m_bnCloseWin.EnableWindow( nSelectedItems > 0 );
    m_bnActivate.EnableWindow( nSelectedItems == 1 );
}


void CWindowListDialog::OnSaveWin()
{
  for ( POSITION pos = m_lstWindows.GetFirstSelectedItemPosition(); pos != 0; )
  {
    int nItem = m_lstWindows.GetNextSelectedItem( pos );
    HWND hWnd = ( HWND )m_lstWindows.GetItemData( nItem );
    ASSERT( ::IsWindow( hWnd ) );
    ::SendMessage( hWnd, WM_COMMAND, ID_FILE_SAVE, 0 );
  }

  ReloadWindowList();
}