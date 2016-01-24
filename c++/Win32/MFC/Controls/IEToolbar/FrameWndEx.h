//-------------------------------------------------------------------//
// CFrameWndEx class
//-------------------------------------------------------------------//
// MDM	4/2/2004 3:12:19 PM
// This class sets up a frame with support for the self-contained SizableReBar.
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#ifndef __FRAMEWNDEX_H__
#define __FRAMEWNDEX_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define ID_WINDOW_LIST							16925			// No associated resource, just make sure ID is unique
// 
// Ideally, resources would be specified on a class-by-class basis.  Unfortunately,
// Visual Studio does not handle that well.  Different projects have different resource
// files that contain all of the project's resources in one place.  Make sure
// you provide the resources matching the above defines in your resource file.
// You must also include this file in the resource file's "Resource Set Includes"
// ( see "Resource Includes" on the View menu ).
//
// We want to exclude the remainder of the include file when dealing with App Studio.
#ifndef RC_INVOKED
/////////////////////////////////////////////////////////////////////////

// Where should this go?  GlobalData.h or in the block above?  Not sure...
#define IDW_MENUBAR                     0xE81A
// #include "Resource.h"


#include "BmpMenuOwnerWnd.h"
#include "MenuBar.h"
#include "SizableReBar.h"
#include "..\..\WinAppEx.h"

/////////////////////////////////////////////////////////////////////////////
// CFrameWndBase

template< class TBase >
class CFrameWndBase : public CBmpMenuOwnerWnd< TBase >
{
// Constructors
public:
    CFrameWndBase( bool bEmbedded = false );
    ~CFrameWndBase();

// Attributes
public:
    CSizableReBar   m_wndReBar;
    CMenuBar        m_wndMenuBar;

// Operations
public:
    void ShowWindow( int nCmdShow, LPCTSTR lpszProfileName );

   // CControlBar manipulation functions.
	void ShowBar( CControlBar& bar, bool bShow = true );
	void DockControlBarLeftOf(CControlBar* Bar, CControlBar* LeftOf);
	void DockControlBarTopLeft(CControlBar& bar);
	void DockControlBarTopRight(CControlBar& bar);

	// Just a helper to avoid having to specify a point.
	void FloatBar( CControlBar& bar )
	{
		FloatControlBar( &bar, CPoint( 0, 0 ) );
	}

// Overrides
public:
    virtual BOOL LoadFrame( UINT nIDResource,
        DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
        CWnd* pParentWnd = 0, CCreateContext* pContext = 0 );

// Implementation data
protected:
    CString m_strProfileName;
    bool    m_bInPlace;

    static LPCTSTR  m_lpszFrameWndPlacementEntry;

// Message map functions
protected:
    afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnSysCommand( UINT nID, LPARAM lParam );
    afx_msg LRESULT OnMenuChar( UINT nChar, UINT nFlags, CMenu* pMenu );
    afx_msg BOOL OnNcActivate( BOOL bActive );
    afx_msg void OnSettingChange( UINT uFlags, LPCTSTR lpszSection );
    afx_msg void OnClose();
    afx_msg void OnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu );
    afx_msg void OnMenuSelect( UINT nItemID, UINT nFlags, HMENU hSysMenu );

	// MDM	4/4/2004 10:59:11 PM
	// This indicates if we have an embedded frame.
	// If so, we route messages back to the parent, etc.
	bool m_bEmbedded;
};

/////////////////////////////////////////////////////////////////////////////
// CFrameWndEx frame

class /*GUILIB_EXT_CLASS*/ CFrameWndEx : public CFrameWndBase< CFrameWnd >
{
	typedef CFrameWndBase< CFrameWnd > inherited;
    DECLARE_DYNCREATE( CFrameWndEx )

// Constructors
public:
	CFrameWndEx( bool bEmbedded = false )
	:
		// Call base class constructor
		inherited( bEmbedded ),

		// Init vars
		m_bActive	( false )

	{}

	virtual ~CFrameWndEx();

   // CControlBar manipulation functions.
	void ShowBar( CControlBar& bar, bool bShow = true );
	void DockControlBarLeftOf(CControlBar* Bar, CControlBar* LeftOf);
	void DockControlBarTopLeft(CControlBar& bar);
	void DockControlBarTopRight(CControlBar& bar);

// Overrides
protected:
    virtual void DelayUpdateFrameMenu( HMENU hMenuAlt );
    virtual void OnUpdateFrameMenu( HMENU hMenuAlt );

// Generated message map functions
protected:
    //{{AFX_MSG(CFrameWndEx)
	//}}AFX_MSG

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

    DECLARE_MESSAGE_MAP()

	bool	m_bActive;

};

/////////////////////////////////////////////////////////////////////////////
// CMDIClientWndEx

class CMDIFrameWndEx;

class CMDIClientWndEx : public CWnd
{
    DECLARE_DYNCREATE( CMDIClientWndEx )

// Constructors
public:
    CMDIClientWndEx();
    ~CMDIClientWndEx();

// Implementation
protected:
    CMDIFrameWndEx* GetParentFrame() const;

// Implementation data
protected:
    HMENU m_hWindowMenu;

// Generated message map functions
protected:
    //{{AFX_MSG(CMDIClientWndEx)
    //}}AFX_MSG
    afx_msg LRESULT OnMDISetMenu( WPARAM wParam, LPARAM lParam );
    afx_msg LRESULT OnMDIRefreshMenu( WPARAM wParam, LPARAM lParam );
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMDIFrameWndEx frame

class /*GUILIB_EXT_CLASS*/ CMDIFrameWndEx : public CFrameWndBase< CMDIFrameWnd >
{
    DECLARE_DYNCREATE( CMDIFrameWndEx )

    friend CMDIClientWndEx;

// Constructors
public:
    CMDIFrameWndEx();
    ~CMDIFrameWndEx();

// Overrides
protected:
    virtual void DelayUpdateFrameMenu( HMENU hMenuAlt );
    virtual void OnUpdateFrameTitle( BOOL bAddToTitle );

// Implementation data
protected:

   // MDM	2/24/2005 4:59:19 PM
   // Updated to allow CMDIClientWndEx-derived client windows.
   virtual CMDIClientWndEx* CreateMDIClientWndForSubclassing()
   {
	   m_pwndMDIClient = new CMDIClientWndEx;
	   return m_pwndMDIClient;
   }
   CMDIClientWndEx* m_pwndMDIClient;

// Generated message map functions
protected:
    //{{AFX_MSG(CMDIFrameWndEx)
    afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
    afx_msg void OnWindowList();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWndEx frame

class /*GUILIB_EXT_CLASS*/ CMDIChildWndEx : public CBmpMenuOwnerWnd< CMDIChildWnd >
{
    DECLARE_DYNCREATE( CMDIChildWndEx )

// Constructors
public:
    CMDIChildWndEx();
    ~CMDIChildWndEx();

// Generated message map functions
protected:
    //{{AFX_MSG(CMDIChildWndEx)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CFrameWndBase out-of-line functions

template< class TBase >
LPCTSTR CFrameWndBase< TBase >::m_lpszFrameWndPlacementEntry = _T("FrameWndPlacement");

template< class TBase >
CFrameWndBase< TBase >::CFrameWndBase( bool bEmbedded ) 
:
	m_bInPlace( false ),
	m_bEmbedded( bEmbedded )

{
}

template< class TBase >
CFrameWndBase< TBase >::~CFrameWndBase()
{
}

template< class TBase >
void CFrameWndBase< TBase >::ShowWindow( int nCmdShow, LPCTSTR lpszProfileName )
{
    m_strProfileName = lpszProfileName;
    if ( !m_strProfileName.IsEmpty() )
    {
        WINDOWPLACEMENT* lpwndpl;
        UINT nBytes;

        if ( AfxGetApp()->GetProfileBinary( m_strProfileName,
                m_lpszFrameWndPlacementEntry, ( LPBYTE* )&lpwndpl, &nBytes ) )
        {
            CRect rcDesktop;
            GetDesktopWindow()->GetWindowRect( rcDesktop );

            // Only restore if window intersects the screen
            CRect rcTemp;
            if ( IntersectRect( rcTemp, &lpwndpl->rcNormalPosition, rcDesktop ) )
            {
                nCmdShow = ( lpwndpl->showCmd == SW_SHOWMINIMIZED ) ? SW_SHOWNORMAL : lpwndpl->showCmd;
                SetWindowPlacement( lpwndpl );
            }

            delete lpwndpl;
        }
    }

    CBmpMenuOwnerWnd< TBase >::ShowWindow( nCmdShow );
}

template< class TBase >
BOOL CFrameWndBase< TBase >::LoadFrame( UINT nIDResource, DWORD dwDefaultStyle,
                                        CWnd* pParentWnd, CCreateContext* pContext )
{
    if ( !TBase::LoadFrame( nIDResource, dwDefaultStyle, pParentWnd, pContext ) )
    {
        return FALSE;
    }

    VERIFY( m_wndMenuBar.SetMenu( ::GetMenu( m_hWnd ) ) );
    VERIFY( SetMenu( 0 ) );

    return TRUE;
}

template< class TBase >
int CFrameWndBase< TBase >::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CBmpMenuOwnerWnd< TBase >::OnCreate( lpCreateStruct ) == -1 )
    {
        return -1;
    }

	 if ( !m_wndReBar.Create( this, IDW_SIZABLEREBAR,
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
				CBRS_ALIGN_TOP | CBRS_SIZE_DYNAMIC ) )
	 {
		  return -1;
	 }

    if ( !m_bEmbedded )
	 {
		 if ( !m_wndMenuBar.CreateEx( this,
					TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TRANSPARENT | TBSTYLE_CUSTOMERASE, // Toolbar styles
					WS_CHILD | WS_VISIBLE |                                             // Window styles
					CBRS_ALIGN_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_HIDE_INPLACE,    // Control bar styles
					CRect( 0, 0, 0, 0 ), IDW_MENUBAR ) )
		 {
			  return -1;
		 }

		 if ( !m_wndReBar.AddBar( &m_wndMenuBar, 0, 0, RBBS_FIXEDBMP ) )
		 {
			  return -1;
		 }

		 m_wndMenuBar.SetOwner( &m_wndReBar );
		 m_wndMenuBar.UpdateMenuBar();

	}


    return 0;
}


template< class TBase >
void CFrameWndBase< TBase >::OnSysCommand( UINT nID, LPARAM lParam )
{
    if ( !m_wndMenuBar.FrameOnSysCommand( nID, lParam ) )
    {
        CBmpMenuOwnerWnd< TBase >::OnSysCommand( nID, lParam );
    }
}

template< class TBase >
LRESULT CFrameWndBase< TBase >::OnMenuChar( UINT nChar, UINT nFlags, CMenu* pMenu )
{
    if ( m_wndMenuBar.FrameOnMenuChar( nChar, nFlags, pMenu ) )
    {
        return MAKELRESULT( 0, 1 );
    }

    return CBmpMenuOwnerWnd< TBase >::OnMenuChar( nChar, nFlags, pMenu );
}

template< class TBase >
BOOL CFrameWndBase< TBase >::OnNcActivate( BOOL bActive )
{
    CFrameWnd* pFrame = GetTopLevelFrame();
    ASSERT_VALID( pFrame );

    m_wndMenuBar.FrameOnNcActivate(
        ( bActive || ( pFrame->m_nFlags & WF_STAYACTIVE ) ) && pFrame->IsWindowEnabled() );

    return CBmpMenuOwnerWnd< TBase >::OnNcActivate( bActive );
}

template< class TBase >
void CFrameWndBase< TBase >::OnSettingChange( UINT uFlags, LPCTSTR lpszSection )
{
    CWinAppEx::GetInstance()->UpdateSystemParameters();    // reload cached metrics

    CBmpMenuOwnerWnd< TBase >::OnSettingChange( uFlags, lpszSection );
}

template< class TBase >
void CFrameWndBase< TBase >::OnClose()
{
    if ( m_lpfnCloseProc == 0 )
    {
        if ( !m_strProfileName.IsEmpty() )
        {
            WINDOWPLACEMENT wndpl;
            wndpl.length = sizeof( wndpl );

            if ( GetWindowPlacement( &wndpl ) )
            {
                AfxGetApp()->WriteProfileBinary( m_strProfileName,
                    m_lpszFrameWndPlacementEntry, ( LPBYTE )&wndpl, wndpl.length );
            }
        }
    }

    CBmpMenuOwnerWnd< TBase >::OnClose();
}

template< class TBase >
void CFrameWndBase< TBase >::OnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu )
{
    CBmpMenuOwnerWnd< TBase >::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );

    m_wndMenuBar.FrameOnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );
}

template< class TBase >
void CFrameWndBase< TBase >::OnMenuSelect( UINT nItemID, UINT nFlags, HMENU hSysMenu )
{
    CBmpMenuOwnerWnd< TBase >::OnMenuSelect( nItemID, nFlags, hSysMenu );

	// MDM This isn't working for me...
	/*
	#if _MFC_VER == 0x0700
		m_wndMenuBar.FrameOnMenuSelect( nItemID, nFlags, ( ( CMenu* )hSysMenu )->GetSafeHmenu() );
	#else
	#endif
	*/
	m_wndMenuBar.FrameOnMenuSelect( nItemID, nFlags, hSysMenu );
}


// CControlBar manipulation functions.
template< class TBase >
void CFrameWndBase< TBase >::ShowBar( CControlBar& bar, bool bShow )
{	
	m_wndReBar.GetReBarCtrl().ShowBand( 
		m_wndReBar.GetReBarCtrl().IDToIndex( ( UINT )bar.GetDlgCtrlID() ), 
		bShow
	);

}


// The following 3 functions need updating to work with IEToolbar.
template< class TBase >
void CFrameWndBase< TBase >::DockControlBarLeftOf(CControlBar* Bar, CControlBar* LeftOf)
{
   // TO DO
   // This is how you do standard control bar handling.
   // We need to update to do it the "IEToolbar" way.

   CRect rect;
	DWORD dw;
	UINT n;
	
	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout(TRUE);
	
	LeftOf->GetWindowRect(&rect);
	rect.OffsetRect(1,0);
	dw=LeftOf->GetBarStyle();
	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;
	
	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line. By calculating a rectangle, we
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar,n,&rect);

}


template< class TBase >
void CFrameWndBase< TBase >::DockControlBarTopLeft(CControlBar& bar)
{
   // TO DO
}
template< class TBase >
void CFrameWndBase< TBase >::DockControlBarTopRight(CControlBar& bar)
{
   // TO DO
}


/////////////////////////////////////////////////////////////////////////////
#endif	// RC_INVOKED
#endif  // !__FRAMEWNDEX_H__
