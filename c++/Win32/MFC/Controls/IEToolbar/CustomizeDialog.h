//-------------------------------------------------------------------//
// CCustomizeDialog class
//-------------------------------------------------------------------//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#ifndef __CUSTOMIZEDIALOG_H__
#define __CUSTOMIZEDIALOG_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define IDD_CUSTOMIZE							120		// 16899
#define IDC_CB_TEXTOPTIONS						16900
#define IDC_CB_ICONOPTIONS						16901
#define IDS_TO_TEXTLABELS						16902
#define IDS_TO_TEXTONRIGHT						16903
#define IDS_TO_NOTEXTLABELS					16904
#define IDS_IO_SMALLICONS						16905
#define IDS_IO_LARGEICONS						16906
#define IDS_SEPARATOR							16907

#define IDC_LB_AVAILABLE    0x00C9  // determined with Spy++
#define IDC_LB_CURRENT      0x00CB

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

// #include "Resource.h"

#include "..\..\FontDlg.h"													// Base class, allows custom font.

#include "..\..\GlobalData.h"

/////////////////////////////////////////////////////////////////////////////
// Options

enum ETextOptions
{
    toTextLabels    =  0,
    toTextOnRight   =  1,
    toNoTextLabels  =  2,
    toNone          = -1,
};

enum EIconOptions
{
    ioSmallIcons    =  0,
    ioLargeIcons    =  1,
    ioNone          = -1,
};

/////////////////////////////////////////////////////////////////////////////
// COptionsDialog dialog

class CCustomizeDialog;

class COptionsDialog : public FontDlg
{
	typedef FontDlg inherited;

// Construction
public:
    COptionsDialog( ETextOptions eTextOptions,
                    EIconOptions eIconOptions );

// Dialog Data
    //{{AFX_DATA(COptionsDialog)
    enum { IDD = IDD_CUSTOMIZE };
    CComboBox m_cbTextOptions;
    CComboBox m_cbIconOptions;
    //}}AFX_DATA

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(COptionsDialog)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Operations
public:
    bool SelectTextOption( ETextOptions eTextOptions );
    bool SelectIconOption( EIconOptions eIconOptions );

// Implementation
protected:
    CCustomizeDialog* GetCustomizeDialog() const;

// Implementation data
protected:
    ETextOptions m_eTextOptions;
    EIconOptions m_eIconOptions;

// Generated message map functions
protected:
    //{{AFX_MSG(COptionsDialog)
    virtual BOOL OnInitDialog();
    afx_msg void OnTextOptions();
    afx_msg void OnIconOptions();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CCustomizeDialog dialog

class CToolBarEx;

class CCustomizeDialog : public CWnd
{
    DECLARE_DYNAMIC( CCustomizeDialog );

// Construction
public:
    CCustomizeDialog( CToolBarEx* pToolBar );

// Operations
public:
    void SetTextOptions( ETextOptions eTextOptions, bool bInDialog );
    void SetIconOptions( EIconOptions eIconOptions, bool bInDialog );

    void AddTextOption( CComboBox& cbTextOptions, ETextOptions eTextOptions, UINT nStringID );
    void AddIconOption( CComboBox& cbIconOptions, EIconOptions eIconOptions, UINT nStringID );

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CCustomizeDialog)
    protected:
    virtual void PostNcDestroy();
    //}}AFX_VIRTUAL

// Implementation
protected:
    CSize GetButtonSize() const;

// Implementation data
protected:
    COptionsDialog  m_dlgOptions;
    CToolBarEx*     m_pToolBar;

// Generated message map functions
protected:
    //{{AFX_MSG(CCustomizeDialog)
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    //}}AFX_MSG
    LRESULT OnInitDialog( WPARAM wParam, LPARAM lParam );
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif	// RC_INVOKED
#endif  // !__CUSTOMIZEDIALOG_H__
