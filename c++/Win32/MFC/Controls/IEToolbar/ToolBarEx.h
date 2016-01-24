//-------------------------------------------------------------------//
// CToolBarEx class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#ifndef __TOOLBAREX_H__
#define __TOOLBAREX_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define IDS_CUSTOMIZE								16910
#define ID_CUSTOMIZE									16911			// No associated resource, just make sure ID is unique
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


#include "CustomizeDialog.h"

#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CToolBarEx

struct TBBUTTONEX
{
    TBBUTTON tbinfo;
    bool     bInitiallyVisible;
};

class /*GUILIB_EXT_CLASS*/ CToolBarEx : public CToolBar
{
    DECLARE_DYNAMIC( CToolBarEx );

    friend class CToolBarPopup;
    friend class CCustomizeDialog;

// Construction
public:
    CToolBarEx();
    ~CToolBarEx();

    BOOL Create( CWnd* pParentWnd, DWORD dwStyle, UINT nID = AFX_IDW_TOOLBAR );

// Attributes
public:
    void SetTextOptions( ETextOptions eTextOptions, bool bUpdate = true );
    ETextOptions GetTextOptions() const;
    void SetIconOptions( EIconOptions eIconOptions, bool bUpdate = true );
    EIconOptions GetIconOptions() const;

// Operations
public:

	void SetBitmaps( 
		UINT nIDSmallCold, 
		UINT nIDSmallHot, 
		UINT nIDSmallDisabled,
      UINT nIDLargeCold, 
		UINT nIDLargeHot, 
		UINT nIDLargeDisabled,
		EIconOptions eIconOptionsDefault, 
		CSize szImageSmall,
		CSize szImageLarge,	
		COLORREF clrMask = RGB( 255, 0, 255 ) 
	);

	void SetButtons( 
		int nNumButtons, 
		TBBUTTONEX* lpButtons,
      ETextOptions eTextOptionsDefault 
	);

	void LoadState( LPCTSTR lpszProfileName );
	void SaveState( LPCTSTR lpszProfileName );

	CSize GetSmallImageSize() { return m_szImageSmall; }
	CSize GetLargeImageSize() { return m_szImageLarge; }

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CToolBarEx)
    protected:
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //}}AFX_VIRTUAL

    virtual LRESULT DoCustomDraw( NMHDR* pNMHDR, CWnd* pToolBar );

    virtual bool HasButtonText( UINT nID );
    virtual bool HasButtonTip( UINT nID );
    virtual void GetButtonText( UINT nID, CString& strText );
    virtual void GetButtonTip( UINT nID, CString& strTip );

public:
    virtual void Init();
    virtual bool IsTextOptionAvailable( ETextOptions eTextOptions ) const;
    virtual bool IsIconOptionAvailable( EIconOptions eIconOptions ) const;

	// MDM	9/28/2004 8:47:08 PM
	void PushButtonManually( UINT nID, bool bPushed = true );

	void DisableCustomize() { m_bAllowCustomize = false; }

    // Returns true if the toolbar button uButtonId is in a down state
    bool IsToolbarButtonDown(UINT uButtonId);

// Implementation
protected:
   CReBarCtrl& GetParentReBarCtrl() const;
   int GetParentBandIndex() const;
	void ReloadButtons();
	void UpdateParentBandInfo();
   bool GetButtonInfo( UINT nID, TBBUTTON& tbinfo );

   static LRESULT CALLBACK CBTProc( int nCode, WPARAM wParam, LPARAM lParam );

// Implementation data
protected:
    CArray< TBBUTTONEX, TBBUTTONEX& > m_aButtons;

    ETextOptions    m_eTextOptions;
    EIconOptions    m_eIconOptions;
    ETextOptions    m_eTextOptionsDefault;
    EIconOptions    m_eIconOptionsDefault;
    UINT            m_nIDSmallCold;
    UINT            m_nIDSmallHot;
    UINT            m_nIDSmallDisabled;
    UINT            m_nIDLargeCold;
    UINT            m_nIDLargeHot;
    UINT            m_nIDLargeDisabled;
    CImageList      m_imageListCold;
    CImageList      m_imageListHot;
    CImageList      m_imageListDisabled;
    COLORREF        m_clrMask;
    HGLOBAL         m_hToolbarData;

    static LPCTSTR              m_lpszStateInfoEntry;
    static CToolBarEx*          m_pToolBar;
    static CCustomizeDialog*    m_pCustomizeDlg;
    static HHOOK                m_hCBTHook;

	CSize m_szImageSmall;
	CSize m_szImageLarge;

	bool m_bAllowCustomize;

// Generated message map functions
protected:
    //{{AFX_MSG(CToolBarEx)
    afx_msg void OnCustomize();
    //}}AFX_MSG

    afx_msg LRESULT OnReBarContextMenu( WPARAM wParam, LPARAM lParam );
    afx_msg LRESULT OnReBarChevronPushed( WPARAM wParam, LPARAM lParam );

    afx_msg void OnCustomDraw( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnGetInfoTip( NMHDR* pNMHDR, LRESULT* pResult );

    // Toolbar customization
    afx_msg void OnBeginAdjust( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnEndAdjust( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnQueryInsert( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnQueryDelete( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnInitCustomize( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnGetButtonInfo( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnReset( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnToolBarChange( NMHDR* pNMHDR, LRESULT* pResult );

    // Saving and restoring toolbar
    afx_msg void OnSave( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnRestore( NMHDR* pNMHDR, LRESULT* pResult );
    DECLARE_MESSAGE_MAP();
};

/////////////////////////////////////////////////////////////////////////////
// CToolBarPopup

class CToolBarPopup : public CWnd
{
    DECLARE_DYNAMIC( CToolBarPopup );

// Construction
public:
    CToolBarPopup( CToolBarEx* pToolBar );
    ~CToolBarPopup();

// Operations
public:
    bool ShowPopup( UINT nFlags, CPoint pt, CRect& rcExclude );

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CToolBarPopup)
    protected:
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    //}}AFX_VIRTUAL

// Implementation
protected:
    void SetPosition( UINT nFlags, CPoint pt, CRect& rcExclude );
    void Show( UINT nFlags, const CRect& rc );
    void OnKeyDown( UINT nChar );
    void DrawBorder( CDC* pDC );

    static LRESULT CALLBACK KeyboardProc( int code, WPARAM wParam, LPARAM lParam );

// Implementation data
protected:
    CToolBarEx*     m_pToolBar;
    CToolBarCtrl    m_tbCtrl;
    bool            m_bOverTbCtrl;
    bool            m_bTextLabels;
    bool            m_bFlatMenu;

    static HHOOK            m_hKeyboardHook;
    static CToolBarPopup*   m_pPopup;

// Generated message map functions
protected:
    //{{AFX_MSG(CToolBarPopup)
    afx_msg void OnCaptureChanged(CWnd *pWnd);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnNcPaint();
    //}}AFX_MSG
    afx_msg LRESULT OnPrint( WPARAM wParam, LPARAM lParam );
    afx_msg void OnEndModalLoop();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif	// RC_INVOKED
#endif  // !__MENUBAR_H__
