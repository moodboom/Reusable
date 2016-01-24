//-------------------------------------------------------------------//
//
// NOTE: Derived from code provided by Pierre Mellinand on 
// CodeGuru, 10/99.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "StdAfx.h"
#include <afxtempl.h>

#include "StatusBarProgress.h"	// We drop one into one of our panes.

#include "StatusBarEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


////////////////////////////////////////////////////////////////////////
// StatusBarExPaneControlInfo
StatusBarExPaneControlInfo::StatusBarExPaneControlInfo()
:
	// Init vars.
	m_hWnd			( NULL	),
	m_bAutodelete	( TRUE	)
{
}


///////////////////////////////////////////////////////////////////////////////////
// StatusBarExPane


//-------------------------------------------------------------------//
// StatusBarExPane()																	//
//-------------------------------------------------------------------//
StatusBarExPane::StatusBarExPane()
:
	// Init vars.
	nID( 0 ),
	cxText( 0 ),
	nStyle( SBPS_NORMAL )

{
}


//-------------------------------------------------------------------//
// StatusBarExPane()																	//
//-------------------------------------------------------------------//
StatusBarExPane::StatusBarExPane( const StatusBarExPane& sbepSource )
{
	*this = sbepSource;
}


//-------------------------------------------------------------------//
// operator=()																			//
//-------------------------------------------------------------------//
StatusBarExPane& StatusBarExPane::operator=( const StatusBarExPane& sbepSource )
{
	nID		= sbepSource.nID;
	cxText	= sbepSource.cxText;
	nStyle	= sbepSource.nStyle;
	strText	= sbepSource.strText;
	return *this;
}


////////////////////////////////////////////////////////////////////////
// StatusBarEx
IMPLEMENT_DYNCREATE(StatusBarEx,CStatusBar)
BEGIN_MESSAGE_MAP(StatusBarEx, CStatusBar)
	//{{AFX_MSG_MAP(StatusBarEx)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------//
// StatusBarEx()																		//
//-------------------------------------------------------------------//
StatusBarEx::StatusBarEx()
:

	// Init vars.
	m_pStatusProgress( 0 )

{
}


//-------------------------------------------------------------------//
// ~StatusBarEx()																		//
//-------------------------------------------------------------------//
StatusBarEx::~StatusBarEx()
{
	for( int i = 0; i < m_vPanes.size(); i++ )
	{
		if	( 
				m_vPanes[i]->m_hWnd 
			&& ::IsWindow(m_vPanes[i]->m_hWnd->m_hWnd) 
		)
			m_vPanes[i]->m_hWnd->CloseWindow();
		
		if ( m_vPanes[i]->m_bAutodelete )
			delete m_vPanes[i]->m_hWnd;

		delete m_vPanes[i];
	}

	if ( m_pStatusProgress )
		delete m_pStatusProgress;
}


//-------------------------------------------------------------------//
// AddDynamicPane()																	//
//-------------------------------------------------------------------//
BOOL StatusBarEx::AddDynamicPane( int position, UINT paneID, HICON hIcon )
{
	std::vector<StatusBarExPane> vPanes;

	StatusBarExPane statusPane;
	int i;

	i = 0;
	while( GetStatusPane(i,statusPane) )
	{
		vPanes.push_back( statusPane );
		i++;
	}
	if( position < 0 )
		position = 0;
	if( position > vPanes.size() )
		position = vPanes.size()-1;

	for(i = 0; i < vPanes.size(); i ++ )
	{
		if( paneID == vPanes[i].nID )
		{
			TRACE(_T("XFXStatusBar::AddDynamicPane(): Pane ID already exists \n"));
			return FALSE;
		}
	}

	StatusBarExPane new_statusPane;
	new_statusPane.nID = paneID;
	
	vPanes.insert( &vPanes[position], new_statusPane );
	/*
	if( vPanes.size() )
		vPanes.InsertAt(position,new_statusPane);
	else
		vPanes.Add(new_statusPane);
	*/

	UINT * pIndicators = new UINT[vPanes.size()];
	for( i = 0; i < vPanes.size(); i++ )
		pIndicators[i]=vPanes[i].nID;

	SetRedraw(FALSE);
	SetIndicators(pIndicators,vPanes.size());

	// Replace infos of indicators ..
	for(i = 0; i < vPanes.size(); i++ )
	{
		if( vPanes[i].nID != paneID )
		{
			SetPaneInfo(i,vPanes[i].nID,vPanes[i].nStyle,vPanes[i].cxText);
			SetPaneText(i,vPanes[i].strText);
		}
		
		// MDM	8/23/01 9:57:02 AM
		// This default sucks!
		// It hardcodes a useless width, and sets the text to empty.
		// CStatusBar::SetIndicators() already set the text using the resource ID.
		/*
		else
		{
			SetPaneWidth(i,50);
			SetPaneText(i,vPanes[i].strText);
		}
		*/
	}

	delete pIndicators;

	// CStatusBar or the underlying control must internally keep track of 
	// icons and the panes they have been assigned to.  If we insert a
	// new pane dynamically, we will throw all this off.
	//
	// To fix this, we keep track internally of assigned icons and
	// the corresponding INDICATOR, so we can reassign them properly
	// on dynamic pane insertion/removal.
	m_vIcons.insert( &m_vIcons[ position ], hIcon );
	UpdatePaneIcons();

	SetRedraw(TRUE);
	PositionControls();
	Invalidate(TRUE);	
	return TRUE;
}


//-------------------------------------------------------------------//
// RemovePane()																		//
//-------------------------------------------------------------------//
void StatusBarEx::RemovePane(int nPaneID)
{
	// Get the index, we will need this for icon management, later.
	int nBarIndex = CommandToIndex( nPaneID );
	
	// MDM	10/31/2003 3:41:12 PM
	// If it doesn't exist, there's nothing to remove.
	if ( nBarIndex == -1 )
		return;

	SetRedraw(FALSE);

	CWnd * pwnd = NULL;
	for( int i = 0; i < m_vPanes.size(); i++ )
	{
		if( m_vPanes[i]->m_nPaneID == nPaneID )
		{
			if( m_vPanes[i]->m_bAutodelete )
				pwnd = m_vPanes[i]->m_hWnd;
			delete m_vPanes[i];
			m_vPanes.erase( &m_vPanes[i] );
			break;
		}
	}
	if( pwnd )
	{
		pwnd->DestroyWindow();
		delete pwnd;
	}
	
	std::vector<StatusBarExPane> arPanes;
	StatusBarExPane statusPane;
	i = 0;
	while( GetStatusPane(i,statusPane) )
	{
		if( statusPane.nID != (UINT)nPaneID )
			arPanes.push_back( statusPane );
		i++;
	}
	// Now remove the indicator
	UINT* pIndicators = new UINT[arPanes.size()];
	for(i=0;i<arPanes.size();i++)
		pIndicators[i]=arPanes[i].nID;
	SetIndicators(pIndicators,arPanes.size());

	// Replace infos of indicators ..
	for(i = 0; i < arPanes.size(); i++ )
	{
		SetPaneInfo(i,arPanes[i].nID,arPanes[i].nStyle,arPanes[i].cxText);
		SetPaneText(i,arPanes[i].strText);
	}
	delete pIndicators;

	// CStatusBar or the underlying control must internally keep track of 
	// icons and the panes they have been assigned to.  If we insert a
	// new pane dynamically, we will throw all this off.
	//
	// To fix this, we keep track internally in THIS classs of assigned icons,
	// and reassign them on dynamic pane changes.
	m_vIcons.erase( &m_vIcons[ nBarIndex ] );
	UpdatePaneIcons();

	SetRedraw(TRUE);
	PositionControls();
	Invalidate(TRUE);

}


//-------------------------------------------------------------------//
// SetIcon()																			//
//-------------------------------------------------------------------//
// Here, we set the icon of the requested pane.  This function
// is similar to the base class, but we use an indicator ID
// instead of a pane index.  We can therefore keep track of the
// icons as we dynamically add/remove panes.
//-------------------------------------------------------------------//
void StatusBarEx::SetIcon( int nPaneID, HICON hIcon )
{
	int nBarIndex = CommandToIndex( nPaneID );
	ASSERT( nBarIndex != -1 );
	if ( nBarIndex != -1 )
	{
		GetStatusBarCtrl().SetIcon( nBarIndex, hIcon );

		// Now back up the information.
		m_vIcons[nBarIndex] = hIcon;
	}
}


//-------------------------------------------------------------------//
// UpdatePaneIcons()																	//
//-------------------------------------------------------------------//
// Here, we just rip through the panes and reset the icons.
//-------------------------------------------------------------------//
void StatusBarEx::UpdatePaneIcons()
{
	for ( int nA = 0; nA < m_vIcons.size(); nA++ )
	{
		// This is a valid index, right?
		ASSERT( GetItemID( nA ) != -1 );

		GetStatusBarCtrl().SetIcon( nA, m_vIcons[nA] );
	}
}


//-------------------------------------------------------------------//
// AttachControl()																		//
//-------------------------------------------------------------------//
BOOL StatusBarEx::AttachControl(CWnd * pWnd, int paneID, BOOL bAutodeleteControl)
{

	StatusBarExPaneControlInfo* pPanInfo = GetStatusPaneControl(paneID);
	if( pPanInfo )
		return FALSE;

	int idx = CommandToIndex( paneID ) ;
	if( idx == -1 )
		return FALSE;

	StatusBarExPaneControlInfo * pPan = new StatusBarExPaneControlInfo;
	pPan->m_nPaneID		=  paneID;
	pPan->m_hWnd			=  pWnd;
	pPan->m_bAutodelete	= bAutodeleteControl;

	m_vPanes.push_back(pPan);
	PositionControls();
	Invalidate(TRUE);
	return TRUE;
}


//-------------------------------------------------------------------//
// GetStatusPaneControl()																	//
//-------------------------------------------------------------------//
StatusBarExPaneControlInfo* StatusBarEx::GetStatusPaneControl(int nPaneID)
{
	for(int i = 0; i < m_vPanes.size(); i++ )
	{
		if( m_vPanes[i]->m_nPaneID == nPaneID )
			return m_vPanes[i];
	}
	return NULL;
}


//-------------------------------------------------------------------//
// SetPaneWidth()																		//
//-------------------------------------------------------------------//
void StatusBarEx::SetPaneWidth(int index, int cxWidth)
{
	UINT nID,nStyle;
	int width;
	GetPaneInfo(index,nID,nStyle,width);
	SetPaneInfo(index,nID,nStyle,cxWidth);
}


//-------------------------------------------------------------------//
// OnSize()																				//
//-------------------------------------------------------------------//
void StatusBarEx::OnSize(UINT nType, int cx, int cy) 
{
	CStatusBar::OnSize(nType, cx, cy);
	PositionControls();

}


//-------------------------------------------------------------------//
// PositionControls()																//
//-------------------------------------------------------------------//
void StatusBarEx::PositionControls()
{
	int h,v,s;
	GetStatusBarCtrl( ).GetBorders( h, v, s ) ;


	for(int i = 0; i < m_vPanes.size(); i++ )
	{
		CRect rect;
		
		int index = CommandToIndex( m_vPanes[i]->m_nPaneID );
		GetItemRect(index,rect);
		if( GetPaneStyle(index) & SBPS_NOBORDERS == SBPS_NOBORDERS)
			m_vPanes[i]->m_hWnd->MoveWindow(rect);
		else
		{
			rect.top+=v;
			rect.bottom-=v;
			rect.left+=h+s;
			rect.right-=(h+s);
			m_vPanes[i]->m_hWnd->MoveWindow(rect);
		}
		
	}
}


//-------------------------------------------------------------------//
// GetStatusPane()																	//
//-------------------------------------------------------------------//
BOOL StatusBarEx::GetStatusPane(int nIndex, StatusBarExPane & xfxpane)
{
	if( nIndex < m_nCount  && nIndex >= 0 )
	{
		GetPaneInfo( nIndex,  xfxpane.nID, xfxpane.nStyle, xfxpane.cxText ) ;
		GetPaneText( nIndex , xfxpane.strText );
		return TRUE;
	}
	return FALSE;
}


//-------------------------------------------------------------------//
// SetMaxProgress()																	//
//-------------------------------------------------------------------//
// Here, we begin the process of displaying progress in 
// the status bar.  Call this FIRST, then use SetProgress()
// as needed, then call EndProgress().  Always.  :>
//-------------------------------------------------------------------//
void StatusBarEx::SetMaxProgress( int nMax )
{
	if ( m_pStatusProgress == 0 )
	{
		// Make us a nice new progress control for the pane.
		m_pStatusProgress = new StatusBarProgress(
			_T(""),		// Text to the left of the progress bar.
			99,			// Width as percentage of pane (NOTE: 100% causes schtum)
			nMax,			// Max progress
			FALSE,		// bSmooth
			1				// Pane ( default = 0 )
		);
	}

	m_pStatusProgress->SetRange( nMax );

}


//-------------------------------------------------------------------//
// SetProgress()																		//
//-------------------------------------------------------------------//
void StatusBarEx::SetProgress( int nPos )
{
	ASSERT( m_pStatusProgress );
	m_pStatusProgress->SetPos( nPos );
}


//-------------------------------------------------------------------//
// EndProgress()																		//
//-------------------------------------------------------------------//
void StatusBarEx::EndProgress()
{

	ASSERT( m_pStatusProgress );
	if ( m_pStatusProgress )
	{
		delete m_pStatusProgress;
		m_pStatusProgress = 0;
	}
	
}


// OLD
/*
//-------------------------------------------------------------------//
// OnUpdateCmdUI()																	//
//-------------------------------------------------------------------//
// This "advanced overridable" (ooo MS u scarin me!) updates
// our panes on idle time.
//-------------------------------------------------------------------//
void StatusBarEx::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHandler )
{
	// We're supposed to call all the update handlers
	// for all the ID's for all the status bar panes here.

	// Can we manage to fix the problem of the icon being set to
	// the wrong pane(s) here?  Seems like a command update 
	// handler prob, where I set the icon but by the time
	// the update occurs, the pane indexes have changed.

	// First, determine if this is a command to update the icon.
	// If so, 
	
	inherited::OnUpdateCmdUI( pTarget, bDisableIfNoHandler );


	// Here's some crappy example code...
	*
	MMStatusCmdUI state;
	state.m_pOther = this;
	state.m_pMenu = (CMenu*)MM_STATUS_PANE_UPDATE;
	state.m_nIndexMax = (UINT)m_nCount;
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = GetItemID(state.m_nIndex);

		// allow the statusbar itself to have update handlers
		if (
			CWnd::OnCmdMsg(
				GetItemID(state.m_nIndex), CN_UPDATE_COMMAND_UI, &state, NULL))
			continue;

		// allow target (owner) to handle the remaining updates
		state.DoUpdate(pTarget, FALSE);
	}
	*
}
*/


//-------------------------------------------------------------------//
// OnChildNotify()																	//
//-------------------------------------------------------------------//
// This can let us know if one of our panes was clicked or
// double-clicked.  Check out "NM_DBLCLK", then locate, in MSDN
// for other available notifications.  As of now, all we
// "catch" is single clicks.
//
// This is pretty much a copy of CStatusBar::OnChildNotify().
// The only difference is here, we check for NM_CLICK, not 
// NM_DBLCLK.
//
// The frame calls this function when it receives notification about
// the status bar, and we turn the notification into commands sent 
// to the parent.  The frame can then process status bar indicator
// ID's as commands.
//-------------------------------------------------------------------//
BOOL StatusBarEx::OnChildNotify(UINT message,WPARAM wParam,LPARAM lParam,LRESULT* pResult)
{
	switch(message)
	{
		case WM_NOTIFY:
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;
			if (
					NM_CLICK == pNMHDR->code
			//	|| NM_DBLCLK == pNMHDR->code
			) {
				NMMOUSE* pNMMouse = (NMMOUSE*)lParam;
				
				// If you click just next to a pane, I have seen dwItemSpec
				// get set to -2 (what does THAT imply?  not sure...).
				if ( ( (int)pNMMouse->dwItemSpec ) >= 0 )
				{
					UINT uCommandId = GetItemID(pNMMouse->dwItemSpec);
					this->GetParent()->SendMessage(WM_COMMAND,uCommandId,0);
				}
			}
		}
		break;
	default:break;
	}
	return CStatusBar::OnChildNotify(message,wParam,lParam,pResult);
}


// End of file ///////////////////////////////////////////////////////////////
