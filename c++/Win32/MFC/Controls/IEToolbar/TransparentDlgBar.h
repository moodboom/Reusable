//-------------------------------------------------------------------//
// TransparentDlgBar class
//-------------------------------------------------------------------//
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//

#if !defined(TRANSPARENT_DLG_BAR_H)
#define TRANSPARENT_DLG_BAR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define ID_TRANSPARENT						16928			// No associated resource, just make sure ID is unique
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


#include "..\..\BackgroundDlg\BackgroundUtil.h"


class TransparentDlgBar : public CDialogBar, public CBackgroundUtil
{
	typedef CDialogBar inherited;

    DECLARE_DYNAMIC( TransparentDlgBar )
public:
	TransparentDlgBar();

	CDC* m_pDC;

// Implementation data
protected:
    bool m_bTransparent;

// Message map functions
protected:
    afx_msg void OnTransparent();
    afx_msg LRESULT OnReBarContextMenu( WPARAM wParam, LPARAM lParam );
    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
    afx_msg void OnMove( int x, int y );
    afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
    DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif	// RC_INVOKED
#endif // !defined(TRANSPARENT_DLG_BAR_H)
