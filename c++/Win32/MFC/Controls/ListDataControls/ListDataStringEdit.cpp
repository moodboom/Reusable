//-------------------------------------------------------------------//
// ListDataStringEdit.cpp : implementation file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "ListDataMasterList.h"			// We notify this on KillFocus()

#include "ListDataStringEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListDataStringEdit


//-------------------------------------------------------------------//
// ListDataStringEdit()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataStringEdit::ListDataStringEdit()
:
	// Init vars.
	m_bUpdated		( false	),
	m_bMultiPrompt	( true	)

{
}


//-------------------------------------------------------------------//
// ~ListDataStringEdit()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataStringEdit::~ListDataStringEdit()
{
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
void ListDataStringEdit::Initialize(
	CString*					pNewDBVar,
	CString*					pNewAltDBVar,
	int						nNewListColumn
) {

	ListDataControlData<CString>::Initialize(
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
CString ListDataStringEdit::GetValueAsString()
{
	return CurrentMultiValue;
}
CString ListDataStringEdit::GetAltVarAsString()
{
	return *pAltDatabaseVar;
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ListDataStringEdit, CEdit)
	//{{AFX_MSG_MAP(ListDataStringEdit)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------//
// GetControlValue()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataStringEdit::ExtractControlValue()
{

	GetWindowText( CurrentMultiValue );

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
void ListDataStringEdit::DisplayControlValue()
{
	m_bBypassChangesSave = true;
	SetWindowText( LPCTSTR( *pDatabaseVar ) );
	m_bBypassChangesSave = false;
}


/////////////////////////////////////////////////////////////////////////////
// ListDataStringEdit message handlers


//-------------------------------------------------------------------//
// OnKillFocus()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataStringEdit::OnKillFocus(CWnd* pNewWnd) 
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
void ListDataStringEdit::OnSetFocus(CWnd* pOldWnd) 
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
void ListDataStringEdit::OnUpdate() 
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
