//-------------------------------------------------------------------//
// CMenuBar class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#ifndef __MENUBAR_H__
#define __MENUBAR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define IDS_TOOLTIP_CLOSE							16912
#define IDS_TOOLTIP_MINIMIZE						16913
#define IDS_TOOLTIP_MAXIMIZE						16914
#define IDS_TOOLTIP_RESTORE						16915

#define IDBUTTON_FIRST      65000
#define IDBUTTON_SYSMENU    65000
#define IDBUTTON_LAST       65100

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


#include "..\..\GlobalData.h"

#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CMenuBarButtonXxx - helper classes

class CMenuBarButton
{
// Construction
public:
    CMenuBarButton();
    virtual ~CMenuBarButton();

// Operations
public:
    void DrawButton( CDC* pDC, bool bFrameActive );
    bool HitTest( CPoint pt ) const;
    void SetMDIChild( HWND hWndMDIChild );
    void SetMenuBarRect( LPCRECT lpRect );
    void SetTheme( HTHEME hTheme );
    bool HideButton( bool bHide );
    bool PushButton( bool bPush );
    bool IsPushed() const;

    static CSize GetButtonSize();

// Overrides
public:
    virtual CRect GetButtonRect() const = 0;
    virtual UINT GetSysCommandID() const = 0;
    virtual bool IsEnabled() const;
    virtual bool IsVisible() const;

// Implementation
protected:
    DWORD GetMDIChildStyle() const;

// Implementation data
protected:
    HWND    m_hWndMDIChild;
    CRect   m_rcMenuBar;
    bool    m_bHidden;
    bool    m_bPushed;
    HTHEME  m_hTheme;
};

class CMenuBarButtonMin : public CMenuBarButton
{
// Overrides
public:
    virtual CRect GetButtonRect() const;
    virtual UINT GetSysCommandID() const;
    virtual bool IsEnabled() const;
    virtual bool IsVisible() const;
};

class CMenuBarButtonMax : public CMenuBarButton
{
// Overrides
public:
    virtual CRect GetButtonRect() const;
    virtual UINT GetSysCommandID() const;
    virtual bool IsEnabled() const;
    virtual bool IsVisible() const;
};

class CMenuBarButtonClose : public CMenuBarButton
{
// Overrides
public:
    virtual CRect GetButtonRect() const;
    virtual UINT GetSysCommandID() const;
};

/////////////////////////////////////////////////////////////////////////////
// CMenuBar

template< class TBase > class CFrameWndBase;

class /*GUILIB_EXT_CLASS*/ CMenuBar : public CToolBar
{
    DECLARE_DYNAMIC( CMenuBar );

    friend CFrameWndBase< CFrameWnd >;
    friend CFrameWndBase< CMDIFrameWnd >;

// Construction
public:
    CMenuBar();
    ~CMenuBar();

// Operations
public:
    bool SetMenu( HMENU hMenu );
    HMENU GetMenu() const;

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMenuBar)
    //}}AFX_VIRTUAL

    virtual void OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler );
    virtual int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

// Implementation
protected:
    CReBarCtrl& GetParentReBarCtrl() const;
    int GetParentBandIndex() const;
    void SetButtonWidth( UINT nID, int nWidth );
    void UpdateMenuBar();
    void RepositionSysButtons( CRect rcMenuBar );
    void EnterTrackingMode( int nItem );
    void TrackChevronMenu( CRect& rcChevron, int nItem );
    void TrackPopupMenu();
    void ContinueTracking( bool bSelectFirst );
    void ExitTrackingMode();
    void ShowChevronMenu( int nItem );
    bool IsItemClipped( int nItem ) const;
    bool IsOverChevron( CPoint pt ) const;
    bool OnButtonDown( UINT nFlags, CPoint pt, bool bLeft );

// Implementation
protected:
    void HookMessageProc( UINT message, WPARAM wParam, LPARAM lParam );
    bool FrameOnSysCommand( UINT nID, LPARAM lParam );
    bool FrameOnMenuChar( UINT nChar, UINT nFlags, CMenu* pMenu );
    void FrameOnNcActivate( BOOL bActive );
    void FrameOnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu );
    void FrameOnMenuSelect( UINT nItemID, UINT nFlags, HMENU hSysMenu );
    void OpenTheme();
    void CloseTheme();

// Static members
protected:
    static HHOOK m_hMsgHook;
    static CMenuBar* m_pMenuBar;
    static LRESULT CALLBACK MessageProc( int code, WPARAM wParam, LPARAM lParam );

// Implementation data
protected:
    CFont   m_fontMenu;
    CPoint  m_ptMouseLast;

    HWND    m_hWndMDIChild;
    HWND    m_hWndOldFocus;
    HMENU   m_hMenu;
    HMENU   m_hMenuTracking;

    int     m_nItem;
    bool    m_bItemTracking;
    bool    m_bItemDropped;
    bool    m_bPrimaryMenu;
    bool    m_bSubmenuItem;
    bool    m_bEscape;
    bool    m_bContinue;
    bool    m_bSelectFirst;
    bool    m_bButtonCapture;
    bool    m_bFrameActive;

    CTypedPtrArray< CPtrArray, CMenuBarButton* > m_aMenuBarButtons;

    HTHEME  m_hTheme;

// Generated message map functions
protected:
    //{{AFX_MSG(CMenuBar)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnCaptureChanged(CWnd *pWnd);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg UINT OnGetDlgCode();
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    //}}AFX_MSG

    afx_msg LRESULT OnShowPopupMenu( WPARAM wParam, LPARAM lParam );
    afx_msg LRESULT OnReBarChildSize( WPARAM wParam, LPARAM lParam );
    afx_msg LRESULT OnReBarChevronPushed( WPARAM wParam, LPARAM lParam );
    afx_msg LRESULT OnThemeChanged( WPARAM wParam, LPARAM lParam );

    afx_msg void OnUpdateMenuButton( CCmdUI* pCmdUI );

    // Toolbar control notifications
    afx_msg void OnDropDown( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnHotItemChange( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnCustomDraw( NMHDR* pNMHDR, LRESULT* pResult );
    DECLARE_MESSAGE_MAP();
};

/////////////////////////////////////////////////////////////////////////////
#endif	// RC_INVOKED
#endif  // !__MENUBAR_H__
