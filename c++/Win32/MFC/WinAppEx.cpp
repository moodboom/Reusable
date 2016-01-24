//-------------------------------------------------------------------//
// WinAppEx class
//-------------------------------------------------------------------//
// MDM	4/2/2004 3:10:58 PM
// This is a fairly lightweight class with support functions required 
// by IEToolbar.  Some of the functionality is duplication of stuff
// in BaseApp or other of my helper classes, but it's not worth
// integrating right now...
//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "WinAppEx.h"

#include <afxadv.h>     // CRecentFileList

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecentFileListEx

class CRecentFileListEx : public CRecentFileList
{
// Constructors
public:
    CRecentFileListEx( CRecentFileList* pRecentFileList );
    ~CRecentFileListEx();

// Overrides
public:
    virtual void UpdateMenu( CCmdUI* pCmdUI );
};

/////////////////////////////////////////////////////////////////////////////
// CWinAppEx

IMPLEMENT_DYNAMIC( CWinAppEx, CWinApp )

CWinAppEx::CWinAppEx( LPCTSTR lpszAppName /*=0*/ ) : CWinApp( lpszAppName )
{
    // Determine system type
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof( osvi );
    VERIFY( ::GetVersionEx( &osvi ) );

    switch ( osvi.dwPlatformId )
    {
        case VER_PLATFORM_WIN32_NT:
            if ( osvi.dwMajorVersion >= 5 )
            {
                if ( osvi.dwMinorVersion >= 1 )
                {
                    m_eOs = osWinXP;
                }
                else
                {
                    m_eOs = osWin2K;
                }
            }
            else
            {
                m_eOs = osWinNT;
            }
            break;

        case VER_PLATFORM_WIN32_WINDOWS:
            if ( ( osvi.dwMajorVersion > 4 ) ||
                 ( osvi.dwMajorVersion == 4 && osvi.dwMinorVersion > 0 ) )
            {
                m_eOs = osWin98;
            }
            else
            {
                m_eOs = osWin95;
            }
            break;

        default:
            m_eOs = osUnknown;
            break;
    }

    m_bBitmappedMenus = IsWin50();

    // Get system-wide parameters
    UpdateSystemParameters();
}

CWinAppEx::~CWinAppEx()
{
}

/////////////////////////////////////////////////////////////////////////////
// Operations

BOOL CWinAppEx::TrackPopupMenuEx( HMENU hMenu, UINT nFlags, int x, int y,
                                  CWnd* pWnd, LPTPMPARAMS lptpm /*=0*/ )
{
    _AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
    HWND  hWndOld  = pThreadState->m_hTrackingWindow;
    HMENU hMenuOld = pThreadState->m_hTrackingMenu;
    pThreadState->m_hTrackingWindow = pWnd->GetSafeHwnd();
    pThreadState->m_hTrackingMenu   = hMenu;

    BOOL bOK = ::TrackPopupMenuEx( hMenu, nFlags, x, y, pWnd->GetSafeHwnd(), lptpm );

    pThreadState->m_hTrackingWindow = hWndOld;
    pThreadState->m_hTrackingMenu   = hMenuOld;

    return bOK;
}

void CWinAppEx::LoadStdProfileSettings( UINT nMaxMRU /*=_AFX_MRU_COUNT*/ )
{
    CWinApp::LoadStdProfileSettings( nMaxMRU );

    CRecentFileList* pRecentFileList = m_pRecentFileList;
    m_pRecentFileList = new CRecentFileListEx( pRecentFileList );
    delete pRecentFileList;
}

void CWinAppEx::UpdateSystemParameters()
{
    m_info.cbSize = sizeof( NONCLIENTMETRICS );
    VERIFY( ::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof( m_info ), &m_info, 0 ) );

    if ( IsWin50() )
    {
        VERIFY( ::SystemParametersInfo( SPI_GETMENUANIMATION, 0, &m_bMenuAnimation, 0 ) );

        if ( IsWin2K() )
        {
            VERIFY( ::SystemParametersInfo( SPI_GETMENUUNDERLINES, 0, &m_bMenuUnderlines, 0 ) );
            VERIFY( ::SystemParametersInfo( SPI_GETUIEFFECTS, 0, &m_bUIEffects, 0 ) );
            VERIFY( ::SystemParametersInfo( SPI_GETMENUFADE, 0, &m_bMenuFade, 0 ) );

            if ( IsWinXP() )
            {
                VERIFY( ::SystemParametersInfo( SPI_GETFLATMENU, 0, &m_bFlatMenu, 0 ) );
            }
        }
    }
}

bool CWinAppEx::GetMenuAnimation() const
{
    ASSERT( IsWin50() );
    return ( ( !IsWin2K() || m_bUIEffects ) && m_bMenuAnimation );
}

bool CWinAppEx::GetMenuUnderlines() const
{
    ASSERT( IsWin2K() );
    return ( m_bUIEffects && m_bMenuUnderlines );
}

bool CWinAppEx::GetMenuFade() const
{
    ASSERT( IsWin2K() );
    return ( m_bUIEffects && m_bMenuFade );
}

bool CWinAppEx::GetFlatMenu() const
{
    ASSERT( IsWinXP() );
    return ( m_bUIEffects && m_bFlatMenu );
}

void CWinAppEx::SetBitmappedMenus( bool bBitmappedMenus )
{
    ASSERT( !bBitmappedMenus || IsWin50() );
    m_bBitmappedMenus = bBitmappedMenus;
}

bool CWinAppEx::GetBitmappedMenus() const
{
    return m_bBitmappedMenus;
}

void CWinAppEx::AddMenuIcon( CMapUIntToInt& mapIDToImage, UINT nBitmapID,
                             COLORREF clrMask /*=RGB(255,0,255)*/ )
{
    CBitmap bmp;
    VERIFY( bmp.LoadBitmap( nBitmapID ) );
    int nFirst = GetMenuImageList()->Add( &bmp, clrMask );
    ASSERT( nFirst != -1 );

    for ( POSITION pos = mapIDToImage.GetStartPosition(); pos != 0; )
    {
        UINT nID; int nImage;
        mapIDToImage.GetNextAssoc( pos, nID, nImage );
        m_mapIDToImage[ nID ] = nFirst + nImage;
    }
}

void CWinAppEx::AddMenuIcon( UINT nID, HICON hIcon )
{
    int nImage = GetMenuImageList()->Add( hIcon );
    ASSERT( nImage != -1 );
    m_mapIDToImage[ nID ] = nImage;
}

void CWinAppEx::AddMenuIcon( UINT nID, LPCTSTR lpszFileName )
{
    SHFILEINFO sfi;
    if ( ::SHGetFileInfo( lpszFileName, 0, &sfi, sizeof( sfi ),
            SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES ) )
    {
        int nImage = -1;
        if ( !m_mapFileIcons.Lookup( sfi.iIcon, nImage ) )
        {
            // Cache file icon
            nImage = GetMenuImageList()->Add( sfi.hIcon );
            ASSERT( nImage != -1 );
            m_mapFileIcons[ sfi.iIcon ] = nImage;
        }

        m_mapIDToImage[ nID ] = nImage;
        VERIFY( ::DestroyIcon( sfi.hIcon ) );
    }
    else
    {
        m_mapIDToImage.RemoveKey( nID );
    }
}

void CWinAppEx::ReplaceMenuIcon( UINT nID, HICON hIcon )
{
    int nImage = -1;
    if ( m_mapIDToImage.Lookup( nID, nImage ) )
    {
        nImage = GetMenuImageList()->Replace( nImage, hIcon );
        ASSERT( nImage != -1 );
        m_mapIDToImage[ nID ] = nImage;
    }
    else
    {
        AddMenuIcon( nID, hIcon );
    }
}

void CWinAppEx::RemoveMenuIcon( UINT nID )
{
    m_mapIDToImage.RemoveKey( nID );
}

void CWinAppEx::SetMenuIcons( CMenu* pMenu, bool bSubmenus /*=false*/ )
{
    if ( GetBitmappedMenus() && ( pMenu != 0 ) )
    {
        bool bBmp         = false;
        bool bBmpAndCheck = false;

        for ( UINT nIndex = 0, nItems = pMenu->GetMenuItemCount(); nIndex < nItems; nIndex++ )
        {
            UINT nID = pMenu->GetMenuItemID( nIndex );
            if ( nID != ( UINT )-1 )
            {
                if ( nID != 0 )
                {
                    MENUITEMINFO_WIN50 mii;
                    mii.cbSize = sizeof( mii );
                    mii.fMask  = MIIM_BITMAP;

                    int nImage = GetMenuImage( nID );
                    if ( nImage != -1 )
                    {
                        mii.hbmpItem = HBMMENU_CALLBACK;
                        VERIFY( ::SetMenuItemInfo( pMenu->m_hMenu, nIndex, TRUE, &mii ) );
                    }
                    else
                    {
                        VERIFY( ::GetMenuItemInfo( pMenu->m_hMenu, nIndex, TRUE, &mii ) );
                        if ( mii.hbmpItem == 0 )
                        {
                            continue;
                        }
                    }

                    bBmp         = true;
                    bBmpAndCheck = bBmpAndCheck ||
                        ( pMenu->GetMenuState( nID, MF_BYCOMMAND ) & MF_CHECKED );
                }
            }
            else if ( bSubmenus )
            {
                SetMenuIcons( pMenu->GetSubMenu( nIndex ) );
            }
        }

        MENUINFO_WIN50 mi;
        mi.cbSize  = sizeof( mi );
        mi.fMask   = MIM_STYLE;
        VERIFY( ::GetMenuInfoWin50( pMenu->m_hMenu, &mi ) );

        if ( bBmp && !bBmpAndCheck )
        {
            mi.dwStyle |= MNS_CHECKORBMP;
        }
        else
        {
            mi.dwStyle &= ~MNS_CHECKORBMP;
        }
        ::SetMenuInfoWin50( pMenu->m_hMenu, &mi );
    }
}

CImageList* CWinAppEx::GetMenuImageList()
{
    if ( m_imageList.m_hImageList == 0 )
    {
        VERIFY( m_imageList.Create( szMenuIcon.cx, szMenuIcon.cy,
            ILC_COLOR8 | ILC_MASK, 1, 0 ) );
    }

    return &m_imageList;
}

int CWinAppEx::GetMenuImage( UINT nID ) const
{
    int nImage = -1;
    return m_mapIDToImage.Lookup( nID, nImage ) ? nImage : -1;
}

/////////////////////////////////////////////////////////////////////////////
// CRecentFileListEx

CRecentFileListEx::CRecentFileListEx( CRecentFileList* pRecentFileList )
    : CRecentFileList(
        pRecentFileList->m_nStart,
        pRecentFileList->m_strSectionName,
        pRecentFileList->m_strEntryFormat,
        pRecentFileList->m_nSize,
        pRecentFileList->m_nMaxDisplayLength )
{
    for ( int nMRU = 0; nMRU < m_nSize; nMRU++ )
    {
        m_arrNames[ nMRU ] = pRecentFileList->m_arrNames[ nMRU ];
    }
}

CRecentFileListEx::~CRecentFileListEx()
{
}

/////////////////////////////////////////////////////////////////////////////
// Overrides

void CRecentFileListEx::UpdateMenu( CCmdUI* pCmdUI )
{
    UINT nID = pCmdUI->m_nID;   // ID_FILE_MRU_FIRST
    CRecentFileList::UpdateMenu( pCmdUI );

    CWinAppEx* pApp = CWinAppEx::GetInstance();
    for ( int nMRU = 0; ( nMRU < m_nSize ) && !m_arrNames[ nMRU ].IsEmpty(); nMRU++, nID++ )
    {
        pApp->AddMenuIcon( nID, m_arrNames[ nMRU ] );
    }
}
