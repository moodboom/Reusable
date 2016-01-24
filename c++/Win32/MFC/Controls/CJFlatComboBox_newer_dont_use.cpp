// CJFlatComboBox.cpp : implementation file
// Copyright © 1998-1999 CodeJock.com, All Rights Reserved.
// See ReadMe.txt for TERMS OF USE.
//
/////////////////////////////////////////////////////////////////////////////
/****************************************************************************
 *
 * $Date: 2002/08/23 12:01:17 $
 * $Revision: 1.1 $
 * $Archive: /CodeJock/CJLibrary/CJFlatComboBox.cpp $
 *
 * $History: CJFlatComboBox.cpp $
 * 
 * *****************  Version 15  *****************
 * User: Kirk Stowell Date: 10/30/99   Time: 6:01p
 * Updated in $/CodeJock/CJLibrary
 * Overrode OnEraseBkgnd(...) and re-wrote OnPaint() to handle flicker
 * free drawing.
 * 
 * *****************  Version 14  *****************
 * User: Kirk Stowell Date: 10/30/99   Time: 2:14p
 * Updated in $/CodeJock/CJLibrary
 * Added COLORREF members to class to eliminate continuous calls to
 * GetSysColor(...). Overrode OnSysColorChange() to re-initialize COLORREF
 * variables. 
 * 
 * *****************  Version 13  *****************
 * User: Kirk Stowell Date: 8/31/99    Time: 1:11a
 * Updated in $/CodeJockey/CJLibrary
 * Updated copyright and contact information.
 * 
 * *****************  Version 12  *****************
 * User: Kirk Stowell Date: 8/30/99    Time: 12:50a
 * Updated in $/CodeJockey/CJLibrary
 * Hiren Patel <hpatel@spss.com> - Added ReleaseDC(pDC) when window is
 * disabled.
 * 
 * *****************  Version 11  *****************
 * User: Kirk Stowell Date: 7/25/99    Time: 10:00p
 * Updated in $/CodeJockey/CJLibrary
 * 
 * *****************  Version 10  *****************
 * User: Kirk Stowell Date: 7/25/99    Time: 12:30a
 * Updated in $/CodeJockey/CJLibrary
 * 
 * *****************  Version 9  *****************
 * User: Kirk Stowell Date: 5/17/99    Time: 9:24p
 * Updated in $/CodeJockey/CJ60Lib
 * Gilberto Botaro [seafox@techno.com.br] - Added code for auto
 * completion.
 * 
 * *****************  Version 8  *****************
 * User: Kirk Stowell Date: 5/03/99    Time: 10:44p
 * Updated in $/CodeJockey/CJ60Lib
 * Fixed code ommisions.
 * 
 * *****************  Version 7  *****************
 * User: Kirk Stowell Date: 5/03/99    Time: 8:42p
 * Updated in $/CodeJockey/CJ60Lib
 * Danilo Lax [Danilo_Lax@www2.alpeng.com] - The control is painted flat
 * when the mouse leaves the box, even if the control has the focus, now
 * checking for the focus in OnTimer(.
 * 
 * Removed, unhandled msgs are processed by
 * CComboBox::PreTranslateMessage(pMsg). Used reflected msgs
 * BN_GETFOCUS/CBN_KILLFOCUS instead of WM_GETFOCUS/WM_KILLFOCUS.
 * 
 * ::GetFocus() doesn't return CFlatComboBox::m_hWnd, even if the control
 * has the focus (don't know why, maybe ::GetFocus() only returns the
 * handle to the EditField part of the ComboBox), used flag m_bHasFocus
 * instead.
 * 
 * *****************  Version 6  *****************
 * User: Kirk Stowell Date: 4/03/99    Time: 4:26p
 * Updated in $/CodeJockey/CJ60Lib
 * Added comments and cleaned up code.
 * 
 * *****************  Version 5  *****************
 * User: Kirk Stowell Date: 1/22/99    Time: 4:26p
 * Updated in $/CodeJockey/CJ60Lib
 * Fixed accelerator problem reported by YOSH YAMASHITA
 * (yosh@muj.biglobe.ne.jp) with Matt Weagle's solution of overloading
 * PreTranslateMessage().
 * 
 * *****************  Version 4  *****************
 * User: Kirk Stowell Date: 1/02/99    Time: 4:25p
 * Updated in $/CodeJockey/CJ60Lib
 * Renamed class from CJComboBox to CCJFlatComboBox, changed DRAWSTATE to
 * enum, removed method Offset(), and replaced with a direct call to
 * ::GetSystemMetrics().
 * 
 * *****************  Version 3  *****************
 * User: Kirk Stowell Date: 11/02/98   Time: 4:24p
 * Updated in $/CodeJockey/CJ60Lib
 * Added OnSetFocus(), OnKillFocus() and modified OnPaint() so the control
 * remains beveled until it actually looses focus - (Vachik
 * Hovhannissian).
 * 
 * *****************  Version 2  *****************
 * User: Kirk Stowell Date: 10/05/98   Time: 4:24p
 * Updated in $/CodeJockey/CJ60Lib
 * Fixed offset bug to use GetSystemMetrics(), instead of hard coded
 * values -
 * Todd Brannam (tbrannam@msn.com).
 * 
 * *****************  Version 1  *****************
 * User: Kirk Stowell Date: 10/02/98   Time: 4:23p
 * Created in $/CodeJockey/CJ60Lib
 * First release version.
 *
 ***************************************************************************/
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CJFlatComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCJFlatComboBox

CCJFlatComboBox::CCJFlatComboBox()
{
	m_bLBtnDown		= FALSE;
	m_bPainted		= FALSE;
	m_bHasFocus		= FALSE;
	m_bAutoComp		= FALSE;
	m_clrHilite		= ::GetSysColor(COLOR_BTNHIGHLIGHT);
	m_clrShadow		= ::GetSysColor(COLOR_BTNSHADOW);
	m_clrNormal		= ::GetSysColor(COLOR_BTNFACE);
}

CCJFlatComboBox::~CCJFlatComboBox()
{
}

IMPLEMENT_DYNAMIC(CCJFlatComboBox, CComboBox)

BEGIN_MESSAGE_MAP(CCJFlatComboBox, CComboBox)
	//{{AFX_MSG_MAP(CCJFlatComboBox)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_CONTROL_REFLECT(CBN_SETFOCUS,   OnSetFocus)
	ON_CONTROL_REFLECT(CBN_KILLFOCUS,  OnKillFocus)
	ON_CONTROL_REFLECT(CBN_EDITUPDATE, OnEditUpdate)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCJFlatComboBox message handlers

void CCJFlatComboBox::OnMouseMove(UINT nFlags, CPoint point) 
{
	SetTimer(1,10,NULL);
	OnTimer(1);
	CComboBox::OnMouseMove(nFlags, point);
}

void CCJFlatComboBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bLBtnDown = TRUE;
	CComboBox::OnLButtonDown(nFlags, point);
}

void CCJFlatComboBox::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bLBtnDown = FALSE;
	Invalidate();
	CComboBox::OnLButtonUp(nFlags, point);
}

void CCJFlatComboBox::OnTimer(UINT nIDEvent) 
{
	UNUSED_ALWAYS(nIDEvent);

	if (m_bLBtnDown==TRUE) { // OnLButtonDown, show PRESSED.
		KillTimer (1);
		if (m_bPainted == TRUE) {
			DrawCombo(PRESSED, m_clrShadow,	m_clrHilite);
			m_bPainted = FALSE;
		}
	} else if (!PointInRect() && !m_bHasFocus) { // If mouse leaves, show flat.
		KillTimer (1);
		if (m_bPainted == TRUE) {
			DrawCombo(NORMAL, m_clrNormal, m_clrNormal);
			m_bPainted = FALSE;
		}
	} else { // On mouse over, show RAISED.
		if (m_bPainted == TRUE) {
			return;
		} else {
			m_bPainted = TRUE;
			DrawCombo(RAISED, m_clrShadow, m_clrHilite);
		}
	}
}

void CCJFlatComboBox::OnPaint() 
{
	ModifyStyleEx (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE,
		0, SWP_FRAMECHANGED);

	CPaintDC dc(this);

	// KStowell - Get the client rect.
	CRect rcClient;
	GetClientRect( &rcClient );

	// KStowell - Create a memory device-context. This is done to help reduce
	// screen flicker, since we will paint the entire control to the
	// off screen device context first.
	CDC memDC;
	CBitmap bitmap;
	memDC.CreateCompatibleDC(&dc);
	bitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	// let the control do its default drawing.
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );

	if (PointInRect() || m_bHasFocus) {
		DrawCombo(RAISED, m_clrShadow, m_clrHilite, &memDC);
	} else {
		DrawCombo(NORMAL, m_clrNormal, m_clrNormal, &memDC);
	}

	// KStowell - Copy the memory device context back into the original DC via BitBlt().
	dc.BitBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), &memDC, 0,0, SRCCOPY);

	// KStowell - Cleanup resources.
	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
	bitmap.DeleteObject();
}

void CCJFlatComboBox::DrawCombo(STATE eState, COLORREF clrTopLeft, COLORREF clrBottomRight, CDC* pDC)
{
	CRect rcItem;
	GetClientRect(&rcItem);
	bool bRelease = false;

	// if no device context was passed in, get the device context
	// for the CWnd client area ...
	if(pDC == NULL) {
		pDC = GetDC();
		bRelease = true;
	}

	// Cover up dark 3D shadow.
	pDC->Draw3dRect(rcItem, clrTopLeft, clrBottomRight);
	rcItem.DeflateRect(1,1);
	
	if (!IsWindowEnabled()) {
		pDC->Draw3dRect(rcItem, m_clrHilite, m_clrHilite);
	} else {
		pDC->Draw3dRect(rcItem, m_clrNormal, m_clrNormal);
	}

	// Cover up dark 3D shadow on drop arrow.
	rcItem.DeflateRect(1,1);
	rcItem.left = rcItem.right-::GetSystemMetrics(SM_CXHTHUMB);;
	pDC->Draw3dRect(rcItem, m_clrNormal, m_clrNormal);
	
	// Cover up NORMAL 3D shadow on drop arrow.
	rcItem.DeflateRect(1,1);
	pDC->Draw3dRect(rcItem, m_clrNormal, m_clrNormal);
	
	if (!IsWindowEnabled()) {
		if(bRelease == true) {
			ReleaseDC(pDC);
		}
		return;
	}

	rcItem.InflateRect(0,1);
	switch (eState)
	{
	case NORMAL:
		pDC->Draw3dRect(rcItem, m_clrHilite, m_clrHilite);
		rcItem.left -= 1;
		pDC->Draw3dRect(rcItem, m_clrHilite, m_clrHilite);
		break;

	case RAISED:
		pDC->Draw3dRect(rcItem, m_clrHilite, m_clrShadow);
		break;

	case PRESSED:
		rcItem.OffsetRect(1,1);
		pDC->Draw3dRect(rcItem, m_clrShadow, m_clrHilite);
		break;
	}

	// KStowell - cleanup.
	if(bRelease == true) {
		ReleaseDC(pDC);
	}
}

void CCJFlatComboBox::OnSetFocus()
{
	m_bHasFocus = TRUE;
	DrawCombo(RAISED, m_clrShadow, m_clrHilite);
}

void CCJFlatComboBox::OnKillFocus() 
{
	m_bHasFocus = FALSE;
	DrawCombo(NORMAL, m_clrNormal, m_clrNormal);	
}

BOOL CCJFlatComboBox::PreTranslateMessage(MSG* pMsg) 
{
	if(IsWin95()) // alexander Golde [alex@viona.de]
	{
		// Make sure that the keystrokes continue to the appropriate handlers
		if (pMsg->message == WM_KEYDOWN || pMsg->message==WM_KEYUP)
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
		
		// Catch the Alt key so we don't choke if focus 
		// is going to an owner drawn button
		if (pMsg->message == WM_SYSCHAR)
			return TRUE;
	}
		
	return CComboBox::PreTranslateMessage(pMsg);
}

void CCJFlatComboBox::OnEditUpdate() 
{
	// if we are not to auto update the text, get outta here
	if (!m_bAutoComp) 
		return;
	
	// Get the text in the edit box
	CString str;
	GetWindowText(str);
	int nLength = str.GetLength();
	
	// Currently selected range
	DWORD dwCurSel = GetEditSel();
	WORD dStart = LOWORD(dwCurSel);
	WORD dEnd   = HIWORD(dwCurSel);
	
	// Search for, and select in, and string in the combo box that is prefixed
	// by the text in the edit box
	if (SelectString(-1, str) == CB_ERR)
	{
		SetWindowText(str);		// No text selected, so restore what was there before
		if (dwCurSel != CB_ERR)
			SetEditSel(dStart, dEnd);	//restore cursor postion
	}
	
	// Set the text selection as the additional text that we have added
	if (dEnd < nLength && dwCurSel != CB_ERR)
		SetEditSel(dStart, dEnd);
	else
		SetEditSel(nLength, -1);
}

BOOL CCJFlatComboBox::IsWin95()
{
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof(info);
	GetVersionEx( &info );
	return ((VER_PLATFORM_WIN32_WINDOWS == info.dwPlatformId) && (info.dwMinorVersion == 0));
}

void CCJFlatComboBox::OnSysColorChange() 
{
	CComboBox::OnSysColorChange();

	m_clrHilite	= ::GetSysColor(COLOR_BTNHIGHLIGHT);
	m_clrShadow	= ::GetSysColor(COLOR_BTNSHADOW);
	m_clrNormal	= ::GetSysColor(COLOR_BTNFACE);
}

BOOL CCJFlatComboBox::OnEraseBkgnd(CDC* pDC) 
{
	// KStowell - overridden for flicker-free drawing.
	UNUSED_ALWAYS(pDC);
	return TRUE;
}
