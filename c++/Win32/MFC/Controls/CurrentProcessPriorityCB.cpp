// CurrentProcessPriorityCB.cpp : implementation file
//

#include "stdafx.h"
#include "..\AppHelpers.h"					// For bWin2000OrGreater()


#include "CurrentProcessPriorityCB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CurrentProcessPriorityCB

CurrentProcessPriorityCB::CurrentProcessPriorityCB(
	bool bImmediatelyUpdate
) :

	// Init vars.
	m_bImmediatelyUpdate( bImmediatelyUpdate )

{
}

CurrentProcessPriorityCB::~CurrentProcessPriorityCB()
{
}


BEGIN_MESSAGE_MAP(CurrentProcessPriorityCB, CComboBoxEx)
	//{{AFX_MSG_MAP(CurrentProcessPriorityCB)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CurrentProcessPriorityCB message handlers


//-------------------------------------------------------------------//
// PreSubclassWindow()																//
//-------------------------------------------------------------------//
// Here, we do our initialization: Load up available priorities!
//
// NOTE: the following never gets hit.
// int CurrentProcessPriorityCB::OnCreate(LPCREATESTRUCT lpCreateStruct) 
//
//-------------------------------------------------------------------//
void CurrentProcessPriorityCB::PreSubclassWindow() 
{
	CComboBoxEx::PreSubclassWindow();

	// Make sure the styles were set correctly for the CB resource.
	// NO SORTING and DROP LIST are required.
	// We were hoping this code would do it:
	/*
			ModifyStyle(
				CBS_SORT,
				CBS_DROPDOWNLIST
			);
	*/
	// Unfortunately, we could not find a way to enforce this in
	// code.  We tried calling ModifyStyle here, in PreSubclassWindow(),
	// but changes had no effect.  We tried changing the style in
	// PreCreateWindow(), and in OnCreate(), but they never get called.
	// We cannot override CComboBox::Create, as it isn't virtual.
	// So we are left with getting the style and ASSERTing that it is
	// correct, until we find a better way.
	// Note that the DDown and DDList defines bits 'overlap':
	//		#define CBS_DROPDOWN          0x0002L
	//		#define CBS_DROPDOWNLIST      0x0003L
	//		#define CBS_SORT              0x0100L
	// This means that we should examine CBS_DROPDOWNLIST, and ignore
	// CBS_DROPDOWN, which will always be set.
	ASSERT( ( GetStyle() & CBS_DROPDOWNLIST ) == CBS_DROPDOWNLIST );

	// ALSO MAKE SURE YOU ARE USING A CComboBoxEx RESOURCE, NOT A CComboBox!
	// TO DO
	// How can we check on this?  For one, this fires even though there IS
	// no sort style available!
	// ASSERT( ( GetStyle() & CBS_SORT ) == 0 );

	// Now fill the list up.
	
	// First prep...
	int nItem;
	COMBOBOXEXITEM cbItem;
	ZeroMemory( &cbItem, sizeof( COMBOBOXEXITEM ) );
	cbItem.mask = CBEIF_TEXT | CBEIF_LPARAM;		// | CBEIF_IMAGE
	DWORD dwCurrentPriority = GetPriorityClass( GetCurrentProcess() );
	
	// Then fill...
											cbItem.pszText = _T( "Very low (risky!)"	);	cbItem.iItem = GetCount(); cbItem.lParam = IDLE_PRIORITY_CLASS				;	nItem = InsertItem( &cbItem );	if ( dwCurrentPriority == (DWORD)cbItem.lParam ) SetCurSel( nItem );
	
	// TO DO
	// Wrap these in the correct #define's so that we can compile on NT 4.	
	if ( bWin2000OrGreater() )	{	cbItem.pszText = _T("Below normal"			); cbItem.iItem = GetCount(); cbItem.lParam = BELOW_NORMAL_PRIORITY_CLASS	;	nItem = InsertItem( &cbItem );	if ( dwCurrentPriority == (DWORD)cbItem.lParam ) SetCurSel( nItem );	}
											cbItem.pszText = _T("Normal"					); cbItem.iItem = GetCount(); cbItem.lParam = NORMAL_PRIORITY_CLASS			;	nItem = InsertItem( &cbItem );	if ( dwCurrentPriority == (DWORD)cbItem.lParam ) SetCurSel( nItem );
	if ( bWin2000OrGreater() )	{	cbItem.pszText = _T("Above normal"			); cbItem.iItem = GetCount(); cbItem.lParam = ABOVE_NORMAL_PRIORITY_CLASS	;	nItem = InsertItem( &cbItem );	if ( dwCurrentPriority == (DWORD)cbItem.lParam ) SetCurSel( nItem );	}
											cbItem.pszText = _T("High"						); cbItem.iItem = GetCount(); cbItem.lParam = HIGH_PRIORITY_CLASS				;	nItem = InsertItem( &cbItem );	if ( dwCurrentPriority == (DWORD)cbItem.lParam ) SetCurSel( nItem );
											cbItem.pszText = _T("Very high (risky!)"	); cbItem.iItem = GetCount(); cbItem.lParam = REALTIME_PRIORITY_CLASS		;	nItem = InsertItem( &cbItem );	if ( dwCurrentPriority == (DWORD)cbItem.lParam ) SetCurSel( nItem );

	ASSERT( GetCurSel() != CB_ERR );
	
	// TO DO
	// Also, provide a static function that sets the current process priority, to restore results on program startup.
	//		Um, just use SetPriorityClass( GetCurrentProcess(), dw ), silly.
	// Also, use the return value from SetProcessPriorityFromSelection() to save results.
	// Also, attempt to add small bitmaps for each item.
}


//-------------------------------------------------------------------//
// SetProcessPriorityFromSelection()											//
//-------------------------------------------------------------------//
// Here, we actually set the current process's priority, based
// on the current CB selection.  Typically call this in your dialog's
// OnOK() function.
//-------------------------------------------------------------------//
DWORD CurrentProcessPriorityCB::SetProcessPriorityFromSelection()
{
	// Get the current selection.
	int nItem = GetCurSel();
	if ( nItem == CB_ERR )
	{
		ASSERT( false );
		return -1;
	}

	// Get the priority.
	DWORD dw = GetItemData( nItem );

	// Set the new priority for the current process.
	VERIFY( SetPriorityClass( GetCurrentProcess(), dw ) );

	return dw;
}


//-------------------------------------------------------------------//
// OnSelChange()																		//
//-------------------------------------------------------------------//
// Immediately update the priority if requested.
//-------------------------------------------------------------------//
void CurrentProcessPriorityCB::OnSelChange() 
{
	if ( m_bImmediatelyUpdate )
		SetProcessPriorityFromSelection();
}
