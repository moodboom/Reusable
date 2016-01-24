/////////////////////////////////////////////////////////////////////////
//
// CEmbeddedFrame            
//
// Created: Mar 12, 2001
// Last Modified: August 03, 2000
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
    only have docking windows in a certain section of the splitter.
    
/*///////////////////////////////////////////////////////////////////////
#if !defined(AFX_EMBEDDEDFRAME_H__A7908055_AB32_472B_A73D_INCLUDED_)
#define AFX_EMBEDDEDFRAME_H__A7908055_AB32_472B_A73D_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
/// Header file for the window that docks within this frame
/////////////////////////////////////////////////////////////////////////////
#include ".\DockingWnd.h"

class CEmbeddedFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CEmbeddedFrame)

// Attributes
public:
	CEmbeddedFrame();     
	virtual ~CEmbeddedFrame();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmbeddedFrame)
	protected:
	virtual void PostNcDestroy();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
    CDockingWnd         m_wndDockingWnd;
	CEdit				m_wndClient;
    CFont               m_font;
	bool				m_bActive;

	// Generated message map functions
	//{{AFX_MSG(CEmbeddedFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
        
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMBEDDEDFRAME_H__A7908055_AB32_472B_A73D_INCLUDED_)
