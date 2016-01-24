//-------------------------------------------------------------------//
// ListDataMasterList.cpp : implementation file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "ListDataMasterList.h"

#include "..\..\BaseProgressDlg.h"			// We show progress on longer updates.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListDataMasterList


//-------------------------------------------------------------------//
// ListDataMasterList()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataMasterList::ListDataMasterList(
	CString	strUniqueRegistryName	,
	bool		bUseExternalData			,
	UINT		NoItemMenuID				,
	UINT		OneItemMenuID				,
	UINT		MultiItemMenuID
) :

	// Call base class.
	inherited(
		strUniqueRegistryName	,
		bUseExternalData			,
		NoItemMenuID				,
		OneItemMenuID				,
		MultiItemMenuID	
	),

	// Init vars.
	m_nControlCount	         ( 0		),
	m_bActive		            ( false	),
	m_bProcessingPrevSelection ( false  )

{
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// This damned function is necessary for any operations where you
// need access to the control window before it is shown.
//-------------------------------------------------------------------//
void ListDataMasterList::Initialize()
{
	// Call base class.
	inherited::Initialize();

	// We don't need to do anything here right now.
	// The base class set the styles for us.

}


//-------------------------------------------------------------------//
// ~ListDataMasterList()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
ListDataMasterList::~ListDataMasterList()
{
}


//-------------------------------------------------------------------//
// GetItemID()																			//
//-------------------------------------------------------------------//
// These return the lParam as the item's database ID.
// Derived classes may override as needed.
//-------------------------------------------------------------------//
long ListDataMasterList::GetItemID( int nItem )
{
	return (long) GetItemData( nItem );
}
long ListDataMasterList::GetItemID( LPARAM lParam )
{
	return (long) lParam;
}


//-------------------------------------------------------------------//
// AddControl()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataMasterList::AddControl(
	ListDataControlBase*		pNewControl
) {

	// Add the control to our list and increment our counter.
	// The counter will not change value once we are set up.  We use
	// it so we don't have to keep looking up the array size over
	// and over.
	DataControls.Add( pNewControl );
	m_nControlCount++;

}


//-------------------------------------------------------------------//
// RemoveControl()																	//
//-------------------------------------------------------------------//
// This is used during dynamic column removal.
//-------------------------------------------------------------------//
void ListDataMasterList::RemoveControl(
	int nIndex
) {
	DataControls.RemoveAt( nIndex );
	m_nControlCount--;
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ListDataMasterList, ListDataMasterList::inherited)
// BEGIN_MESSAGE_MAP(ListDataMasterList, CListCtrl)

	//{{AFX_MSG_MAP(ListDataMasterList)
	ON_NOTIFY_REFLECT(LVN_ODCACHEHINT, OnOdcachehint)
	//}}AFX_MSG_MAP

	// This is now an _EX so the parent can snag it, too.
	// THIS WOULDN'T COMPILE!  Shouldn't matter since
	// it is going to be caught by the base class... rrr...
	// ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED, OnItemchanged)

	// MS does not send this for owner data lists!  Fookers!
	// ON_NOTIFY_REFLECT(LVN_ITEMCHANGING, OnItemchanging)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ListDataMasterList message handlers


//-------------------------------------------------------------------//
// CheckControlForChanges()														//
//-------------------------------------------------------------------//
// This function is called when any control is losing focus upon
// entering the list.  We determine if the control is a data
// control, and if so, we call the data control version of this 
// function.
//-------------------------------------------------------------------//
void ListDataMasterList::CheckControlForChanges( 
	CWnd* pControl 
) {

	if ( pControl ) 
	{
		// We need to determine if this control is one of our data controls.
		
		// Unfortunately, dynamic_cast<> didn't seem up to the task of converting
		// the CWnd* to a DMCB*.  I guess it can't work its way up and then
		// back down the inheritance chain.
		// Note that we need to enable RTTI to use dynamic_cast.
		// ListDataControlBase* pDataControl = dynamic_cast<ListDataControlBase*> (pControl);

		// We'll compare the HWND of the control to the HWND's of our data controls.
		// TO DO
		// Can we make this look prettier by comparing CWnd* instead of HWND?
		HWND ControlHwnd = pControl->GetSafeHwnd();
		int i;
		for ( i = 0; i < m_nControlCount; i++ )
			if ( DataControls[i]->pControlWnd->GetSafeHwnd() == ControlHwnd )
				break;

		// If we found this to be a control, we want to save pending changes, as 
		// it is losing focus.
		if ( i < m_nControlCount )
			SavePendingChanges( DataControls[i] );

	}

}


//-------------------------------------------------------------------//
// SavePendingChanges()																//
//-------------------------------------------------------------------//
// This function is called when one of our data controls is losing 
// focus.  It is time to update all the selected items in the list
// with the value in our control.
//
// We loop through the current selection, and find the corresponding
// record set for each.  Then, we tell the control to update its 
// database variable for each set we find.
//
// Make sure that the data control's internal value has been updated 
// with the contents of the control before calling this function.
//-------------------------------------------------------------------//
void ListDataMasterList::SavePendingChanges( 
	ListDataControlBase* pControl,
	bool						bIncludeDBUpdate
) {
   // We need to track that processing is happening.
   // This prevents a nasty bug where OnOdcachehint() changes
   // the prev selection WHILE we are processing it.
   m_bProcessingPrevSelection = true;

	// Make sure we have the control's value.
	pControl->ExtractControlValue();

	// Save changes to all selected items.
	
	// Set a wait cursor if there are a number of items to update.
	CWaitCursor* pWC = 0;
	if ( GetSelectedCount() > 10 )
		pWC = new CWaitCursor;
	
	LongVector::iterator itps;
	for ( itps = m_lvPreviousSelection.begin(); itps != m_lvPreviousSelection.end(); itps++ )
	{
		int nItem = *itps;

		if ( bIncludeDBUpdate )
		{
			// Find new record set.
			if ( SeekRecord( GetItemID( nItem ) ) )
			{
			   // Has the value for this control changed?
				if ( pControl->bNeedsUpdate() )
				{
					// We have a record that needs updating, 
					// save the control's value to the db.
					EditCurrentRecord( nItem );

					pControl->SetDBFromControl();
					
					// Before we do the update, allow derived classes
					// to take action.  This allows members of the set
					// to be updated without the need for another 
					// Edit()/Update() session.
					
					// TO DO
					// Update the derived version of this to handle
					// any needed changes to whatever is controlling
					// the master set.
					
					SetEditHasOccurred();

					// TO DO
					// Also notify other potential master lists, providing the set.

					// Save the result.
					SaveCurrentRecord( nItem );

				}
			}
		}

		// Now update the list item.
		pControl->UpdateListItem( nItem );

	}

	// Only now that we have saved do we want to clear the 
	// multi-value data.
	pControl->ClearCount();

	// Clean up.
	if ( pWC )
		delete pWC;

   // Does this need to be a mutex lock?
   m_bProcessingPrevSelection = false;

}


//-------------------------------------------------------------------//
// ReloadControls()																	//
//-------------------------------------------------------------------//
// This function is called when we need to totally update the
// controls based on the current selection.  This occurs when
// for example the master list for a set of controls changes.
//-------------------------------------------------------------------//
void ListDataMasterList::ReloadControls()
{
	int i;

	int nItem = GetNextItem( -1, LVNI_SELECTED );
	
	// If no selection, clear the controls and return.
	if ( nItem == -1 )
	{
		for ( i = 0; i < m_nControlCount; i++ ) 
		{
			DataControls[i]->ClearCount();
			
			// TO DO
			// Clear the control contents.
			// This will require all derived classes to provide an override.
			// DataControls[i]->ClearControl();
		}
		return;
	}
	
	bool bFirstItem = true;
	while ( nItem != -1 ) {

		// Find new record set.
		/*
		COleVariant IDCriterion( GetItemID( nItem ), VT_I4 );
		if (
			m_pSet->Seek(
				_T("="),
				&IDCriterion
			) != FALSE
		) {
		*/
		if ( SeekRecord( GetItemID( nItem ) ) )
		{
			
			// If this is the first item...
			if ( bFirstItem ) 
			{
				// Set the record set's values into the data controls,
				// and clear the count.  Both these functions know to 
				// use the values in the current record set.
				for ( i = 0; i < m_nControlCount; i++ ) 
				{
					DataControls[i]->SetControlFromDB();
					DataControls[i]->ClearCount();
				}

				UpdateData( FALSE );

				bFirstItem = false;

			} else 
			{
				// Get the current data.
				UpdateData();

				// Update the controls as needed.
				for ( i = 0; i < m_nControlCount; i++ ) 
					DataControls[i]->AddSelectedItem();

			}		

		}

		nItem = GetNextItem( nItem, LVNI_SELECTED );

	}

}


//-------------------------------------------------------------------//
// OnOdcachehint()																	//
//-------------------------------------------------------------------//
// Since MS removed the LVS_ITEMCHANGING notification for owner data
// list controls (STOOPID FOOKERS), we hijack this message, which
// appears to be always sent right before the selection changes
// (in addition to another 50 other times it is sent, oh well...).
//-------------------------------------------------------------------//
void ListDataMasterList::OnOdcachehint(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if ( !m_bProcessingPrevSelection )
	{
		NMLVCACHEHINT* pCacheHint = (NMLVCACHEHINT*)pNMHDR;

		// We keep track of the selected items here, so we
		// know what to update on LVN_ITEMCHANGED messages.
		m_lvPreviousSelection.clear();
		int nA = GetNextItem( -1, LVNI_SELECTED );
		while ( nA != -1 )
		{
			m_lvPreviousSelection.push_back( nA );
			nA = GetNextItem( nA, LVNI_SELECTED );
		}
	}

	*pResult = 0;
}


//-------------------------------------------------------------------//
// OnItemchanged()																	//
//-------------------------------------------------------------------//
BOOL ListDataMasterList::OnItemchanged( NMHDR* pNMHDR, LRESULT* pResult ) 
{
   // Base class does checkbox handling if needed...
	inherited::OnItemchanged( pNMHDR, pResult );

	int nA;	

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	// Check the first control.  If we are no longer its master, then
	// skip the entire update process, as another master is currently
	// in charge.
	if ( DataControls.GetSize() == 0 || DataControls[0]->pMaster != this )
   {
      return FALSE;
   }

	// If we are newly selecting this item...
	if ( 
			pNMListView->uNewState & LVIS_SELECTED
		&&	!( pNMListView->uOldState & LVIS_SELECTED	)
	) {

		// Save pending changes.
		CheckControlForChanges( GetParent()->GetFocus() );

		// Get the corresponding "record set" using whatever
		// database scheme the user is providing.
		if ( 
				pNMListView->iItem
			&&	SeekRecord( GetItemID( pNMListView->iItem ) ) 
		) {
			// If there aren't any other items selected...
			// if ( GetSelectedCount() == 0 ) 
			if ( GetSelectedCount() == 1 ) 
			{
				// Set the record set's values into the data controls,
				// and clear the count.  Both these functions know to 
				// use the values in the current record set.
				for ( int i = 0; i < m_nControlCount; i++ ) 
				{
					DataControls[i]->SetControlFromDB();
					DataControls[i]->ClearCount();

					// This goes back and updates the text in the list column.
					// Since we can have multiple master lists, lists that have
					// not been master can get out of date.  This helps keep
					// them in synch some.
					// TO DO
					// Improve this so that highlighted items in any list
					// with any number of selected items get updated.
					DataControls[i]->UpdateListItem( pNMListView->iItem );
				}

				UpdateData( FALSE );

			} else 
			{
				// Get the current data.
				UpdateData();

				// Update the controls as needed.
				for ( nA = 0; nA < m_nControlCount; nA++ ) 
				{
					DataControls[nA]->AddSelectedItem();
				}

			}		
		
		}

	} else if ( 
			pNMListView->uOldState & LVIS_SELECTED
		&&	!( pNMListView->uNewState & LVIS_SELECTED	)
	) {
		// Save pending changes.
		CheckControlForChanges( GetParent()->GetFocus() );

		// If there are other items still selected...
		if ( GetSelectedCount() > 0 ) 
		{		
			// Get the corresponding "record set" using whatever
			// database scheme the user is providing.
			// if ( SeekRecord( GetItemID( pNMListView->lParam ) ) )
			if ( 
					pNMListView->iItem
				&&	SeekRecord( GetItemID( pNMListView->iItem ) ) 
			) {
				// Update the controls as needed.
				for ( nA = 0; nA < m_nControlCount; nA++ ) 
				{
					DataControls[nA]->RemoveSelectedItem();
				}

			}
		
		}		

	}

	// Lots of people rely on this coming through to the parent.
	// Always let the parent have a crack at it.
	// NOTE TO MICROSOFT: SUCK IT!  Docs say return TRUE...
	return FALSE;	
}


// Bastard MS no longer sends this message for Owner Data lists.
/*
//-------------------------------------------------------------------//
// OnItemchanging()																	//
//-------------------------------------------------------------------//
// This function handles changes to the selection.  
//
//
// Setting The Selection
//
// When the selection is first set, all the data controls are filled
// with data from the corresponding record set.
//
//
// Adding To The Selection
//
// When adding to the selection, the controls need to be told, so
// they can adjust themselves to the values they are representing,
// displaying in "multi-mode" as needed.
// Also, ordinarily, changes to records represented by lines in the 
// list are made when a control loses focus.  However, if the control 
// is losing focus to the list control, we need to do the saving
// here, because by the time the control loses the focus, the 
// selection will have changed.
//
//
// Removing From The Selection
//
// As in adding, the controls need to be made aware of removals from 
// the selection, for adjusting their displays as needed.
//
//-------------------------------------------------------------------//
void ListDataMasterList::OnItemchanging(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	// Check the first control.  If we are no longer its master, then
	// skip the entire update process, as another master is currently
	// in charge.
	if ( DataControls[0]->pMaster != this )
		return;

	int i;	
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;


	// DEBUG
	TRACE( _T("Selection change caught via LVN_ITEMCHANGING, item %d, from state %d to %d\n"), pNMListView->iItem, pNMListView->uOldState, pNMListView->uNewState );


	// If we are newly selecting this item...
	if ( 
			pNMListView->uNewState & LVIS_SELECTED
		&&	!( pNMListView->uOldState & LVIS_SELECTED	)
	) {

		// Save pending changes.
		CheckControlForChanges( GetParent()->GetFocus() );

		// Get the corresponding "record set" using whatever
		// database scheme the user is providing.
		if ( SeekRecord( GetItemID( pNMListView->lParam ) ) )
		{

			// If there aren't any other items selected...
			if ( GetSelectedCount() == 0 ) 
			{
				// Set the record set's values into the data controls,
				// and clear the count.  Both these functions know to 
				// use the values in the current record set.
				for ( int i = 0; i < m_nControlCount; i++ ) 
				{
					DataControls[i]->SetControlFromDB();
					DataControls[i]->ClearCount();

					// This goes back and updates the text in the list column.
					// Since we can have multiple master lists, lists that have
					// not been master can get out of date.  This helps keep
					// them in synch some.
					// TO DO
					// Improve this so that highlighted items in any list
					// with any number of selected items get updated.
					DataControls[i]->UpdateListItem( pNMListView->iItem );
				}

				UpdateData( FALSE );

			} else 
			{
				// Get the current data.
				UpdateData();

				// Update the controls as needed.
				for ( i = 0; i < m_nControlCount; i++ ) 
				{
					DataControls[i]->AddSelectedItem();
				}

			}		
		
		}

	} else if ( 
			pNMListView->uOldState & LVIS_SELECTED
		&&	!( pNMListView->uNewState & LVIS_SELECTED	)
	) {

		// Save pending changes.
		CheckControlForChanges( GetParent()->GetFocus() );

		// If there are other items still selected...
		// if ( GetSelectedCount() > 0 ) 
		if ( m_lvPreviousSelection.size() > 1 ) 
		{		
			// Get the corresponding "record set" using whatever
			// database scheme the user is providing.
			if ( SeekRecord( GetItemID( pNMListView->lParam ) ) )
			{
				// Update the controls as needed.
				for ( i = 0; i < m_nControlCount; i++ ) 
				{
					DataControls[i]->RemoveSelectedItem();
				}

			}
		
		}		

	}
	
}
*/

