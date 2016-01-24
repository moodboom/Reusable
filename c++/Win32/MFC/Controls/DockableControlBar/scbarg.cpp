//-------------------------------------------------------------------//
// This is the sizing control bar with a gripper (but no caption).
//
//
// NOTE: Derived from code provided by Cristi Posea on 
// CodeGuru, 8/2000.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

// sizecbar.cpp : implementation file
//

#include "stdafx.h"

// MDM	10/6/01 6:03:12 PM
// I don't want to have to include this header in the stdafx.h file!
#include "scbarg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////
// CSizingControlBarG

IMPLEMENT_DYNAMIC(CSizingControlBarG, baseCSizingControlBarG);

CSizingControlBarG::CSizingControlBarG( bool bUseMaxButton )
:
   // Init vars.
   m_bUseMaxButton( bUseMaxButton ),
   m_btnMax( true )

{
    m_cyGripper = 12;
}

CSizingControlBarG::~CSizingControlBarG()
{
}

BEGIN_MESSAGE_MAP(CSizingControlBarG, baseCSizingControlBarG)
    //{{AFX_MSG_MAP(CSizingControlBarG)
    ON_WM_NCLBUTTONUP()
    ON_WM_NCHITTEST()
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////
// CSizingControlBarG message handlers

/////////////////////////////////////////////////////////////////////////
// Mouse Handling
//

void CSizingControlBarG::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
    if (nHitTest == HTCLOSE)
    {
        m_pDockSite->ShowControlBar(this, FALSE, FALSE); // hide
        
        // This is for hide post-processing, it's up to derived classes to actually
        // do anything...
        OnHideBar();
    }

   // MDM
   if ( nHitTest == HTMAXBUTTON)
   {
      // "Maximize" this control bar.
      // This is up to the derived class to implement.
      MaximizeBar();
   }

    baseCSizingControlBarG::OnNcLButtonUp(nHitTest, point);
}

void CSizingControlBarG::NcCalcClient(LPRECT pRc, UINT nDockBarID)
{
    CRect rcBar(pRc); // save the bar rect

    // subtract edges
    baseCSizingControlBarG::NcCalcClient(pRc, nDockBarID);

    if (!HasGripper())
        return;

    CRect rc(pRc); // the client rect as calculated by the base class

    BOOL bHorz = (nDockBarID == AFX_IDW_DOCKBAR_TOP) ||
                 (nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);

    if (bHorz)
        rc.DeflateRect(m_cyGripper, 0, 0, 0);
    else
        rc.DeflateRect(0, m_cyGripper, 0, 0);

    // set position for the "x" (hide bar) button
    CPoint ptOrgBtn;
    if (bHorz)
        ptOrgBtn = CPoint(rc.left - 13, rc.top);
    else
        ptOrgBtn = CPoint(rc.right - 12, rc.top - 13);

    m_biHide.Move(ptOrgBtn - rcBar.TopLeft());

   // MDM
   // Now move the max button.
   if ( m_bUseMaxButton )
   {
      if (bHorz)
         ptOrgBtn = CPoint(rc.left - 13 - 13, rc.top);
      else
         ptOrgBtn = CPoint(rc.right - 12 - 13, rc.top - 13); // MDM huh?

      m_btnMax.Move(ptOrgBtn - rcBar.TopLeft());
   }

    *pRc = rc;
}

void CSizingControlBarG::NcPaintGripper(CDC* pDC, CRect rcClient)
{
    if (!HasGripper())
        return;

    // paints a simple "two raised lines" gripper
    // override this if you want a more sophisticated gripper
    CRect gripper = rcClient;
    CRect rcbtn = m_bUseMaxButton ? m_btnMax.GetRect() : m_biHide.GetRect();
    BOOL bHorz = IsHorzDocked();

    gripper.DeflateRect(1, 1);
    if (bHorz)
    {   // gripper at left
        gripper.left -= m_cyGripper;
        gripper.right = gripper.left + 3;
        gripper.top = rcbtn.bottom + 3;
    }
    else
    {   // gripper at top
        gripper.top -= m_cyGripper;
        gripper.bottom = gripper.top + 3;
        gripper.right = rcbtn.left - 3;
    }

    pDC->Draw3dRect(gripper, ::GetSysColor(COLOR_BTNHIGHLIGHT),
        ::GetSysColor(COLOR_BTNSHADOW));

    gripper.OffsetRect(bHorz ? 3 : 0, bHorz ? 0 : 3);

    pDC->Draw3dRect(gripper, ::GetSysColor(COLOR_BTNHIGHLIGHT),
        ::GetSysColor(COLOR_BTNSHADOW));

    m_biHide.Paint(pDC);

   // MDM
   // Draw the max button as needed.
   if ( m_bUseMaxButton && !IsFloating() )
		m_btnMax.Paint(pDC);
}


// MDM VC8 "fixed" the return value from several macros.
// I'll make it backwards-compatible for now...
#if _MSC_VER >= 1400
	LRESULT CSizingControlBarG::OnNcHitTest(CPoint point)
#else
	UINT CSizingControlBarG::OnNcHitTest(CPoint point)
#endif
{
    CRect rcBar;
    GetWindowRect(rcBar);

    UINT nRet = baseCSizingControlBarG::OnNcHitTest(point);
    if (nRet != HTCLIENT)
        return nRet;

    CRect rc = m_biHide.GetRect();
    rc.OffsetRect(rcBar.TopLeft());
    if (rc.PtInRect(point))
        return HTCLOSE;

   if ( m_bUseMaxButton )
   {
      rc = m_btnMax.GetRect();
      rc.OffsetRect(rcBar.TopLeft());
      if (rc.PtInRect(point))
         return HTMAXBUTTON;
   }

    return HTCLIENT;
}

/////////////////////////////////////////////////////////////////////////
// CSizingControlBarG implementation helpers

void CSizingControlBarG::OnUpdateCmdUI(CFrameWnd* pTarget,
                                      BOOL bDisableIfNoHndler)
{
    UNUSED_ALWAYS(bDisableIfNoHndler);
    UNUSED_ALWAYS(pTarget);

    if (!HasGripper())
        return;

    BOOL bNeedPaint = FALSE;

    CPoint pt;
    ::GetCursorPos(&pt);
    BOOL bHit = (OnNcHitTest(pt) == HTCLOSE);
    BOOL bLButtonDown = (::GetKeyState(VK_LBUTTON) < 0);

    BOOL bWasPushed = m_biHide.bPushed;
    m_biHide.bPushed = bHit && bLButtonDown;

    BOOL bWasRaised = m_biHide.bRaised;
    m_biHide.bRaised = bHit && !bLButtonDown;

    bNeedPaint |= (m_biHide.bPushed ^ bWasPushed) ||
                  (m_biHide.bRaised ^ bWasRaised);

   // MDM
   // Now the max button.
   if ( m_bUseMaxButton )
   {
      bHit = (OnNcHitTest(pt) == HTMAXBUTTON);
      bWasPushed = m_btnMax.bPushed;
      m_btnMax.bPushed = bHit && bLButtonDown;

      bWasRaised = m_btnMax.bRaised;
      m_btnMax.bRaised = bHit && !bLButtonDown;

      bNeedPaint |= (m_btnMax.bPushed ^ bWasPushed) ||
                  (m_btnMax.bRaised ^ bWasRaised);
   }

    if (bNeedPaint)
        SendMessage(WM_NCPAINT);
}

/////////////////////////////////////////////////////////////////////////
// CSCBButton

CSCBButton::CSCBButton( bool bMax )
:
   // Init vars.
   m_bMax( bMax )
{
    bRaised = FALSE;
    bPushed = FALSE;
}

void CSCBButton::Paint(CDC* pDC)
{
    CRect rc = GetRect();

    if (bPushed)
        pDC->Draw3dRect(rc, ::GetSysColor(COLOR_BTNSHADOW),
            ::GetSysColor(COLOR_BTNHIGHLIGHT));
    else
        if (bRaised)
            pDC->Draw3dRect(rc, ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));

    COLORREF clrOldTextColor = pDC->GetTextColor();
    pDC->SetTextColor(::GetSysColor(COLOR_BTNTEXT));
    int nPrevBkMode = pDC->SetBkMode(TRANSPARENT);
    CFont font;
    int ppi = pDC->GetDeviceCaps(LOGPIXELSX);
    int pointsize = MulDiv(60, 96, ppi); // 6 points at 96 ppi
    font.CreatePointFont(pointsize, _T("Marlett"));
    CFont* oldfont = pDC->SelectObject(&font);

   pDC->TextOut(
      ptOrg.x + 2, 
      ptOrg.y + 2, 
            m_bMax
         ?  CString(_T("0"))     // max-like
         :  CString(_T("r"))     // x-like
   ); 

    pDC->SelectObject(oldfont);
    pDC->SetBkMode(nPrevBkMode);
    pDC->SetTextColor(clrOldTextColor);
}

BOOL CSizingControlBarG::HasGripper() const
{
#if defined(_SCB_MINIFRAME_CAPTION) || !defined(_SCB_REPLACE_MINIFRAME)
    // if the miniframe has a caption, don't display the gripper
    if (IsFloating())
        return FALSE;
#endif //_SCB_MINIFRAME_CAPTION

    return TRUE;
}