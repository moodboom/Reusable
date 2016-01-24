//-------------------------------------------------------------------//
// ListDataControlData.h : header file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined(LIST_DATA_CONTROL_DATA_H)
	#define LIST_DATA_CONTROL_DATA_H

#include <afx.h>										// For CString

#include "..\..\StandardTypes.h"

#include "ListDataControlBase.h"					// Base class

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// ListDataControlData


template <class T>
class ListDataControlData : public ListDataControlBase
{

// Construction
public:
   
	ListDataControlData();

   ~ListDataControlData();

	// This is used to give us our initial required data.
	// Then we require additional calls to SetMaster() and
	// pMaster->AddControl().
	void Initialize(
		T*							pNewDBVar,
		T*							pNewAltDBVar,
		int						nNewListColumn,
		CWnd*						pNewCWnd
	);

	// Base class overrides.
	void AddSelectedItem();
	void RemoveSelectedItem();
	void SetDBFromControl();
	void SetControlFromDB();
	bool bNeedsUpdate();

	T GetCurrentValue()
	{
		return CurrentMultiValue;
	}

	void UpdateVariables(
		T* pNewDBVar,
		T* pNewAltDBVar
	);

protected:	

	// This is the current value contained within our control.
	T		CurrentMultiValue;

	// This points to the variable within the record set that
	// is represented by our control.
	T*		pDatabaseVar;

	// This points to the variable within an "alternate" record set.
	// It is used for alternate purposes other than display of the
	// currently-selected list item.  For example, it is used
	// to hold the database value of items newly added to the list.
	T*		pAltDatabaseVar;

};


//-------------------------------------------------------------------//
// ListDataControlData()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
template <class T>
ListDataControlData<T>::ListDataControlData() 
{
}


//-------------------------------------------------------------------//
// ~ListDataControlData()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
template <class T>
ListDataControlData<T>::~ListDataControlData()
{
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// Use this function, along with SetMaster(), when there are 
// potentially more than one master that is changable.
//-------------------------------------------------------------------//
template <class T>
void ListDataControlData<T>::Initialize(
	T*							pNewDBVar,
	T*							pNewAltDBVar,
	int						nNewListColumn,
	CWnd*						pNewCWnd
) {
	pDatabaseVar		= pNewDBVar;
	pAltDatabaseVar	= pNewAltDBVar;
	pControlWnd			= pNewCWnd;
	nListColumn			= nNewListColumn;
}


//-------------------------------------------------------------------//
// SetDBFromControl()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
template <class T>
void ListDataControlData<T>::SetDBFromControl()
{
	*pDatabaseVar = CurrentMultiValue;
}


//-------------------------------------------------------------------//
// SetControlFromDB()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
template <class T>
void ListDataControlData<T>::SetControlFromDB()
{
	CurrentMultiValue = *pDatabaseVar;
	DisplayControlValue();
}


//-------------------------------------------------------------------//
// AddSelectedItem()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
template <class T>
void ListDataControlData<T>::AddSelectedItem()
{

	if ( *pDatabaseVar != CurrentMultiValue ) 
	{
		if ( nValuesCount == 0 )
		{
			SetMultiDisplay();
		}
		nValuesCount++;
	}

}


//-------------------------------------------------------------------//
// RemoveSelectedItem()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
template <class T>
void ListDataControlData<T>::RemoveSelectedItem() 
{

	if ( *pDatabaseVar != CurrentMultiValue ) 
	{
		nValuesCount--;
		if ( nValuesCount == 0 ) {

			SetMultiDisplay( false );

		}
	}
}


//-------------------------------------------------------------------//
// bNeedsUpdate()																		//
//-------------------------------------------------------------------//
// This function returns true only if the db value does not match
// the currently displayed value.
//-------------------------------------------------------------------//
template <class T>
bool ListDataControlData<T>::bNeedsUpdate()
{
	return ( CurrentMultiValue != *pDatabaseVar );
}


//-------------------------------------------------------------------//
// UpdateVariables()																	//
//-------------------------------------------------------------------//
// Here, we allow the caller to adjust the location of the variables
// that store our data.  This is required when the storage area
// changes (typically after a re-allocation).
//-------------------------------------------------------------------//
template <class T>
void ListDataControlData<T>::UpdateVariables(
	T* pNewDBVar,
	T* pNewAltDBVar
) {
	pDatabaseVar		= pNewDBVar;
	pAltDatabaseVar	= pNewAltDBVar;
}


#endif // !defined(LIST_DATA_CONTROL_DATA_H)
