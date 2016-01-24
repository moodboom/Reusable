//-------------------------------------------------------------------//
// ListDataLongEdit.cpp : implementation file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "ListDataMasterList.h"			// We notify this on KillFocus()

#include "ListDataLongEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListDataLongEdit


//-------------------------------------------------------------------//
// ListDataLongEdit()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataLongEdit::ListDataLongEdit()
:

	// Init vars.
	m_bUpdated		( false	),
	m_bMultiPrompt	( true	)

{
}


//-------------------------------------------------------------------//
// ~ListDataLongEdit()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataLongEdit::~ListDataLongEdit()
{
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
void ListDataLongEdit::Initialize(
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
CString ListDataLongEdit::GetValueAsString()
{
	return strOutput( CurrentMultiValue );
}
CString ListDataLongEdit::GetAltVarAsString()
{
	return strOutput( *pAltDatabaseVar );
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ListDataLongEdit, CEdit)
	//{{AFX_MSG_MAP(ListDataLongEdit)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------//
// GetControlValue()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataLongEdit::ExtractControlValue()
{

	CString strValue;
	GetWindowText( strValue );
	CurrentMultiValue = _ttol( LPCTSTR( strValue ) );

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
void ListDataLongEdit::DisplayControlValue()
{
	m_bBypassChangesSave = true;

	SetWindowText( strOutput( *pDatabaseVar ) );

	m_bBypassChangesSave = false;
}


//-------------------------------------------------------------------//
// strOutput()																			//
//-------------------------------------------------------------------//
// This is the helper that does the formatting.
// Some derived classes override for special display formatting.
//-------------------------------------------------------------------//
CString ListDataLongEdit::strOutput( Long lValue )
{
	CString strValue;
	strValue.Format( _T("%d" ), lValue );
	return strValue;
}


/////////////////////////////////////////////////////////////////////////////
// ListDataLongEdit message handlers


//-------------------------------------------------------------------//
// OnKillFocus()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataLongEdit::OnKillFocus(CWnd* pNewWnd) 
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
void ListDataLongEdit::OnSetFocus(CWnd* pOldWnd) 
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
void ListDataLongEdit::OnUpdate() 
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
