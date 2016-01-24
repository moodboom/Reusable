// ListDataByteCombo.cpp : implementation file
//

#include "stdafx.h"

#include "ListDataMasterList.h"			// We notify this on selection change.

#include "ListDataByteCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListDataByteCombo


//-------------------------------------------------------------------//
// ListDataByteCombo()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataByteCombo::ListDataByteCombo()
{
}


//-------------------------------------------------------------------//
// ~ListDataByteCombo()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataByteCombo::~ListDataByteCombo()
{
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
void ListDataByteCombo::Initialize(
	BYTE*						pNewDBVar,
	BYTE*						pNewAltDBVar,
	int						nNewListColumn,
	int						nNewMaxIndex
) {

	nMaxIndex = nNewMaxIndex;

	ListDataControlData<BYTE>::Initialize(
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
CString ListDataByteCombo::GetValueAsString()
{
	// Get the string for the current multi value.
	return GetTextAtIndex( CurrentMultiValue );
}


//-------------------------------------------------------------------//
// GetAltVarAsString()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
CString ListDataByteCombo::GetAltVarAsString()
{
	// Get the string for the "alternate" var.
	return GetTextAtIndex( *pAltDatabaseVar );
}


//-------------------------------------------------------------------//
// GetTextAtIndex()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
CString ListDataByteCombo::GetTextAtIndex( BYTE Index )
{
	if ( Index < 0 || Index > nMaxIndex )
		Index = nMaxIndex;
	CString strTemp;
	GetLBText( Index, strTemp );
	return strTemp;
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ListDataByteCombo, CComboBox)
	//{{AFX_MSG_MAP(ListDataByteCombo)
	ON_WM_SETFOCUS()
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelchange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------//
// GetControlValue()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataByteCombo::ExtractControlValue()
{

	CurrentMultiValue = (BYTE) GetCurSel();

}


//-------------------------------------------------------------------//
// SetControlValue()																	//
//-------------------------------------------------------------------//
// Here, we set the control to contain the data in our current
// record set var.
//-------------------------------------------------------------------//
void ListDataByteCombo::DisplayControlValue()
{
	// SetCurSel() does NOT fire off an OnSelChange() msg, so it is OK to call
	// without the m_bBypassChangesSave wrapper.  But just in case...
	m_bBypassChangesSave = true;

	SetCurSel( *pDatabaseVar );
	
	m_bBypassChangesSave = false;
}


/////////////////////////////////////////////////////////////////////////////
// ListDataByteCombo message handlers


//-------------------------------------------------------------------//
// OnSelchange()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataByteCombo::OnSelchange() 
{
	
	// Time to update.
	SavePendingChanges();

}


//-------------------------------------------------------------------//
// OnSetFocus()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataByteCombo::OnSetFocus(CWnd* pOldWnd) 
{
	CComboBox::OnSetFocus(pOldWnd);
	
	SetMultiDisplay( false );
	
}


