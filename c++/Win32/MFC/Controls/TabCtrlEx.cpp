// TabCtrlEx.cpp : implementation file
//

#include "stdafx.h"

#include "..\MemDC.h"

#include "TabCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabCtrlEx

CTabCtrlEx::CTabCtrlEx()
{
	m_crSelColour     = RGB(0,0,255);
	m_crUnselColour   = RGB(50,50,50);
}

CTabCtrlEx::~CTabCtrlEx()
{
	m_SelFont.DeleteObject();
	m_UnselFont.DeleteObject();
}

BEGIN_MESSAGE_MAP(CTabCtrlEx, CTabCtrl)
	//{{AFX_MSG_MAP(CTabCtrlEx)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabCtrlEx message handlers

int CTabCtrlEx::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTabCtrl::OnCreate(lpCreateStruct) == -1) return -1;
	ModifyStyle(0, TCS_OWNERDRAWFIXED);
	
	return 0;
}

void CTabCtrlEx::PreSubclassWindow() 
{	
	CTabCtrl::PreSubclassWindow();
	ModifyStyle(0, TCS_OWNERDRAWFIXED);
}

//-------------------------------------------------------------------//
// OnEraseBkgnd()																		//
//-------------------------------------------------------------------//
// We override the background painting to prevent a lot of flashing.
//-------------------------------------------------------------------//
BOOL CTabCtrlEx::OnEraseBkgnd( CDC* pDC )
{
	// Tell the caller that we took care of the job.
	return TRUE;
}


void CTabCtrlEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{	
	CRect rect = lpDrawItemStruct->rcItem;
	int nTabIndex = lpDrawItemStruct->itemID;
	if (nTabIndex < 0) return;
	BOOL bSelected = (nTabIndex == GetCurSel());

	TCHAR label[64];
	TC_ITEM tci;
	tci.mask = TCIF_TEXT|TCIF_IMAGE;
	tci.pszText = label;     
	tci.cchTextMax = 63;    	
	if (!GetItem(nTabIndex, &tci )) return;

	// MDM	12/12/2000 12:27:08 PM
	// This causes problems - the mem DC does not use the same font, for example.
	/*
	// Use CMemDC to reduce flicker.
	CDC* pDrawDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	if (!pDrawDC) return;
	CMemDC pDC( pDrawDC );
	*/

	// Original code
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	if (!pDC) return;
	int nSavedDC = pDC->SaveDC();

	// For some bizarre reason the rcItem you get extends above the actual
	// drawing area. We have to workaround this "feature".
	rect.top += ::GetSystemMetrics(SM_CYEDGE);

	// TRACE( _T("TabCtrlEx DrawItem[%3d,%3d,%3d,%3d ]\n"), rect.top, rect.bottom, rect.Width(), rect.Height() );

	pDC->SetBkMode(TRANSPARENT);
	pDC->FillSolidRect(rect, ::GetSysColor(COLOR_BTNFACE));

	// Draw image
	CImageList* pImageList = GetImageList();
	if (pImageList && tci.iImage >= 0) {

		rect.left += pDC->GetTextExtent(_T(" ")).cx;		// Margin

		// Get height of image so we 
		IMAGEINFO info;
		pImageList->GetImageInfo(tci.iImage, &info);
		CRect ImageRect(info.rcImage);
		int nYpos = rect.top;

		pImageList->Draw(pDC, tci.iImage, CPoint(rect.left, nYpos), ILD_TRANSPARENT);
		rect.left += ImageRect.Width();
	}

	if (bSelected) {
		pDC->SetTextColor(m_crSelColour);
		pDC->SelectObject(&m_SelFont);
		rect.top -= ::GetSystemMetrics(SM_CYEDGE);
		pDC->DrawText(label, rect, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
	} else {
		pDC->SetTextColor(m_crUnselColour);
		pDC->SelectObject(&m_UnselFont);
		pDC->DrawText(label, rect, DT_SINGLELINE|DT_BOTTOM|DT_CENTER);
	}

	// Original code
	pDC->RestoreDC(nSavedDC);
}

/////////////////////////////////////////////////////////////////////////////
// CTabCtrlEx operations

void CTabCtrlEx::SetColours(COLORREF bSelColour, COLORREF bUnselColour)
{
	m_crSelColour = bSelColour;
	m_crUnselColour = bUnselColour;
	Invalidate();
}

void CTabCtrlEx::SetFonts(CFont* pSelFont, CFont* pUnselFont)
{
	ASSERT(pSelFont && pUnselFont);

	LOGFONT lFont;
	int nSelHeight, nUnselHeight;

	m_SelFont.DeleteObject();
	m_UnselFont.DeleteObject();

	pSelFont->GetLogFont(&lFont);
	m_SelFont.CreateFontIndirect(&lFont);
	nSelHeight = lFont.lfHeight;

	pUnselFont->GetLogFont(&lFont);
	m_UnselFont.CreateFontIndirect(&lFont);
	nUnselHeight = lFont.lfHeight;

	SetFont( (nSelHeight > nUnselHeight)? &m_SelFont : &m_UnselFont);
}


void CTabCtrlEx::SetFonts(int nSelWeight,   BOOL bSelItalic,   BOOL bSelUnderline,
                          int nUnselWeight, BOOL bUnselItalic, BOOL bUnselUnderline)
{
	// Free any memory currently used by the fonts.
	m_SelFont.DeleteObject();
	m_UnselFont.DeleteObject();

	// Get the current font
	LOGFONT lFont;
	CFont *pFont = GetFont();
	if (pFont)
		pFont->GetLogFont(&lFont);
	else {
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 
		                            sizeof(NONCLIENTMETRICS), &ncm, 0));
		lFont = ncm.lfMessageFont; 
	}

	// Create the "Selected" font
	lFont.lfWeight = nSelWeight;
	lFont.lfItalic = bSelItalic;
	lFont.lfUnderline = bSelUnderline;
	m_SelFont.CreateFontIndirect(&lFont);

	// Create the "Unselected" font
	lFont.lfWeight = nUnselWeight;
	lFont.lfItalic = bUnselItalic;
	lFont.lfUnderline = bUnselUnderline;
	m_UnselFont.CreateFontIndirect(&lFont);

	SetFont( (nSelWeight > nUnselWeight)? &m_SelFont : &m_UnselFont);
}
