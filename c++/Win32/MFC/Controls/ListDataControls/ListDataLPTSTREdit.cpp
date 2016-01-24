//-------------------------------------------------------------------//
// ListDataLPTSTREdit.cpp : implementation file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "ListDataMasterList.h"			// We notify this on KillFocus()

#include "ListDataLPTSTREdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListDataLPTSTREdit


//-------------------------------------------------------------------//
// ListDataLPTSTREdit()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataLPTSTREdit::ListDataLPTSTREdit()
:

	// Init vars.
	m_bUpdated			( false	),
	m_bMultiPrompt		( true	)

{
	// This MUST start as zero since it is a pointer.
	CurrentMultiValue = 0;
}


//-------------------------------------------------------------------//
// ~ListDataLPTSTREdit()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataLPTSTREdit::~ListDataLPTSTREdit()
{
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
void ListDataLPTSTREdit::Initialize(
	LPTSTR*					pNewDBVar,
	LPTSTR*					pNewAltDBVar,
	int						nNewListColumn
) {

	ListDataControlData<LPTSTR>::Initialize(
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
CString ListDataLPTSTREdit::GetValueAsString()
{
	return CString( CurrentMultiValue );
}
CString ListDataLPTSTREdit::GetAltVarAsString()
{
	return CString( pAltDatabaseVar? *pAltDatabaseVar : _T("") );
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ListDataLPTSTREdit, CEdit)
	//{{AFX_MSG_MAP(ListDataLPTSTREdit)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------//
// GetControlValue()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataLPTSTREdit::ExtractControlValue()
{
	// Deallocate.
	delete[] CurrentMultiValue;
	
	// Get currently displayed value.
	CString strCurrent;
	GetWindowText( strCurrent );

	// Reallocate.
	int nLength = strCurrent.GetLength();
	CurrentMultiValue = new TCHAR[ nLength + 1 ];
	_tcscpy( CurrentMultiValue, strCurrent );

}


//-------------------------------------------------------------------//
// DisplayControlValue()															//
//-------------------------------------------------------------------//
// Here, we set the control to contain the data in our current
// record set var.
// Note that we specifically bypass saving of changes.  We call this
// function when changing the master record, when changing the 
// master, etc., so we are not necessarily in need of saving.
//-------------------------------------------------------------------//
void ListDataLPTSTREdit::DisplayControlValue()
{
	m_bBypassChangesSave = true;
	SetWindowText( *pDatabaseVar );
	m_bBypassChangesSave = false;
}


//-------------------------------------------------------------------//
// SetDBFromControl()																//
//-------------------------------------------------------------------//
void ListDataLPTSTREdit::SetDBFromControl()
{
	// BASE CLASS IMPLEMENTATION:
	// *pDatabaseVar = CurrentMultiValue;

	// Deallocate.
	delete[] ( *pDatabaseVar );
	
	// Reallocate.
	int nLength = _tcsclen( CurrentMultiValue );
	( *pDatabaseVar ) = new TCHAR[ nLength + 1 ];
	_tcscpy( ( *pDatabaseVar ), CurrentMultiValue );

}


//-------------------------------------------------------------------//
// SetControlFromDB()																//
//-------------------------------------------------------------------//
void ListDataLPTSTREdit::SetControlFromDB()
{
	// BASE CLASS IMPLEMENTATION:
	// CurrentMultiValue = *pDatabaseVar;
	
	// Deallocate.
	delete[] CurrentMultiValue;
	
	// We MAY not have anything to copy.
	if ( 
			pDatabaseVar == 0 
		|| ( *pDatabaseVar ) == 0
	)
	{
		CurrentMultiValue = 0;
		return;
	}

	// Reallocate.
	int nLength = _tcsclen( *pDatabaseVar );
	CurrentMultiValue = new TCHAR[ nLength + 1 ];
	_tcscpy( CurrentMultiValue, ( *pDatabaseVar ) );

	// Now display as usual.	
	DisplayControlValue();
}


/////////////////////////////////////////////////////////////////////////////
// ListDataLPTSTREdit message handlers


//-------------------------------------------------------------------//
// OnKillFocus()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataLPTSTREdit::OnKillFocus(CWnd* pNewWnd) 
{

	if ( m_bUpdated )
	{
		// Don't prompt here.
		// That was already taken care of in OnUpdate().
		SavePendingChanges(
			true,
			m_bMultiPrompt
		);
	}
	m_bUpdated = false;

	inherited::OnKillFocus(pNewWnd);
	
}


//-------------------------------------------------------------------//
// OnSetFocus()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataLPTSTREdit::OnSetFocus(CWnd* pOldWnd) 
{

	inherited::OnSetFocus(pOldWnd);
	
	SetMultiDisplay( false );
	
	// Prompt next time we need to.
	m_bMultiPrompt = true;
	
}

//-------------------------------------------------------------------//
// OnUpdate()																			//
//-------------------------------------------------------------------//
// Here, we keep track of the fact that a change was made to the 
// text.  We update the list as well.
//-------------------------------------------------------------------//
void ListDataLPTSTREdit::OnUpdate() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the inherited::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	if ( !m_bBypassChangesSave )
	{
		m_bUpdated = true;	
		
		// This will update the list text, without saving to the db yet.
		SavePendingChanges( 
			false,					// bIncludeDBUpdate
			m_bMultiPrompt
		);	
	
		// Since we may have prompted, don't do it again until we have a new focus.
		m_bMultiPrompt = false;
	}

}
