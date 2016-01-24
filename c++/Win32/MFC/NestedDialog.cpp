// NestedDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NestedDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// NestedDialog dialog


//-------------------------------------------------------------------//
// NestedDialog()																		//
//-------------------------------------------------------------------//
NestedDialog::NestedDialog(
	UINT	DialogID,
	CWnd* pParent
)
: 
	// Call base class.
	inherited( DialogID, pParent )
{
	//{{AFX_DATA_INIT(NestedDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


//-------------------------------------------------------------------//
// DoDataExchange()																	//
//-------------------------------------------------------------------//
void NestedDialog::DoDataExchange(CDataExchange* pDX)
{
	inherited::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NestedDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(NestedDialog, inherited)
	//{{AFX_MSG_MAP(NestedDialog)
	ON_WM_RBUTTONDOWN()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NestedDialog message handlers


// MDM	1/5/2001 11:56:06 AM
// NOTE: Here, we are trapping specific messages that we want to reroute.
// If this starts getting busy, PLEASE rewrite to use PreTranslateMsg().

//-------------------------------------------------------------------//
// OnRButtonDown()																	//
//-------------------------------------------------------------------//
// Here, we want to make sure this gets passed up to the top-level
// dialog.
//-------------------------------------------------------------------//
void NestedDialog::OnRButtonDown(UINT nFlags, CPoint point) 
{
	ClientToScreen( &point );
	CDialog* pParent = GetTopmostDlg();
	pParent->ScreenToClient( &point );

	pParent->SendMessage( 
		WM_RBUTTONDOWN, 
		(WPARAM)nFlags, 
		MAKELPARAM( point.x, point.y )
	);

	// Don't call base class.
	// inherited::OnRButtonDown(nFlags, point);
}


//-------------------------------------------------------------------//
// GetTopmostDlg()																	//
//-------------------------------------------------------------------//
CDialog* NestedDialog::GetTopmostDlg()
{
	if ( GetSafeHwnd() == 0 || GetParent() == 0 || GetParent()->GetSafeHwnd() == 0 )
		return this;

	// Get the topmost dlg parent.
	CWnd* pParent = this;
	CWnd* pParentTry = GetParent();
	while ( 
			pParentTry
		&&	pParentTry->GetSafeHwnd()
		&&	dynamic_cast<CDialog*>( pParentTry )
	) {
		pParent = pParentTry;
		pParentTry = pParentTry->GetParent();
	}
	ASSERT( dynamic_cast<CDialog*>( pParent ) );

	return dynamic_cast<CDialog*>( pParent );
}


//-------------------------------------------------------------------//
// OnEraseBkgnd()																		//
//-------------------------------------------------------------------//
// The base class does the background draw for us.
// We just need to properly set up the offset.
//-------------------------------------------------------------------//
BOOL NestedDialog::OnEraseBkgnd(CDC* pDC) 
{
	// First, offset the background by the amount the nested dialog
	// is offset from its immediate parent.
	CWnd* pParent = GetParent();

	if ( pParent )
	{
		CRect rectParent;
		CRect rectThis;
		
		pParent->GetClientRect( &rectParent );
		pParent->ClientToScreen( &rectParent );

		GetClientRect( &rectThis );
		ClientToScreen( &rectThis );

		if ( m_BkgndStyle == BDBS_TILE )
		{
			SetOffset(
				rectThis.left - rectParent.left,
				rectThis.top - rectParent.top
			);

		} else if ( m_BkgndStyle == BDBS_CENTER )
		{
			SetOffset(
				rectParent.left - rectThis.left + ( rectParent.Width()  - rectThis.Width()  ) / 2,
				rectParent.top  - rectThis.top  + ( rectParent.Height() - rectThis.Height() ) / 2
			);
		}

	}

	// // Now let BkDialog do the background paint.
	return inherited::OnEraseBkgnd(pDC);

}


