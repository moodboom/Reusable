// ListDataByteSlider.cpp : implementation file
//

#include "stdafx.h"

#include "ListDataMasterList.h"			// We notify this on KillFocus()

#include "ListDataByteSlider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListDataByteSlider


//-------------------------------------------------------------------//
// ListDataByteSlider()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataByteSlider::ListDataByteSlider()
{
}


//-------------------------------------------------------------------//
// ~ListDataByteSlider()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataByteSlider::~ListDataByteSlider()
{
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
void ListDataByteSlider::Initialize(
	BYTE*						pNewDBVar,
	BYTE*						pNewAltDBVar,
	int						nNewListColumn
) {

	ListDataControlData<BYTE>::Initialize(
		pNewDBVar,
		pNewAltDBVar,
		nNewListColumn,
		this
	);

	// Here, we set typical default slider ranges.
	// These should work very well in most cases.  We are always dealing
	// with a byte value, and typically use 0-100.
	// Call these functions again after calling Initialize() in the 
	// case of exceptions.
	SetRange( 0, 100 ); 
	SetTicFreq(	10 );

}


//-------------------------------------------------------------------//
// GetValueAsString()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
CString ListDataByteSlider::GetValueAsString()
{
	CString strValue;
	strValue.Format( _T("%d"), CurrentMultiValue );
	return strValue;
}
CString ListDataByteSlider::GetAltVarAsString()
{
	// DEBUG
	// TRACE( _T("Address of control variable = %d\n"), (int)pAltDatabaseVar );

	CString strValue;
	strValue.Format( _T("%d"), *pAltDatabaseVar );
	return strValue;
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ListDataByteSlider, inherited)
	//{{AFX_MSG_MAP(ListDataByteSlider)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------//
// ExtractControlValue()															//
//-------------------------------------------------------------------//
void ListDataByteSlider::ExtractControlValue()
{

	CurrentMultiValue = (BYTE) GetPos();

}


//-------------------------------------------------------------------//
// DisplayControlValue()															//
//-------------------------------------------------------------------//
// Here, we set the control to contain the data in our current
// record set var.
//-------------------------------------------------------------------//
void ListDataByteSlider::DisplayControlValue()
{
	// SetPos() does NOT fire off an HSCROLL msg, so it is OK to call
	// without the m_bBypassChangesSave wrapper.  But just in case...
	m_bBypassChangesSave = true;

	SetPos( *pDatabaseVar );

	// DEBUG
	// TRACE( _T("Display control value, value: %s\n"), GetValueAsString() );

	m_bBypassChangesSave = false;
	
}


/////////////////////////////////////////////////////////////////////////////
// ListDataByteSlider message handlers


//-------------------------------------------------------------------//
// OnHScroll()																			//
//-------------------------------------------------------------------//
// MS SUCKS!
// This function should be within ListDataXxxSlider, but the slider
// control sends this message to its parent, and doesn't otherwise
// know it is happening.  So it must be placed in the parent that
// contains this control.
//
// TO DO
// Get this msg reflected properly to the slider!!
// This also needs m_bMultiPrompt handling so we only prompt
// on the FIRST change, not every.
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
void ListDataByteSlider::OnSetFocus(CWnd* pOldWnd) 
{
	inherited::OnSetFocus(pOldWnd);
	
	SetMultiDisplay( false );
	
}
