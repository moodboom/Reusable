//-------------------------------------------------------------------//
// FontPropertySheet.cpp : implementation file
//
// This code is a combination of things learned from:
//
//		MSDN PropFont sample
//		CodeGuru sample at http://www.codeguru.com/propertysheet/safeproppage.shtml	
//		google newsgroups for "CPropertySheet author:pudelko"
//
//	Copyright © 2002 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include <afxpriv.h>					// For CDialogTemplate

#include "FontPropertySheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Nasty function from MSDN's PropFont sample:

#define WM_RESIZEPAGE WM_APP+1
enum { CDF_CENTER, CDF_TOPLEFT, CDF_NONE };
// helper function which sets the font for a window and all its children
// and also resizes everything according to the new font
void ChangeDialogFont(CWnd* pWnd, CFont* pFont, int nFlag)
{
	CRect windowRect;

	// grab old and new text metrics
	TEXTMETRIC tmOld, tmNew;
	CDC * pDC = pWnd->GetDC();
	CFont * pSavedFont = pDC->SelectObject(pWnd->GetFont());
	pDC->GetTextMetrics(&tmOld);
	pDC->SelectObject(pFont);
	pDC->GetTextMetrics(&tmNew);
	pDC->SelectObject(pSavedFont);
	pWnd->ReleaseDC(pDC);

	long oldHeight = tmOld.tmHeight+tmOld.tmExternalLeading;
	long newHeight = tmNew.tmHeight+tmNew.tmExternalLeading;

	if (nFlag != CDF_NONE)
	{
		// calculate new dialog window rectangle
		CRect clientRect, newClientRect, newWindowRect;

		pWnd->GetWindowRect(windowRect);
		pWnd->GetClientRect(clientRect);
		long xDiff = windowRect.Width() - clientRect.Width();
		long yDiff = windowRect.Height() - clientRect.Height();
	
		newClientRect.left = newClientRect.top = 0;
		newClientRect.right = clientRect.right * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
		newClientRect.bottom = clientRect.bottom * newHeight / oldHeight;

		if (nFlag == CDF_TOPLEFT) // resize with origin at top/left of window
		{
			newWindowRect.left = windowRect.left;
			newWindowRect.top = windowRect.top;
			newWindowRect.right = windowRect.left + newClientRect.right + xDiff;
			newWindowRect.bottom = windowRect.top + newClientRect.bottom + yDiff;
		}
		else if (nFlag == CDF_CENTER) // resize with origin at center of window
		{
			newWindowRect.left = windowRect.left - 
							(newClientRect.right - clientRect.right)/2;
			newWindowRect.top = windowRect.top -
							(newClientRect.bottom - clientRect.bottom)/2;
			newWindowRect.right = newWindowRect.left + newClientRect.right + xDiff;
			newWindowRect.bottom = newWindowRect.top + newClientRect.bottom + yDiff;
		}
		pWnd->MoveWindow(newWindowRect);
	}

	pWnd->SetFont(pFont);

	// iterate through and move all child windows and change their font.
	CWnd* pChildWnd = pWnd->GetWindow(GW_CHILD);

	while (pChildWnd)
	{
		pChildWnd->SetFont(pFont);
		pChildWnd->GetWindowRect(windowRect);

		CString strClass;
		::GetClassName(pChildWnd->m_hWnd, strClass.GetBufferSetLength(32), 31);
		strClass.MakeUpper();
		if(strClass==_T("COMBOBOX"))
		{
			CRect rect;
			pChildWnd->SendMessage(CB_GETDROPPEDCONTROLRECT,0,(LPARAM) &rect);
			windowRect.right = rect.right;
			windowRect.bottom = rect.bottom;
		}

		pWnd->ScreenToClient(windowRect);
		windowRect.left = windowRect.left * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
		windowRect.right = windowRect.right * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
		windowRect.top = windowRect.top * newHeight / oldHeight;
		windowRect.bottom = windowRect.bottom * newHeight / oldHeight;
		pChildWnd->MoveWindow(windowRect);
		
		pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
	}
}

/////////////////////////////////////////////////////////////////////////////
// FontPropertySheet


//-------------------------------------------------------------------//
// GLOBALS STATICS CONSTANTS
//-------------------------------------------------------------------//

SimpleFont* FontPropertySheet::s_psfCustom = 0;
CFont			FontPropertySheet::s_cfCustom;

//-------------------------------------------------------------------//


IMPLEMENT_DYNAMIC(FontPropertySheet, CPropertySheet)


//-------------------------------------------------------------------//
// FontPropertySheet()																//
//-------------------------------------------------------------------//
FontPropertySheet::FontPropertySheet()
:
	// Call base class.
	inherited(),

	// Init vars
	m_bDirty( false )

{}


//-------------------------------------------------------------------//
// FontPropertySheet()																//
//-------------------------------------------------------------------//
FontPropertySheet::FontPropertySheet(
	UINT nIDCaption, 
	CWnd* pParentWnd, 
	UINT iSelectPage
) :
	
	// Call base class
	inherited( nIDCaption, pParentWnd, iSelectPage ),

	// Init vars
	m_bDirty( false )

{
}


//-------------------------------------------------------------------//
// FontPropertySheet()																//
//-------------------------------------------------------------------//
FontPropertySheet::FontPropertySheet(
	LPCTSTR pszCaption, 
	CWnd* pParentWnd, 
	UINT iSelectPage// , 
) :
	
	// Call base class
	inherited( pszCaption, pParentWnd, iSelectPage ),

	// Init vars
	m_bDirty( false )

{
}


//-------------------------------------------------------------------//
// SetCustomFont()																	//
//-------------------------------------------------------------------//
void FontPropertySheet::SetCustomFont( SimpleFont& sf )
{ 
	s_psfCustom = &sf; 

	if ( s_cfCustom.m_hObject )
		VERIFY ( s_cfCustom.DeleteObject() );

	VERIFY(
		s_cfCustom.CreatePointFont( 
			s_psfCustom->m_nSize,
			LPCTSTR( s_psfCustom->m_strFace ) 
		)
	);

}


//-------------------------------------------------------------------//
// ~FontPropertySheet()																//
//-------------------------------------------------------------------//
FontPropertySheet::~FontPropertySheet()
{
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(FontPropertySheet, inherited)
	//{{AFX_MSG_MAP(FontPropertySheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FontPropertySheet message handlers

BOOL FontPropertySheet::OnInitDialog() 
{
	ModifyStyle(WS_SYSMENU, 0);   

	BOOL bReturn = inherited::OnInitDialog();

	if ( bUseCustomFont() )
	{
		CPropertyPage* pPage = GetActivePage ();
		ASSERT (pPage);

		// change the font for the sheet
		ChangeDialogFont (this, &s_cfCustom, CDF_CENTER);
		
		// change the font for each page
		for (int iCntr = 0; iCntr < GetPageCount (); iCntr++)
		{
			VERIFY (SetActivePage (iCntr));
			CPropertyPage* pPage = GetActivePage ();
			ASSERT (pPage);
			ChangeDialogFont (pPage, &s_cfCustom, CDF_CENTER);
		}

		VERIFY (SetActivePage (pPage));

		// set and save the size of the page
		CTabCtrl* pTab = GetTabControl ();
		ASSERT (pTab);

		if (m_psh.dwFlags & PSH_WIZARD)
		{
			pTab->ShowWindow (SW_HIDE);
			GetClientRect (&m_rctPage);

			CWnd* pButton = GetDlgItem (ID_WIZBACK);
			ASSERT (pButton);
			CRect rc;
			pButton->GetWindowRect (&rc);
			ScreenToClient (&rc);
			m_rctPage.bottom = rc.top-2;
		}
		else
		{
			pTab->GetWindowRect (&m_rctPage);
			ScreenToClient (&m_rctPage);
			pTab->AdjustRect (FALSE, &m_rctPage);
		}

		// resize the page	
		pPage->MoveWindow (&m_rctPage);

	}

	return bReturn;
}


BOOL FontPropertySheet::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	NMHDR* pnmh = (LPNMHDR) lParam;

	// the sheet resizes the page whenever it is activated so we need to size it correctly
	if (TCN_SELCHANGE == pnmh->code)
		PostMessage (WM_RESIZEPAGE);
	
	return inherited::OnNotify(wParam, lParam, pResult);
}


LONG FontPropertySheet::OnResizePage (UINT, LONG)
{
	// resize the page
	CPropertyPage* pPage = GetActivePage ();
	ASSERT (pPage);
	pPage->MoveWindow (&m_rctPage);

	return 0;
}


BOOL FontPropertySheet::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// the sheet resizes the page whenever the Apply button is clicked so we need to size it correctly
	if (ID_APPLY_NOW == wParam ||
		ID_WIZNEXT == wParam ||
		ID_WIZBACK == wParam)
		PostMessage (WM_RESIZEPAGE);
	
	return inherited::OnCommand(wParam, lParam);
}




