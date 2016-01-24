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

#ifndef __WINAPPEX_H__
#define __WINAPPEX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GlobalData.h" // CMapUIntToInt and CMapIntToInt

/////////////////////////////////////////////////////////////////////////////
// CWinAppEx

const CSize szMenuIcon( 16, 16 );

class /*GUILIB_EXT_CLASS*/ CWinAppEx : public CWinApp
{
    DECLARE_DYNAMIC( CWinAppEx );

// Enums
public:
    enum EOs
    {
        osWin95,
        osWin98,
        osWinNT,
        osWin2K,
        osWinXP,
        osUnknown,
    };

// Construction
public:
	CWinAppEx( LPCTSTR lpszAppName = 0 );
	~CWinAppEx();

// Operations
public:
    static CWinAppEx* GetInstance();
    static BOOL TrackPopupMenuEx( HMENU hMenu, UINT nFlags,
        int x, int y, CWnd* pWnd, LPTPMPARAMS lptpm = 0 );

// System-wide info
    void UpdateSystemParameters();

    EOs GetOs() const;

    bool IsWin50() const;
    bool IsWinNT() const;
    bool IsWin2K() const;
    bool IsWinXP() const;

    bool GetMenuAnimation() const;
    bool GetMenuUnderlines() const;
    bool GetMenuFade() const;
    bool GetFlatMenu() const;

    const NONCLIENTMETRICS& GetNonClientMetrics() const;

// Menu icon cache related stuff
    void LoadStdProfileSettings( UINT nMaxMRU = _AFX_MRU_COUNT );

    void SetBitmappedMenus( bool bBitmappedMenus );
    bool GetBitmappedMenus() const;

    void AddMenuIcon( CMapUIntToInt& mapIDToImage, UINT nBitmapID, COLORREF clrMask = RGB( 255, 0, 255 ) );
    void AddMenuIcon( UINT nID, HICON hIcon );
    void AddMenuIcon( UINT nID, LPCTSTR lpszFileName );
    void ReplaceMenuIcon( UINT nID, HICON hIcon );
    void RemoveMenuIcon( UINT nID );

    void SetMenuIcons( CMenu* pMenu, bool bSubmenus = false );

    CImageList* GetMenuImageList();
    int GetMenuImage( UINT nID ) const;

// Implementation data
protected:
    EOs                 m_eOs;
    NONCLIENTMETRICS    m_info;
    BOOL                m_bMenuAnimation;   // W98/W2K
    BOOL                m_bMenuUnderlines;  // W98/W2K
    BOOL                m_bMenuFade;        // W2K
    BOOL                m_bUIEffects;       // W2K
    BOOL                m_bFlatMenu;        // XP

    bool                m_bBitmappedMenus;
    CImageList          m_imageList;
    CMapUIntToInt       m_mapIDToImage;
    CMapIntToInt        m_mapFileIcons;
};

/////////////////////////////////////////////////////////////////////////////
// CWinAppEx inline functions

inline CWinAppEx* CWinAppEx::GetInstance()
    { return STATIC_DOWNCAST( CWinAppEx, AfxGetApp() ); }
inline CWinAppEx::EOs CWinAppEx::GetOs() const
    { return m_eOs; }
inline bool CWinAppEx::IsWin50() const
    { return ( m_eOs == osWin98 || m_eOs == osWin2K || m_eOs == osWinXP ); }
inline bool CWinAppEx::IsWinNT() const
    { return ( m_eOs == osWinNT || m_eOs == osWin2K || m_eOs == osWinXP ); }
inline bool CWinAppEx::IsWin2K() const
    { return ( m_eOs == osWin2K || m_eOs == osWinXP ); }
inline bool CWinAppEx::IsWinXP() const
    { return ( m_eOs == osWinXP ); }
inline const NONCLIENTMETRICS& CWinAppEx::GetNonClientMetrics() const
    { return m_info; }

/////////////////////////////////////////////////////////////////////////////
#endif  // !__WINAPPEX_H__
