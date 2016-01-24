//-------------------------------------------------------------------//
// ListDataControl.cpp : implementation file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"						// For standard Windows schmegma

#include <afxdao.h>						// For Dao access - see "DATABASE ABSTRACTION PROBLEM" note in log

#include "ListDataControlBase.h"

#include "ListDataMasterList.h"		// We change items in UpdateListItem() 
#include "..\..\DisplayMessage.h"	// For KillableWarning()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//-------------------------------------------------------------------//
// ListDataControlBase()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataControlBase::ListDataControlBase() 
:

	// Init vars.
	pMaster					(	0		),
	nValuesCount			(	0		),
	bMultiModeOn			(	false ),
	m_bWarnOnMultiUpdate	(	false	),
	m_bBypassChangesSave	(	false	)

{
}


//-------------------------------------------------------------------//
// ~ListDataControlBase()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataControlBase::~ListDataControlBase()
{
}


//-------------------------------------------------------------------//
// SetMaster()																			//
//-------------------------------------------------------------------//
void ListDataControlBase::SetMaster(
	ListDataMasterList*	pNewMaster
) {
	// Save control data to database first on resets.
	// Nah, I can't imagine a case where we got here without
	// the control first losing focus, which would have already
	// forced a save of pending changes.
	/*
	pMaster->SavePendingChanges( this );
	*/

	pMaster			= pNewMaster;
	pMasterSet		= 0;

	// Now update control data from the database.
	// Bypass saving anything.
	m_bBypassChangesSave = true;
	SetControlFromDB();
	m_bBypassChangesSave = false;
}


//-------------------------------------------------------------------//
// SetMaster()																			//
//-------------------------------------------------------------------//
void ListDataControlBase::SetMaster(
	CDaoRecordset*			pNewMasterSet
) {

	pMaster			= 0;
	pMasterSet		= pNewMasterSet;

	// We need to clear the control's multi status.
	// Single-set controls don't use it.
	ClearCount();
	
	// Now update control data from the database.
	// Bypass saving anything.
	m_bBypassChangesSave = true;
	SetControlFromDB();
	m_bBypassChangesSave = false;
}


//-------------------------------------------------------------------//
// SavePendingChanges()																//
//-------------------------------------------------------------------//
// This gets called by the derived control when appropriate to
// save changes.  Typically this occurs when the control loses
// focus, but may also occur when (for example) a combobox selection
// is changed.
//-------------------------------------------------------------------//
void ListDataControlBase::SavePendingChanges( 
	bool	bIncludeDBUpdate,
	bool	bMultiPrompt
) {	
	// One and only one of the following should be set.
	ASSERT( 
			( pMaster && pMasterSet == 0 )
		|| ( pMasterSet && pMaster == 0 )
	);

	if ( m_bBypassChangesSave )
		return;
	
	if ( pMaster )
	{
		// We may need to get permission to do the update first
		// if it is for multiple items.  This depends on the
		// way the coder set up the control and the preferences
		// of the user.
		static bool sbDontAsk = false;
		static bool sbDoTheUpdate;
		bool bCheckMulti = bMultiPrompt && m_bWarnOnMultiUpdate && ( nValuesCount > 0 );

		if ( bCheckMulti )
		{
			KillableMessage(
				CString( LPCSTR( m_MultiWarningID ) ), 
				CString( LPCSTR( m_MultiWarningLabelID ) ), 
				sbDontAsk,
				sbDoTheUpdate
			);

			// Don't allow the user to always reply "No" to multi-changes.
			// They will just have to be careful not to edit with more than 
			// one selection in that case!!  We don't want to just start 
			// chucking all their fool changes.  Iiiiii-diots!!
			if ( sbDontAsk && !sbDoTheUpdate )
				sbDontAsk = false;
		}
		
		if ( !bCheckMulti || sbDoTheUpdate )
		{
			// We tell the list to update its selected items.
			pMaster->SavePendingChanges( this, bIncludeDBUpdate );
		}

	} else
	{
		ExtractControlValue();
		pMasterSet->Edit();
		SetDBFromControl();

		// TO DO
		// Notify all potential master lists that a change has occurred,
		// and provide pMasterSet for details.

		pMasterSet->Update();
	}

}


//-------------------------------------------------------------------//
// UpdateListItem()																	//
//-------------------------------------------------------------------//
// This updates the item in the list control with our new value.
//-------------------------------------------------------------------//
void ListDataControlBase::UpdateListItem( int nItem )
{

	if ( nItem != -1 && nListColumn != -1 )
		pMaster->SetItemText( 
			nItem,
			nListColumn,
			LPCTSTR( GetValueAsString() )		
		);

}


//-------------------------------------------------------------------//
// UpdateListItem()																	//
//-------------------------------------------------------------------//
// This updates the item in the list control with our new value.
// This special version allows specification of a temporary
// master.  It is typically used after newly adding an item to a list,
// to update all the list column text.  It does not use the value
// in the standard database set var, it uses the "alternate".
//-------------------------------------------------------------------//
void ListDataControlBase::UpdateListItemFromAlternateSet( 
	int						nItem,
	ListDataMasterList*	pTempMasterList
) {
	if ( nItem != -1 && nListColumn != -1 )
		pTempMasterList->SetItemText( 
			nItem,
			nListColumn,
			LPCTSTR( GetAltVarAsString() )
		);
}


//-------------------------------------------------------------------//
// SetMultiDisplay()																	//
//-------------------------------------------------------------------//
// This version of the function modifies the extended
// style of the control to turn on/off the static edge property,
// and then invalidates and repaints the control.  This
// is the preferred way of showing the multi-value status.
// Derived controls that need special handling can override
// this function to suit their needs.
//-------------------------------------------------------------------//
void ListDataControlBase::SetMultiDisplay( bool bMulti )
{
	
	// We keep track of the current mode, so we only redraw
	// when absolutely needed, to reduce flicker.
	if ( bMulti != bMultiModeOn ) {

		bMultiModeOn = bMulti;
	
		pControlWnd->ModifyStyleEx(
			bMulti?	0						:	WS_EX_STATICEDGE,
			bMulti?	WS_EX_STATICEDGE	:	0
		);
		CRect rect;
		pControlWnd->GetWindowRect( &rect );
		CWnd* pParent = pControlWnd->GetParent();
		pParent->ScreenToClient(	rect );
		pParent->InvalidateRect(	rect );
		pParent->UpdateWindow();

	}
	
}
