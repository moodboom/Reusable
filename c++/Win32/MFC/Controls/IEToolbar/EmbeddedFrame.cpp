/////////////////////////////////////////////////////////////////////////
//
// CEmbeddedFrame            
//
// Created: Mar 12, 2001
// Last Modified: April 12, 2001
//
/////////////////////////////////////////////////////////////////////////
// Copyright (C) 2001 by Greg Winkler, Intelligent Systems Inc.
// (docking_windows@intelligentsystems.net)
//
// All rights reserved.
//
// This code is free for personal and commercial use, providing this 
// notice remains intact in the source files and all eventual changes are
// clearly marked with comments.
//
// You must obtain the author's consent before you can include this code
// in a software library.
//
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc. for
// the docking window code to cristi@datamekanix.com or post them 
// at the message board at the site.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc. for
// this code to docking_windows@intelligentsystems.net.
//
/////////////////////////////////////////////////////////////////////////
/*///////////////////////////////////////////////////////////////////////

    Description:

    The quickest way to get Cristi Posea's docking windows to dock 
    in an Active X control, without having to write a lot of the docking 
    code yourself, is to use the MFC CFrameWnd class.

    It turns out that this is incredibly easy. You simply derive a class
    from CFrameWnd and and instance of it on-the-fly as your control is created
    (in the WM_CREATE handler). The trick is to use the WS_CHILD 
    style and NOT the WS_OVERLAPPEDWINDOW style, which is the class'
    default.

    When your control is sized (in the WM_SIZE handler), position the 
    embedded frame so it's the size you want.

    Incidently, I've used this same approach in splitter windows to 
    only have docking windows in a certain section of the splitter. Just
	remember to relay WM_IDLEUPDATECMDUI to your main frame to your
	embedded frames.
    
/*///////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DockCtrl.h"
#include "EmbeddedFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmbeddedFrame

IMPLEMENT_DYNCREATE(CEmbeddedFrame, CFrameWnd)

CEmbeddedFrame::CEmbeddedFrame()
: m_bActive(false)
{
}

CEmbeddedFrame::~CEmbeddedFrame()
{
}


BEGIN_MESSAGE_MAP(CEmbeddedFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CEmbeddedFrame)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmbeddedFrame message handlers

void CEmbeddedFrame::PostNcDestroy() 
{
    /////////////////////////////////////////////////////////////////////////
	/*///////////////////////////////////////////////////////////////////////
        The default implementation of PostNcDestroy will call 
        
          delete this;

        Since we created the frame window as a member variable, not on the
        heap, this will cause wild assertions and other nastiness to happen, 
        so I override and don't call the base class
	/*///////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////
	
//	CFrameWnd::PostNcDestroy();
    
}

/*////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

    METHOD:     int CEmbeddedFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 

    PURPOSE:    Called to handle the WM_CREATE message. After calling the
                base class OnCreate(), then you create the docking window
                (m_wndMyBar) and dock it within the frame.

    PARAMETERS: LPCREATESTRUCT  lpCreateStruct (only passed to base)

    RETURNS:    < 0 if creation not successful

    NOTES:      I tried to define _SCB_REPLACE_MINIFRAME and got some 
                really interesting behavior when m_wndMyBar is undocked.

//////////////////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////
int CEmbeddedFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    /////////////////////////////////////////////////////////////////////////
    /// The regular CMainFrame in the doc-view MFC
    /// apps receive WM_IDLEUPDATECMDUI messages from the CWinThread::OnIdle(), 
    /// and called RecalcLayout if the idleLayout flag was set by a 
    /// previous call to DelayRecalcLayout(). Since an ActiveX control 
    /// doesn't have this mechanism, I have to fake it (my thanks to Cristi 
    /// for this fix).
    /////////////////////////////////////////////////////////////////////////
	SetTimer(0, 100, NULL);
	
    /////////////////////////////////////////////////////////////////////////
    /// create an edit control within the frame so there's something 
    /// interesting in there...
    /////////////////////////////////////////////////////////////////////////
	m_wndClient.Create( WS_VISIBLE|WS_CHILD, 
                        CRect(0,0,0,0), 
                        this, 
                        AFX_IDW_PANE_FIRST );
	m_wndClient.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

    /////////////////////////////////////////////////////////////////////////
	/// older versions of Windows* (NT 3.51 for instance)
	/// fail with DEFAULT_GUI_FONT
    /////////////////////////////////////////////////////////////////////////
	if (!m_font.CreateStockObject(DEFAULT_GUI_FONT))
		if (!m_font.CreatePointFont(80, "MS Sans Serif"))
			return -1;

	m_wndClient.SetFont(&m_font);

    m_wndClient.SetWindowText( _T( "This window is a child of the frame." ) );
	
    /////////////////////////////////////////////////////////////////////////
    /// Create the docking window and dock it into the frame...
    /////////////////////////////////////////////////////////////////////////
    if (!m_wndDockingWnd.Create(_T("Docking Window"), this, IDC_DOCKING_WND))
    {
        TRACE0("Failed to create Docking Window\n");
        return -1;      // fail to create
	}

    /////////////////////////////////////////////////////////////////////////
    /// styles suggested by Cristi Posea.
    /////////////////////////////////////////////////////////////////////////
    m_wndDockingWnd.SetBarStyle(m_wndDockingWnd.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);


    /////////////////////////////////////////////////////////////////////////
    /// Use CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT to only dock on sides,
    /// CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM to only dock on top and bottom
    /////////////////////////////////////////////////////////////////////////
	EnableDocking(CBRS_ALIGN_ANY);
    
    /////////////////////////////////////////////////////////////////////////
    /// from Cristi Posea's documentation
    /////////////////////////////////////////////////////////////////////////
#ifdef _SCB_REPLACE_MINIFRAME
    m_pFloatingFrameClass = RUNTIME_CLASS(CSCBMiniDockFrameWnd);
#endif //_SCB_REPLACE_MINIFRAME


    /////////////////////////////////////////////////////////////////////////
    /// Use CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT to only dock on sides,
    /// CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM to only dock on top and bottom
    /////////////////////////////////////////////////////////////////////////
    m_wndDockingWnd.EnableDocking(CBRS_ALIGN_ANY);

    /////////////////////////////////////////////////////////////////////////
    /// Actually dock it into the frame slot (dockbar), otherwise it'll be
	/// fixed into position (acting like non-docking control bars)
    /////////////////////////////////////////////////////////////////////////
    DockControlBar(&m_wndDockingWnd, AFX_IDW_DOCKBAR_LEFT);
	
	return 0;
}


/*////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

    METHOD:	    void CEmbeddedFrame::OnTimer(UINT nIDEvent) 

    PURPOSE:    WM_TIMER handler.
    
                Simulates the WM_IDLEUPDATECMDUI behavior of doc-view
                framework.

				This code is thanks to Cristi Posea.

	PARAMETERS:	UINT nIDEvent - index of timer event fired. I only use 0.

    RETURNS:    None

//////////////////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////
void CEmbeddedFrame::OnTimer(UINT nIDEvent) 
{
	CFrameWnd::OnTimer(nIDEvent);

	if (nIDEvent != 0)
		return;

	if (m_hWnd != NULL)
	{
		if (m_nShowDelay == SW_HIDE)
			ShowWindow(m_nShowDelay);


		if (IsWindowVisible() ||
			m_nShowDelay >= 0)
		{
			AfxCallWndProc(this, m_hWnd,
				WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0);
			SendMessageToDescendants(
				WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0, TRUE, TRUE);
		}
		if (m_nShowDelay > SW_HIDE)
			ShowWindow(m_nShowDelay);
		m_nShowDelay = -1;

		/////////////////////////////////////////////////////////////////////
		// send WM_IDLEUPDATECMDUI to the floating miniframes
		/////////////////////////////////////////////////////////////////////
	    POSITION pos = m_listControlBars.GetHeadPosition();
	    while (pos != NULL)
		{
			CControlBar* pBar = (CControlBar*) m_listControlBars.GetNext(pos);
			ASSERT(pBar != NULL);
			
			/////////////////////////////////////////////////////////////////
			// skip if not created yet or if it is not a floating CDockBar
			/////////////////////////////////////////////////////////////////
			if (pBar->m_hWnd == NULL ||
				pBar->GetDlgCtrlID() != AFX_IDW_DOCKBAR_FLOAT)
				continue;

			CFrameWnd* pFrameWnd = pBar->GetParentFrame();

			if (pFrameWnd->m_hWnd != NULL && pFrameWnd != this)
			{
				if (pFrameWnd->m_nShowDelay == SW_HIDE)
					pFrameWnd->ShowWindow(pFrameWnd->m_nShowDelay);
				if (pFrameWnd->IsWindowVisible() ||
					pFrameWnd->m_nShowDelay >= 0)
				{
					AfxCallWndProc(pFrameWnd, pFrameWnd->m_hWnd,
						WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0);
					pFrameWnd->SendMessageToDescendants(WM_IDLEUPDATECMDUI,
						(WPARAM)TRUE, 0, TRUE, TRUE);
				}
				if (pFrameWnd->m_nShowDelay > SW_HIDE)
					pFrameWnd->ShowWindow(pFrameWnd->m_nShowDelay);
				pFrameWnd->m_nShowDelay = -1;
			}
		}

		/////////////////////////////////////////////////////////////////////
		// find if the top level parent is the active window
		/////////////////////////////////////////////////////////////////////
		bool bActive = (GetTopLevelParent() == GetForegroundWindow());
		if (bActive != m_bActive)
		{
			/////////////////////////////////////////////////////////////////
			// notify the floating miniframes of state change
			/////////////////////////////////////////////////////////////////
			NotifyFloatingWindows(bActive ? FS_ACTIVATE : FS_DEACTIVATE);
			m_bActive = bActive;
		}
	}
}


/*////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

    METHOD:	    BOOL CEmbeddedFrame::PreCreateWindow(CREATESTRUCT& cs) 

    PURPOSE:    Called before frame is created.

    PARAMETERS:	CREATESTRUCT& cs

    RETURNS:    TRUE if creation should continue

//////////////////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////
BOOL CEmbeddedFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;

    /////////////////////////////////////////////////////////////////////////
    /// remove the CLIENTEDGE style, so you can't tell the frame's there
    /////////////////////////////////////////////////////////////////////////
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return TRUE;
}


/*////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

    METHOD:	    void CEmbeddedFrame::OnDestroy() 

    PURPOSE:    WM_DESTROY handler - one must always clean up after oneself.
                Kill the timer we set in OnCreate()

    PARAMETERS:	none

    RETURNS:    none

//////////////////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////
void CEmbeddedFrame::OnDestroy() 
{
	KillTimer(0);

	CFrameWnd::OnDestroy();
}
