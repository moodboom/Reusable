//-------------------------------------------------------------------//
// FontDlg.cpp : implementation file
//
//	Copyright © 2002 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "FontDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------//
// GLOBALS STATICS CONSTANTS
//-------------------------------------------------------------------//

SimpleFont* FontDlg::s_psfCustom = 0;

//-------------------------------------------------------------------//
FontDlg::FontDlg( LPCTSTR lpszTemplateName, CWnd* pParentWnd )
 : inherited( lpszTemplateName, pParentWnd )	
{
}
	
FontDlg::FontDlg( UINT nIDTemplate, CWnd* pParentWnd )
 : inherited( nIDTemplate	, pParentWnd )	
{
	//{{AFX_DATA_INIT(FontDlg)
	//}}AFX_DATA_INIT
}
	
//-------------------------------------------------------------------//
// ~FontDlg()																			//
//-------------------------------------------------------------------//
FontDlg::~FontDlg()
{}


void FontDlg::SetCustomFont( SimpleFont& sf )
{ 
	s_psfCustom = &sf; 
}


//-------------------------------------------------------------------//
// DoModal()																			//
//-------------------------------------------------------------------//
// Modal dlgs use this.
// This uses undocumented CDialogTemplate to set a custom font.
//-------------------------------------------------------------------//
int FontDlg::DoModal()
{
	CDialogTemplate dlt;
	int nResult;

	// MDM	1/29/2003 11:27:22 AM
	// Load the DLGINIT resource.
	// We need this puppy for initial combo box strings - also for 
	// ActiveX control initialization.
	HGLOBAL			 hRsrc = NULL;
	void				 *pDlgInit = NULL;
	HINSTANCE hInst	 = AfxFindResourceHandle(m_lpszTemplateName, RT_DLGINIT);
	HRSRC 	 hDlgInit = ::FindResource(hInst, m_lpszTemplateName, RT_DLGINIT);
	if (hDlgInit != NULL)
	{
		hRsrc 	= LoadResource(hInst, hDlgInit);
		pDlgInit = LockResource(hRsrc);
	}

	// load dialog template
	if (!dlt.Load( m_lpszTemplateName ) ) return -1;	

	// auto_ptr<SIZE> psz(new SIZE);
	// dlt.GetSizeInPixels(psz.get());
	
	// SIZE sz;
	// dlt.GetSizeInPixels( &sz );

	// if (psz->cx > SMALL_FONT_MAIN_WIDTH)	// MFC has resized it due to large fonts! (note: you can use other tests here for large fonts as well)
	{
		// dlt.SetFont("arial", 6);			// this font makes everything happy!

		SetTemplateFont( dlt );

		// get pointer to the modified dialog template
		DLGTEMPLATE* pdata = (DLGTEMPLATE*)GlobalLock(dlt.m_hTemplate);

		// let MFC know that we are using our own template
		m_lpszTemplateName = NULL;

		// MDM	1/29/2003 11:29:17 AM
		// Use the undocumented version of this function that uses the RC_DLGINIT info.
		InitModalIndirect( pdata, m_pParentWnd, pDlgInit );

		// display dialog box
		nResult = CDialog::DoModal();

		// unlock memory object
		GlobalUnlock(dlt.m_hTemplate);
	}
	
	// else
	// 	nResult = CDialog::DoModal();

	// Free memory allocated for DLGINIT resource.
	if (pDlgInit != NULL && hRsrc != NULL)
	{
		UnlockResource(hRsrc);
		FreeResource(hRsrc);
	}

	return nResult;
}


//-------------------------------------------------------------------//
// Create()																				//
//-------------------------------------------------------------------//
// Modeless dlgs use this.
// This uses undocumented CDialogTemplate to set a custom font.
//
// NOTE the diff between this and CDialog() version - don't 
// call that one!
//-------------------------------------------------------------------//
BOOL FontDlg::Create( CWnd* pParentWnd )
{
   CDialogTemplate dlt;

   if(!dlt.Load(m_lpszTemplateName))
      return FALSE;

   // Set the font.
	SetTemplateFont( dlt );

   // get pointer to the modified dialog template
   LPSTR pdata = (LPSTR)GlobalLock(dlt.m_hTemplate);

   return CDialog::CreateIndirect(pdata, pParentWnd);
}


//-------------------------------------------------------------------//
// SetTemplateFont()																	//
//-------------------------------------------------------------------//
void FontDlg::SetTemplateFont( CDialogTemplate& dlt )
{
	if ( bUseCustomFont() )
	{
		dlt.SetFont(
			LPCTSTR( s_psfCustom->m_strFace ), 
			s_psfCustom->m_nSize / 10			// Size in points = size in tenths / 10
		);
	}
}

BEGIN_MESSAGE_MAP(FontDlg, CDialog)
	//{{AFX_MSG_MAP(FontDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL FontDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN)
	{
		DWORD def_id=GetDefID();
		if (def_id != 0)
		{
			CWnd *wnd=FromHandle(pMsg->hwnd);
			TCHAR class_name[16];
			if (GetClassName(wnd->GetSafeHwnd(), class_name, sizeof(class_name)/sizeof(TCHAR))!=0)
			{
				if (!_tcsnicmp(class_name, _T("edit"), 5)) 
				{
//					if (GetDlgItem(LOWORD(def_id))) {
//						GetDlgItem(LOWORD(def_id))->SetFocus();
//					}
					return TRUE;	// discard the message!
				}
			}
		}
	}
	if (pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_ESCAPE)
	{
		CWnd* wnd_cancel = GetDlgItem(IDCANCEL);
		if (NULL != wnd_cancel)
		{
			CWnd *wnd=FromHandle(pMsg->hwnd);
			TCHAR class_name[16];
			if (GetClassName(wnd->GetSafeHwnd(), class_name, sizeof(class_name)/sizeof(TCHAR))!=0)
			{
				if (!_tcsnicmp(class_name, _T("edit"), 5)) 
				{
//					wnd_cancel->SetFocus();
					return TRUE;	// discard the message!
				}
			}
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
