/////////////////////////////////////////////////////////////////////////////
// BlockCtrl


#include "stdafx.h"

#include <algorithm>											// For _MIN(), _MAX(), must come AFTER stdafx.h to avoid conflicting definitions.
using namespace std;											// WARNING: Make sure your types match exactly or you will likely get weird errors from predicate version.

#include <RandomHelpers.h>									// For stuffing random debug info.

#include "BlockCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////
// Globals, constants, statics
///////////////////////////////////////////////////////
//

const int cnXBorder = 10;
const int cnYBorder = 10;

const int cnBlockWidth = 17;
const int cnBlockHeight = 22;

const COLORREF BlockColorMap[] =
{	
	RGB(   0,   0,   0 ),	// BC_BLACK
	RGB( 255, 255, 255 ),	// BC_WHITE
	RGB( 100,   0, 100 ),	// BC_PURPLE
	RGB( 255, 255,  50 ),	// BC_YELLOW
	RGB( 200,   0,   0 ),	// BC_RED
	RGB( 235, 128,  25 ),	// BC_ORANGE
	RGB(  50, 180, 180 ),	// BC_TEAL
	RGB( 200, 200, 255 ),	// BC_LIGHT_BLUE
	RGB(   0,   0, 200 ),	// BC_DARK_BLUE
	RGB( 128, 128, 128 ),	// BC_GRAY
	RGB(  64,  64,  64 ),	// BC_DARK_GRAY
	RGB(  50, 200,  50 ),	// BC_GREEN
	RGB(   0, 150,   0 ),	// BC_DARK_GREEN
};

// const COLORREF cRGBHighlight = RGB( 150, 150, 175 );	// Blue
// const COLORREF cRGBHighlight = RGB( 241,  22,  45 );	// Red
const COLORREF cRGBHighlight	= RGB( 245, 225,  86 );	// Yellow

const COLORREF cRGBSelected	= RGB(   0,   0,   0 );	// Black

//
///////////////////////////////////////////////////////


//-------------------------------------------------------------------//
// BlockCtrl()																			//
//-------------------------------------------------------------------//
BlockCtrl::BlockCtrl(
	int	nBlockCount
) :
	
	// Init vars.
	m_nSelectedBlock			( 0				),
	m_nFirstDisplayedBlock	( 0				)		// used to control scrolling
	
{
	// Allocate the block data array space.
	btvBlockData.reserve	( nBlockCount	);
	btvBlockData.resize	( nBlockCount	);

	// This never changes, why keep restuffing it?
	m_si.cbSize = sizeof SCROLLINFO;
	m_si.fMask = SIF_ALL;
	m_si.nMin = 0;

}


//-------------------------------------------------------------------//
// ~BlockCtrl()																		//
//-------------------------------------------------------------------//
BlockCtrl::~BlockCtrl()
{
}


//-------------------------------------------------------------------//
// Resize()																				//
//-------------------------------------------------------------------//
// Typically call this function before initializing the block ctrl
// data and displaying it.
//
// See STL vector documentation for the result of calling this on an 
// existing block ctrl.
//-------------------------------------------------------------------//
void BlockCtrl::Resize( int nNewBlockCount )
{
	// Allocate the block data array space.
	btvBlockData.reserve	( nNewBlockCount	);
	btvBlockData.resize	( nNewBlockCount	);
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// Similar to the tree control situation, we were not able to find a 
// built-in init function, so we were forced to add one of our own, 
// which must be called manually by the object containing this one,
// AFTER the control is created.
//
// Here, we create our scrollbar child.
//
// We also set up the pointer to the window that we will notify
// when a new block is selected.  You CAN pass 0 if you don't want 
// notification, but then why are you using this class?  It's not
// THAT pretty.  Trap WM_USER_BLOCK_SELECTED.
//-------------------------------------------------------------------//
void BlockCtrl::Initialize(
	CWnd* pSelectNotifyWnd
) {

	// Who you gonna call when they come fo' ya?
	m_pSelectNotifyWnd = pSelectNotifyWnd;
	
	CRect rect;
	GetClientRect( &rect );

	m_BlockScrollbar.Create(
		WS_CHILD | WS_VISIBLE | SBS_RIGHTALIGN | SBS_VERT,
		rect,
		this,
		
		// WARNING: Make sure you left room for a control ID
		// value at this number!  We just add 900 to minimize
		// collisions in 99% of situations, but it is no 
		// guarantee.  Sorry!
		GetDlgCtrlID() + 900

	);

}


//-------------------------------------------------------------------//
// GetMinWidth()																		//
//-------------------------------------------------------------------//
// Return this for the parent's sake.  We won't repaint ourselves
// once we are less than a block wide.
//-------------------------------------------------------------------//
DWORD BlockCtrl::GetMinWidth()
{
	return 
			cnBlockWidth 
		+	GetSystemMetrics( SM_CXVSCROLL ) 
		+	cnXBorder * 2
		+	4;												// 3D edge
}


//-------------------------------------------------------------------//
// SetAllBlockData()																	//
//-------------------------------------------------------------------//
// Stuff the given block type into all blocks.
//-------------------------------------------------------------------//
void BlockCtrl::SetAllBlockData( BlockType& btDefault )
{
	for ( int nA = 0; nA < GetBlockCount(); nA++ )
		SetBlockType( nA, btDefault );
}


//-------------------------------------------------------------------//
// RandomizeBlockData()																//
//-------------------------------------------------------------------//
// Randomly change a handful of blocks (~20%) to the given color.
//-------------------------------------------------------------------//
void BlockCtrl::RandomizeBlockData( int nColor )
{
	for ( int nA = 0; nA < GetBlockCount(); nA++ )
	{
		BlockType btTemp = GetBlockType( nA );

		if ( usRandom( 4 ) < 1 )
		{
			btTemp.m_bcColor = (BLOCK_COLOR)nColor;
			SetBlockType( nA, btTemp );
		}

	}

}


//-------------------------------------------------------------------//
// RandomHighlightBlockData()														//
//-------------------------------------------------------------------//
// Randomly change a handful of blocks (~20%) to highlighted.
//-------------------------------------------------------------------//
void BlockCtrl::RandomHighlightBlockData()
{
	for ( int nA = 0; nA < GetBlockCount(); nA++ )
	{
		if ( usRandom( 4 ) < 1 )
		{
			SetBlockHighlight( nA, (BLOCK_COLOR)ulRandom( BLOCK_COLOR_COUNT ) );
		}

	}

}


//-------------------------------------------------------------------//
// GetScreenMeasurements()															//
//-------------------------------------------------------------------//
// Consolidated measurements calcs.
//-------------------------------------------------------------------//
inline void BlockCtrl::GetScreenMeasurements( BCScreenMeasurements& bcsm )
{
	GetClientRect( &bcsm.rectClient );

	// Calc the block control and scrollbar rect's.
	bcsm.rectSB = bcsm.rectClient;
	bcsm.rectSB.DeflateRect(
		cnXBorder + 2, 
		cnYBorder + 2
	);
	bcsm.rectBC = bcsm.rectSB;
	bcsm.rectSB.left = bcsm.rectSB.right - GetSystemMetrics( SM_CXVSCROLL );
	bcsm.rectBC.DeflateRect(
		0,
		0,
		GetSystemMetrics( SM_CXVSCROLL ),
		0
	);

	// Get the rows and cols, now that we have our final target area.
	bcsm.nRowDrawCount = bcsm.rectBC.Height() / cnBlockHeight + 1;
	bcsm.nColDrawCount = bcsm.rectBC.Width()  / cnBlockWidth;
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(BlockCtrl, CStatic)
	//{{AFX_MSG_MAP(BlockCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BlockCtrl message handlers


//-------------------------------------------------------------------//
// OnEraseBkgnd()																		//
//-------------------------------------------------------------------//
BOOL BlockCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;	// do not erase background
}


//-------------------------------------------------------------------//
// OnPaint()																			//
//-------------------------------------------------------------------//
// Fire up the paint function.
//-------------------------------------------------------------------//
void BlockCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// You called Initialize() in your dialog, right? :>
	_ASSERT( m_BlockScrollbar.GetSafeHwnd() );
	
	DrawBlocks( &dc );

}


//-------------------------------------------------------------------//
// DrawBlocks()																		//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void BlockCtrl::DrawBlocks( CDC *pDCa )
{
	// Get DC if no DC is passed
	CDC *pDC = pDCa;
	if(pDCa == NULL) pDC = GetDC();

	// Get the current screen measurements.
	BCScreenMeasurements bcsm;
	GetScreenMeasurements( bcsm );	

	// Create temporary work DC (memory DC) and bitmap
	CDC dcMem;
	CBitmap bmTemp;
	dcMem.CreateCompatibleDC(pDC);
	bmTemp.CreateCompatibleBitmap( 
		pDC,
		bcsm.rectClient.Width() + 2,
		bcsm.rectClient.Height() + 2
	);
	CBitmap *pbmOld = dcMem.SelectObject(&bmTemp);

	// Draw edge border, then exclude it.
	CBrush WorkSpaceBrush( GetSysColor( COLOR_BTNFACE ) );
	CBrush *pOldBrush = dcMem.SelectObject( &WorkSpaceBrush );
	CPen *pOldPen = (CPen *)dcMem.SelectStockObject( NULL_PEN );
	
	dcMem.Rectangle(&bcsm.rectClient);
	
	dcMem.Draw3dRect(
		&bcsm.rectClient,
		(COLORREF)GetSysColor(COLOR_3DSHADOW),
		(COLORREF)GetSysColor(COLOR_3DHILIGHT)
	);
	bcsm.rectClient.DeflateRect( 1, 1 );
	
	dcMem.Draw3dRect(
		&bcsm.rectClient,
		(COLORREF)GetSysColor(COLOR_3DDKSHADOW),
		(COLORREF)GetSysColor(COLOR_3DLIGHT)
	);

	dcMem.SelectStockObject( BLACK_PEN);

	// Draw the rows all the way to the clipping area.
	if ( bcsm.nRowDrawCount <= 0 || bcsm.nColDrawCount <= 0 )
		return;

	// Set up the scrollbar info.
	
	// If we can fit it all...
	if ( 
			m_nFirstDisplayedBlock == 0
		&&	bcsm.nRowDrawCount * bcsm.nColDrawCount >= GetBlockCount()
	) {
		m_BlockScrollbar.EnableWindow( FALSE );
	
	// Otherwise, set it up.
	} else
	{
		m_BlockScrollbar.EnableWindow( TRUE );
		m_si.nMax = GetBlockCount() / bcsm.nColDrawCount + 1;
		m_si.nPos = m_nFirstDisplayedBlock / bcsm.nColDrawCount;
		m_si.nPage = bcsm.nRowDrawCount;
		m_BlockScrollbar.SetScrollInfo( &m_si, FALSE );
	}

	// Position the scrollbar.
	m_BlockScrollbar.MoveWindow( bcsm.rectSB );

	//
	///////////////////////////////


	// Create clipping region.
	// This prevents us from drawing over the bottom and
	// right edges with minimal effort.
	CRgn rgnRect;
	rgnRect.CreateRectRgn( 
		bcsm.rectBC.left,
		bcsm.rectBC.top,
		bcsm.rectBC.right,
		bcsm.rectBC.bottom + cnXBorder - 2
	);
	dcMem.SelectClipRgn( &rgnRect );

	// Draw the blocks.

	// Create all the pens and brushes we will need.
	// Note that these will clean up after themselves when
	// they go out of scope.
	CPen penHighlighted		( PS_SOLID		, 1, cRGBHighlight	);
	CPen penSelected			( PS_DOT			, 1, cRGBSelected		);	// Options: PS_DASH, PS_DOT, PS_DASHDOT, PS_DASHDOTDOT, PS_ALTERNATE
   
	// Did you define the BlockColorMap to match the BLOCK_COLORS enums?
	ASSERT( BLOCK_COLOR_COUNT == sizeof BlockColorMap / sizeof BlockColorMap[0] );

	vector<CBrush*> brushes;
	int nA;
	for ( nA = 0; nA < BLOCK_COLOR_COUNT; nA++ )
		brushes.push_back( new CBrush( BlockColorMap[ nA ] ) );

	// When the block control size is changed, the first block may no longer 
	// be a col-multiple.  Force it to be, so when the user scrolls back up,
	// they do not have a strange first line.
	if ( m_nFirstDisplayedBlock % bcsm.nColDrawCount != 0 )
		m_nFirstDisplayedBlock = ( m_nFirstDisplayedBlock / bcsm.nColDrawCount ) * bcsm.nColDrawCount;

	int nCurrentBlock = m_nFirstDisplayedBlock;
	BlockType btTemp;

	for ( nA = 0; nA < bcsm.nRowDrawCount && nCurrentBlock < GetBlockCount(); nA++ )
	{
		for ( int nB = 0; nB < bcsm.nColDrawCount && nCurrentBlock < GetBlockCount(); nB++ )
		{
			BlockType& btTemp = btvBlockData.at( nCurrentBlock );

			// Draw the block!

			// Reset to the background brush before we get started.
			dcMem.SelectObject( &WorkSpaceBrush );

			CRect rectBlock( 
				CPoint(
					bcsm.rectBC.left + nB * cnBlockWidth,
					bcsm.rectBC.top  + nA * cnBlockHeight
				),
				CSize(
					cnBlockWidth,
					cnBlockHeight
				)
			);

			// Already cleared the whole area.
			/*
			// Clear it first.
			dcMem.SelectObject( &WorkSpaceBrush );
			dcMem.SelectStockObject( NULL_PEN );
			dcMem.Rectangle( rectBlock );
			*/
			
			// Selected.
			if ( nCurrentBlock == m_nSelectedBlock )
			{
				dcMem.SelectObject( &penSelected );
				dcMem.Rectangle( rectBlock );
			}

			// Highlight.
			if ( btTemp.m_bcHighlight != BLOCK_COLOR_COUNT )
			{
				// TO DO
				// Rectangle() writes with current pen and brush
				// we may need to expand rect here to match previous results.
				// dcMem.SelectObject( &penHighlighted );
				
				
				// DEBUG
				rectBlock.InflateRect( 5, 5, 5, 5 );


				dcMem.SelectStockObject( NULL_PEN );
				dcMem.SelectObject( brushes[ btTemp.m_bcHighlight ] );
				
				// Don't overwrite the selected frame.
				if ( nCurrentBlock != m_nSelectedBlock )
					dcMem.Rectangle( rectBlock );
				

				// DEBUG
				rectBlock.DeflateRect( 5, 5, 5, 5 );


				rectBlock.DeflateRect( 1, 1, 1, 1 );
				dcMem.Rectangle( rectBlock );
				rectBlock.DeflateRect( 1, 1, 1, 1 );
			} else
				rectBlock.DeflateRect( 2, 2, 2, 2 );
			
			// Fill the rect.
			dcMem.SelectObject( brushes[ btTemp.m_bcColor ] );
			dcMem.SelectStockObject( NULL_PEN );
			dcMem.Rectangle( rectBlock );

			// Next, the partial fill.
			if ( 
					btTemp.m_bcPartialFill != BLOCK_COLOR_COUNT 
				&& btTemp.m_ubPercentFill > 0
			) {
				CRect rectPartial( rectBlock );
				rectPartial.top -= rectPartial.Height() * _MIN( 100 - btTemp.m_ubPercentFill, 0 ) / 100;
				dcMem.SelectObject( brushes[ btTemp.m_bcPartialFill ] );
				dcMem.SelectStockObject( NULL_PEN );
				dcMem.Rectangle( rectPartial );
			}
			
		   // Next, the 3D border.
		   dcMem.Draw3dRect(
				rectBlock, 
				
				// Top left    
				( nCurrentBlock == m_nSelectedBlock )
					?	(COLORREF)GetSysColor( COLOR_3DSHADOW	)
					:	(COLORREF)GetSysColor( COLOR_3DHILIGHT	),

				// Bottom right
				( nCurrentBlock == m_nSelectedBlock )
					?	(COLORREF)GetSysColor( COLOR_3DHILIGHT	)
					:	(COLORREF)GetSysColor( COLOR_3DSHADOW	)		
			);

			// Draw the 'X' as requested.
			if ( btTemp.m_bCrossedOut )
			{
				dcMem.SelectStockObject( BLACK_PEN);
				dcMem.MoveTo( rectBlock.left	+ 1, rectBlock.top		+ 1 );
				dcMem.LineTo( rectBlock.right - 1, rectBlock.bottom	- 1 );
				dcMem.MoveTo( rectBlock.right - 2, rectBlock.top		+ ( ( nCurrentBlock == m_nSelectedBlock ) ? 1 : 2 ) );
				dcMem.LineTo( rectBlock.left	+ ( ( nCurrentBlock == m_nSelectedBlock ) ? 0 : 1 ), rectBlock.bottom	- 1 );
			}

			// Maybe not the most efficient way, but pretty cheap...
			nCurrentBlock++;

		}

	}
	
	// Finally draw to the screen from dcMem.
	pDC->BitBlt(
		0,
		0,
		bcsm.rectClient.Width() + 2,
		bcsm.rectClient.Height() + 2,
		&dcMem,
		0,
		0,
		SRCCOPY
	);

	// Cleanup.
	dcMem.SelectObject(pOldBrush);
	WorkSpaceBrush.DeleteObject();
	dcMem.SelectObject(pbmOld);
	bmTemp.DeleteObject();
	dcMem.SelectObject(pOldPen);

	if ( pDCa == NULL ) 
		ReleaseDC( pDC );

	for ( nA = 0; nA < BLOCK_COLOR_COUNT; nA++ )
		delete brushes[nA];

}


//-------------------------------------------------------------------//
// GetBlockNumber()																	//
//-------------------------------------------------------------------//
// Get a block number for the clicked point.
//-------------------------------------------------------------------//
int BlockCtrl::GetBlockNumber( CPoint& point )
{
	// point is in client coords, we are good to go...

	BCScreenMeasurements bcsm;
	GetScreenMeasurements( bcsm );	

	int nRow = ( point.y - bcsm.rectBC.top	) / cnBlockHeight;
	int nCol = ( point.x - bcsm.rectBC.left ) / cnBlockWidth ;

	return m_nFirstDisplayedBlock + nCol + nRow * bcsm.nColDrawCount;
}


//-------------------------------------------------------------------//
// OnVScroll()																			//
//-------------------------------------------------------------------//
// Handle clicks on our scrollbar here.
//-------------------------------------------------------------------//
void BlockCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// DEBUG
	// Force a breakpoint.
	// _ASSERT( false );	
	
	SCROLLINFO ScrollInfo;
	pScrollBar->GetScrollInfo( &ScrollInfo );  // get information about the scroll

	BCScreenMeasurements bcsm;
	GetScreenMeasurements( bcsm );
	
	// DEBUG
	// Force a breakpoint.
	// _ASSERT( false );

	int nTotalRowCount = ( GetBlockCount() + bcsm.nColDrawCount - 1 ) / bcsm.nColDrawCount;

	// int nLastFirstDisplayedBlock = ( nTotalRowCount - bcsm.nRowDrawCount ) * bcsm.nColDrawCount;
	int nLastFirstDisplayedBlock = ( nTotalRowCount - bcsm.nRowDrawCount + 1 ) * bcsm.nColDrawCount;

	switch( nSBCode )
	{
		case SB_BOTTOM:         //Scrolls to the lower right. 
			break;

		case SB_ENDSCROLL:      //Ends scroll. 
			break;

		case SB_LINEDOWN:       //Scrolls one line down. 
			if ( m_nFirstDisplayedBlock < nLastFirstDisplayedBlock )
			{
				pScrollBar->SetScrollPos(
						pScrollBar->GetScrollPos()
					+	1
				);
				m_nFirstDisplayedBlock = _MIN(
					nLastFirstDisplayedBlock,
					m_nFirstDisplayedBlock + bcsm.nColDrawCount
				);
				Invalidate();
			}
			break;

		case SB_PAGEDOWN:       //Scrolls one page down. 
			if ( m_nFirstDisplayedBlock < nLastFirstDisplayedBlock )
			{
				pScrollBar->SetScrollPos(
					_MIN(
						ScrollInfo.nMax,
						(int)( pScrollBar->GetScrollPos() + ScrollInfo.nPage )
					)
				);
				m_nFirstDisplayedBlock = _MIN( 
					nLastFirstDisplayedBlock, 
					m_nFirstDisplayedBlock + bcsm.nColDrawCount * bcsm.nRowDrawCount 
				);
				Invalidate();
			}
			break;

		case SB_LINEUP:         //Scrolls one line up. 
			if ( m_nFirstDisplayedBlock > 0 )
			{
				pScrollBar->SetScrollPos(
						pScrollBar->GetScrollPos() 
					-	1
				);
				m_nFirstDisplayedBlock -= bcsm.nColDrawCount;
				
				// We should always hit zero dead-on.
				ASSERT( m_nFirstDisplayedBlock >= 0 );
				
				Invalidate();
			}
			break;

		case SB_PAGEUP:         //Scrolls one page up. 
			if ( m_nFirstDisplayedBlock > 0 )
			{
				pScrollBar->SetScrollPos(
						pScrollBar->GetScrollPos() 
					-	ScrollInfo.nPage
				);
				m_nFirstDisplayedBlock = _MAX( 
					0, 
					m_nFirstDisplayedBlock - bcsm.nColDrawCount * bcsm.nRowDrawCount 
				);

				Invalidate();
			}
			break;

		// The user has dragged the scroll box (thumb) and released the mouse button. 
		// The nPos parameter indicates the position of the scroll box at the end of 
		// the drag operation.
		case SB_THUMBPOSITION:  
			m_nFirstDisplayedBlock = nPos * bcsm.nColDrawCount;
			m_nFirstDisplayedBlock = _MAX( 0, m_nFirstDisplayedBlock );
			m_nFirstDisplayedBlock = _MIN( m_nFirstDisplayedBlock, nLastFirstDisplayedBlock );
			Invalidate();			
			break;

		//The user is dragging the scroll box. This message is sent repeatedly until 
		//the user releases the mouse button. The nPos parameter indicates the position 
		// that the scroll box has been dragged to. 
		case SB_THUMBTRACK:
			pScrollBar->SetScrollPos( nPos );
			m_nFirstDisplayedBlock = nPos * bcsm.nColDrawCount;
			m_nFirstDisplayedBlock = _MAX( 0, m_nFirstDisplayedBlock );
			m_nFirstDisplayedBlock = _MIN( m_nFirstDisplayedBlock, nLastFirstDisplayedBlock );
			Invalidate();			
			break;

		case SB_TOP:            //Scrolls to the upper left. 
			break;
	}

	// Do we need this?
	CStatic::OnVScroll( nSBCode, nPos, pScrollBar );

	// If the first block is not a col-multiple, we screwed
	// something up somewhere upstream.
	ASSERT( m_nFirstDisplayedBlock % bcsm.nColDrawCount == 0 );

}


//-------------------------------------------------------------------//
// OnLButtonDown()																	//
//-------------------------------------------------------------------//
// Get the block under the cursor, if one exists.
//-------------------------------------------------------------------//
void BlockCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// DEBUG
	// Force a breakpoint.
	// _ASSERT( false );

	int nBlockNumber = GetBlockNumber( point );
	if ( nBlockNumber >= 0 && nBlockNumber < GetBlockCount() )
	{
		SelectBlock( nBlockNumber );
		Invalidate();
	}

	CStatic::OnLButtonDown(nFlags, point);
}


//-------------------------------------------------------------------//
// SelectBlock()																		//
//-------------------------------------------------------------------//
// Select the block and notify as requested.
//-------------------------------------------------------------------//
void	BlockCtrl::SelectBlock( int nBlockNumber ) 
{ 
	m_nSelectedBlock = nBlockNumber; 
	if ( m_pSelectNotifyWnd )
		m_pSelectNotifyWnd->SendMessage( 
			WM_USER_BLOCK_SELECTED, 
			(WPARAM) nBlockNumber,
			(LPARAM) GetSafeHwnd()
		);
}


