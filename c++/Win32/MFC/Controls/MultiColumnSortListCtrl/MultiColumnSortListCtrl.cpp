//-------------------------------------------------------------------//
// MultiColumnSortListCtrl.cpp : implementation file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//


#include "stdafx.h"

#include <afxtempl.h>								// For CArray

#include <commctrl.h>								// For Platform SDK filter resources

#include "..\..\AppRegistryData.h"				// For access to g_pAppRegData and registry get/set functions
														
#include "..\..\AppHelpers.h"						// For bCommonControlsUpToDate().

#include "..\..\DisplayMessage.h"

#include "..\..\SubstringSearcher.h"			// For SubstringSearcher class, used in filtering

#include "..\..\..\TimeHelpers.h"				// For CalcTimeSpan(), used during timing tests

#include "..\..\..\..\AssertHelpers.h"			// For ASSERT()

#include "..\..\..\..\PragmaMessages.h"		// For #pragma _TODO()

#include "..\..\..\..\STLContainers.h"			// For TraceVector()

#include "MultiColumnSortListCtrl.h"
#include "MultiColumnSortClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-------------------------------------------------------------------//
// Globals Statics Constants														//
//-------------------------------------------------------------------//

#define WM_USER_SAVE_COLUMN_ORDER		WM_USER + 1
// #define WM_USER_SET_ZERO_COL_WIDTH		WM_USER + 2

// These define the ending half of the registry keys.
// They are combined with m_strUniqueName.
const TCHAR szTotalColumnCountKey	[]	= _T("TotalColumnCount"	);
const TCHAR szSortedColumnCountKey	[]	= _T("SortedColumnCount");
const TCHAR szHiddenColumnCountKey	[]	= _T("HiddenColumnCount");
const TCHAR szColumnSortKey			[]	= _T("ColumnSort"			);
const TCHAR szColumnSortStatesKey	[]	= _T("SortedStates"		);
const TCHAR szColumnOrderKey			[]	= _T("ColumnOrder"		);
const TCHAR szColumnWidthsKey			[]	= _T("ColumnWidth"		);

// This SUCKASS global is needed to pass the this pointer
// to our static column sort function.  See MCSLC_SortFunction().
MultiColumnSortListCtrl* g_pThis = 0;

//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// MultiColumnSortListCtrl()														//
//-------------------------------------------------------------------//
MultiColumnSortListCtrl::MultiColumnSortListCtrl(
	CString	strUniqueRegistryName	,
	bool		bUseExternalData			,
	int		nItemCountEstimate		,
	int		nItemAllocationIncrement,
	UINT		NoItemMenuID				,
	UINT		OneItemMenuID				,
	UINT		MultiItemMenuID			,
	bool		bAllowSorting				,
	bool		bShowHeader
) :

	// Init vars.
	m_nInsertStartCount				( 0							),
	m_bUseExternalData				( bUseExternalData		),
	m_strUniqueName					( strUniqueRegistryName	),
	m_NoItemMenuID						( NoItemMenuID				),
	m_OneItemMenuID					( OneItemMenuID			),
	m_MultiItemMenuID					( MultiItemMenuID			),
	m_bAllowSorting					( bAllowSorting			),
	m_bShowHeader						( bShowHeader				),
	m_nLastClickedColumn				( -1							),
	m_bRegistrySettingsLoaded		( false						)

{

	if ( m_bUseExternalData )
	{
		// Set up starting params.
		// The user may call this later to update these values.
		EstimateItemCount( 
			nItemCountEstimate,
			nItemAllocationIncrement
		);
	}

}


//-------------------------------------------------------------------//
// ~MultiColumnSortListCtrl()														//
//-------------------------------------------------------------------//
MultiColumnSortListCtrl::~MultiColumnSortListCtrl()
{
	// Make sure you matched up your InsertStart/End() calls!  :>
	ASSERT( m_nInsertStartCount == 0 );

	// NOTE: we can't do this here, because the base class destructor 
	// will not be able to walk back down the derived classes.
	// So this was moved to DestroyWindow().
	//
	// Clean up item data allocations.
	// DeleteAllItemData();
}


//-------------------------------------------------------------------//
// IMPLEMENT_MENUXP()																//
//-------------------------------------------------------------------//
// This sets us up so that menus are "CMenuXP"-style.
//-------------------------------------------------------------------//
IMPLEMENT_MENUXP( MultiColumnSortListCtrl, inherited, true);


//-------------------------------------------------------------------//
// ShowFilterBar()																	//
//-------------------------------------------------------------------//
// This toggles the filter bar display.  Note that there is a LOT
// more that you will need to do to get filtering up and running.
// Visit the sample at CodeGuru for more details.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::ShowFilterBar( bool bShow )
{
	#ifdef COMPILE_WITH_FILTER_SUPPORT

		//	If you get a compile error here, there are TWO THINGS you must
		// verify:
		// 
		//	1) You must have a version of the Platform SDK installed
		//		from mid-2000 or so (or more recent).
		//
		// 2) You must have your common controls updated to AT LEAST
		//		IE 5.0 or greater, or the Platform SDK includes
		//		WILL BYPASS THE DEFINITIONS!
		//
		// If you don't want to have to comply with this requirement
		// in your compile environment, don't define 
		// COMPILE_WITH_FILTER_SUPPORT.
		//
		// You will need to be at this level to compile this code.
		#if (_WIN32_IE < 0x0500)
			Your build environment does not support this functionality!  See note above...
		#endif

		// Add/remove filter bar, as requested.
		m_ctlHeaderCtrl.ModifyStyle( 
			bShow? 0 : HDS_FILTERBAR,
			bShow? HDS_FILTERBAR : 0
		);

		// TO DO
		// GET THE FREAKIN THING TO REDRAW CORRECTLY!
		// Invalidate, so header is resized.
		// m_ctlHeaderCtrl.Invalidate();
		// m_ctlHeaderCtrl.UpdateWindow();
		// Invalidate();
		// UpdateWindow();
		// OnNcPaint();
		/*
		CRect rectRedraw;
		GetWindowRect( rectRedraw );
		MoveWindow( rectRedraw );
		m_ctlHeaderCtrl.SendMessage( WM_NCPAINT );
		m_ctlHeaderCtrl.SendMessage( WM_PAINT );
		m_ctlHeaderCtrl.GetWindowRect( rectRedraw );
		m_ctlHeaderCtrl.MoveWindow( rectRedraw );
		SendMessage( WM_NCPAINT );
		SendMessage( WM_PAINT );
		Invalidate();
		UpdateWindow();
		m_ctlHeaderCtrl.SendMessage( HDM_LAYOUT );
		CRect rectRedraw;
		GetWindowRect( rectRedraw );
		MoveWindow( rectRedraw );
		Invalidate();
		UpdateWindow();
		m_ctlHeaderCtrl.GetClientRect( rectRedraw );
		m_ctlHeaderCtrl.ClientToScreen( rectRedraw );
		m_ctlHeaderCtrl.MoveWindow( rectRedraw );
		m_ctlHeaderCtrl.Invalidate();  
		m_ctlHeaderCtrl.UpdateWindow();
		m_ctlHeaderCtrl.SendMessage( HDM_LAYOUT );
		*/

		// Set up for filtering, if requested.
		if ( bShow )
		{
			if ( !bCommonControlsUpToDate( 5 ) )
			{
				DisplayMessage( IDS_FILTERING_NOT_AVAILABLE );
			}

			// Set timeout to a quarter of a second = 250 ms.
			// This is an amount of time that allows for moderately fast typing
			// to be done without interruption, yet should be almost unnoticably
			// slower than no delay.
			// TO DO
			// Make a param, perhaps from zero to 2 seconds.
			m_ctlHeaderCtrl.SendMessage( 
				HDM_SETFILTERCHANGETIMEOUT, 
				0, 
				250
			);

		}

	#else

		// Just IGNORE the request.
		// DisplayMessage( IDS_FILTERING_NOT_INCLUDED );
		// ASSERT( false );
		MessageBeep( MB_ICONASTERISK );

	#endif

}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP(MultiColumnSortListCtrl, CListCtrl)
BEGIN_MESSAGE_MAP(MultiColumnSortListCtrl, MultiColumnSortListCtrl::inherited)
	//{{AFX_MSG_MAP(MultiColumnSortListCtrl)
	ON_WM_DESTROY()
	ON_COMMAND(IDC_RIGHT_CLICK_KEY, OnRightClickKey)
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_COMMAND(IDC_SELECT_ALL, OnSelectAll)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_COMMAND(IDC_AUTOSIZE_COLUMNS, OnAutosizeColumns)
	//}}AFX_MSG_MAP

	// Pulled this out and made it _EX, so parents can get this message too.
	ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED, OnItemchanged)

	// For filter notifications.  We had to do it ourselves, as ClassWizard 
	// seems filter-braindead as of right now (mid-2000).
	#ifdef COMPILE_WITH_FILTER_SUPPORT

		ON_NOTIFY(HDN_FILTERCHANGE, 0, OnFilterChange)
		ON_NOTIFY(HDN_FILTERBTNCLICK, 0, OnFilterButtonClicked)

	#endif

	// We post changes to column order.
	ON_MESSAGE( WM_USER_SAVE_COLUMN_ORDER, OnSaveColumnOrder )
	
	// We post changes to reset width to zero.
	// ON_MESSAGE( WM_USER_SET_ZERO_COL_WIDTH, OnSetZeroColWidth )

	// We handle requests to hide/unhide columns.  These requests come
	// from our pop-up menus.
	ON_COMMAND_RANGE( 
		IDC_TOGGLE_COLUMN_HIDDEN_STATUS_BASE,
		IDC_TOGGLE_COLUMN_HIDDEN_STATUS_LAST,
		OnToggleColumnHiddenStatus
	)

	// For CMenuXP functionality...
	ON_MENUXP_MESSAGES()

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// MultiColumnSortListCtrl message handlers


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// This function is necessary for any operations where you
// need access to the control window before it is shown, such as
// setting the extended styles.
//
// This function is also responsible for loading settings from 
// the registry.  
//
// NOTE: You must make SURE you have inserted your columns in the 
// derived class, or in the parent class, before calling this 
// base class version, for the registry functions to work.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::Initialize()
{
	// Set lists with headers to "report view".
	// ASSERT( !m_bShowHeader || ( GetStyle() & LVS_REPORT ) );
	DWORD dwStyle = GetStyle();
	if ( 
			m_bShowHeader 
		&& ( ( dwStyle & LVS_REPORT ) == 0 ) 
	
	) {
		dwStyle &= ~LVS_ICON;
		dwStyle &= ~LVS_SMALLICON;
		dwStyle &= ~LVS_LIST;
		dwStyle = ::SetWindowLong(
			GetSafeHwnd(), 
			GWL_STYLE,
			dwStyle | LVS_REPORT
		);
	}

	// Can we FORCE LVS_OWNERDATA if it is not already set properly?  
	// This MAY be dangerous...looks like it is NOT working...
	// If you hit the ASSERTs, you will have to adjust your
	// "owner data" flag settings yourself, in the resource editor.
	if ( 
			m_bUseExternalData
		&& ( ( dwStyle & LVS_OWNERDATA ) == 0 ) 
	
	) {
		dwStyle = ::SetWindowLong(
			GetSafeHwnd(), 
			GWL_STYLE,
			dwStyle | LVS_OWNERDATA
		);
	}
	ASSERT( !m_bUseExternalData || GetStyle() & LVS_OWNERDATA );
	ASSERT(  m_bUseExternalData || ( GetStyle() & LVS_OWNERDATA ) == 0 );

	// Set our extended styles.
	if ( m_bShowHeader )
		SetExtendedStyle( 
				GetExtendedStyle() 
			|	LVS_EX_FULLROWSELECT 
			|	LVS_EX_HEADERDRAGDROP 
			|	LVS_EX_LABELTIP			// Let's try this out, sounds cool!
		);

	// Hook up header ctrl.
	// m_ctlHeaderCtrl.SubclassWindow( GetDlgItem(0)->m_hWnd );		
	m_ctlHeaderCtrl.SubclassWindow( GetHeaderCtrl()->m_hWnd );		
	
	///////////////////////////////////////////////////////////////////////////////
	// 2 POSSIBILITIES
	///////////////////////////////////////////////////////////////////////////////
	//

	// 1) Hiding columns REMOVES columns:
	/*
	// The derived class is required to insert its columns before
	// calling this base class.  We will be getting the column count
	// now and assuming that this is the absolute total column count.
	// Hidden columns will be subsequently removed, so it is important
	// to have this number set correctly here and now.
	ASSERT( GetColumnCount() > 0 );
	SetAbsoluteColumnCount( GetColumnCount() );

	// Set up the bitsets to be the correct size.
	// This prepares things for the next two functions.
	m_bsColumnVisible			.SetSize( GetAbsoluteColumnCount() );	m_bsColumnVisible			.Reset();
	m_bsNumericColumns		.SetSize( GetAbsoluteColumnCount() );	m_bsNumericColumns		.Reset();
	m_bsColSortStates			.SetSize( GetAbsoluteColumnCount() );	m_bsColSortStates			.Reset();
	m_bsStartSortAscending	.SetSize( GetAbsoluteColumnCount() );	m_bsStartSortAscending	.Reset();
	*/

	///////////////////////////////////////////////////////////////////////////////

	// 2) Hiding columns makes them ZERO WIDTH:

	// The derived class is required to insert its columns before
	// calling this base class.  We will be using GetColumnCount()
	// from here on out for a reliable count.
	ASSERT( GetAbsoluteColCount() > 0 );

	// Set up the sort states bitset to be the correct size.
	// This prepares things for the next two functions.
	m_bsColSortStates.SetSize( GetAbsoluteColCount() );
	m_bsColSortStates.Reset();

	///////////////////////////////////////////////////////////////////////////////

	// Did you follow the reg-storage instructions at the top of the header file?
	ASSERT( g_pAppRegData );
	
	// Get the user settings from the registry.
	RegGetColumnSortOrderWidths();

	// We track the fact that we have attempted to restore the 
	// registry settings.  Subsequent calls to InsertColumn() 
	// will now force the registry to be updated with new
	// column data.
	m_bRegistrySettingsLoaded = true;
}


//-------------------------------------------------------------------//
// OnDestroy()																			//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::OnDestroy() 
{
	// MDM
	// No need for this here!
	// This was prolly the author's way of fixing the bug with
	// HDN_ENDDRAG.  See OnEndDrag() for details.
	// SaveColumnWidths();	

	TRACE( _T("Quitting, %s list cleanup start...\n"), m_strUniqueName );

	// Automatically clean up any data allocations.
	#pragma _TOTHINK( "Destructor blues..." )
	// Note that we need to do this HERE instead of in the 
	// destructor because the destructor cannot cascade down
	// the DeleteLPARAMData() virtual function once we are 
	// in the base class.  WTF, there must be a way around this,
	// other than having to always do cleanup in the most-derived
	// class, which might change under different circumstances.
	// Hmmmmmm...
	DeleteAllItemData();

	TRACE( _T("Quitting, %s list cleanup end...\n"), m_strUniqueName );

	inherited::OnDestroy();
}


//-------------------------------------------------------------------//
// ResortAllItems()																	//
//-------------------------------------------------------------------//
// This resorts all items.  It is typically called after restoring
// the col sorting from the registry and inserting the initial
// items, but before the list is first shown.
//
// Note that we assume here that sorting information has been 
// previously properly loaded.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::ResortAllItems()
{
	// Here, we simulate a click on the header of the last sort 
	// column.  This will redo all sorting.

	// This makes sure the sort direction is not toggled.
	m_nLastClickedColumn = -1;

	if ( m_vlSortedColumns.size() > 0 )
		SortColumn( m_vlSortedColumns.back(), m_vlSortedColumns.size() > 1 );

}


//-------------------------------------------------------------------//
// InsertColumn()																		//
//-------------------------------------------------------------------//
// Be careful, doing this dynamically using external data with a 
// lot of items already in the list will cost ya.  :P
//-------------------------------------------------------------------//
int MultiColumnSortListCtrl::InsertColumn(
	int		nColumn,
	LPCTSTR	szColumnName,
	int		nFormat, 
	int		nWidth, 
	int		nSubItem
) {
	// TO DO
	// NORMALIZE the requested index!
	
	// Do a "regular" column insert.
	int nRegColumn = inherited::InsertColumn( nColumn, szColumnName, nFormat, nWidth, nSubItem );
	
	SetColumnWidth( nRegColumn, nWidth );

	AddDefaultColCachedData( nColumn );

	if ( m_bRegistrySettingsLoaded )
	{
		RefreshAbsVisColLookup();
		ForceRegistrySave();
	}

	if ( m_bUseExternalData )
	{
		// We need to go through EVERY ITEM and reallocate its internal string buffer.
		// PAIN!
		int nNewColCount = GetAbsoluteColCount();
		for ( ListItemDataVector::iterator itData = m_Data.begin(); itData < m_Data.end(); itData++ )
		{
			// We MUST always have allocated data!
			ASSERT( (*itData) != 0 );

			// Hold on to the old strings so we can copy 'em...
			ustring* pOldStrings = (*itData)->m_pstrColumnStrings;			

			// Now allocate space for new strings.
			(*itData)->m_pstrColumnStrings = new ustring[ nNewColCount ];

			// Now copy all the old strings.
			for ( int nColLoop = 0; nColLoop < nNewColCount; nColLoop++ )
			{
				if ( nColLoop != nColumn )
				{
					int nPrevCol = ( nColLoop < nColumn ) ? nColLoop : nColLoop - 1;
					(*itData)->m_pstrColumnStrings[ nColLoop ] = pOldStrings[ nPrevCol ];
				}
			}

			// Clean up.
			delete[] pOldStrings;
		}
	}
	
	return nColumn;

}


//-------------------------------------------------------------------//
// DeleteColumn()																		//
//-------------------------------------------------------------------//
// This will delete the column and make sure the registry settings
// are updated to match.  Only use this to dynamically and
// permanently remove columns.
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::DeleteColumn(
	int nAbsCol
) {
	// This should only be done "dynamically".
	// See SetDefaultColumnSortingAndHiding() for an explanation
	// of how to initialize the list.
	ASSERT( m_bRegistrySettingsLoaded );
	if ( m_bRegistrySettingsLoaded )
	{
		// First, hide the column.
		if ( GetVisibleCol( nAbsCol ) != -1 )
			HideColumn( nAbsCol );

		// Now, we need to remove the column from our internal hidden array.
		// MDM Updated to satisfy VS 2005.
		// m_vHiddenColData.erase( &m_vHiddenColData.at( GetHiddenIndex( nAbsCol ) ) );
		m_vHiddenColData.erase( m_vHiddenColData.begin() + GetHiddenIndex( nAbsCol ) );

		// Now recalc everything.
		RefreshAbsVisColLookup();

		// Now clean up the registry.
		ForceRegistrySave();

		if ( m_bUseExternalData )
		{
			// We need to go through EVERY ITEM and reallocate its internal string buffer.
			// PAIN!
			int nNewColCount = GetAbsoluteColCount();
			for ( ListItemDataVector::iterator itData = m_Data.begin(); itData < m_Data.end(); itData++ )
			{
				// We MUST always have allocated data!
				ASSERT( (*itData) != 0 );

				// Hold on to the old strings so we can copy 'em...
				ustring* pOldStrings = (*itData)->m_pstrColumnStrings;			

				// Now allocate space for new strings.
				(*itData)->m_pstrColumnStrings = new ustring[ nNewColCount ];

				// Now copy all the old strings.
				for ( int nColLoop = 0; nColLoop < nNewColCount; nColLoop++ )
				{
					int nPrevCol = ( nColLoop < nAbsCol ) ? nColLoop : nColLoop + 1;
					(*itData)->m_pstrColumnStrings[ nColLoop ] = pOldStrings[ nPrevCol ];
				}

				// Clean up.
				delete[] pOldStrings;
			}
		}
		
		return true;
	}

	return false;
}


//-------------------------------------------------------------------//
// EstimateItemCount()																//
//-------------------------------------------------------------------//
// Note: this function is optional but highly recommended.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::EstimateItemCount( 
	int nItemCountEstimate, 
	int nItemAllocationIncrement
) {
	ASSERT( m_bUseExternalData );
	m_Data.reserve( nItemCountEstimate + 1 );
	m_nItemAllocationIncrement = nItemAllocationIncrement;
}


// OLD
// This doesn't make sense, we have to allocate upon addition of each item.
/*
//-------------------------------------------------------------------//
// EndColumnModifications()														//
//-------------------------------------------------------------------//
// OK here it comes, ALLOCATE SPACE FOR THE DATA!!
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::EndColumnModifications()
{
	ASSERT( m_bUseExternalData );

	// Make enough column strings for each item.
	for ( ListItemDataVector::iterator itData = m_Data.begin(); itData < m_Data.end(); itData++ )
	{
		// TO DO
		// THIS IS NOT SET UP AT ALL to handle dynamic column insertions!
		ASSERT( (*itData) == 0 );

		(*itData) = new ListItemData;
		(*itData)->m_pstrColumnStrings = new ustring[ GetAbsoluteColCount() ];
	}

}
*/


//-------------------------------------------------------------------//
// InsertItem()																		//
//-------------------------------------------------------------------//
int MultiColumnSortListCtrl::InsertItem(
	int		nBeforeItem,
	LPCTSTR	szMainString,
	LPARAM	lParam
) {
	if ( m_bUseExternalData )
	{
		// MDM	3/13/2003 7:16:29 AM
		// Thought I needed this because of the side effect caused by
		// "???", below...
		/*
		// MDM	2/5/2003 9:47:31 AM
		// We are doing EVERYTHING ourselves at this point, except
		// tracking the selection.  We still need to call the base 
		// class to keep that synched up.
		nBeforeItem = inherited::InsertItem( 
			LVIF_PARAM					,		// UINT nMask,
			nBeforeItem					,		// int nItem,        
			_T("")						, 		// LPCTSTR lpszItem, 
			0								,		// UINT nState,      
			0								,		// UINT nStateMask,  
			0								,		// int nImage,       
			lParam								// LPARAM lParam
		);
		*/

		// Create our new item.
		ListItemData* plidNew = new ListItemData(
			new ustring[ GetAbsoluteColCount() ],
			lParam
		);
		plidNew->m_pstrColumnStrings[ 0 ].assign( szMainString );

		if ( 
				nBeforeItem >= GetItemCount() 
			||	nBeforeItem == -1
		) {
			// We need to return the actual new item number.
			nBeforeItem = GetItemCount();

			// Now shove it on the end!
			m_Data.push_back( plidNew );
		
			// DEBUG
			// WARNING: This can take up a LOT of time on large list loads...
			// TRACE( _T("Inserting item [%s] at the end as item %d...\n"), szMainString, nBeforeItem );

		} else 
		{
			// Potentially EXPENSIVE!  Avoid this!
			// Well now silly, u can't go having a cow just cause
			// somebody did an INSERT!
			// ASSERT( false );

			// DEBUG
			// TRACE( _T("Inserting item [%s] *IN THE MIDDLE* before item %d...\n"), szMainString, nBeforeItem );
			// TracePrintSelection( _T("Start inserting, sel = ") );

			// MDM Updated to satisfy Visual Studio 2005.
			// m_Data.insert( m_Data[ nBeforeItem ], plidNew );
			m_Data.insert( m_Data.begin() + nBeforeItem, plidNew );

			// MDM	12/22/2002 8:26:04 PM
			// Selected items below our newly inserted item are now off.  Adjust.
			CArray <int, int> anSelectedItems;
			anSelectedItems.SetSize( 0, 50 );
			POSITION pos = GetFirstSelectedItemPosition();
			if ( pos != NULL)
			{
				while ( pos )
				{
					int nItem = GetNextSelectedItem( pos );
					anSelectedItems.Add( nItem );
					
				}
			}
			for ( int nA = anSelectedItems.GetUpperBound(); nA >= 0 && anSelectedItems[nA] >= nBeforeItem; nA-- )
			{
				SetItemState( anSelectedItems[nA]		, 0				, LVIS_SELECTED );
				SetItemState( anSelectedItems[nA] + 1	, LVIS_SELECTED, LVIS_SELECTED );
			}

			// DEBUG
			// TracePrintSelection( _T("Done  inserting, sel = ") );
		}

		// We need to set the CListCtrl count using OUR item count override.  This
		// will also force a redraw of the affected items.
		// SKIP THIS if we are inserting a whole bunch of items.
		if ( m_nInsertStartCount == 0 )
			ResetAndRedraw();

	} else
	{
		// Call our other version which just calls the base class version.
		nBeforeItem = InsertItem( 
			LVIF_PARAM  | LVIF_TEXT	,		// UINT nMask,
			nBeforeItem					,		// int nItem,        
			szMainString				, 		// LPCTSTR lpszItem, 
			0								,		// UINT nState,      
			0								,		// UINT nStateMask,  
			0								,		// int nImage,       
			lParam								// LPARAM lParam
		);

	}

	return nBeforeItem;
	
}


//-------------------------------------------------------------------//
// InsertItem()																		//
//-------------------------------------------------------------------//
// This emulates one of the full versions of CListCtrl::InsertItem().
//-------------------------------------------------------------------//
int MultiColumnSortListCtrl::InsertItem(
	UINT		nMask,
	int		nItem,
	LPCTSTR	lpszItem,
	UINT		nState,
	UINT		nStateMask,
	int		nImage,
	LPARAM	lParam
) {
	if ( m_bUseExternalData )
	{
		// TO DO
		// FOr now this is just a quick stuff using the provided name.
		// WE ARE NOT MANAGING anything but text and params, let
		// the user know if they are expecting more than that.
		ASSERT( ( nMask & (~(LVIF_PARAM  | LVIF_TEXT) ) ) == 0 );

		nItem = InsertItem( nItem, lpszItem, lParam );

		// NOTE: This is all done by the above line:
		/*
		// We need to set the CListCtrl count using OUR item count override.  This
		// will also force a redraw of the affected items.
		// SKIP THIS if we are inserting a whole bunch of items.
		if ( m_nInsertStartCount == 0 )
			ResetAndRedraw();
		*/

		return nItem;
	
	} else
	{
		// Do a "regular" item insert.
		return inherited::InsertItem( 
			nMask							,		// UINT nMask,       
			nItem							,		// int nItem,        
			lpszItem						, 		// LPCTSTR lpszItem, 
			nState						,		// UINT nState,      
			nStateMask					,		// UINT nStateMask,  
			nImage						,		// int nImage,       
			lParam								// LPARAM lParam     
		);
	}

}


//-------------------------------------------------------------------//
// SetItemText()																		//
//-------------------------------------------------------------------//
// Set a column's text directly.
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::SetItemText( 
	int		nItem,
	int		nColumn,
	LPCTSTR	lpszString
) {
	bool bResult = true;

	if ( m_bUseExternalData )
	{
		// Normalize the fargin' numbers so we don't go outside the arrays.
		ASSERT( nItem < GetItemCount() );
		ASSERT( nColumn < GetAbsoluteColCount() );
		nItem = min( nItem, GetItemCount() - 1 );
		nColumn = min( nColumn, GetAbsoluteColCount() - 1 );

		// DEBUG
		// WARNING: This can take up a LOT of time on large list loads...
		// TRACE( _T("Setting text [%s] for item %d, column %d...\n"), lpszString, nItem, nColumn );

		m_Data[nItem]->m_pstrColumnStrings[nColumn].assign( lpszString );

		// MDM 05/15/06
		// Only redraw if we're not in the middle of a large insert.
		// We're trying to optimize the time it takes to stuff the master list.
		// It takes WAY longer than it should.  
		if ( m_nInsertStartCount == 0 )
		{
			// TO DO
			// Other optimizations described below...
			// Although I think the above one should take care of things...

			// if ( this col is not hidden && item+col is visible)
				// We want to redraw the changes.
				RedrawItems( nItem, nItem );

   			// TO DO
				// Is there a more efficient way (ie can we just draw the one column)?
				// Yes, just invalidate the rect.
				// GetSubItemRect( 
				// InvalidateRect( 
		}
	
	} else
	{
		// "Normal" call.
		bResult = ( inherited::SetItemText( nItem, nColumn, lpszString ) != FALSE );
	}
	
	return bResult;
}


//-------------------------------------------------------------------//
// GetItemText()																		//
//-------------------------------------------------------------------//
CString MultiColumnSortListCtrl::GetItemText( int nItem, int nColumn )
{
	if ( m_bUseExternalData )
	{
		// MDM
		// Unfortunately MFC pushes us out of range here on occasion - caught by VC8...
		// ASSERT( nItem < m_Data.size() );
		if ( nItem < (int)m_Data.size() )
		{
   		return CString( m_Data[nItem]->m_pstrColumnStrings[nColumn].c_str() );

      } else
      {
         return CString();
      }

	} else
	{
		return inherited::GetItemText( nItem, nColumn );
	}
}


//-------------------------------------------------------------------//
// GetItemText()																		//
//-------------------------------------------------------------------//
int MultiColumnSortListCtrl::GetItemText( int nItem, int nColumn, LPTSTR lpszText, int nLen )
{
	if ( m_bUseExternalData )
	{
		// MDM
		// Unfortunately MFC pushes us out of range here on occasion - caught by VC8...
		// ASSERT( nItem < m_Data.size() );
		if ( nItem < (int)m_Data.size() )
		{
			_tcsncpy( lpszText, m_Data[nItem]->m_pstrColumnStrings[nColumn].c_str(), nLen );
   		return _tcslen( lpszText );

      } else
      {
         return 0;
		}
		
	} else
	{
		return inherited::GetItemText( nItem, nColumn, lpszText, nLen );
	}
}


//-------------------------------------------------------------------//
// SetItemData()																		//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::SetItemData( int nItem, LPARAM lParam )
{
	// Normalize the fargin' numbers so we don't go outside the arrays.
	ASSERT( nItem < GetItemCount() );
	nItem = min( nItem, GetItemCount() - 1 );

	// DEBUG
	// WARNING: This can take up a LOT of time on large list loads...
	// TRACE( _T("Setting item %d LPARAM to %d...\n"), nItem, (uLong)lParam );
	
	if ( m_bUseExternalData )
	{
		m_Data[nItem]->m_lParam = lParam;
	} else
	{
		inherited::SetItemData( nItem, lParam );
	}
}


//-------------------------------------------------------------------//
// GetItemData()																		//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
LPARAM MultiColumnSortListCtrl::GetItemData( int nItem )
{
	// Old-school:
	// return inherited::GetItemData( nItem );

	// This might happen on occasion, I saw it once
	// when I was dragging while the song changed...
	// ASSERT( nItem < GetItemCount() );
	#pragma _TODO("Why are we getting this ASSERT!!?!?  PITA...")
	if ( nItem >= GetItemCount() )
		nItem = GetItemCount() - 1;

	// DEBUG
	// WARNING: This can take up a LOT of time on large list loads...
	// TRACE( _T("Getting item %d, item count = %d,"), nItem, GetItemCount() );
	// TRACE( _T(" value = %d...\n"), m_Data[ nItem ].m_lParam );

	return ( 
			m_bUseExternalData
		?	m_Data[ nItem ]->m_lParam
		:	inherited::GetItemData( nItem )
	);
}


//-------------------------------------------------------------------//
// DeleteItem()																		//
//-------------------------------------------------------------------//
// Here, we zap an item from the list.
// This will call DeleteLPARAMData(), which is chained through
// derived classes to properly clean up extra lParam data, etc.
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::DeleteItem( int nItem, bool bRedraw )
{
	int nItemCount = GetItemCount();
	ASSERT( nItem < nItemCount );
	if ( nItem >= nItemCount )
		return false;
	
	// Do the cleanup of the lParam data.
	// This can get overridden for cleanup all the way down through 
	// our derived classes.
	DeleteLPARAMData( GetItemData( nItem ) );
	
	if ( !m_bUseExternalData )
		return ( inherited::DeleteItem( nItem ) != FALSE );

	// Kill any strings that have been allocated.
	if ( m_Data[nItem]->m_pstrColumnStrings )
		delete[] m_Data[nItem]->m_pstrColumnStrings;

	// DEBUG
	// TracePrintSelection( _T("Start deleting, sel = ") );

	// Now kill the item.
	delete m_Data[nItem];

	// MDM UPdated to satisfy Visual Studio 2005.
	// m_Data.erase( &m_Data[nItem] );
	m_Data.erase( m_Data.begin() + nItem );

	// MDM	12/22/2002 8:26:04 PM
	// Selected items below our newly inserted item are now off.  Adjust.
	CArray <int, int> anSelectedItems;
	anSelectedItems.SetSize( 0, 50 );
	POSITION pos = GetFirstSelectedItemPosition();
	if ( pos != NULL)
	{
		while ( pos )
		{
			int nLoopItem = GetNextSelectedItem( pos );
			if ( nLoopItem >= nItem )
				anSelectedItems.Add( nLoopItem );			
		}
	}
	for ( int nA = 0; nA < anSelectedItems.GetSize(); nA++ )
	{
		SetItemState( anSelectedItems[nA]			, 0				, LVIS_SELECTED );
		
		if ( ( anSelectedItems[nA] - 1 ) > 0 )
			SetItemState( anSelectedItems[nA] - 1	, LVIS_SELECTED, LVIS_SELECTED );
	}

	// DEBUG
	// TracePrintSelection( _T("Done  deleting, sel = ") );

	// Now we need to reset the list.
	// If bRedraw is false, we assume the caller will take care 
	// of this later...
	if ( bRedraw )
		ResetAndRedraw();

	return true;
}


//-------------------------------------------------------------------//
// OnGetdispinfo()																	//
//-------------------------------------------------------------------//
// Windows is asking for the text to display, at item(iItemIndex), 
// subitem(pItem->iSubItem).
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if ( m_bUseExternalData )
	{
		LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

		LV_ITEM* pItem= &(pDispInfo)->item;

		if ( pItem->mask & LVIF_TEXT )
		{
			// We're going out of bounds here, dang it.
			// ASSERT( pItem-iItem < m_Data.size() );
			if ( pItem->iItem < (int)m_Data.size() )
			{
				lstrcpyn(
					pItem->pszText, 
					m_Data[ pItem->iItem ]->m_pstrColumnStrings[ GetAbsoluteCol( pItem->iSubItem ) ].c_str(),
					pItem->cchTextMax
				);
			}
			
			*pResult = 0;
			
		} else if ( pItem->mask & LVIF_PARAM )
		{
			*pResult = m_Data[ pItem->iItem ]->m_lParam;

		} else
		{
			ASSERT( ( pItem->mask & LVIF_STATE ) == 0 );
			*pResult = 0;
		}
	} else
	{
		*pResult = 0;
	}
}


//-------------------------------------------------------------------//
// GetVisibleColName()																//
//-------------------------------------------------------------------//
CString MultiColumnSortListCtrl::GetVisibleColName( int nVisCol )
{
	LVCOLUMN col;
	TCHAR szNameBuffer[ 701 ];
	col.pszText = szNameBuffer;
	col.cchTextMax = 700;
	col.mask = LVCF_TEXT;

	if ( GetColumn( nVisCol, &col ) )
		return CString( col.pszText );
	else 
		return CString();
}


//-------------------------------------------------------------------//
// SetColumnName()																	//
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::SetColumnName( int nAbsCol, CString& strNewColName )
{
	bool bReturn;

	int nCol = GetVisibleCol( nAbsCol );
	if ( nCol != -1 )
	{
		const int cnMaxLength = 700;
		LVCOLUMN lvCol;
		TCHAR szNameBuffer[ cnMaxLength + 1 ];
		lvCol.pszText = szNameBuffer;
		lvCol.cchTextMax = cnMaxLength;
		lvCol.mask = LVCF_TEXT;

		// Get current column structure, just to be safe.
		bReturn = ( GetColumn( nCol, &lvCol ) != FALSE );
		
		// Now set the string.
		_tcsncpy( lvCol.pszText, LPCTSTR( strNewColName ), cnMaxLength );
		bReturn &= ( SetColumn( nCol, &lvCol ) != FALSE );
	
	} else
	{
		nCol = GetHiddenIndex( nAbsCol );
		m_vHiddenColData[nCol].strColHeading = strNewColName;
		bReturn = true;
	}

	return bReturn;
}


//-------------------------------------------------------------------//
// GetHeaderFilterInfo()															//
//-------------------------------------------------------------------//
CString	MultiColumnSortListCtrl::GetHeaderFilterInfo( 
	int		nVisibleCol
) {
	#ifdef COMPILE_WITH_FILTER_SUPPORT
		
		TCHAR szBuffer[ 1024 ];
		ASSERT( m_ctlHeaderCtrl.GetSafeHwnd() );
		HDITEM			hdItem;
		HD_TEXTFILTER	hdTextFilter;
		ZeroMemory( &hdItem, sizeof( HDITEM ) );					// Init Structure
		hdItem.mask = ( HDI_FILTER );									// Set Mask
		hdItem.type = HDFT_ISSTRING;									// Set String Filter Type
		hdTextFilter.pszText = szBuffer;								// Set up a buffer for return string
		hdTextFilter.cchTextMax	= 1024;								// Buffer size
		hdItem.pvFilter = &hdTextFilter;
		m_ctlHeaderCtrl.GetItem( nVisibleCol, &hdItem );		// Get Item Information

		// Stuff the return string.
		return CString( szBuffer );

	#else

		// ASSERT( false );
		return CString();
	
	#endif

}


//-------------------------------------------------------------------//
// OnFilterChange()																	//
//-------------------------------------------------------------------//
void	MultiColumnSortListCtrl::OnFilterChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMHEADER	*pNMHdr = (NMHEADER*)pNMHDR;
	CWnd		*pParent = GetParent();

	*pResult = 0;											// Default RetCode
	if( pParent )											// If Parent Exists
	{
		pNMHdr -> hdr.idFrom = GetDlgCtrlID();				// Update Structure Data
		// *pResult = pParent -> SendNotifyMessage( WM_NOTIFY, 
		// 		GetDlgCtrlID(), (LPARAM)pNMHdr );			// Forward Message
		

		if( pNMHdr -> iItem == -1 )						// If On All Filters
		{
			TRACE( _T( "FilterChanged On ALL Columns...\n" ) );
		} else											// If On A Single Filter
		{
			// Get the new filter string.
			ustring strFilter( LPTSTR( LPCTSTR( GetHeaderFilterInfo( pNMHdr->iItem ) ) ) );
			TRACE2( _T( "FilterChanged to %s on visible column %d...\n" ), strFilter.c_str(), pNMHdr->iItem );
			
			// We ignore all filtering that occurs below the min filter length threshold.
			#pragma _TODO( "Make min filter length into a user pref" )
			// if ( strFilter.size() > 0 )
			{
				int nAbsCol = GetAbsoluteCol( pNMHdr->iItem );
				int nPrevLength = m_vColCachedData[ nAbsCol ].m_strPrevFilter.size();
				
				#pragma _TOTHINK( "Optimize refiltering..." )
				// There's only one situation, although hopefully common, where we
				// can keep the items that have been already filtered out from
				// having to be readded for refiltering: if the new filter term
				// is an extension of the previous.
				//
				// NOTE: there are some possible ways to improve this.  
				//
				//		1) if the new string is a subset of the previous string...
				//				Scan m_FilteredOutData
				//					if ( changed col filter passes )
				//						if ( other col filters pass )
				//							readd
				//
				//		2) We track which items were removed from which filters.  Then
				//			we know what we need to rescan on changes.  Gets hairy fast.
				//
				bool bResetFilter = 
				(
						(int)strFilter.size() < nPrevLength
					||	(
							// 	strFilter.Left( nPrevLength ).CompareNoCase( m_vColCachedData[ nAbsCol ].m_strPrevFilter ) != 0
							// && strFilter.Right( nPrevLength ).CompareNoCase( m_vColCachedData[ nAbsCol ].m_strPrevFilter ) != 0
								_tcsicmp(
									strFilter.substr( 0, nPrevLength ).c_str(),
									m_vColCachedData[ nAbsCol ].m_strPrevFilter.c_str()
								) != 0
							&& _tcsicmp(
									strFilter.substr( strFilter.size() - nPrevLength, nPrevLength ).c_str(),
									m_vColCachedData[ nAbsCol ].m_strPrevFilter.c_str()
								) != 0
						)
				);
				
				// Now that we have determined if we need to refilter, we can back up the NEW prev filter.
				// We will subsequently use this as the "current" filter, so do it now!
				m_vColCachedData[ nAbsCol ].m_strPrevFilter = strFilter;

				// Refilter the whole kit and caboodle as needed.
				if ( bResetFilter )
				{
					// First, we dump all previously filtered strings back into the search pool.
					ResetFilter();
					
					// Now we re-filter em, based on AND logic across **ALL** filters, for all cols.
					for ( int nFilterCol = 0; nFilterCol < GetAbsoluteColCount(); nFilterCol++ )
					{
						// Provides nicer looking code.  Shuddup about the 10 microseconds, ya big C hack!
						ustring& strColFilter = m_vColCachedData[ nFilterCol ].m_strPrevFilter;

						// Is there a col filter?
						if ( !strColFilter.empty() )
						{
							// Set up for scanning.
							int nItemCount = GetItemCount();
							SubstringSearcher sss(
								strColFilter,
								_MAX_PATH
							);

							for ( int nA = nItemCount - 1; nA >= 0; nA-- )
							{
								// Does this col's filter eliminate this item?
								if ( 
									!sss.bContainsSubstring(
										m_Data[ nA ]->m_pstrColumnStrings[ nFilterCol ]
									)
								) {
									// Yes, remove the item and proceed with the next item.
									m_FilteredOutData.push_back( m_Data[ nA ] );
									m_Data.erase( m_Data.begin() + nA );
								}
							}
						}

					}
					
				} else
				{
					if ( strFilter.size() > 0 )
					{
						// Move all currently unfiltered strings that get filtered by the newly changed filter 
						// into the backup m_FilteredOutData array.
						int nItemCount = GetItemCount();
						SubstringSearcher sss(
							strFilter,
							_MAX_PATH
						);

						for ( int nA = nItemCount - 1; nA >= 0; nA-- )
						{
							if ( 
								!sss.bContainsSubstring(
									m_Data[ nA ]->m_pstrColumnStrings[ nAbsCol ]
								)
							) {
								m_FilteredOutData.push_back( m_Data[ nA ] );
								m_Data.erase( m_Data.begin() + nA );
							}
						}
					}
				}

				// Refresh.
				ResetAndRedraw();
			}

		}

	}
	return;													// Done!
}


//-------------------------------------------------------------------//
// ResetFilter()																		//
//-------------------------------------------------------------------//
// Re-add all previously filtered items.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::ResetFilter()
{
	// Dump all strings back into the search pool.
	m_Data.reserve( m_Data.size() + m_FilteredOutData.size() );
	for ( int nA = 0; nA < (int)m_FilteredOutData.size(); nA++ )
	{
		m_Data.push_back( m_FilteredOutData[ nA ] );
	}
	m_FilteredOutData.clear();

	// Ouch.
	#pragma _TOTHINK( "CHECK ON THIS: This is done by ResortAllItems(), I am pretty sure..." )
	// SetItemCountEx( GetItemCount() );
	ResortAllItems();
}


//-------------------------------------------------------------------//
// OnFilterButtonClicked()															//
//-------------------------------------------------------------------//
void	MultiColumnSortListCtrl::OnFilterButtonClicked(NMHDR* pNMHDR, LRESULT* pResult) 
{
	#ifdef COMPILE_WITH_FILTER_SUPPORT

		NMHDFILTERBTNCLICK	*pNMFButtonClick = (NMHDFILTERBTNCLICK*)pNMHDR;
		NMHEADER			*pNMHdr = (NMHEADER*)pNMHDR;
		CWnd				*pParent = GetParent();

		*pResult = 0;											// Default RetCode
		if( pParent )											// If Parent Exists
		{
			pNMHdr -> hdr.idFrom = GetDlgCtrlID();				// Update Structure Data
			
			// *pResult = pParent -> SendNotifyMessage( WM_NOTIFY, 
			// 		GetDlgCtrlID(), (LPARAM)pNMHdr );			// Forward Message

			// TO DO
			// Totally clear out any previous filter.
		}

	#else

		// Do nothing.
		// ASSERT( false );

	#endif
}


//-------------------------------------------------------------------//
// [Un]SelectAll()																	//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::SelectAll()
{
	int nItem = -1;
	while ( ( nItem = GetNextItem( nItem, LVNI_ALL ) ) != -1 ) 
		SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
}
void MultiColumnSortListCtrl::UnselectAll()
{
	int nItem = -1;
	while ( ( nItem = GetNextItem( nItem, LVNI_SELECTED ) ) != -1 ) 
		SetItemState( nItem, 0, LVIS_SELECTED );
}


//-------------------------------------------------------------------//
// DeleteSelection()																	//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::DeleteSelection( bool bRehighlight )
{
	// DEBUG
	// TracePrintSelection( _T("Start deleting, sel = ") );

	// Loop through selection and store the indexes.
	// Allocate a big enough grow size to minimize allocations.
	CArray <int, int> anSelectedItems;
	anSelectedItems.SetSize( 0, 50 );

	int nA = -1, nB = -1;
	while ( ( nA = GetNextItem( nA, LVNI_SELECTED ) ) != -1 ) 
	{
		// Save the first selected item for the del-del-del
		// convenience functionality, below.
		if ( nB == -1 )
			nB = nA;

		anSelectedItems.Add( nA );
	}

	// We need to go backwards when deleting so we don't
	// screw up the indexing.
	for ( nA = anSelectedItems.GetUpperBound(); nA >= 0; nA-- )
		DeleteItem( anSelectedItems[nA], false );

	// Kill selection.
	UnselectAll();

	// This may have come from a drag/drop - don't rehighlight
	// in that more-complicated situation...
	if ( bRehighlight )
	{
		// Reselect the item underneath the previous first selection.  
		// This makes del-del-del convenient.
		if ( nB < 0 )
		{
			nB = GetItemCount() - 1;
		}
		if ( nB >= 0 )
		{
			SetItemState( nB, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		}
	}

	if ( m_bUseExternalData )
		ResetAndRedraw();

	// DEBUG
	// TracePrintSelection( _T("Done  deleting, sel = ") );
}


//-------------------------------------------------------------------//
// bSingleItemSelected()															//
//-------------------------------------------------------------------//
// This tells us if there is only one item selected.
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::bSingleItemSelected()
{
	int nLoop = GetNextItem( -1, LVNI_SELECTED );
	if ( nLoop != -1 ) 
	{
		nLoop = GetNextItem( nLoop, LVNI_SELECTED );
		if ( nLoop == -1 ) 
			return true;
	}

	return false;
}


// For debugging...
#ifdef _DEBUG
	void MultiColumnSortListCtrl::TracePrintSelection( LPCTSTR szComment )
	{
		ustring strTemp, strMsg( szComment );
		int nA = -1;
		while ( ( nA = GetNextItem( nA, LVNI_SELECTED ) ) != -1 ) 
		{
			strTemp += _T(", ");
			strTemp.AppendNumber( nA );
		}

		if ( strTemp.size() )
		{
			strMsg += strTemp.substr( 2, strTemp.size() - 2 );
		}
		strMsg += _T("\n");

		TRACE( strMsg.c_str() );
	}

	void MultiColumnSortListCtrl::TraceState()
	{
		if ( m_bRegistrySettingsLoaded )
		{
			Long* plOrder = new Long[ GetVisibleColCount() ];
			ASSERT( sizeof Long == sizeof INT );
			GetColumnOrderArray(
				(LPINT) plOrder,
				GetVisibleColCount()
			);

			TRACE( _T("Visible:") );
			int n;
			for ( n = 0; n < GetVisibleColCount(); n++ )
				TRACE( _T(" %d %s;"), *( plOrder + n ), GetVisibleColName( GetVisibleCol( *( plOrder + n ) ) ) );
			TRACE( _T("\n") );
			TRACE( _T("Hidden:") );
			for ( n = 0; n < GetHiddenColCount(); n++ )
				TRACE( _T(" %d %s;"), m_vHiddenColData[ n ].nAbsColNumber, m_vHiddenColData[ n ].strColHeading );
			TRACE( _T("\n") );

			// Clean up.
			delete[] plOrder;

			// Now we want to display the number of columns in the internally managed data arrays...
			// Let's try to grab the first item, then walk its column strings...
			/*
			int nItemCount = m_Data.size();
			TRACE( _T("%d items in list... "), nItemCount );
			if ( nItemCount > 0 )
			{
				int nItem = 0;
				ListItemData* pData = m_Data[nItem];
				pData->m_pstrColumnStrings[nColumn].c_str()
				TRACE( _T("each row contains %d columns... "), nItemCount );
			}
			TRACE( _T("\n") );
			*/

		}
	}
#endif


//-------------------------------------------------------------------//
// DeleteAllItems()																	//
//-------------------------------------------------------------------//
// Deletes all items contained in the list.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::DeleteAllItems()
{
	if ( m_bUseExternalData )
	{
		// Clean up all allocations.
		DeleteAllItemData();
		
		// Now wipe out the list.
		m_Data.clear();
		m_FilteredOutData.clear();

		// Now we need to reset the list count.
		#pragma _TODO( "Optimize to avoid FULL REDRAW." )
		ResetAndRedraw();

	} else
	{
		inherited::DeleteAllItems();
	}

}


//-------------------------------------------------------------------//
// DeleteAllItemData()																//
//-------------------------------------------------------------------//
// Helper function called by DeleteAllItems(), and in destructor.
// You should never call this directly, or you'll end up
// whacking your items' data but not the items.  Bad.
//
// We want a function to do this, separate from iterating through
// DeleteItem(), because this is a BIT more optimized.  We never 
// bother to do one erase() after another with this method.  Instead, 
// DeleteAllItems() just clears the whole thing in one whack after 
// all item data is cleaned up.
//-------------------------------------------------------------------//-
void MultiColumnSortListCtrl::DeleteAllItemData()
{
	// Clean up our allocations for storing item owner data.
	// Also hit DeleteLPARAMData() so the derived class 
	// can clean up lParam data as needed.
	ListItemDataVector::iterator itData;
	for ( itData = m_Data.begin(); itData < m_Data.end(); itData++ )
	{
		DeleteLPARAMData( (*itData)->m_lParam );
		delete[] (*itData)->m_pstrColumnStrings;
		delete (*itData);
	}
	for ( itData = m_FilteredOutData.begin(); itData < m_FilteredOutData.end(); itData++ )
	{
		DeleteLPARAMData( (*itData)->m_lParam );
		delete[] (*itData)->m_pstrColumnStrings;
		delete (*itData);
	}
}


//-------------------------------------------------------------------//
// OnRightClickKey()																	//
//-------------------------------------------------------------------//
// Fire off a right click menu request on use of the VK_APPS key.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::OnRightClickKey() 
{

	// Try to get a reasonable item over which to
	// place the right-click menu.
	int nCurrentItem = GetNextItem( -1, LVNI_FOCUSED );
	if ( nCurrentItem == -1 )
		nCurrentItem = GetNextItem( -1, LVNI_SELECTED );
	
	CRect ItemRect;
	if ( !GetItemRect( nCurrentItem, &ItemRect, LVIR_LABEL ) )
		ItemRect = CRect( 0, 0, 0, 0 );

	DisplayRightClickMenu( 
		CPoint( ItemRect.left + 1, ItemRect.bottom - 1 ) 
	);
	
}


//-------------------------------------------------------------------//
// OnRButtonDown()																	//
//-------------------------------------------------------------------//
// Fire up the right click menu.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{

	// Don't call the base class.
	// inherited::OnRButtonDown(nFlags, point);

	// Try to display the right click menu.
	DisplayRightClickMenu( point );

}


//-------------------------------------------------------------------//
// DisplayRightClickMenu()															//
//-------------------------------------------------------------------//
// This function does the work in popping up the right-click menu.
// It is called whenever the mouse is used, or the VK_APPS right-
// click-key (on MS natural keyboard, etc.) is pressed.
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::DisplayRightClickMenu(
	CPoint&		point
) {

	bool bReturn = false;
	
	// Force this window to have the focus before putting up the
	// menu.  This way the OnCmdMsg routing will make its way
	// back to us when it checks the popup menu commands.
	SetFocus();

	// Get the appropriate menu ID.
	UINT MenuID;

	// Get the index of the item under the cursor.
	UINT uiHitFlags = ( LVHT_ONITEMICON | LVHT_ONITEMLABEL );
	int nUnderCursor = HitTest( point, &uiHitFlags );

	// Deselect all but the one under the cursor,
	// if neither the shift nor control keys are
	// pressed and there is an item directly under
	// the cursor that is not already highlighted.
	if (
		( nUnderCursor != -1 ) &&
		( ::GetKeyState( VK_SHIFT   ) >= 0		) &&
		( ::GetKeyState( VK_CONTROL ) >= 0		) &&
		( GetItemState( nUnderCursor, LVIS_SELECTED ) != LVIS_SELECTED )
	) {

		// Deselect all selected items.
		int nItem = GetNextItem( -1, LVNI_SELECTED );
		while ( nItem != -1 ) 
		{
			SetItemState( nItem, 0, LVIS_SELECTED );
			nItem = GetNextItem( nItem, LVNI_SELECTED );
		}

		// Set the highlight and focus to the new item.
		SetItemState( nUnderCursor, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}

	int nSelected = GetNextItem( -1, LVNI_SELECTED );
	if ( nSelected == -1 && nUnderCursor == -1 )
		MenuID = m_NoItemMenuID;
	else 
	{
		nSelected = GetNextItem( nSelected, LVNI_SELECTED );
		if ( nSelected == -1 )
			MenuID = m_OneItemMenuID;
		else
			MenuID = m_MultiItemMenuID;
	}

	// If we ended up with a valid menu ID...
	if ( MenuID ) 
	{
		// Convert coordinates for TPM.
		ClientToScreen( &point );
			
		// Get popup menu.
		// MDM Do some checks, for when we're low on GDI resources, sigh...
		CMenu Menu;
		if ( Menu.LoadMenu( MenuID ) == FALSE )
			return false;
		CMenu* pPopupMenu = Menu.GetSubMenu(0);
		if ( pPopupMenu == 0 )
			return false;
		
		////////////////////////////////////////////////////////////////////////////////
		// Allow derived classes to provide ON_COMMAND_UPDATE()
		// handlers for messages on the popup menu.
		//
		// This only works in frames...
		// SendMessage( WM_INITMENUPOPUP, (WPARAM) pPopupMenu->GetSafeHmenu(), 0L );
		//
		// The following was ripped from CFrameWnd::OnInitMenuPopup() in 
		// MFC\SRC\WINFRM.CPP:
		////////////////////////////////////////////////////////////////////////////////
		//

		CCmdUI state;
		state.m_pMenu = pPopupMenu;
		state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
		for ( 
			state.m_nIndex = 0; 
			state.m_nIndex < state.m_nIndexMax;
			state.m_nIndex++
		) {
			state.m_nID = pPopupMenu->GetMenuItemID( state.m_nIndex );
			if ( state.m_nID == 0 )
				continue; // menu separator or invalid cmd - ignore it

			ASSERT(state.m_pOther == NULL);
			ASSERT(state.m_pMenu != NULL);
			if (state.m_nID != (UINT)-1)
			{
				// normal menu item
				// Auto enable/disable if frame window has 'm_bAutoMenuEnable'
				//    set and command is _not_ a system command.
				state.m_pSubMenu = NULL;
				state.DoUpdate(this, state.m_nID < 0xF000);
			}

		}
		
		//
		////////////////////////////////////////////////////////////////////////////////


		// Allow derived classes to prepare popup menu,
		// e.g., by adding checks where needed.
		PrepareMenu( pPopupMenu, MenuID, nUnderCursor );

		// Always add the "Show columns" popup menu to the end.
		CMenu ColMenu;
		ColMenu.CreatePopupMenu();
		AddColumnHidingMenu( &ColMenu );
		pPopupMenu->InsertMenu(
			-1,									// Append
			MF_POPUP | MF_BYPOSITION,
			(UINT)ColMenu.Detach(), 
			CString( _T("Show column") )
		);

		// Show popup menu.
		pPopupMenu->TrackPopupMenu(
			TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
			point.x, 
			point.y, 
			this
		);	

		// We did the popup, return true.
		bReturn = true;

	}

	return bReturn;

}


//-------------------------------------------------------------------//
// OnColumnclick()																	//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Skip clicks if sorting is off.
	if ( m_bAllowSorting )
	{
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

		// Let's multi-column sort if ctrl OR shift are pressed...
		bool bSortingChanged;
		if (
				( ::GetKeyState( VK_CONTROL	) < 0 )
			||	( ::GetKeyState( VK_SHIFT		) < 0 )
		)
			bSortingChanged = SortColumn( GetAbsoluteCol( pNMListView->iSubItem ), MULTI_COLUMN_SORT );
		else
			bSortingChanged = SortColumn( GetAbsoluteCol( pNMListView->iSubItem ), SINGLE_COLUMN_SORT );
		
		// Sorting changed, save new sort order.
		SaveColumnSorting();

		// Refresh as needed.
		if ( bSortingChanged )
			Invalidate();
	}

	*pResult = 0;
}


//-------------------------------------------------------------------//
// MCSLC_SortFunction()																//
//-------------------------------------------------------------------//
// This helper for SortColumn() is used to multi-column sort the 
// entire list in one sort pass.
//
// MDM NOTE: We changed qsort calls to std::sort calls for VS2005
// compatibility.  However, VC6 does a crappy job with std::sort.
// Therefore, we use this in VS2005, and qsort in VC6.
//-------------------------------------------------------------------//
struct MCSLC_SortStruct
{
	bool operator()( ListItemData*& plid1, ListItemData*& plid2 )
	{
		int equal = 0;

		// We want a pointer to our list control, so we can
		// get to the column sort order data.
		// I HATE STATIC FUNCTIONS!  :>
		MultiColumnSortListCtrl* pThis = g_pThis;

		// Now sort based on plid1->m_pstrColumnStrings
		// and m_vlSortedColumns.  Note that we start at the
		// FIRST sorted column, and only walk down the
		// columns as needed.
		for( int nA = 0; nA < pThis->GetSortedColumnCount(); nA++ )
		{
			// Get the current sort state (ASCENDING/DESCENDING) for this column.
			SORT_STATE ssCol = pThis->GetColumnSortState( pThis->GetSortedColumn( nA ) );
			
		   // Is this an alpha or numeric sort? 
		   if ( pThis->IsColumnNumeric( pThis->GetSortedColumn( nA ) ) )
		   {
			   int nFirst = _ttoi( plid1->m_pstrColumnStrings[ pThis->GetSortedColumn( nA ) ].c_str() );
			   int nSecond = _ttoi( plid2->m_pstrColumnStrings[ pThis->GetSortedColumn( nA ) ].c_str() );
			   if ( nFirst == nSecond )
				   equal = 0;
			   else if ( nFirst < nSecond )	
				   equal = -1;
			   else
				   equal = 1;

		   } else
		   {
			   equal = _tcsicmp( 
				   plid1->m_pstrColumnStrings[ pThis->GetSortedColumn( nA ) ].c_str(), 
				   plid2->m_pstrColumnStrings[ pThis->GetSortedColumn( nA ) ].c_str()
			   );
			}
			
			// Switch if descending.
			if ( ssCol == DESCENDING )
				equal *= -1;

			// As long as we found a difference there is no need to continue.
			if ( equal != 0 )
				break;

		}
			
		return ( equal != 0 );
	}

};


//-------------------------------------------------------------------//
// MCSLC_SortFunction()																//
//-------------------------------------------------------------------//
// For VC6, we'll use qsort, since std::sort seems to barf...
//-------------------------------------------------------------------//
int MCSLC_SortFunction(const void *v1, const void *v2)
{
	int equal = 0;

	// We want a pointer to our list control, so we can
	// get to the column sort order data.
	// I HATE STATIC FUNCTIONS!  :>
	MultiColumnSortListCtrl* pThis = g_pThis;

	// Get back our data types.
	ListItemData* plid1 = *( (ListItemData**) v1 );
	ListItemData* plid2 = *( (ListItemData**) v2 );

	// Now sort based on plid1->m_pstrColumnStrings
	// and m_vlSortedColumns.  Note that we start at the
	// FIRST sorted column, and only walk down the
	// columns as needed.
	for( int nA = 0; nA < pThis->GetSortedColumnCount(); nA++ )
	{
		// Get the current sort state (ASCENDING/DESCENDING) for this column.
		SORT_STATE ssCol = pThis->GetColumnSortState( pThis->GetSortedColumn( nA ) );
		
		// Is this an alpha or numeric sort? 
		if ( pThis->IsColumnNumeric( pThis->GetSortedColumn( nA ) ) )
		{
			int nFirst = _ttoi( plid1->m_pstrColumnStrings[ pThis->GetSortedColumn( nA ) ].c_str() );
			int nSecond = _ttoi( plid2->m_pstrColumnStrings[ pThis->GetSortedColumn( nA ) ].c_str() );
			if ( nFirst == nSecond )
				equal = 0;
			else if ( nFirst < nSecond )	
				equal = -1;
			else
				equal = 1;

		} else
		{
			equal = _tcsicmp( 
				plid1->m_pstrColumnStrings[ pThis->GetSortedColumn( nA ) ].c_str(), 
				plid2->m_pstrColumnStrings[ pThis->GetSortedColumn( nA ) ].c_str()
			);
		}

		// Switch if descending.
		if ( ssCol == DESCENDING )
			equal *= -1;

		// As long as we found a difference there is no need to continue.
		if ( equal != 0 )
			break;

	}
		
	return equal;
}


//-------------------------------------------------------------------//
// SortColumn()																		//
//-------------------------------------------------------------------//
// This function is the heart of the class. This will get called 
// automatically when you click a header, and if you press control 
// while clicking the header, a multi column sort will take place 
// (ie: sorting the current columns within all the previously 
// control+clicked columns). The multi column sort saves all the 
// previously control+clicked columns in an array and sorts them 
// in reverse order. So if you click column 0, 2, 3, 5 while 
// holding control, it will sort 5, 3, 2, 0. ( this acheives a
// muli-column sort).
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::SortColumn( int nAbsCol, bool bSortingMultipleColumns )
{	
	// Time the load, if requested.
	// #define SHOW_LIST_CTRL_SORT_TIME
	#ifdef SHOW_LIST_CTRL_SORT_TIME
		struct _timeb tbStart;
		_ftime( &tbStart );
	#endif
	
	m_bSorting = true;

	// If we just handled a click on this column...
	if ( nAbsCol == m_nLastClickedColumn )
	{
		// Toggle the state.
		// We allow three states, ascending, descending and none,
		// and we cycle through on repeated clicks.
		// Cycle: none, ascending, none, descending, none, etc.
		// The "none" state was added after sorting persistence
		// was added, so that the sorting could be turned off,
		// rather than perpetually persist.

		// If the column is not currently sorted,
		// toggle the sorting from what it LAST was, and save it.
		if ( !bIsColumnSorted( nAbsCol ) )
		{
			SetColumnSortState( nAbsCol , (SORT_STATE)!GetColumnSortState( nAbsCol ) );
		 
		// Otherwise, we were previously sorted.  
		// Toggle sorting OFF and return.
		} else 
		{
			// Remove this column from the sorted col array.
			UnsetColumnSorted( nAbsCol );

			// Remove the header sort image.
			m_ctlHeaderCtrl.RemoveSortImage( GetVisibleCol( nAbsCol ) );

			// No need to sort, just return.
			m_bSorting = false;
			return false;
		}


	// If we are newly clicking this item, set the starting direction.
	} else
	{
		SetColumnSortState( nAbsCol, bStartSortAscending( nAbsCol ) ? ASCENDING : DESCENDING );
	}
	
	// Keep the last clicked column around for next time.
	m_nLastClickedColumn = nAbsCol;
		
	if( bSortingMultipleColumns )
	{
		// We just clicked on the last column, add it to the sorted list if
		// it was not already sorted, or move it to the end if it was.
		if ( !bIsColumnSorted( nAbsCol ) )
			m_vlSortedColumns.push_back( nAbsCol );
		else
			MoveSortedColumnToEnd( nAbsCol );

		// NO NEED FOR THIS WITH THE NEW ListView_SortItemsEx() OF COMMCTRL.H version 5.80!!
		// Inject the item index into the data stored in the lParam, while
		// preserving the existing data.
		// InjectItemIntoLParam();
		
		for( int i = m_vlSortedColumns.size() - 1; i >= 0 ; i-- )
		{

			// Get the current sort state (ASCENDING/DESCENDING) for this column.
			SORT_STATE ssEachItem = GetColumnSortState( m_vlSortedColumns[i] );

			// Reset the sort image for every column.
			// If we clicked on a column that was previously higher in
			// the sort rank, it has now dropped in rank, and the other
			// columns need to "slide up".  The images all need to be 
			// updated to reflect this.
			m_ctlHeaderCtrl.SetSortImage( 
				GetVisibleCol( m_vlSortedColumns[i] ), 
				ssEachItem,
				i
			);

			if ( !m_bUseExternalData )
			{
				// Sort.
				CSortClass csc( this, m_vlSortedColumns[i], IsColumnNumeric( m_vlSortedColumns[i] ) );	
				csc.Sort( ssEachItem == ASCENDING );
			}

		}

		// NO NEED FOR THIS WITH THE NEW ListView_SortItemsEx() OF COMMCTRL.H version 5.80!!
		// Restore the original lParam data.
		// RemoveItemFromLParam();
	
	} else
	{
		SORT_STATE ssEachItem = GetColumnSortState( nAbsCol );

		// Clear the current sorting.
		ClearSorting();

		// Add the new sort column and set up the image in the header.
		m_vlSortedColumns.push_back( nAbsCol );
		m_ctlHeaderCtrl.SetSortImage( GetVisibleCol( nAbsCol ), ssEachItem );

		if ( !m_bUseExternalData )
		{
			// NO NEED FOR THIS WITH THE NEW ListView_SortItemsEx() OF COMMCTRL.H version 5.80!!
			// Inject the item index into the data stored in the lParam, while
			// preserving the existing data.
			// InjectItemIntoLParam();
			
			// Sort.
			CSortClass csc( this, nAbsCol, IsColumnNumeric( nAbsCol ) );	
			csc.Sort( ssEachItem == ASCENDING );

			// NO NEED FOR THIS WITH THE NEW ListView_SortItemsEx() OF COMMCTRL.H version 5.80!!
			// Restore the original lParam data.
			// RemoveItemFromLParam();

		}
	}

	if ( m_bUseExternalData )
	{
		// Set up the stooopid global this pointer hack.
		// TO DO
		// This is RIPE for a collision!!!  LOCK HERE!!!
		// Or figure out a better way.  Like killing the
		// sort on a new sort request???
		ASSERT( g_pThis == 0 );
		g_pThis = this;

		// DEBUG
		// Trying to determine why std::sort() is TOTALLY BROKEN IN VC6!!
		// Faggetit, just use qsort...
		// TraceVector( m_Data );
		// std::sort( m_Data.begin(), m_Data.end() - 10, MCSLC_SortStruct() );
		// TraceVector( m_Data );

		// Sort our multi-column-sorted data in ONE PASS wahoo!
		// #if _MFC_VER < 0x0700
      // MDM The std::sort isn't sorting correctly, revert for now...
      #if 1

			// For VC6, we'll use qsort, since std::sort seems to barf...
			void* pVoid = &( *( m_Data.begin() ) );
         qsort( pVoid, GetItemCount(), sizeof( ListItemData* ), MCSLC_SortFunction );

		#else

			// MDM Updated for Visual Studio 2005.
			std::sort( m_Data.begin(), m_Data.end(), MCSLC_SortStruct() );

		#endif
		
		g_pThis = 0;

		// Force a redraw.
		#pragma _TOTHINK( "We might not always want this redraw!" )
		ResetAndRedraw();

	}

	m_bSorting = false;

	#ifdef SHOW_LIST_CTRL_SORT_TIME
		struct _timeb tbStop;
		_ftime( &tbStop );
		CString strMsg;
		strMsg.Format( 
			_T("List sorted in %.6f seconds..."), 
			CalcTimeSpan( 
				tbStart,
				tbStop
			)
		);
		DisplayMessage( strMsg );
	#endif

	return true;

}


//-------------------------------------------------------------------//
// ClearSorting()																		//
//-------------------------------------------------------------------//
// Clear the current sorting.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::ClearSorting()
{
	// Reset the header images.
	m_ctlHeaderCtrl.RemoveAllSortImages();

	// Reset the sorted columns array.
	m_vlSortedColumns.clear();
}


//-------------------------------------------------------------------//
// AutoSizeColumn()																	//
//-------------------------------------------------------------------//
// Utility function to size columns based on its data.
// written by Roger Onslow
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::AutoSizeColumn( int iColumn )
{
	SetRedraw(false);
	
	SetColumnWidth(iColumn,LVSCW_AUTOSIZE);
	int wc1 = GetColumnWidth( iColumn );
	SetColumnWidth(iColumn,LVSCW_AUTOSIZE_USEHEADER);
	int wc2 = GetColumnWidth( iColumn );

	// int wc = _MAX( MINCOLWIDTH, _MAX( wc1, wc2 ) );
	int wc = max( wc1, wc2 );
	
	// if( wc > MAXCOLWIDTH )
	// 	wc = MAXCOLWIDTH;
	
	SetColumnWidth( iColumn,wc );  
	SetRedraw(true);

}


//-------------------------------------------------------------------//
// bIsColumnSorted()																	//
//-------------------------------------------------------------------//
// Searches the sorted column array to determine if a column is 
// sorted.
//-------------------------------------------------------------------//
bool	MultiColumnSortListCtrl::bIsColumnSorted( int iItem )
{
	// Search for the item.
	LongVector::iterator it;
	for ( 
		it = m_vlSortedColumns.begin();
		it < m_vlSortedColumns.end();
		it++
	) {
		if ( *it == iItem )
			return true;
	}

	return false;
}


//-------------------------------------------------------------------//
// GetColumnSortState()																//
//-------------------------------------------------------------------//
// This gets the sort state of a column.
//-------------------------------------------------------------------//
SORT_STATE MultiColumnSortListCtrl::GetColumnSortState( int iItem )
{
    ASSERT( iItem < m_bsColSortStates.nGetSize() );
    if ( iItem >= m_bsColSortStates.nGetSize() )
        return ASCENDING;
        
	// return ((m_hColumnSortStates) & ( 1 << iItem )) ? ASCENDING : DESCENDING;
	return ( m_bsColSortStates.bIsSet( iItem ) ) ? ASCENDING : DESCENDING;
}


//-------------------------------------------------------------------//
// SetColumnSortState()																//
//-------------------------------------------------------------------//
// This sets the sort state of a column.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::SetColumnSortState( int iItem, SORT_STATE bSortState )
{
	if( bSortState != GetColumnSortState( iItem ) )
		// m_hColumnSortStates ^= (1 << iItem);
		m_bsColSortStates.bToggle( iItem );
}


//-------------------------------------------------------------------//
// MoveSortedColumnToEnd()															//
//-------------------------------------------------------------------//
// This function will move a clicked column to the end of the 
// combined column list. This is useful when you move backwards 
// through column clicks.  Like click columns: 0, 1, 2, 1. The array 
// will hold [0,1,2] after the first 3 clicks, this function will 
// change it to [0,2,1] after the 4th click.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::MoveSortedColumnToEnd( int iItem )
{
	// Remove the item, then add it to the end.
	UnsetColumnSorted( iItem );
	m_vlSortedColumns.push_back( iItem );
}


//-------------------------------------------------------------------//
// UnsetColumnSorted()																//
//-------------------------------------------------------------------//
// This function removes sorting from a column.
// It is used on a column that has been cycled past ascending 
// or descending sorting. It is also used by 
// MoveSortedColumnToEnd() when a column is re-added to
// the end of the sorted columns.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::UnsetColumnSorted( int iItem )
{
	// Search for the item.
	for ( 
		LongVector::iterator it = m_vlSortedColumns.begin();
		it < m_vlSortedColumns.end();
		it++
	) {
		if ( *it == iItem )
		{
			m_vlSortedColumns.erase( it );
			break;
		}
	}
}


//-------------------------------------------------------------------//
// SetColumnNumeric()																//
//-------------------------------------------------------------------//
// Utility function to set a column that will contain only 
// numeric values.  Speeds up the sorting if this is set on the 
// right columns.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::SetColumnNumeric( int nCol )
{
	m_vlNumericColumns.push_back( nCol );
}


//-------------------------------------------------------------------//
// IsColumnNumeric()																	//
//-------------------------------------------------------------------//
// Utility function to tell you if the given column is set as numeric.
//-------------------------------------------------------------------//
const bool MultiColumnSortListCtrl::IsColumnNumeric( int nCol ) const
{
	for( int i = 0; i < (int)m_vlNumericColumns.size(); i++ )
	{	
		if( m_vlNumericColumns[ i ] == nCol )
			return true;
	}
	return false;
}


//-------------------------------------------------------------------//
// UnsetColumnNumeric()																//
//-------------------------------------------------------------------//
// Utility function to remove the numeric status of a column.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::UnsetColumnNumeric( int nCol )
{
	// Iterate through the numeric columns and remove if found.
	for ( 
		LongVector::iterator it = m_vlNumericColumns.begin();
		it < m_vlNumericColumns.end();
		it++
	) {
		if ( *it == nCol )
		{
			m_vlNumericColumns.erase( it );
			break;
		}
	}
}


//-------------------------------------------------------------------//
// AddDefaultColCachedData()																//
//-------------------------------------------------------------------//
// This adds default cached data for new columns.  By default, 
// new columns are not sorted and the start sort is set to
// ascending.  Previous filter is empty.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::AddDefaultColCachedData( int nCol )
{
	// Verify the size of the array.
	ASSERT( m_vColCachedData.size() == nCol );

	// Set the default cached values.  
	// One is maintained for each column at all times.
	m_vColCachedData.push_back( ColCachedData() );

}


//-------------------------------------------------------------------//
// SetColumnStartSort()																//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::SetColumnStartSortDescending( int nCol )
{
    if ( (unsigned int)nCol < m_vColCachedData.size() )
        m_vColCachedData[ nCol ].m_bSortedAscending = false;
}


//-------------------------------------------------------------------//
// bStartSortAscending()															//
//-------------------------------------------------------------------//
const bool MultiColumnSortListCtrl::bStartSortAscending( int nCol ) const
{
    if ( (unsigned int)nCol >= m_vColCachedData.size() )
        return false;
        
	return m_vColCachedData[ nCol ].m_bSortedAscending;
}


//-------------------------------------------------------------------//
// OnNotify()																			//
//-------------------------------------------------------------------//
//
// WARNINGS!
//
//	HDN_ENDDRAG
// -----------
// This notification is captured after moving a column.
// This is when we want to save the column order, as needed.
// WARNING!
// There was one problem with this notification:
//		1) It appears that the HDN_ENDDRAG notification is sent BEFORE 
//			the column order is actually changed (duh, MS).  We POST
//			a message that then calls SaveColumnOrder().
//
// HDN_ENDTRACK
// ------------
// This notification is captured after resizing a column.
// This is when we want to save the column sizing, as needed.
// There was one problem with this notification:
//		1) Apparently COMMCTRL.H's define for HDN_ENDTRACK is f'ed.
//			I was getting the "A" (ASCII) define under NT, which should have
//			been using the "W" (wide) Unicode version.  
//			Since we let ClassWizard supply us with this notification, 
//			hopefully everything is wired properly.
//			Original note in OnNotify(): We'll just check for either
//			HDN_ENDTRACKA or HDN_ENDTRACKW, since neither appears to 
//			"collide" with other notifications.
//			See COMMCTRL.H, ~line 844 (search for HDN_ENDTRACK).
//
//-------------------------------------------------------------------//
BOOL MultiColumnSortListCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	
	// The structure provided is an NMHEADER struct, which
	// BEGINS with an NMHDR struct.
	// It is a bit confusing because NMHDR has NOTHING to do 
	// specifically with header controls; it is the generic struct
	// for all notification msgs.
	NMHDR* pNH = (NMHDR*) lParam;
	NMHEADER* pHeader = (NMHEADER*) lParam;

	BOOL bReturn = inherited::OnNotify(wParam, lParam, pResult);

	// DEBUG
	// We're not getting ENDTRACK notifications??
	// TRACE( _T("List notification: %d\t%d\t%d\n"), pNH->code, HDN_ENDTRACK, HDN_ENDDRAG );

	if ( pNH->code == HDN_ENDDRAG )		// 0xfffffeca
	{
		// OLD
		// Watch for drags of hidden columns, that's a no no!
		// This should be prevented because we never let their size 
		// get greater than zero.
		// ASSERT( !bIsColumnHidden( pHeader->iItem ) );

		// Update the column order array.
		// Don't forget, we need to POST it, dammit!
		PostMessage( WM_USER_SAVE_COLUMN_ORDER, pHeader->iItem );

	// Apparently COMMCTRL.H's define for HDN_ENDTRACK is f'ed.
	// I was getting the "A" (ASCII) define under NT, which should have
	// been using the "W" (wide) Unicode version.  We'll just check
	// for either, since neither appears to "collide" with other notifications.
	// See COMMCTRL.H, ~line 844 (search for HDN_ENDTRACK).
	} else if ( 
			pNH->code == HDN_ENDTRACKA
		||	pNH->code == HDN_ENDTRACKW
	) {

		// Update the column widths array.
		SaveColumnWidths();

	// #pragma _DEBUGCODE("Testing")
	/*
	} else
	{
		TRACE1( _T("Got NH code %d...\n"), pNH->code );
	*/
	}

	return bReturn;

}


//-------------------------------------------------------------------//
// RefreshAbsVisColLookup()														//
//-------------------------------------------------------------------//
// Here, we maintain indexes from abs to vis columns and back.
// They will always be similar to the following:
//
//		Absolutes:	0	1	2	3	4	5	6	7	8	9
//		AbsToVis:	0	-1	1	-1	-1	2	-1	3	-1	-1
//		VisToAbs:	0		2			5		7
//
//	It should be apparent which columns are hidden.
//
// How do we build these arrays?  There is only one place from which
// we can get this information - the hidden column data array.
// We loop through that, building the AbsToVis array.  Then it
// is easy to build the VisToAbs from the AbsToVis array.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::RefreshAbsVisColLookup()
{
	int nA;
	int nR;

	// Resize the lookup arrays as needed.
	m_lvAbsToVisLookup.resize( GetAbsoluteColCount() );
	m_lvVisToAbsLookup.resize( GetVisibleColCount() );
	
	// Init the AbsToVis array.
	for ( nA = 0; nA < GetAbsoluteColCount(); nA++ )
		m_lvAbsToVisLookup[nA] = 0;

	// Loop through the hidden columns, setting their status in the AbsToVis array.
	for ( nA = 0; nA < GetHiddenColCount(); nA++ )
		m_lvAbsToVisLookup[ m_vHiddenColData[ nA ].nAbsColNumber ] = -1;
	
	// Now loop through the AbsToVis array, adjusting the "0=visible" status
	// to the actual visible column number.	
	nR = 0;
	for ( nA = 0; nA < GetAbsoluteColCount(); nA++ )
		if ( m_lvAbsToVisLookup[nA] == 0 )
			m_lvAbsToVisLookup[nA] = nR++;	
	
	// Set up the visible col lookup array.
	nA = 0;
	for ( nR = 0; nR < GetVisibleColCount(); nR++ )
	{
		while ( m_lvAbsToVisLookup[ nA ] == -1 )
			nA++;

		m_lvVisToAbsLookup[nR] = nA++;
	}

}


//-------------------------------------------------------------------//
// DisplayColumnHidingMenu()														//
//-------------------------------------------------------------------//
// This function displays a menu for hiding/unhiding columns, at
// the supplied point (in SCREEN COORDINATES).
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::DisplayColumnHidingMenu( CPoint& point )
{
	// Display a menu of all columns, in current order, with visible columns 
	// checked, and hidden columns last and unchecked.
	CMenu ColMenu;
	ColMenu.CreatePopupMenu();

	// This does the work.
	AddColumnHidingMenu( &ColMenu );

	ColMenu.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
		point.x, 
		point.y, 
		this
	);

	VERIFY( ColMenu.DestroyMenu() );
}

//-------------------------------------------------------------------//
// AddColumnHidingMenu()															//
//-------------------------------------------------------------------//
// Add a menu of all columns, in current order, with visible 
// columns checked, and hidden columns last and unchecked.
// Use the provided popup menu, adding an item for each column.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::AddColumnHidingMenu( 
	CMenu*	pPopupMenu,
	int		nInsertPosition
) {
	// Get the column order array.
	// This contains all the VISIBLE columns.
	Long* plOrder = new Long[ GetVisibleColCount() ];
	ASSERT( sizeof Long == sizeof INT );
	GetColumnOrderArray(
		(LPINT) plOrder,
		GetVisibleColCount()
	);

	// Loop through the visible columns.
	int nA, nAbsCol, nVisCol;
	for ( nA = 0; nA < GetVisibleColCount(); nA++ )
	{
		nVisCol = *( plOrder + nA );
		nAbsCol = GetAbsoluteCol( nVisCol );
		ASSERT( nVisCol >= 0 && nVisCol < GetVisibleColCount() );
		ASSERT( nAbsCol >= 0 && nAbsCol < GetAbsoluteColCount() );

		pPopupMenu->AppendMenu( 
			MF_STRING | MF_ENABLED,
			IDC_TOGGLE_COLUMN_HIDDEN_STATUS_BASE + nAbsCol,
			GetVisibleColName( nVisCol )
		);

		pPopupMenu->CheckMenuItem( 
			IDC_TOGGLE_COLUMN_HIDDEN_STATUS_BASE + nAbsCol,
			MF_BYCOMMAND | MF_CHECKED
		);
	}

	for ( nA = 0; nA < GetHiddenColCount(); nA++ )
	{
		pPopupMenu->AppendMenu(
			MF_STRING | MF_ENABLED,
			IDC_TOGGLE_COLUMN_HIDDEN_STATUS_BASE + m_vHiddenColData[nA].nAbsColNumber,
			m_vHiddenColData[nA].strColHeading
		);
	}

	// Clean up.
	delete[] plOrder;

}


//-------------------------------------------------------------------//
// HideColumn()																		//
//-------------------------------------------------------------------//
// Remove the column and back it up to the hidden columns array.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::HideColumn( int nAbsCol )
{
	ASSERT( 
			!m_bRegistrySettingsLoaded
		||	GetHiddenColCount() == RegGetHiddenColCount() 
	);

	// DEBUG
	// TRACE( _T("\nHidden Count = %d\n"), GetHiddenColumnCount() );

	int nVisCol = GetVisibleCol( nAbsCol );

	// If this column is already hidden, just return.
	// Most of the time we don't hit this case.
	if ( nVisCol == -1 )
		return;

	// First, we "back up" the column's current info, so we
	// can restore it later as needed.  
	// ABSOLUTE					RELATIVE
	LVCOLUMN col;
	// col.mask = LVCF_FMT | LVCF_IMAGE | LVCF_ORDER | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.mask = LVCF_FMT | LVCF_WIDTH;
	GetColumn( nVisCol, &col );
	HiddenColData hcd;
	hcd.nAbsColNumber = nAbsCol;
	hcd.nFormat = col.fmt;
	hcd.nWidth = col.cx;
	hcd.strColHeading = GetVisibleColName( nVisCol );
	
	// This only works when we use a deque, but the debugger
	// doesn't work very well with them, and I am not 
	// familiar with the dynamic storage like I am with vector.
	// m_vHiddenColData.push_front( hcd );
	m_vHiddenColData.insert( m_vHiddenColData.begin(), hcd );
	
	// What DOES that iSubItem mean?  Not this...
	// ASSERT( col.iSubItem == nVisCol );

	// Set the hidden status.
	// m_bsColHiddenStatus.Set( nAbsCol );
	
	// Now totally kill the column, as far as the 
	// list control is concerned.
	inherited::DeleteColumn( nVisCol );

	// Refresh the relative<->absolute lookup tables.
	// We need to do this before saving!
	RefreshAbsVisColLookup();
	
	// Update the hidden status to the registry.
	// Skip it if we are just adjusting while loading!
	if ( m_bRegistrySettingsLoaded )
	{
		// Update the hidden status to the registry.
		// This includes the hidden count, and likely the order.
		// We don't worry about the order if we were just hiding
		// the last column, as it would not change.
		RegSetHiddenColCount();
		// if ( nCol < GetVisibleColCount() )		// I'm not taking this chance right now - getting it working first!
			SaveColumnOrder();
	}

}


//-------------------------------------------------------------------//
// ShowColumn()																		//
//-------------------------------------------------------------------//
// This shows the specified abs column.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::ShowColumn( int nAbsCol )
{
	ASSERT( 
			!m_bRegistrySettingsLoaded
		||	GetHiddenColCount() == RegGetHiddenColCount() 
	);
	ASSERT( nAbsCol >=0 && nAbsCol < GetAbsoluteColCount() );

	// DEBUG
	// TRACE( _T("\nHidden Count = %d\n"), GetHiddenColumnCount() );

	// OLD
	// Set the hidden status.
	// m_bsColHiddenStatus.Set( nAbsCol, false );
	
	int nHiddenCol = GetHiddenIndex( nAbsCol );

	if ( nHiddenCol == -1 )
		return;
	
	// Get all the data we will need.  We are about to delete it.
	LVCOLUMN lvcNew;
	CString strColHeading = m_vHiddenColData[nHiddenCol].strColHeading;
	lvcNew.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
	lvcNew.fmt = m_vHiddenColData[nHiddenCol].nFormat;
	lvcNew.pszText = strColHeading.GetBuffer( 0 );
	lvcNew.cx = m_vHiddenColData[nHiddenCol].nWidth;
	lvcNew.iOrder = GetVisibleColCount();

	// Whack it from the hidden col data array.
	m_vHiddenColData.erase( m_vHiddenColData.begin() + nHiddenCol );

	// We need to call refresh so we can get the vis col from absolute.
	// But we need to call insert b4 refresh so count is correct.
	// We need to calc the visible col on our own, insert the col, and 
	// then we can refresh.
	
	// We start assuming that no cols are hidden and the vis col
	// is the same as the absolute.
	// Then, we take away one for ev abs col before ours that is 
	// hidden.
	int nVisCol = nAbsCol;
	int nA;
	for ( nA = 0; nA < nAbsCol; nA++ )
		if ( GetVisibleCol( nA ) == -1 )
			nVisCol--;

	// Now Insert it back in, at the far right.
	inherited::InsertColumn( 
		// GetVisibleCol( nAbsCol ),
		nVisCol,
		&lvcNew 
	);

	// Refresh the relative<->absolute lookup tables.
	RefreshAbsVisColLookup();

	// Update the hidden status to the registry.
	RegSetHiddenColCount();
	SaveColumnOrder();

}


//-------------------------------------------------------------------//
// OnToggleColumnHiddenStatus()													//
//-------------------------------------------------------------------//
// We handle requests to hide/unhide columns.  These requests come
// from our pop-up menus.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::OnToggleColumnHiddenStatus( UINT nID )
{
	int nAbsCol = nID - IDC_TOGGLE_COLUMN_HIDDEN_STATUS_BASE;

	ASSERT( nAbsCol >= 0 && nAbsCol < GetAbsoluteColCount() );
	if ( nAbsCol >= 0 && nAbsCol < GetAbsoluteColCount() )
	{
		if ( GetVisibleCol( nAbsCol ) == -1 )
		{
			TRACE( "Showing col %d via right-click menu...\n", nAbsCol );
			ShowColumn( nAbsCol );

		} else
		{	
			// We are dealing with a straight visible ( i.e., relative ) column number.
			TRACE( "Hiding col %d via right-click menu...\n", nAbsCol );
			HideColumn( nAbsCol );
		}

	}

}


//-------------------------------------------------------------------//
// REGISTRY STORAGE OF SORTING, ORDER AND WIDTHS
//-------------------------------------------------------------------//

//-------------------------------------------------------------------//
// OnSaveColumnOrder()																//
//-------------------------------------------------------------------//
// This allows us to POST notifications of column changes.  At the
// time we receive the notification via MFC, the column structure
// has not yet been updated.
//-------------------------------------------------------------------//
LRESULT MultiColumnSortListCtrl::OnSaveColumnOrder(
	WPARAM uID,
	LPARAM lEvent
) {
	// OLD
	/*
	// If the user just dropped the column to the far left, we
	// need to make sure the hidden columns are still first.
	VerifyHiddenOrder( uID );
	*/

	// NOW we can save 'em.
	SaveColumnOrder();
	
	return 0;
}


//-------------------------------------------------------------------//
// SaveColumnWidths()																//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::SaveColumnWidths()
{
	int nAbsColCount = GetAbsoluteColCount();
	
	// Set Total Column Count as needed.
	RegSetTotalColumnCount( nAbsColCount );

	// Set up storage for all widths.
	Long* plColWidths = new Long[ nAbsColCount ];

	// First start with the visible column widths, extracted 
	// directly from the list.  
	int n;
	for( n = 0; n < GetVisibleColCount(); n++ )
	{
		// Get the width of the nth visible column, and stuff
		// it in the right absolute slot.
		*( plColWidths + GetAbsoluteCol( n ) ) = GetColumnWidth( n );
	}

	// Now we copy out the hidden column widths.
	for ( n = 0; n < GetHiddenColCount(); n++ )
		*( plColWidths + m_vHiddenColData[ n ].nAbsColNumber ) = m_vHiddenColData[ n ].nWidth;

	// Now save to the registry.
	g_pAppRegData->SetBinary(
		LPCTSTR( m_strUniqueName + szColumnWidthsKey ), 
		(void*) plColWidths,
		nAbsColCount * sizeof( Long )
	);

	delete[] plColWidths;
}


//-------------------------------------------------------------------//
// SaveColumnOrder()																	//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::SaveColumnOrder()
{
	int n;

	int nAbsColCount = GetAbsoluteColCount();
	int nVisColCount = GetVisibleColCount();

	Long* plOrder = new Long[ nAbsColCount ];

	// Set Total Column Count as needed.
	RegSetTotalColumnCount( nAbsColCount );
	
	// Get the current visible column order directly from the list.
	ASSERT( sizeof Long == sizeof INT );
	GetColumnOrderArray(
		(LPINT) plOrder,
		GetVisibleColCount()
	);

	// Now we need to convert them from vis indexes to absolute.
	for ( n = 0; n < nVisColCount; n++ )
		*( plOrder + n ) = GetAbsoluteCol( *( plOrder + n ) );

	// Now tack on the hidden order.
	for ( n = nVisColCount; n < nAbsColCount; n++ )
	{
		*( plOrder + n ) = m_vHiddenColData[ n - nVisColCount ].nAbsColNumber;
	}

	g_pAppRegData->SetBinary(
		LPCTSTR( m_strUniqueName + szColumnOrderKey ), 
		(void*) plOrder,
		nAbsColCount * sizeof( Long )
	);

	delete[] plOrder;
}


//-------------------------------------------------------------------//
// SaveColumnSorting()																//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::SaveColumnSorting() 
{
	// Save the column sort count.
	g_pAppRegData->SetDWord(
		LPCTSTR( m_strUniqueName + szSortedColumnCountKey ), 
		m_vlSortedColumns.size()
	);

	// If we have sorting, save it out.
	if ( m_vlSortedColumns.size() > 0 )
	{
		// The nasty casting is for VS 2005 - you can't cast any class, etc.
		// to a void*, only simple types, so you have to get the integer,
		// then get a pointer to it, then cast.
		g_pAppRegData->SetBinary(
			LPCTSTR( m_strUniqueName + szColumnSortKey ), 
			&( *m_vlSortedColumns.begin() ),
			m_vlSortedColumns.size() * sizeof( Long )
		);
		g_pAppRegData->SetBinary(
			LPCTSTR( m_strUniqueName + szColumnSortStatesKey ), 
			(void*) m_bsColSortStates.GetBitData(),
			m_bsColSortStates.GetBitDataDepth() * sizeof( BitReg )
		);
	}

}


//-------------------------------------------------------------------//
// RegGetTotalColumnCount()														//
//-------------------------------------------------------------------//
int MultiColumnSortListCtrl::RegGetTotalColumnCount()
{
	// Get Total Column Count from registry.
	return (int) g_pAppRegData->GetDWord(
		LPCTSTR( m_strUniqueName + szTotalColumnCountKey ), 
		0, 0 
	);
}


//-------------------------------------------------------------------//
// RegSetTotalColumnCount()														//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::RegSetTotalColumnCount( int nColCount )
{
	if ( nColCount != RegGetTotalColumnCount() )
	{
		g_pAppRegData->SetDWord( 
			LPCTSTR( m_strUniqueName + szTotalColumnCountKey ), 
			nColCount
		);
	}
}


//-------------------------------------------------------------------//
// RegGetSortedColumnCount()														//
//-------------------------------------------------------------------//
int MultiColumnSortListCtrl::RegGetSortedColumnCount()
{
	// Get Column Count from registry.
	return (int) g_pAppRegData->GetDWord( 
		LPCTSTR( m_strUniqueName + szSortedColumnCountKey ), 
		0, 0
	);
}


//-------------------------------------------------------------------//
// RegGetHiddenColCount()															//
//-------------------------------------------------------------------//
int MultiColumnSortListCtrl::RegGetHiddenColCount()
{
	// Get Column Count from registry.
	return (int) g_pAppRegData->GetDWord( 
		LPCTSTR( m_strUniqueName + szHiddenColumnCountKey ), 
		0, 0
	);
}


//-------------------------------------------------------------------//
// RegSetHiddenColCount()															//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::RegSetHiddenColCount()
{
	// Set Column Count to registry.
	g_pAppRegData->SetDWord(
		LPCTSTR( m_strUniqueName + szHiddenColumnCountKey ), 
		GetHiddenColCount()
	);
}


//-------------------------------------------------------------------//
// RegGetColumnSortOrderWidths()                                     //
//-------------------------------------------------------------------//
// This function is called in Initialize() to load the sorting,
// order and width registry information and set up the list
// accordingly.
// 
// NOTE: The user should have set up the list's order and widths 
// into a default state before calling Initialize().  This default 
// state will be preserved and saved to the registry if no valid
// registry entries are found.  Also overload 
// SetDefaultColumnSortingAndHiding() as desired.
// 
// When we enter this function, all columns must have been
// inserted (in a visible state).  We are responsible for 
// hiding the columns following the instructions in the reg info.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::RegGetColumnSortOrderWidths()
{
	int nA;
	
	// First, empty the hidden column data array.  It will 
	// expand as we call HideColumn() later.
	m_vHiddenColData.clear();

	// With no cols marked as hidden, this should set
	// all columns to visible; then we can adjust
	// with calls to HideColumn().
	RefreshAbsVisColLookup();

	// The absolute col count should be all ready to go here
	// as the caller should have already inserted all static 
	// columns.
	int nAbsColCount = GetAbsoluteColCount();

    // Get all the registry data and validate it.
    // If we find invalid data, we reset everything.

	Long* plOrder = 0;
	Long* plWidths = 0;
	Long* plSort = 0;
	BitSet bsSortStates( nAbsColCount );

	// First, we need to get the col counts.
	int nHiddenColCount = RegGetHiddenColCount();
	int nSortedColCount = RegGetSortedColumnCount();
	int nVisColCount = nAbsColCount - nHiddenColCount;

    // Validate the counts.
    bool bValid = ( nAbsColCount == RegGetTotalColumnCount() );
    bValid = bValid && ( nHiddenColCount >= 0 && nHiddenColCount <= nAbsColCount );
    bValid = bValid && ( nSortedColCount >= 0 && nSortedColCount <= nAbsColCount );
    
    if ( bValid )
    {
		plOrder = new Long[ nAbsColCount ];

		// Get Column Order.
		bValid = false;
		g_pAppRegData->GetBinary(
			LPCTSTR( m_strUniqueName + szColumnOrderKey ), 
			nAbsColCount * sizeof( Long ),
			(void*) plOrder,
			0,						    // pDefault
			&bValid					    // pbFound
		);

        if ( bValid )
		    for ( nA = nAbsColCount - 1; nA >= nVisColCount; nA-- )
			    bValid = bValid && ( *( plOrder + nA ) < nAbsColCount );
    }

    if ( bValid )
    {
		plWidths = new Long[ nAbsColCount ];
		
		// Get Column Widths.
		bValid = false;
		g_pAppRegData->GetBinary(
			LPCTSTR( m_strUniqueName + szColumnWidthsKey ), 
			nAbsColCount * sizeof( Long ),
			(void*) plWidths,
			0,						    // pDefault
			&bValid				        // pbFound
		);
		
		/*
		if ( bValid )
			for ( nA = 0; nA < nAbsColCount; nA++ )
			{
			    bValid = bValid && ...
			}
		*/
	}
    
    if ( bValid )
    {
		plSort = new Long[ nSortedColCount ];

		// Get Column Sorting and Sort States.
		bValid = false;
		g_pAppRegData->GetBinary(
			m_strUniqueName + szColumnSortKey,
			nSortedColCount * sizeof( Long ),
			(void*) plSort,
			0,							// pDefault
			&bValid						// pbFound
		);

		for ( int i = 0; i < nSortedColCount; i++ )
			bValid = bValid && ( *( plSort + i ) <= nAbsColCount );
        
    }
    
    if ( bValid )
    {
		bValid = false;
		g_pAppRegData->GetBinary(
			m_strUniqueName + szColumnSortStatesKey,
			bsSortStates.GetBitDataDepth() * sizeof( BitReg ),
			(void*) bsSortStates.GetBitData(),
			0,							// pDefault
			&bValid						// pbFound
		);
	}

	// If EVERYTHING checked out, use the registry data.
	if ( bValid )
	{	
		// We are using Longs in the registry, so we know we
		// can count on the size being correct across old and
		// new app versions alike.  Make sure that
		// the destination array matches in size.
		ASSERT( sizeof Long == sizeof INT );

		// Update column order in list.
		
		// Hide all the columns that need hiding.  We want
		// to hide in the opposite direction to preserve order.
		for ( nA = nAbsColCount - 1; nA >= nVisColCount; nA-- )
			HideColumn( *( plOrder + nA ) );
		
		// Next we need to convert from abs to visible cols
		// using our spankin' new conversion arrays.
		for ( nA = 0; nA < nVisColCount; nA++ )
		{
			ASSERT( GetVisibleCol( *( plOrder + nA ) ) >= 0 );
			*( plOrder + nA ) = GetVisibleCol( *( plOrder + nA ) );
		}

		// Update the list with the visible-columns order.
		ASSERT( nVisColCount == GetVisibleColCount() );
		SetColumnOrderArray(
			nVisColCount,
			(LPINT) plOrder
		);

		// Update column widths in list.

		// Update visible column widths.
		for ( nA = 0; nA < nVisColCount; nA++ )
		{
			SetColumnWidth( 
				nA, 												// Vis col
				*( plWidths + GetAbsoluteCol( nA ) )	// Width of corresponding abs col
			);
		}

		// Store the hidden column widths for later restoration.
		for ( nA = 0; nA < nHiddenColCount; nA++ )
		{
			// Stuff the slots using GetHiddenCol() to do the abs col lookup.
			m_vHiddenColData[nA].nWidth = *( plWidths + GetHiddenCol( nA ) );
		}

		// Set Column Sorting and Sort States.  You should
		// update the list with ResortAllItems() after all items have 
		// been added.

		// Copy out the results.
		m_vlSortedColumns.clear();
		for ( int i = 0; i < nSortedColCount; i++ )
			m_vlSortedColumns.push_back( *( plSort + i ) );

		// Use the bitset operator=().
		m_bsColSortStates = bsSortStates;

	} else
	{
        // Some or all of the registry data was invalid or missing.
        // Reset it all.

		// The parent or derived class is responsible for having set
		// order and width defaults before calling the base class 
		// Initialize().  Set any default column sort/hide states.
		SetDefaultColumnSortingAndHiding();

		// Now save to the registry.
		// NOTE: Bypass derived versions of these functions, this is
		// not a true "column updated" situation.
		ForceRegistrySave();
	}

	// Clean up.
	if ( plWidths ) delete[] plWidths;
	if ( plOrder  ) delete[] plOrder;
	if ( plSort   ) delete[] plSort;

}


//-------------------------------------------------------------------//
// ForceRegistrySave()																//
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::ForceRegistrySave()
{
	// Save to the registry.
	// NOTE: Bypass derived versions of these functions, this is
	// not a true "column updated" situation.
	RegSetHiddenColCount();
	MultiColumnSortListCtrl::SaveColumnOrder();
	MultiColumnSortListCtrl::SaveColumnWidths();
	MultiColumnSortListCtrl::SaveColumnSorting();
}


//-------------------------------------------------------------------//
// OnItemchanged()																	//
//-------------------------------------------------------------------//
// Here, we are looking for checkbox changes, so we can propogate
// them to everything that's currently selected.
//-------------------------------------------------------------------//
BOOL MultiColumnSortListCtrl::OnItemchanged( NMHDR* pNMHDR, LRESULT* pResult ) 
{
	// If we're using checkboxes...
	if ( GetExtendedStyle() & LVS_EX_CHECKBOXES )
	{
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		*pResult = 0;

		if (pNMListView->uOldState == 0 && pNMListView->uNewState == 0)
			return FALSE;	// No change

		BOOL bPrevState = (BOOL)(((pNMListView->uOldState & LVIS_STATEIMAGEMASK)>>12)-1);   // Old check box state
		
		if (bPrevState < 0)	// On startup there's no previous state 
			bPrevState = 0; // so assign as false (unchecked)

		// New check box state
		BOOL bChecked=(BOOL)(((pNMListView->uNewState & LVIS_STATEIMAGEMASK)>>12)-1);   
		if (bChecked < 0) // On non-checkbox notifications assume false
			bChecked = 0; 

		if (bPrevState == bChecked) // No change in check box
			return FALSE;
		
		// Now bChecked holds the new check box state.
		
		// Check the item with the focus.
		// It has already had the checkbox marked, but
		// we want the derivable function to be called.
		CheckItem( pNMListView->iItem, bChecked != FALSE );			

		// If Ctrl or Shift are down, go through the entire selection, 
		// if any, and set the same state.
		if (
				( ::GetKeyState( VK_CONTROL	) < 0 )
			||	( ::GetKeyState( VK_SHIFT		) < 0 )
		) {
			int nIndex = GetNextItem( -1, LVNI_SELECTED );
			while ( nIndex != -1 )
			{
				CheckItem( nIndex, bChecked != FALSE );
				nIndex = GetNextItem( nIndex, LVNI_SELECTED );
			}
		}

	}

	// Lots of people rely on this coming through to the parent.
	// Always let the parent have a crack at it.
	// NOTE TO MICROSOFT: SUCK IT!  Docs say return TRUE...
	return FALSE;
}


//-------------------------------------------------------------------//
// OnSelectAll()																		//
//-------------------------------------------------------------------//
// It is recommended that you define the Ctrl-A accelerator for 
// IDC_SELECT_ALL to call this.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::OnSelectAll() 
{
	SelectAll();	
}


//-------------------------------------------------------------------//
// OnAutosizeColumns()																//
//-------------------------------------------------------------------//
// It is recommended that you define the Ctrl-+ accelerator for 
// IDC_AUTOSIZE_COLUMNS to call this.
//-------------------------------------------------------------------//
void MultiColumnSortListCtrl::OnAutosizeColumns() 
{
	for ( int nA = 0; nA < GetVisibleColCount(); nA++ )
		SetColumnWidth( nA, LVSCW_AUTOSIZE );	
}
void MultiColumnSortListCtrl::OnAutosizeColumnsAndHeaders() 
{
	for ( int nA = 0; nA < GetVisibleColCount(); nA++ )
		AutoSizeColumn( nA );
}


//-------------------------------------------------------------------//
// SetFocusToFilter()																//
//-------------------------------------------------------------------//
// This lets us do what it says it lets us do.
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::SetFocusToFilter( int nAbsCol )
{
	if ( !MakeColumnVisible( nAbsCol ) )
		return false;

	int nVisCol = GetVisibleCol( nAbsCol );
	ASSERT( nVisCol != -1 );

	SetFocus();

	LRESULT lResult = SendMessage(
		LVM_ENSUREVISIBLE,
		(WPARAM)0,
		(LPARAM) (BOOL) true		// fPartialOK
	);  
	
	m_ctlHeaderCtrl.SetFocus();

	lResult = SendMessage(
		LVM_ENSUREVISIBLE,
		(WPARAM)0,
		(LPARAM) (BOOL) true		// fPartialOK
	);  
	
	lResult = m_ctlHeaderCtrl.SendMessage(
		HDM_EDITFILTER,
		(WPARAM)nVisCol,           // = (WPARAM) (int) i
		MAKELPARAM( false, 0 )		// discard changes?
	);

	return true;
}


//-------------------------------------------------------------------//
// SetFilterText()																	//
//-------------------------------------------------------------------//
// This lets us do what it says it lets us do.
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::SetFilterText( 
	int nAbsCol, 
	LPTSTR szText
) {
	if ( !MakeColumnVisible( nAbsCol ) )
		return false;

	int nVisCol = GetVisibleCol( nAbsCol );
	ASSERT( nVisCol != -1 );

	LRESULT lResult;
	
	HDTEXTFILTER filter;
	filter.pszText = szText;
	filter.cchTextMax = strlen( szText );

	HDITEM item;
	item.mask = HDI_FILTER;
	item.type = HDFT_ISSTRING;
	item.pvFilter = (void*)&filter;

	lResult = m_ctlHeaderCtrl.SendMessage(
		HDM_SETITEM,
		(WPARAM)nVisCol,           // = (WPARAM) (int) i
		(LPARAM) &item					// = const (LPARAM) (LPHDITEM) phdItem;
	);

	return true;
}


//-------------------------------------------------------------------//
// MakeColumnVisible()																//
//-------------------------------------------------------------------//
// This function un-hides (if needed) and scrolls (if needed) to the 
// provided column.
//
// Based on code from the CodeGuru "Multiline Editable Subitems" 
// article by Rex Myer.
//-------------------------------------------------------------------//
bool MultiColumnSortListCtrl::MakeColumnVisible( int nAbsCol, bool bForceShow )
{
	if ( GetHiddenIndex( nAbsCol ) != -1 )
	{
		if ( !bForceShow )
			return false;
		
		ShowColumn( nAbsCol );
	}

	int nVisCol = GetVisibleCol( nAbsCol );

	// Get the column offset
	int nOffset = 0;
	for( int nA = 0; nA < nVisCol; nA++ )
		nOffset += GetColumnWidth( nA );
	int nColWidth = GetColumnWidth( nVisCol );
	
	CRect rect;
	GetItemRect( 0, &rect, LVIR_BOUNDS );
	
	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect( &rcClient );
	if ( nOffset + rect.left < 0 || nOffset + nColWidth + rect.left > rcClient.right )
	{
		CSize size;
		size.cx = nOffset + rect.left;
		size.cy = 0;
		Scroll( size );
		rect.left -= size.cx;
	}

	return true;
}
