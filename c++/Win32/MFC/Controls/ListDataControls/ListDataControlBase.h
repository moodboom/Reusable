//-------------------------------------------------------------------//
// ListDataControl.h : header file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined(LIST_DATA_CONTROL_BASE_H)
	#define LIST_DATA_CONTROL_BASE_H

#pragma warning(disable : 4995)					// Shut up VS 2003 "deprecated" blabbing

#include <afx.h>										// For CString

#include "..\..\StandardTypes.h"

class ListDataMasterList;							// We notify the list on KillFocus()
class CDaoRecordset;									// A potential master

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// ListDataControlBase


class ListDataControlBase
{

// Construction
public:
   
	ListDataControlBase();

   ~ListDataControlBase();

	// At any given time, this control has one "master", either
	// a ListDataMasterList for which we manage the represented
	// record sets, or one specific CDaoRecordSet, tied directly
	// to us.
	ListDataMasterList*	pMaster;
	CDaoRecordset*			pMasterSet;

	// One of these must be called before using this control.
	// The master can be changed at any time, as well.
	void SetMaster(
		ListDataMasterList*	pNewMaster
	);
	void SetMaster(
		CDaoRecordset*			pNewMasterSet
	);

	CWnd*						pControlWnd;

	int						nListColumn;

	// This calls the master's version, after getting permission 
	// from the user if needed.
	// It is virtual in case derived classes need to take extra
	// action on changes.  MAKE SURE you call the base class!
	virtual void SavePendingChanges( 
		bool	bIncludeDBUpdate	= true,
		bool	bMultiPrompt		= true
	);

	// Our control functions.  We provide the abstract base class 
	// functions, which the master list can call generically for
	// all types of data controls.
	virtual void AddSelectedItem() = 0;
	virtual void RemoveSelectedItem() = 0;
	virtual void SetDBFromControl() = 0;
	virtual void SetControlFromDB() = 0;
	virtual void ExtractControlValue() = 0;			// DDX-ish function.
	virtual void DisplayControlValue() = 0;			// DDX-ish function.
	virtual CString GetValueAsString() = 0;
	virtual CString GetAltVarAsString() = 0;
	virtual bool bNeedsUpdate() = 0;

	// This function can be overridden to perform some action 
	// upon change of a list item.  Make sure derived classes
	// call this base class version.
	virtual void UpdateListItem( int nItem );

	// This special version allows specification of a temporary
	// master.  It is typically used after newly adding an item to a list,
	// to update all the list column text.  It does not use the value
	// in the standard database set var, it uses the "alternate".
	virtual void UpdateListItemFromAlternateSet( 
		int nItem, 
		ListDataMasterList* pTempMasterList
	);

	// This function allows the user to set a killable warning message
	// that will display upon multi-item change attempts.
	void SetMultiWarning( UINT LabelID, UINT WarnID )
	{
		m_bWarnOnMultiUpdate = true;
		m_MultiWarningLabelID = LabelID;
		m_MultiWarningID = WarnID;
	}
	
	int GetValuesCount() { return nValuesCount; }

	// This function clears the current count.  
	// We have recently highlighted a new item and updated the control
	// with the new database value, or have left a control
	// and saved its changes.
	void ClearCount()
	{
		nValuesCount = 0;
		SetMultiDisplay( false );

	}

protected:

	// The number of different values currently contained by
	// the fields referencing this control.
	int	nValuesCount;

	virtual void SetMultiDisplay( bool bMulti = true );
	bool bMultiModeOn;

	// These can be set to require a warning before a multi-item change is made.
	bool	m_bWarnOnMultiUpdate;
	UINT	m_MultiWarningLabelID;
	UINT	m_MultiWarningID;

	// This can be used to temporarily turn off saving of changes to
	// the control.  This is set when we want to update the contents 
	// of the control while making sure that the change is not saved.
	// As an example, we use this when changing the master of the 
	// control on-the-fly.  We need to update the contents of the 
	// control to reflect the new master but do NOT need to save anything.
	bool m_bBypassChangesSave;
};


// 		CDaoRecordset*			pNewMaster,
#endif // !defined(LIST_DATA_CONTROL_BASE_H)
