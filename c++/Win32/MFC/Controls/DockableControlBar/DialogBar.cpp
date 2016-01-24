//-------------------------------------------------------------------//
// DialogBar Implementation
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "DialogBar.h"

#include <PragmaMessages.h>				// For _TODO()

#include <WindowHelpers.h>					// For GetDialogSizeFromTemplate()


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(DialogBar, CSizingControlBarCF);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DialogBar::DialogBar(
	CFrameWnd*	pParent,
	FontDlg*		pDlg,
	UINT			nIDTemplate,
	TCHAR*		pszTitle,
	int			nID,
	DWORD			dwDockStyle,
   bool        bUseMaxButton
) :

   // Call base class.
   inherited( bUseMaxButton ),

	// Init vars.
	m_pDlg			( pDlg			),
	m_nIDTemplate	( nIDTemplate	),
	m_pParent		( pParent		)

{
	CRect rectFrame;
	m_pParent->GetWindowRect( rectFrame );

	CSize szDlg = GetDialogSizeFromTemplate( nIDTemplate, pParent );

	// determine the initial size of the docked dialog
	CSize dockSize;
	if(dwDockStyle & CBRS_ALIGN_LEFT || dwDockStyle & CBRS_ALIGN_RIGHT)
		dockSize = CSize(szDlg.cx+2*GetSystemMetrics(SM_CXSIZEFRAME) ,rectFrame.Height());
	else if(dwDockStyle & CBRS_ALIGN_TOP || dwDockStyle & CBRS_ALIGN_BOTTOM)
		dockSize = CSize(rectFrame.Width(),szDlg.cy+2*GetSystemMetrics(SM_CYSIZEFRAME));
	else
		dockSize = CSize(szDlg.cx,szDlg.cy);

	if (
		!Create(
			pszTitle,
			pParent, 
			dockSize,
			TRUE, 
			nID									// nID, must be unique across bars.
		)
	) {
	  	ASSERT( false );
		return;									// fail to create
	}
	// CBRS_SIZE_DYNAMIC allows the bar to be resized when floating
	SetBarStyle(
			GetBarStyle() 
		|	CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC
	);

	// Dock the control bar.
	EnableDocking(
		dwDockStyle
	);

}


//-------------------------------------------------------------------//
// ~DialogBar()																//
//-------------------------------------------------------------------//
DialogBar::~DialogBar()
{
	if ( m_pDlg )
	{
		m_pDlg->DestroyWindow();
		delete m_pDlg;
	}
}


BEGIN_MESSAGE_MAP(DialogBar, inherited)
	//{{AFX_MSG_MAP(CRegBar)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DialogBar message handlers


//-------------------------------------------------------------------//
// OnCreate()																			//
//-------------------------------------------------------------------//
// Create the bar.
//-------------------------------------------------------------------//
int DialogBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// Use the FontDlg version, not CDialog.
	m_pDlg->Create( this );

	// We want to size our child (the dialog) when we get sized.
	SetSCBStyle( GetSCBStyle() | SCBS_SIZECHILD );

	CRect rectFrame;
	m_pParent->GetWindowRect( rectFrame );

	CRect rectDlg;
	m_pDlg->GetWindowRect( rectDlg );

	// lpCreateStruct->cx = rectDlg.Width();

	if ( inherited::OnCreate(lpCreateStruct) == -1 )
		return -1;

	#pragma _TODO("Figure out how to resize the bar here to match the width of the dialog")
	
	SetWindowPos( NULL, 0, 0, rectDlg.Width(), rectFrame.Height(), SWP_NOZORDER|SWP_NOMOVE );
	
	
	/*

	rectDlg.bottom = rectFrame.Height();
	rectDlg.right = rectDlg.Width();
	rectDlg.top = 0;
	rectDlg.left = 0;
	MoveWindow( rectDlg );

	OnSize( SIZE_RESTORED , rectDlg.Width(), rectFrame.Height() );
	
	m_pDockSite->DelayRecalcLayout();

	// CalcFixedLayout( false, false );

	*/
		   
	return 0;

}


//-------------------------------------------------------------------//
// RecreateDialog()																	//
//-------------------------------------------------------------------//
// This will destroy and reload the embedded dialog.
//
// The embedded dialog is typically created and destroyed along
// with the control bar, but you may have need to recreate it
// on the fly.  We have had to do this e.g. to change a dialog's 
// custom font while using the FontDlg class.
//-------------------------------------------------------------------//
void DialogBar::RecreateDialog()
{
	// Save the current size.
	CRect rectDlg;
	m_pDlg->GetClientRect( &rectDlg );

	m_pDlg->DestroyWindow();

	// Use the FontDlg version, not CDialog.
	m_pDlg->Create( this );

	// Get everything to reset to its previous size.
	m_pDlg->MoveWindow( rectDlg.left, rectDlg.top, rectDlg.Width(), rectDlg.Height() );	
	m_pDlg->Invalidate();
	m_pDlg->UpdateWindow();
	Invalidate();
	UpdateWindow();

}
