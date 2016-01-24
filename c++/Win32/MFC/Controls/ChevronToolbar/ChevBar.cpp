#include "stdafx.h"
#include "ChevBar.h"
#include "ChevDrop.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CChevBar

CChevBar::CChevBar()
{
	m_pDropWindow = NULL;
}

CChevBar::~CChevBar()
{
	if ( m_pDropWindow )
	{
		delete m_pDropWindow;
		m_pDropWindow = NULL;
	}
}


BEGIN_MESSAGE_MAP(CChevBar, CReBar)
	//{{AFX_MSG_MAP(CChevBar)
	//}}AFX_MSG_MAP

	// Reflection message entry for Chevron push
	ON_NOTIFY_REFLECT( RBN_CHEVRONPUSHED, OnChevronPushed )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChevBar

// This method is to add a band to the Rebar
// Toolbars are preferred now. The iID has to be unique for the *toolbar bands* that are added
BOOL	CChevBar::AddBar( CWnd* pBar, int iID, LPCTSTR lpszText, DWORD dwStyle )
{

	CWnd *pTBar = FromHandle ( pBar->m_hWnd );
	ASSERT(pTBar);
	
	DWORD dwStyleNew = dwStyle;

	// Check if the window to be added is from CToolBar
	// Chevron support is only for the control with toolbar as child window
	// and proper id is provided
	if ( pTBar->IsKindOf(RUNTIME_CLASS(CToolBar)) && iID != -1 )
		dwStyleNew |= RBBS_USECHEVRON;
	else
		dwStyleNew &= ~RBBS_USECHEVRON;

	// Add band
	BOOL bReturn = CReBar::AddBar ( pBar, lpszText, NULL, dwStyleNew );
	if ( !bReturn )
		return FALSE;

	// Toolbar is in child band, so set band properties for chevron to show
	// properly
	if ( dwStyleNew & RBBS_USECHEVRON )
	{
		// This looks like toolbar

		CToolBar *pToolBar = (CToolBar*)pTBar;

		CReBarCtrl & refBar = GetReBarCtrl();
		
		// This will be the band we just added
		UINT iBandIndex = refBar.GetBandCount( )-1;

		// BUGBUG:
		// CToolBarCtrl::GetMaxSize( ) doest seem to return proper size when
		// TBSTYLE_DROPDOWN style is set for buttons.. Anybody knows any way ?

		CSize sizeMax;
		pToolBar->GetToolBarCtrl().GetMaxSize ( &sizeMax );
		
		// Set band prop
		REBARBANDINFO rbbi;
		rbbi.cbSize = sizeof(rbbi);
		rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE | RBBIM_ID;
		rbbi.cxMinChild = 0;
		rbbi.cyMinChild = sizeMax.cy;
		rbbi.cx = sizeMax.cx;
		rbbi.cxIdeal = sizeMax.cx;
		rbbi.wID = iID;
		refBar.SetBandInfo( iBandIndex, &rbbi); 
	}

	return TRUE;
}

/*
	This is the handler for chevron push
*/
void CChevBar::OnChevronPushed( NMHDR * pNotifyStruct, LRESULT* result )
{

	NMREBARCHEVRON* pChev = (NMREBARCHEVRON*) pNotifyStruct;
	
	// Has the band id of the chevron that generated this message
	int	iBand = pChev->uBand;

	// Have to get the child window handle this band holds
	REBARBANDINFO	rbinfo;
	rbinfo.cbSize = sizeof(rbinfo);
	rbinfo.fMask = RBBIM_CHILD;
	GetReBarCtrl().GetBandInfo ( iBand, &rbinfo );

	// Check if the child window is a toolbar
	// Some rebar bands may have other windows, so that is left as an exercise
	// to the user :-)
	CWnd *pBar = FromHandle ( rbinfo.hwndChild );
	ASSERT(pBar);
	ASSERT(pBar->IsKindOf(RUNTIME_CLASS(CToolBar)));

	CToolBar *pTBar = (CToolBar *)pBar;

	// Get band rectangle.
	CRect	rectBand;
	GetReBarCtrl ().GetRect( iBand, &rectBand );

	// We are concerned only with the size, but GetRect( ) returns the actual band position
	// So we have to translate its position to 0,0 
	rectBand.right = rectBand.Width ();
	rectBand.left  = 0 ;
	rectBand.bottom = rectBand.Height();
	rectBand.top = 0;
	
	// It depends on the user to decide if this subtraction is necessary, if they feel
	// happy about the chevron hiding a part of a button then remove this code
	CRect rectChevron;
	rectChevron = pChev->rc;
	rectBand.right -= rectChevron.Width ();

	// Screen co-ordinates for Menu to be displayed
	CPoint	ptMenu;
	ptMenu.x = rectChevron.left;
	ptMenu.y = rectChevron.bottom;
	ClientToScreen ( &ptMenu );

	// Create the drop down popup window
	if ( ! m_pDropWindow )
	{
		m_pDropWindow = new CChevDrop;
		if ( m_pDropWindow )
			m_pDropWindow->CreatePopup ( this );
			
	}

	// this CChevDrop object deletes itself when it has been created/shown properly
	if ( m_pDropWindow )
		m_pDropWindow->ShowPopup ( GetParent(), pTBar, rectBand, ptMenu );
		
}
