// ListDataLongSlider.cpp : implementation file
//

#include "stdafx.h"

#include "ListDataMasterList.h"			// We notify this on KillFocus()

#include "ListDataLongSlider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListDataLongSlider


//-------------------------------------------------------------------//
// ListDataLongSlider()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataLongSlider::ListDataLongSlider()
{
}


//-------------------------------------------------------------------//
// ~ListDataLongSlider()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataLongSlider::~ListDataLongSlider()
{
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
void ListDataLongSlider::Initialize(
	Long*						pNewDBVar,
	Long*						pNewAltDBVar,
	int						nNewListColumn
) {

	ListDataControlData<Long>::Initialize(
		pNewDBVar,
		pNewAltDBVar,
		nNewListColumn,
		this
	);

}


//-------------------------------------------------------------------//
// GetValueAsString()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
CString ListDataLongSlider::GetValueAsString()
{
	CString strValue;
	strValue.Format( _T("%d"), CurrentMultiValue );
	return strValue;
}
CString ListDataLongSlider::GetAltVarAsString()
{
	CString strValue;
	strValue.Format( _T("%d"), *pAltDatabaseVar );
	return strValue;
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ListDataLongSlider, CSliderCtrl)
	//{{AFX_MSG_MAP(ListDataLongSlider)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------//
// ExtractControlValue()															//
//-------------------------------------------------------------------//
void ListDataLongSlider::ExtractControlValue()
{

	CurrentMultiValue = (Long) GetPos();

}


//-------------------------------------------------------------------//
// DisplayControlValue()															//
//-------------------------------------------------------------------//
// Here, we set the control to contain the data in our current
// record set var.
//-------------------------------------------------------------------//
void ListDataLongSlider::DisplayControlValue()
{
	// SetPos() does NOT fire off an OnSelChange() msg, so it is OK to call
	// without the m_bBypassChangesSave wrapper.  But just in case...
	m_bBypassChangesSave = true;

	SetPos( *pDatabaseVar );

	// DEBUG
	// TRACE( _T("Display control value, value: %s\n"), GetValueAsString() );

	m_bBypassChangesSave = false;
}


/////////////////////////////////////////////////////////////////////////////
// ListDataLongSlider message handlers


//-------------------------------------------------------------------//
// OnHScroll()																			//
//-------------------------------------------------------------------//
// MS SUCKS!
// This function should be within ListDataXxxSlider, but the slider
// control sends this message to its parent, and doesn't otherwise
// know it is happening.  So it must be placed in the parent that
// contains this control.
//-------------------------------------------------------------------//
/*
void YourDialogClass::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);

	// If the message came from a DAO slider control, update.
	// WARNING: VC8 is stricter about successfully dynamic_cast'ing.
	// This will FAIL if you don't static cast to CSliderCtrl* BEFORE
	// trying to dynamic_cast to the derived class.  Funny, M$'s
	// improved compiler is exposing the crappiness of their original
	// design.
	ListDataByteSlider* pSlider = dynamic_cast< ListDataByteSlider* > ( (CSliderCtrl*)pScrollBar );
	if ( pSlider )
	{
		// Minimize the db work by only saving to the db when "done" scrolling.
		pSlider->SavePendingChanges( nSBCode == SB_ENDSCROLL );

		// TRACE( _T("DAO scroll type %d to %d\n"), nSBCode, nPos );
	}
}
*/


//-------------------------------------------------------------------//
// OnSetFocus()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataLongSlider::OnSetFocus(CWnd* pOldWnd) 
{
	CSliderCtrl::OnSetFocus(pOldWnd);
	
	SetMultiDisplay( false );
	
}
