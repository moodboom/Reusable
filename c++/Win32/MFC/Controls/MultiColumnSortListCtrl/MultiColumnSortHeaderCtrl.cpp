//-------------------------------------------------------------------//
// MultiColumnSortHeaderCtrl.cpp : implementation file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "MultiColumnSortHeaderCtrl.h"
#include "MultiColumnSortListCtrl.h"			// We ask the parent to display the hiding/unhiding menu.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// MultiColumnSortHeaderCtrl


//-------------------------------------------------------------------//
// MultiColumnSortHeaderCtrl()													//
//-------------------------------------------------------------------//
MultiColumnSortHeaderCtrl::MultiColumnSortHeaderCtrl()
{
	m_bSortAsc = FALSE;
	m_nSortCol = -1;
	m_iLastColumn = -1;
	m_bmpArrows[0].LoadMappedBitmap( IDB_ARROW1UP		);
	m_bmpArrows[1].LoadMappedBitmap( IDB_ARROW2UP		);
	m_bmpArrows[2].LoadMappedBitmap( IDB_ARROW3UP		);
	m_bmpArrows[3].LoadMappedBitmap( IDB_ARROW1DOWN		);
	m_bmpArrows[4].LoadMappedBitmap( IDB_ARROW2DOWN		);
	m_bmpArrows[5].LoadMappedBitmap( IDB_ARROW3DOWN		);
}


//-------------------------------------------------------------------//
// ~MultiColumnSortHeaderCtrl()													//
//-------------------------------------------------------------------//
MultiColumnSortHeaderCtrl::~MultiColumnSortHeaderCtrl()
{
	
	for ( int i = 0; i < cnArrowCount; i++ )
	{
		m_bmpArrows[i].DeleteObject();
	}
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(MultiColumnSortHeaderCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(MultiColumnSortHeaderCtrl)
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MultiColumnSortHeaderCtrl message handlers
/////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------//
// SetSortImage()																		//
//-------------------------------------------------------------------//
int MultiColumnSortHeaderCtrl::SetSortImage( 
	int	nCol, 
	BOOL	bAsc,
	int	nSortRank
) {
   
	// Convert nSortRank to the arrow number index.
	nSortRank = min( nSortRank, cnArrowCount / 2 - 1 );
	if ( !bAsc ) nSortRank += ( cnArrowCount / 2 );
	
	CBitmap *pTempBmp = NULL;
	pTempBmp = &m_bmpArrows[ nSortRank ];
	
	HD_ITEM Item;	
	Item.mask = HDI_FORMAT;
	GetItem( nCol, &Item );
	Item.mask = HDI_BITMAP | HDI_FORMAT;	
	Item.fmt |= HDF_BITMAP;
	Item.hbm = (HBITMAP)pTempBmp->GetSafeHandle();
	SetItem( nCol, &Item );
	m_iLastColumn = nCol;
    return 1;
}


//-------------------------------------------------------------------//
// GetLastColumn()																	//
//-------------------------------------------------------------------//
const int MultiColumnSortHeaderCtrl::GetLastColumn() const
{
	return m_iLastColumn;
}


//-------------------------------------------------------------------//
// RemoveSortImage()																	//
//-------------------------------------------------------------------//
void MultiColumnSortHeaderCtrl::RemoveSortImage( int iItem )
{
	if( iItem != -1 )
	{
		HD_ITEM hditem;	
		hditem.mask = HDI_FORMAT;
		GetItem( iItem, &hditem );
		hditem.mask = HDI_BITMAP | HDI_FORMAT;	
		hditem.fmt &= ~HDF_BITMAP;
		SetItem( iItem, &hditem );
	}
}


//-------------------------------------------------------------------//
// RemoveAllSortImages()															//
//-------------------------------------------------------------------//
void MultiColumnSortHeaderCtrl::RemoveAllSortImages()
{
	int iCount = GetItemCount();
	for( int i = 0; i < iCount; i++ )
		RemoveSortImage( i );
}


//-------------------------------------------------------------------//
// OnRButtonUp()																		//
//-------------------------------------------------------------------//
// Here, we request a menu of columns, with checks, to allow
// hiding/unhiding.
//-------------------------------------------------------------------//
void MultiColumnSortHeaderCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CHeaderCtrl::OnRButtonUp(nFlags, point);

	MultiColumnSortListCtrl* pParentList = dynamic_cast<MultiColumnSortListCtrl*>( GetParent() );
	ASSERT( pParentList );
	if ( pParentList )
	{
		ClientToScreen( &point );
		pParentList->DisplayColumnHidingMenu( point );
	}

}


//-------------------------------------------------------------------//
// PreCreateWindow()																	//
//-------------------------------------------------------------------//
BOOL MultiColumnSortHeaderCtrl::PreCreateWindow( CREATESTRUCT& cs ) 
{
	return CHeaderCtrl::PreCreateWindow(cs);
}
