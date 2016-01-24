// BlockCtrl.h : header file
//

#ifndef BLOCK_CONTROL_H
#define BLOCK_CONTROL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
using namespace std;									// REQD for STL classes!

#include <StandardTypes.h>							// For uByte, etc.


///////////////////////////////////////////////////////
// Globals, constants, statics
///////////////////////////////////////////////////////
//

// We fire this off on block selections, so the owner
// can take action.  WATCH OUT FOR ID COLLISIONS, sorry!
#define	WM_USER_BLOCK_SELECTED		WM_USER + 980

// This defines the currently-handled block colors.
// NOTE: You must keep the BlockColorMap defined
// in BlockCtrl.cpp synched up with these defines!
typedef enum 
{
	BC_BLACK,
	BC_WHITE,
	BC_PURPLE,
	BC_YELLOW,
	BC_RED,
	BC_ORANGE,
	BC_TEAL,
	BC_LIGHT_BLUE,
	BC_DARK_BLUE,
	BC_GRAY,
	BC_DARK_GRAY,
	BC_GREEN,
	BC_DARK_GREEN,
	
	BLOCK_COLOR_COUNT,
} BLOCK_COLOR;

class BlockType
{
public:

	BlockType(
		BLOCK_COLOR bcColor			= BC_BLACK				,
		BLOCK_COLOR bcHighlight		= BLOCK_COLOR_COUNT	,
		BLOCK_COLOR bcPartialFill	= BLOCK_COLOR_COUNT	,
		uByte			ubPercentFill	= 0						,
		bool			bCrossedOut		= false
	):
		
		// Init vars.
		m_bcColor		( bcColor			),
		m_bcHighlight	( bcHighlight		),
		m_bcPartialFill( bcPartialFill	),
		m_ubPercentFill( ubPercentFill	),
		m_bCrossedOut	( bCrossedOut		)

	{}

	BLOCK_COLOR		m_bcColor;
	BLOCK_COLOR		m_bcHighlight;
	BLOCK_COLOR		m_bcPartialFill;
	uByte				m_ubPercentFill;
	bool				m_bCrossedOut;

};

typedef vector< BlockType > BlockTypeVector;

typedef struct
{
	CRect rectBC;				// Actual block control
	CRect rectSB;				// Scroll bar
	CRect rectClient;			// Total client area
	int nRowDrawCount;
	int nColDrawCount;

} BCScreenMeasurements;

//
///////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// BlockCtrl window

class BlockCtrl : public CStatic
{
// Construction
public:
	BlockCtrl(
		int	nBlockCount	=	4096
	);

	// Similar to the tree control situation, we were not able to find a 
	// built-in init function, so we were forced to add one of our own, 
	// which must be called manually by the object containing this one.
	virtual void Initialize(
		CWnd* pSelectNotifyWnd		// You CAN pass 0 if you don't want notification.
	);

	void GetScreenMeasurements( BCScreenMeasurements& bcsm );

	void Resize( int nNewBlockCount );

	DWORD GetMinWidth();

	// For debugging.
	void SetAllBlockData( BlockType& btDefault );
	void RandomizeBlockData( int nColor = -1 );
	void RandomHighlightBlockData();

// Attributes
public:

	// Operations
public:

	inline BlockType& GetBlockType( 
		int nBlockNumber
	);
	inline void SetBlockType( 
		int nBlockNumber,
		BlockType& btSettings
	);

	void	SelectBlock( int nBlockNumber );
	inline int GetSelectedBlock() { return m_nSelectedBlock; }
	inline BlockType& GetSelectedBlockType();

	inline BLOCK_COLOR GetBlockColor( 
		int			nBlockNumber
	);
	inline BLOCK_COLOR GetSelectedBlockColor();
	inline void SetBlockColor( 
		int			nBlockNumber,
		BLOCK_COLOR	bcNewColor
	);

	inline BLOCK_COLOR GetBlockHighlight( 
		int			nBlockNumber
	);
	inline void SetBlockHighlight( 
		int			nBlockNumber,
		BLOCK_COLOR	bcNewHighlight
	);

	inline int GetBlockCount();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BlockCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~BlockCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(BlockCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	// Moved outside map to make this public.
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

protected:

	// Protected member functions.

	DECLARE_MESSAGE_MAP()

	int	GetBlockNumber	( CPoint& point );

	void DrawBlocks( CDC *pDCa = NULL );

	
	// Protected member variables.

	BlockTypeVector btvBlockData;
	
	CScrollBar	m_BlockScrollbar;
	SCROLLINFO m_si;
	int m_nFirstDisplayedBlock;		// used to control scrolling

	int m_nSelectedBlock;

	CWnd* m_pSelectNotifyWnd;
	
};


//-------------------------------------------------------------------//
// GetBlockType()																		//
//-------------------------------------------------------------------//
inline BlockType& BlockCtrl::GetBlockType( 
	int nBlockNumber
) {
	return btvBlockData.at( nBlockNumber );
}


//-------------------------------------------------------------------//
// SetBlockType()																		//
//-------------------------------------------------------------------//
inline void BlockCtrl::SetBlockType( 
	int nBlockNumber,
	BlockType& btSettings
) {
	btvBlockData.at( nBlockNumber ) = btSettings;
}


//-------------------------------------------------------------------//
// GetSelectedBlockType()															//
//-------------------------------------------------------------------//
inline BlockType& BlockCtrl::GetSelectedBlockType()
{
	return btvBlockData.at( m_nSelectedBlock );
}


//-------------------------------------------------------------------//
// GetBlockCount()																	//
//-------------------------------------------------------------------//
inline int BlockCtrl::GetBlockCount() 
{ 
	return btvBlockData.size(); 
}


//-------------------------------------------------------------------//
// [Get|Set]BlockColor()														//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
inline BLOCK_COLOR BlockCtrl::GetBlockColor( 
	int			nBlockNumber
) {
	return btvBlockData[ nBlockNumber ].m_bcColor;
}
inline BLOCK_COLOR BlockCtrl::GetSelectedBlockColor()
{
	return GetBlockColor( m_nSelectedBlock );
}
inline void BlockCtrl::SetBlockColor( 
	int			nBlockNumber,
	BLOCK_COLOR	bcNewColor
) {
	btvBlockData[ nBlockNumber ].m_bcColor = bcNewColor;
}

//-------------------------------------------------------------------//
// [Get|Set]BlockHighlight()														//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
inline BLOCK_COLOR BlockCtrl::GetBlockHighlight( 
	int			nBlockNumber
) {
	return btvBlockData[ nBlockNumber ].m_bcHighlight;
}
inline void BlockCtrl::SetBlockHighlight( 
	int			nBlockNumber,
	BLOCK_COLOR	bcNewHighlight
) {
	btvBlockData[ nBlockNumber ].m_bcHighlight = bcNewHighlight;
}


/////////////////////////////////////////////////////////////////////////////


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(BLOCK_CONTROL_H)
