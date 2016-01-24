//-------------------------------------------------------------------//
// CBmpMenuOwnerWnd class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#ifndef __BMPMENUOWNERWND_H__
#define __BMPMENUOWNERWND_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "..\..\WinAppEx.h"

/////////////////////////////////////////////////////////////////////////////
// CBmpMenuOwnerWnd

template< class TBase >
class CBmpMenuOwnerWnd : public TBase
{
// Constructors
public:
    CBmpMenuOwnerWnd();
    ~CBmpMenuOwnerWnd();

// Message map functions
protected:
    afx_msg void OnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu );
    afx_msg void OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct );
    afx_msg void OnMeasureItem( int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct );
};

/////////////////////////////////////////////////////////////////////////////
// CBmpMenuOwnerWnd out-of-line functions

template< class TBase >
CBmpMenuOwnerWnd< TBase >::CBmpMenuOwnerWnd()
{
}

template< class TBase >
CBmpMenuOwnerWnd< TBase >::~CBmpMenuOwnerWnd()
{
}

template< class TBase >
void CBmpMenuOwnerWnd< TBase >::OnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu )
{
    TBase::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );

    CWinAppEx::GetInstance()->SetMenuIcons( pPopupMenu );
}

template< class TBase >
void CBmpMenuOwnerWnd< TBase >::OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct )
{
    CWinAppEx* pApp = CWinAppEx::GetInstance();
    if ( pApp->GetBitmappedMenus() && ( lpDrawItemStruct->CtlType == ODT_MENU ) )
    {
        int nImage = pApp->GetMenuImage( lpDrawItemStruct->itemID );
        if ( nImage != -1 )
        {
            CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
            CRect rcItem( &lpDrawItemStruct->rcItem );
            CImageList* pImageList = pApp->GetMenuImageList();

            if ( lpDrawItemStruct->itemState & ODS_GRAYED )
            {
                // Create a color bitmap.
                CWindowDC windowDC( 0 );
                CDC colorDC;
                colorDC.CreateCompatibleDC( 0 );
                CBitmap colorBmp;
                colorBmp.CreateCompatibleBitmap( &windowDC, szMenuIcon.cx, szMenuIcon.cy );
                CBitmap* oldColorBmp = colorDC.SelectObject( &colorBmp );

                // Create a monochrome bitmap.
                CDC monoDC;
                monoDC.CreateCompatibleDC( 0 );
                CBitmap monoBmp;
                monoBmp.CreateCompatibleBitmap( &monoDC, szMenuIcon.cx, szMenuIcon.cy );
                CBitmap* pOldMonoBmp = monoDC.SelectObject( &monoBmp );
                COLORREF clrBtnHilite = ::GetSysColor( COLOR_BTNHIGHLIGHT );

                // Copy the toolbar button to the color bitmap, make all transparent
                // areas the same color as the button highlight color.
                pImageList->DrawIndirect( &colorDC, nImage, CPoint( 0, 0 ),
                    szMenuIcon, CPoint( 0, 0 ), ILD_NORMAL, SRCCOPY, clrBtnHilite );

                // Copy the color bitmap into the monochrome bitmap. Pixels that
                // have the button highlight color are mapped to the background
                colorDC.SetBkColor( clrBtnHilite );
                monoDC.BitBlt( 0, 0, szMenuIcon.cx, szMenuIcon.cy, &colorDC, 0, 0, SRCCOPY );

                // Draw the monochrome bitmap onto the menu.
                pDC->BitBlt( rcItem.left, rcItem.top + ( rcItem.Height() - szMenuIcon.cy ) / 2,
                    szMenuIcon.cx, szMenuIcon.cy, &monoDC, 0, 0, SRCCOPY );

                // Delete the color DC and bitmap.
                colorDC.SelectObject( oldColorBmp );
                colorDC.DeleteDC();
                colorBmp.DeleteObject();

                // Delete the monochrome DC and bitmap.
                monoDC.SelectObject( pOldMonoBmp );
                monoDC.DeleteDC();
                monoBmp.DeleteObject();
            }
            else
            {
                VERIFY( pImageList->Draw( pDC, nImage, CPoint( rcItem.left,
                    rcItem.top + ( rcItem.Height() - szMenuIcon.cy ) / 2 ), ILD_NORMAL ) );
            }
        }
    }
    else
    {
        TBase::OnDrawItem( nIDCtl, lpDrawItemStruct );
    }
}

template< class TBase >
void CBmpMenuOwnerWnd< TBase >::OnMeasureItem( int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
    CWinAppEx* pApp = CWinAppEx::GetInstance();
    if ( pApp->GetBitmappedMenus() && ( lpMeasureItemStruct->CtlType == ODT_MENU ) )
    {
//        const NONCLIENTMETRICS& info = pApp->GetNonClientMetrics();
//        lpMeasureItemStruct->itemWidth  = max( info.iMenuWidth,  szMenuIcon.cx + 4 );
//        lpMeasureItemStruct->itemHeight = max( info.iMenuHeight, szMenuIcon.cy );

        lpMeasureItemStruct->itemWidth  = szMenuIcon.cx + 4;
        lpMeasureItemStruct->itemHeight = szMenuIcon.cy;
    }
    else
    {
        TBase::OnMeasureItem( nIDCtl, lpMeasureItemStruct );
    }
}

/////////////////////////////////////////////////////////////////////////////
#endif  // !__BMPMENUOWNERWND_H__
