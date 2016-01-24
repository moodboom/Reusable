//-------------------------------------------------------------------//
//
// NOTE: Derived from code provided by Cristi Posea on 
// CodeGuru, 8/2000.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined(__SCBARG_H__)
#define __SCBARG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// MDM	10/6/01 6:03:12 PM
// I don't want to have to include this header in the stdafx.h file!
#include "sizecbar.h"								// Base class

/////////////////////////////////////////////////////////////////////////
// CSCBButton (button info) helper class

class CSCBButton
{
public:
    CSCBButton( bool bMax = false );

    void Move(CPoint ptTo) {ptOrg = ptTo; };
    CRect GetRect() { return CRect(ptOrg, CSize(11, 11)); };
    void Paint(CDC* pDC);

    BOOL    bPushed;
    BOOL    bRaised;

protected:
    CPoint  ptOrg;

   // MDM 
   // Added this so we can track if this is a CLOSE button or a MAX button.
   // Cheap and slutty, I know...
   bool m_bMax;
};

/////////////////////////////////////////////////////////////////////////
// CSizingControlBar control bar

#ifndef baseCSizingControlBarG
#define baseCSizingControlBarG CSizingControlBar
#endif

class CSizingControlBarG : public baseCSizingControlBarG
{
    DECLARE_DYNAMIC(CSizingControlBarG);

// Construction
public:
    CSizingControlBarG( bool bUseMaxButton = false );

// Attributes
public:
    virtual BOOL HasGripper() const;

// Operations
public:

// Overridables
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

// Overrides
public:
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSizingControlBarG)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CSizingControlBarG();
    
protected:
    // implementation helpers
    virtual void NcPaintGripper(CDC* pDC, CRect rcClient);
    virtual void NcCalcClient(LPRECT pRc, UINT nDockBarID);

   // MDM
   // This can be triggered in a derived class (CSizingControlBarCF), 
   // and it's up to a (typically further-)derived class to override 
   // and respond, as well.
   virtual void MaximizeBar() {}

   // MDM
   // This is for hide post-processing, it's up to derived classes to actually
   // do anything...
   virtual void OnHideBar() {}

protected:
    int     m_cyGripper;

    CSCBButton m_biHide;

   // MDM
   // Here, we'll draw a max or restore button to allow
   // one control bar to take up the entire client area.
   // If clicked, we call MaximizeBar() and the derived
   // class can do as it pleases.
   bool m_bUseMaxButton;
   CSCBButton m_btnMax;

// Generated message map functions
protected:
    //{{AFX_MSG(CSizingControlBarG)

	// MDM VC8 "fixed" the return value from several macros.
	// I'll make it backwards-compatible for now...
	#if _MSC_VER >= 1400
		afx_msg LRESULT OnNcHitTest(CPoint point);
	#else
		afx_msg UINT OnNcHitTest(CPoint point);
	#endif

	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

#endif // !defined(__SCBARG_H__)

