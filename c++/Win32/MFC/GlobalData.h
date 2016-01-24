//-------------------------------------------------------------------//
// GlobalData (common settings and constants)
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#ifndef __GLOBALDATA_H__
#define __GLOBALDATA_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DllLoader.h"

#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// Copied from the latest Platform SDK available.
// This should help if you don't have SDK installed on your PC.

// MDM	4/1/2004 2:21:13 PM
// And hurt us if we don't?  This SUCKS. 
// It has stuff we need embedded in it, pull it out, and
// kill the rest (falling back on the Platform SDK).

/////////////////////////////////////////////////////////////////////////////
// Custom ROPs

#define ROP_PSDPxax 0x00B8074A

/////////////////////////////////////////////////////////////////////////////
// Private messages

#define WM_MB_SHOWPOPUPMENU     ( WM_USER + 100 )
#define WM_TB_ENDMODALLOOP      ( WM_USER + 101 )
#define WM_REBAR_CHILDSIZE      ( WM_USER + 102 )
#define WM_REBAR_CHEVRONPUSHED  ( WM_USER + 103 )
#define WM_REBAR_CONTEXTMENU    ( WM_USER + 104 )

/////////////////////////////////////////////////////////////////////////////
// Custom maps

typedef CMap< UINT, UINT, int, int > CMapUIntToInt;
typedef CMap< int, int, int, int >   CMapIntToInt;

/////////////////////////////////////////////////////////////////////////////
// MDM	4/1/2004 2:40:03 PM
// Stuff that is kinda under Platform SDK for specifically targeted builds...
// IEToolbar wants them all so it can hack-fake it as needed.  Crazy shit.

#if ( WINVER < 0x0500 )

struct MENUINFO_WIN50
{
    DWORD   cbSize;
    DWORD   fMask;
    DWORD   dwStyle;
    UINT    cyMax;
    HBRUSH  hbrBack;
    DWORD   dwContextHelpID;
    ULONG   dwMenuData;
};

BOOL GetMenuInfoWin50( HMENU hmenu, const MENUINFO_WIN50* lpcmi );
BOOL SetMenuInfoWin50( HMENU hmenu, const MENUINFO_WIN50* lpcmi );
BOOL AnimateWindowWin50( HWND hwnd, DWORD dwTime, DWORD dwFlags );

#else

#define MENUINFO_WIN50      MENUINFO
#define MENUITEMINFO_WIN50  MENUITEMINFO

#define GetMenuInfoWin50    GetMenuInfo
#define SetMenuInfoWin50    SetMenuInfo
#define AnimateWindowWin50  AnimateWindow

#endif  // WINVER < 0x0500

#if ( WINVER < 0x0501 )

typedef HANDLE HTHEME;

#else

#pragma comment( lib, "UxTheme" )

#define IsThemeActiveXP         IsThemeActive
#define OpenThemeDataXP         OpenThemeData
#define CloseThemeDataXP        CloseThemeData
#define DrawThemeBackgroundXP   DrawThemeBackground
#define SetWindowThemeXP        SetWindowTheme

#endif  // WINVER < 0x0501

// MDM	4/1/2004 2:25:47 PM
// XP-specific bullshit

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED         0x031A
#endif  // !WM_THEMECHANGED

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW           0x00020000
#endif  // !CS_DROPSHADOW

#ifndef SPI_GETFLATMENU
#define SPI_GETFLATMENU         0x1022
#endif  // !SPI_GETFLATMENU

#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT       29
#define COLOR_MENUBAR           30
#endif  // !COLOR_MENUHILIGHT

// MDM	4/1/2004 2:32:58 PM
// Win2k-specific bullshit

#ifndef TPM_NOANIMATION
#define TPM_NOANIMATION         0x4000L
#endif  // !TPM_NOANIMATION

#ifndef DT_HIDEPREFIX
#define DT_HIDEPREFIX           0x00100000
#endif  // !DT_HIDEPREFIX




// MDM	4/1/2004 2:22:59 PM
// Here is the original code from CodeGuru class...
// Let the Platform SDK provide this stuff whenever possible...


/////////////////////////////////////////////////////////////////////////////
// COMCTL32 v5.80 specific:

/*
#ifndef RBBS_USECHEVRON
#define RBBS_USECHEVRON     0x00000200
#define RBHT_CHEVRON        0x0008
#define RB_PUSHCHEVRON      ( WM_USER + 43 )
#define RBN_CHEVRONPUSHED   ( RBN_FIRST - 10 )

typedef struct tagNMREBARCHEVRON
{
    NMHDR hdr;
    UINT uBand;
    UINT wID;
    LPARAM lParam;
    RECT rc;
    LPARAM lParamNM;
} NMREBARCHEVRON, *LPNMREBARCHEVRON;

#endif  // !RBBS_USECHEVRON

#ifndef BTNS_WHOLEDROPDOWN
#define BTNS_WHOLEDROPDOWN      0x0080
#endif  // !BTNS_WHOLEDROPDOWN

#ifndef TBN_INITCUSTOMIZE
#define TBN_INITCUSTOMIZE       ( TBN_FIRST - 23 )
#define TBNRF_HIDEHELP          0x00000001
#endif  // !TBN_INITCUSTOMIZE

#ifndef TBN_SAVE
#define TBN_SAVE                ( TBN_FIRST - 22 )
#define TBN_RESTORE             ( TBN_FIRST - 21 )

typedef struct tagNMTBSAVE
{
    NMHDR hdr;
    DWORD* pData;
    DWORD* pCurrent;
    UINT cbData;
    int iItem;
    int cButtons;
    TBBUTTON tbButton;
} NMTBSAVE, *LPNMTBSAVE;

typedef struct tagNMTBRESTORE
{
    NMHDR hdr;
    DWORD* pData;
    DWORD* pCurrent;
    UINT cbData;
    int iItem;
    int cButtons;
    int cbBytesPerRecord;
    TBBUTTON tbButton;
} NMTBRESTORE, *LPNMTBRESTORE;

#endif  // !TBN_SAVE

/////////////////////////////////////////////////////////////////////////////
// COMCTL32 v5.81 specific:

#ifndef TBSTYLE_EX_MIXEDBUTTONS
#define TBSTYLE_EX_MIXEDBUTTONS         0x00000008
#define TBSTYLE_EX_HIDECLIPPEDBUTTONS   0x00000010
#endif  // !TBSTYLE_EX_MIXEDBUTTONS

#ifndef BTNS_SHOWTEXT
#define BTNS_SHOWTEXT       0x0040
#endif  // !BTNS_SHOWTEXT

#ifndef I_IMAGENONE
#define I_IMAGENONE         (-2)
#endif  // !I_IMAGENONE

/////////////////////////////////////////////////////////////////////////////
// W98 specific:

#ifndef AW_SLIDE
#define AW_SLIDE                0x00040000
#define AW_HOR_POSITIVE         0x00000001
#define AW_HOR_NEGATIVE         0x00000002
#define AW_VER_POSITIVE         0x00000004
#define AW_VER_NEGATIVE         0x00000008
#endif  // !AW_SLIDE

#ifndef TPM_HORPOSANIMATION
#define TPM_HORPOSANIMATION     0x0400L
#define TPM_HORNEGANIMATION     0x0800L
#define TPM_VERPOSANIMATION     0x1000L
#define TPM_VERNEGANIMATION     0x2000L
#endif  // !TPM_HORPOSANIMATION

#ifndef SPI_GETMENUANIMATION
#define SPI_GETMENUANIMATION    0x1002
#define SPI_GETMENUUNDERLINES   0x100A
#endif  // !SPI_GETMENUANIMATION

#if ( WINVER < 0x0500 )

#define MIIM_STRING         0x00000040
#define MIIM_BITMAP         0x00000080
#define MIIM_FTYPE          0x00000100

#define HBMMENU_SYSTEM      ( ( HBITMAP )  1 )
#define HBMMENU_CALLBACK    ( ( HBITMAP ) -1 )

struct MENUITEMINFO_WIN50 : MENUITEMINFO
{
    HBITMAP  hbmpItem;
};

#define MIM_STYLE       0x00000010
#define MNS_NOCHECK     0x80000000
#define MNS_CHECKORBMP  0x04000000

struct MENUINFO_WIN50
{
    DWORD   cbSize;
    DWORD   fMask;
    DWORD   dwStyle;
    UINT    cyMax;
    HBRUSH  hbrBack;
    DWORD   dwContextHelpID;
    ULONG   dwMenuData;
};

BOOL GetMenuInfoWin50( HMENU hmenu, const MENUINFO_WIN50* lpcmi );
BOOL SetMenuInfoWin50( HMENU hmenu, const MENUINFO_WIN50* lpcmi );
BOOL AnimateWindowWin50( HWND hwnd, DWORD dwTime, DWORD dwFlags );

#else

#define MENUINFO_WIN50      MENUINFO
#define MENUITEMINFO_WIN50  MENUITEMINFO

#define GetMenuInfoWin50    GetMenuInfo
#define SetMenuInfoWin50    SetMenuInfo
#define AnimateWindowWin50  AnimateWindow

#endif  // WINVER < 0x0500

/////////////////////////////////////////////////////////////////////////////
// XP specific:

#if ( WINVER < 0x0501 )

typedef HANDLE HTHEME;

#else

#pragma comment( lib, "UxTheme" )

#define IsThemeActiveXP         IsThemeActive
#define OpenThemeDataXP         OpenThemeData
#define CloseThemeDataXP        CloseThemeData
#define DrawThemeBackgroundXP   DrawThemeBackground
#define SetWindowThemeXP        SetWindowTheme

#endif  // WINVER < 0x0501

/////////////////////////////////////////////////////////////////////////////
// W2K specific:

#ifndef DT_HIDEPREFIX
#define DT_HIDEPREFIX           0x00100000
#endif  // !DT_HIDEPREFIX

#ifndef AW_BLEND
#define AW_BLEND                0x00080000
#endif  // !AW_BLEND

#ifndef TPM_NOANIMATION
#define TPM_NOANIMATION         0x4000L
#endif  // !TPM_NOANIMATION

#ifndef COLOR_HOTLIGHT
#define COLOR_HOTLIGHT          26
#endif  // !COLOR_HOTLIGHT

#ifndef SPI_GETMENUFADE
#define SPI_GETMENUFADE         0x1012
#define SPI_GETUIEFFECTS        0x103E
#endif  // !SPI_GETMENUFADE

/////////////////////////////////////////////////////////////////////////////
// XP specific:

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED         0x031A
#endif  // !WM_THEMECHANGED

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW           0x00020000
#endif  // !CS_DROPSHADOW

#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT       29
#define COLOR_MENUBAR           30
#endif  // !COLOR_MENUHILIGHT

#ifndef SPI_GETFLATMENU
#define SPI_GETFLATMENU         0x1022
#endif  // !SPI_GETFLATMENU

/////////////////////////////////////////////////////////////////////////////
// Custom ROPs

#define ROP_PSDPxax 0x00B8074A

/////////////////////////////////////////////////////////////////////////////
// Private messages

#define WM_MB_SHOWPOPUPMENU     ( WM_USER + 100 )
#define WM_TB_ENDMODALLOOP      ( WM_USER + 101 )
#define WM_REBAR_CHILDSIZE      ( WM_USER + 102 )
#define WM_REBAR_CHEVRONPUSHED  ( WM_USER + 103 )
#define WM_REBAR_CONTEXTMENU    ( WM_USER + 104 )

/////////////////////////////////////////////////////////////////////////////
// Custom maps

typedef CMap< UINT, UINT, int, int > CMapUIntToInt;
typedef CMap< int, int, int, int >   CMapIntToInt;

*/



/////////////////////////////////////////////////////////////////////////////
// MDM
// These crazy functions emulate what is available on newer OS'es.
/////////////////////////////////////////////////////////////////////////////
// Inline functions

#if ( WINVER < 0x0500 )
inline BOOL GetMenuInfoWin50( HMENU hmenu, const MENUINFO_WIN50* lpcmi )
{
    CDllLoader dllLoader( _T("User32.dll") );

    typedef BOOL ( WINAPI* GETMENUINFO )( HMENU hmenu, const MENUINFO_WIN50* lpcmi );
    GETMENUINFO pfGetMenuInfo = ( GETMENUINFO )dllLoader.GetProcAddress( "GetMenuInfo" );
    ASSERT( pfGetMenuInfo != 0 );

    return pfGetMenuInfo( hmenu, lpcmi );
}

inline BOOL SetMenuInfoWin50( HMENU hmenu, const MENUINFO_WIN50* lpcmi )
{
    CDllLoader dllLoader( _T("User32.dll") );

    typedef BOOL ( WINAPI* SETMENUINFO )( HMENU hmenu, const MENUINFO_WIN50* lpcmi );
    SETMENUINFO pfSetMenuInfo = ( SETMENUINFO )dllLoader.GetProcAddress( "SetMenuInfo" );
    ASSERT( pfSetMenuInfo != 0 );

    return pfSetMenuInfo( hmenu, lpcmi );
}

inline BOOL AnimateWindowWin50( HWND hwnd, DWORD dwTime, DWORD dwFlags )
{
    CDllLoader dllLoader( _T("User32.dll") );

    typedef BOOL ( WINAPI* ANIMATEWINDOW )( HWND hwnd, DWORD dwTime, DWORD dwFlags );
    ANIMATEWINDOW pfAnimateWindow = ( ANIMATEWINDOW )dllLoader.GetProcAddress( "AnimateWindow" );
    ASSERT( pfAnimateWindow != 0 );

    return pfAnimateWindow( hwnd, dwTime, dwFlags );
}
#endif  // WINVER < 0x0500

#if ( WINVER < 0x0501 )
inline BOOL IsThemeActiveXP()
{
    CDllLoader dllLoader( _T("UxTheme.dll") );

    typedef BOOL ( STDAPICALLTYPE* ISTHEMEACTIVE )();
    ISTHEMEACTIVE pfIsThemeActive = ( ISTHEMEACTIVE )dllLoader.GetProcAddress( "IsThemeActive" );
    ASSERT( pfIsThemeActive != 0 );

    return pfIsThemeActive();
}

inline HTHEME OpenThemeDataXP( HWND hwnd, LPCWSTR pszClassList )
{
    CDllLoader dllLoader( _T("UxTheme.dll") );

    typedef HTHEME ( STDAPICALLTYPE* OPENTHEMEDATA )( HWND hwnd, LPCWSTR pszClassList );
    OPENTHEMEDATA pfOpenThemeData = ( OPENTHEMEDATA )dllLoader.GetProcAddress( "OpenThemeData" );
    ASSERT( pfOpenThemeData != 0 );

    return pfOpenThemeData( hwnd, pszClassList );
}

inline HRESULT CloseThemeDataXP( HTHEME hTheme )
{
    CDllLoader dllLoader( _T("UxTheme.dll") );

    typedef HRESULT ( STDAPICALLTYPE* CLOSETHEMEDATA )( HTHEME hTheme );
    CLOSETHEMEDATA pfCloseThemeData = ( CLOSETHEMEDATA )dllLoader.GetProcAddress( "CloseThemeData" );
    ASSERT( pfCloseThemeData != 0 );

    return pfCloseThemeData( hTheme );
}

inline HRESULT DrawThemeBackgroundXP( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect )
{
    CDllLoader dllLoader( _T("UxTheme.dll") );

    typedef HRESULT ( STDAPICALLTYPE* DRAWTHEMEBACKGROUND )( HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect );
    DRAWTHEMEBACKGROUND pfDrawThemeBackground = ( DRAWTHEMEBACKGROUND )dllLoader.GetProcAddress( "DrawThemeBackground" );
    ASSERT( pfDrawThemeBackground != 0 );

    return pfDrawThemeBackground( hTheme, hdc, iPartId, iStateId, pRect, pClipRect );
}

inline HRESULT SetWindowThemeXP( HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList )
{
    CDllLoader dllLoader( _T("UxTheme.dll") );

    typedef HRESULT ( STDAPICALLTYPE* SETWINDOWTHEME )( HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList );
    SETWINDOWTHEME pfSetWindowTheme = ( SETWINDOWTHEME )dllLoader.GetProcAddress( "SetWindowTheme" );
    ASSERT( pfSetWindowTheme != 0 );

    return pfSetWindowTheme( hwnd, pszSubAppName, pszSubIdList );
}

#endif  // WINVER < 0x0501

/////////////////////////////////////////////////////////////////////////////
#endif  // !__GLOBALDATA_H__
