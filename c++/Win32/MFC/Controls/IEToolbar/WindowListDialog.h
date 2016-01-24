//-------------------------------------------------------------------//
// CWindowListDialog class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//

#ifndef __WINDOWLISTDIALOG_H__
#define __WINDOWLISTDIALOG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define IDD_WINDOW_LIST							16916
#define IDS_WINDOW_LIST							16917
#define IDC_BN_MINIMIZE							16918
#define IDC_BN_TILE_VERT						16919
#define IDC_BN_TILE_HORZ						16920
#define IDC_BN_CASCADE							16921
#define IDC_BN_CLOSE_WIN						16922
#define IDC_BN_ACTIVATE							16923
#define IDC_LST_WINDOWS							16924
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

#include "..\..\FontDlg.h"													// Base class, allows custom font.

/////////////////////////////////////////////////////////////////////////////
// CWindowListDialog dialog

class CWindowListDialog : public FontDlg
{
	typedef FontDlg inherited;

// Construction
public:
    CWindowListDialog( CWnd* pMDIClientWnd, CWnd* pParent = 0 );

// Dialog Data
    //{{AFX_DATA(CWindowListDialog)
    enum { IDD = IDD_WINDOW_LIST };
    CButton m_bnMinimize;
    CButton m_bnTileVert;
    CButton m_bnTileHorz;
    CButton m_bnCascade;
    CButton m_bnCloseWin;
    CButton m_bnActivate;
    CListCtrl   m_lstWindows;
    //}}AFX_DATA

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CWindowListDialog)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    void ReloadWindowList();
    void Activate( int nItem );
    void ArrangeWindows( UINT nMsg, WPARAM wParam = 0, LPARAM lParam = 0 );

// Implementation data
protected:
    CWnd*       m_pMDIClientWnd;
    CImageList  m_imageList;

// Generated message map functions
protected:
    //{{AFX_MSG(CWindowListDialog)
    virtual BOOL OnInitDialog();
    afx_msg void OnActivate();
    afx_msg void OnCloseWin();
    afx_msg void OnCascade();
    afx_msg void OnTileHorz();
    afx_msg void OnTileVert();
    afx_msg void OnMinimize();
    afx_msg void OnDblClkWindows(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    afx_msg void OnKickIdle();
	afx_msg void OnSaveWin();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif	// RC_INVOKED
#endif // !__WINDOWLISTDIALOG_H__
