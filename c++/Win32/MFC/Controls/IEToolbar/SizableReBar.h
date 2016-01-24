//-------------------------------------------------------------------//
// CSizableReBar class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#ifndef __SIZABLEREBAR_H__
#define __SIZABLEREBAR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define IDS_LOCKTOOLBARS							16908
#define ID_LOCKTOOLBARS								16909			// No associated resource, just make sure ID is unique
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

#include <afxtempl.h>
#include <vector>                   // For std::vector<int>

/////////////////////////////////////////////////////////////////////////////
// CSizableReBar

#define IDW_SIZABLEREBAR    ( AFX_IDW_CONTROLBAR_LAST - 1 )

class /*GUILIB_EXT_CLASS*/ CSizableReBar : public CControlBar
{
    DECLARE_DYNAMIC( CSizableReBar );

// Construction
public:
    CSizableReBar();
    ~CSizableReBar();

// Operations
public:
    bool Create( CWnd* pParentWnd, UINT nID, DWORD dwStyle );
    bool AddBar( CWnd* pBar, LPCTSTR pszText = 0, CBitmap* pbmp = 0,
        DWORD dwStyle = RBBS_FIXEDBMP, LPCTSTR lpszTitle = 0,
        bool bAlwaysVisible = true, bool bHasGripper = true, int nStartingWidth = 0 );
    bool AddBar( CWnd* pBar, COLORREF clrFore, COLORREF clrBack, LPCTSTR pszText = 0,
        DWORD dwStyle = 0, LPCTSTR lpszTitle = 0,
        bool bAlwaysVisible = true, bool bHasGripper = true, int nStartingWidth = 0 );
    bool SetBkImage( CBitmap* pbmp );

    void EnableContextMenu( bool bCommonPart = true, bool bCustomPart = true );

    bool Lock( bool bLock = true );
    bool IsLocked();

    void LoadState( LPCTSTR lpszProfileName );
    void SaveState( LPCTSTR lpszProfileName );

    CReBarCtrl& GetReBarCtrl();

	// MDM
	// I need to know what this is set at...
	int nYEdge() { return m_cyEdge; }

   // MDM 
   // I need row heights sorted from top to bottom.
   void GetRowHeights( std::vector<int>& vectHeights );

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSizableReBar)
    public:
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
    virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);
    protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    //}}AFX_VIRTUAL

    virtual void OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHandler );

// Implementation
protected:
    bool DoAddBar( CWnd* pBar, REBARBANDINFO* prbbi, LPCTSTR lpszTitle, bool bAlwaysVisiblem, bool bHasGripper, int nStartingWidth );
    UINT GetSizingEdgeHitCode() const;
    bool GetSizingEdgeRect( const CRect& rcBar, CRect& rcEdge, UINT nHitTest ) const;
    void StartTracking( UINT nHitTest, CPoint pt );
    void StopTracking();
    void OnTrackUpdateSize( CPoint pt );

// Implementation data
protected:
    CReBarCtrl      m_rbCtrl;
    UINT            m_htEdge;
    CSize           m_szOld;
    CPoint          m_ptOld;
    bool            m_bTracking;
    int             m_cxEdge;
    int             m_cyEdge;
    HBITMAP         m_hbmBack;
    bool            m_bCommonPart;
    bool            m_bCustomPart;
    bool            m_bLocked;

    struct BARINFO
    {
        UINT    nID;
        TCHAR   szTitle[ MAX_PATH ];
        bool    bAlwaysVisible;
        bool    bHasGripper;
    };

    CArray< BARINFO, BARINFO& > m_aBars;

    class CFrameWndFriend : public CFrameWnd
    {
        friend CSizableReBar;
    };

    static int m_nStateInfoVersion;
    static LPCTSTR m_lpszStateInfoFormat;
    static LPCTSTR m_lpszStateInfoVersion;
    static LPCTSTR m_lpszStateInfoLocked;
    static LPCTSTR m_lpszStateInfoBand;

// Generated message map functions
protected:
    //{{AFX_MSG(CSizableReBar)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnNcPaint();
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
    afx_msg void OnCaptureChanged(CWnd *pWnd);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG

	// MDM VC8 "fixed" the return value from several macros.
	// I'll make it backwards-compatible for now...
	#if _MSC_VER >= 1400
	    afx_msg LRESULT OnNcHitTest(CPoint point);
	#else
	    afx_msg UINT OnNcHitTest(CPoint point);
	#endif

	afx_msg void OnRecalcParent();

	DECLARE_MESSAGE_MAP();
};

/////////////////////////////////////////////////////////////////////////////
#endif	// RC_INVOKED
#endif  // !__SIZABLEREBAR_H__
