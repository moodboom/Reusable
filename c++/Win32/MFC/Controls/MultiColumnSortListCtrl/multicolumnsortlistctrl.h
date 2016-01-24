//-------------------------------------------------------------------//
// MultiColumnSortListCtrl.h : header file
//
// This class provides the basic list control as it SHOULD be.
//
//		1) It has column sorting capability (DUH! Microsoft! Wake up!).
//			The user can sort multiple columns by holding down Ctrl
//			while clicking the column header.  Repeat clicks toggle the
//			sort direction.  
//		2)	Column reordering and widths are also maintained.  Columns
//			can be hidden by right-clicking on the header and 
//			selecting from an available columns list.
//		3)	It uses virtual functions to save and load the column 
//			sorting, order, widths and visibility to/from the registry at 
//			the appropriate times.  All you have to supply is an original
//			list name, which gets pre-pended to the registry keys.
//		4) It has handling of the data attached to each item via
//			the lParam, via virtual functions that derived classes can
//			easily override.
//		5) It takes provided menu ID's and gives the user right-click
//			menu access.  You supply the menu ID's and add handling for
//			the subsequent messages in your derived class.  It will
//			allow you to provide your own ON_UPDATE_COMMAND_UI handlers.
//		6) It can provide a VIRTUAL list mechanism that manages the 
//			list strings outside of the common control's implementation.
//			If ya want, we will store all the strings for ya, which 
//			makes for lightening-fast sorting and filtering.  Try it, 
//			you will never go back to CListCtrl's typical Microsoft-quality 
//			code again.
//		7) It will store column descriptions.  You can use these to
//			provide tooltips, etc.
//		8) Simplified checkbox support - see ShowCheckboxes().
//			NOTE: Checkboxes are incompatible with (6).
//			You can only use one or the other.  Blame M$.  :(
//		9) Filtering support.
//
//	Usage:
//
//		1) Derive a class.
//		2) Override Initialize() and insert columns with InsertColumn(), 
//			in default order with default widths.  Then call base class 
//			Initialize().
//		3) Override SetDefaultColumnSortingAndHiding() if desired.
//		4) In parent, call Initialize(), add items to the list, and
//			call ResortItems() for the initial sort.
//
// You need to have set up a global AppRegistryData object.
// That is required for us to save our settings.  Note
// that this class is wired to be used with one of the following:
//
//		An MFC application that uses a BaseApp-derived main app class
//		An MFC app that uses the standard CWinApp-derived main app class
//		Situations where there is NO CWinApp class
//
// In the third case, you should call AppRegistryData::SetBaseKey() for your
// AppRegistryData object to specify yourself EXACTLY where you want your
// list control settings to be stored.
//
//	That's about it.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//


#if !defined( MULTI_COLUMN_SORT_LIST_CTRL_H )
#define MULTI_COLUMN_SORT_LIST_CTRL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//

#define	IDC_RIGHT_CLICK_KEY			7000
#define	IDC_SELECT_ALL					70001
#define	IDC_AUTOSIZE_COLUMNS			70002

// These are defined in MultiColumnHeaderCtrl.h:
// #define IDB_ARROW1UP					19000
// #define IDB_ARROW1DOWN				19001
// #define IDB_ARROW2UP					19002
// #define IDB_ARROW2DOWN				19003
// #define IDB_ARROW3UP					19004
// #define IDB_ARROW3DOWN				19005

// The range between these defines need to be reserved for column
// hiding messages.
#define	IDC_TOGGLE_COLUMN_HIDDEN_STATUS_BASE  28000 
#define	IDC_TOGGLE_COLUMN_HIDDEN_STATUS_LAST  28999

#define IDS_FILTERING_NOT_AVAILABLE 6050	// This indicates filter support is not available in the end-user's environment
#define IDS_FILTERING_NOT_INCLUDED	6051	// This indicates filter support was not compiled into the executable

// Since this is a reusable class, we don't have access to a resource file.
//
// 1) Make sure that the above defines are "rolled into" your project's
//		resource file.  Select "Resource Set Includes" on the View menu and add 
//		this header file to the "Read-only symbol directives" listbox.
// 2) Make sure your project includes the associated resources in its
//		resource file.
//
// We want to exclude the remainder of the include file when dealing with App Studio.
#ifndef RC_INVOKED
/////////////////////////////////////////////////////////////////////////


#include "..\..\..\..\STLExtensions.h"		// For vectors ( m_vbStartSortAscending, m_vlSortedColumns, m_vlNumericColumns, etc. )

#include "..\..\..\..\BitSet.h"				// For bitset m_bsColSortStates

#include "..\..\Controls\MenuXP\MenuXP.h"	// For converting our menu's to CMenuXP style

#include "MultiColumnSortHeaderCtrl.h"


/////////////////////////////////////////////////////////////////
// Globals Statics Constants												//
/////////////////////////////////////////////////////////////////
//

#define MULTI_COLUMN_SORT  1
#define SINGLE_COLUMN_SORT 0

// Define this here to support list filtering.  As an alternative, 
// you can define this within your project settings.
// See note in ShowFilterBar() for details.
#define COMPILE_WITH_FILTER_SUPPORT

enum SORT_STATE{ DESCENDING, ASCENDING };

class ListItemData
{
public:
	ListItemData(
		ustring*		pstrNewColStrings	= 0,
		LPARAM		lParam				= 0
	) :
		// Init vars.
		m_pstrColumnStrings	( pstrNewColStrings	),
		m_lParam					( lParam					)
	{}

	// Thought this might help, twas no use...
	/*
	inline ListItemData& operator=( const ListItemData& lidSource )
	{
		m_lParam = lidSource.m_lParam;
		m_pstrColumnStrings = lidSource.m_pstrColumnStrings;
	}
	*/

	// This is a pointer to an array of cell data, one
	// for each column in the list.
	//
	// Note that inserting/deleting columns means
	// we need to reallocate and copy the contents
	// of this array for each item in the list.
	ustring*	m_pstrColumnStrings;

	// The lParam data that every item supports.
	LPARAM							m_lParam;
};

typedef std::vector<ListItemData*> ListItemDataVector;

typedef struct
{	
	int		nAbsColNumber;
	CString	strColHeading;
	int		nFormat;
	int		nWidth;
} HiddenColData;


class ColCachedData
{
public:
	ColCachedData()
	:
		// Init vars.
		m_bSortedAscending( true )

	{
	}

	// TO DO
	// To speed up owner drawing by avoiding always 
	// having to reload these, we keep them cached.
	// LV_COLUMN lvc;
	// LPINT pnColOrder;
	
	ustring	m_strPrevFilter;
	bool		m_bSortedAscending;

};


// Silly M$ made this check a bit tricky.
// This seems to work consistently though, THANKS AGAIN CODEGURU!!
#define LIST_ITEM_CHECKED( a )	UINT((int(a) + 1) << 12)


//
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// MultiColumnSortListCtrl

class MultiColumnSortListCtrl : public CListCtrl
{
	typedef CListCtrl inherited;

	// For CMenuXP functionality.
	DECLARE_MENUXP()

// Construction
public:
	
	// Default constructor.
	// Turning off the header was added, so that derived classes
	// that do not want this base class's functionality can
	// bypass it.  May need updating, never used.
	MultiColumnSortListCtrl(
		CString	strUniqueRegistryName,
		bool		bUseExternalData					= true,
		int		nItemCountEstimate				= 250,		// This should fill most users' screens.
		int		nItemAllocationIncrement		= 50,
		UINT		NoItemMenuID						= 0,
		UINT		OneItemMenuID						= 0,
		UINT		MultiItemMenuID					= 0,
		bool		bAllowSorting						= true,
		bool		bShowHeader							= true
	);

	// Virtual destructors are required to bypass a known bug with proper 
	// constructor/destructor matching across DLL boundaries in the VC++ 
	// compiler.  Always provide one.  See MSDN Q122675, resolution #4.
	virtual ~MultiColumnSortListCtrl();

	// VERY IMPORTANT FUNCTION!
	// Must be called in OnInitDialog().
	// See top of file for details...
	virtual void Initialize();

// Attributes
public:

// Operations
public:

	// EXTERNAL DATA HANDLING

	// ================
	// COLUMN FUNCTIONS
	// ================

	// Insert columns into the list with this.  
	//
	// NOTES: (these may be pre-externaldata & out of date)
	//
	// 1) If we just use this class as-is, without deriving, we 
	//		need to have the parent call this for each initial
	//		column before calling Initialize().
	//
	// 2) If we derive, place InsertColumn() calls in 
	//		Initialize(), keeping your class much more 
	//		self-contained.  Make the initial InsertColumn() calls
	//		BEFORE calling the base class, to keep the
	//		registry in synch.
	//
	//	3) If you call this function after Initialize(), it is
	//		considered a DYNAMIC column add, and the registry will 
	//		be synched.
	//
	//	Note that to the caller, all column indexes are provided
	// as ABSOLUTES.  We convert them internally to the CListCtrl-
	// equivalent, considering that one or more columns may 
	// have been hidden.
	int InsertColumn(
		int		nColumn,
		LPCTSTR	szColumnName,
		int		nFormat						= LVCFMT_LEFT, 
		int		nWidth						= -1, 
		int		nSubItem						= -1
	);

	bool DeleteColumn( int nAbsCol );
	
	void RenameColumn(
		int		nColumn,
		LPCTSTR	szColumnName,
		int		nFormat						= LVCFMT_LEFT, 
		int		nWidth						= -1, 
		int		nSubItem						= -1
	);

	// ================


	////////////////////////////////////////////////////////////
	// Column hiding
	//
	// Columns are hidden by removing them from the CListCtrl
	// domain, storing the col info in member vars of this class.
	// The items' column data are kept in their original storage location;
	// we just service the requests for col strings intelligently,
	// accounting for hidden column status.  There are two basic
	// indexes into the column arrays: absolute, indexed according
	// to the order in which the columns were inserted on initialization,
	// prior to any hiding; and visible, which corresponds directly
	// to the indexes expected by CListCtrl.
	//
	// We can display a menu of columns, with checks indicating
	// whether each column is hidden or not.  We process requests
	// to hide/unhide columns through the command ID's on this
	// menu.  The command ID's that should be reserved for this
	// purpose are:
	//
	//		IDC_TOGGLE_COLUMN_HIDDEN_STATUS_BASE  =	28000 
	//
	//			through 
	//
	//		IDC_TOGGLE_COLUMN_HIDDEN_STATUS_LAST  =	28999
	//
	// These messages are routed through the message map handler.
	// See ON_COMMAND_RANGE in the map, and OnToggleColumnHiddenStatus().
	//
	Long GetVisibleCol( Long lAbsoluteIdx )
	{
		// Make sure the request is valid!
		ASSERT( lAbsoluteIdx < GetAbsoluteColCount() );
		ASSERT( lAbsoluteIdx >= 0 );

		return m_lvAbsToVisLookup[ lAbsoluteIdx ];
	}

	Long GetAbsoluteCol( Long lVisibleIdx )
	{
		// NOTE: You might hit one of these ASSERT's if
		// you forgot to call Initialize() for your list...
		ASSERT( lVisibleIdx >= 0 );
		ASSERT( m_lvVisToAbsLookup[ lVisibleIdx ] < GetAbsoluteColCount() );

		return m_lvVisToAbsLookup[ lVisibleIdx ];
	}

	// Provide the nth hidden col here.
	void ShowColumn( int nHidCol );

	// Provide the abs column here.
	void HideColumn( int nAbsCol );

	int GetAbsoluteColCount()
	{
		return GetVisibleColCount() + GetHiddenColCount();
	}

	int GetVisibleColCount()
	{
		return GetHeaderCtrl()->GetItemCount();
	}
	
	int GetHiddenColCount()
	{
		return m_vHiddenColData.size();
	}
	
	void DisplayColumnHidingMenu( CPoint& point );

	CString GetVisibleColName( int nVisCol );

	// This returns the nth hidden column's absolute index.
	// Good for looping through hidden columns.
	int GetHiddenCol( int nIndex )
	{
		return m_vHiddenColData[nIndex].nAbsColNumber;
	}

	int GetHiddenIndex( int nAbsCol )
	{
		int nIndex = 0;
		while ( 
				nIndex < GetHiddenColCount()
			&&	m_vHiddenColData[nIndex].nAbsColNumber != nAbsCol
		) 
			nIndex++;

		if ( nIndex == GetHiddenColCount() )
			nIndex = -1;

		return nIndex;
	}

	// MDM	2/25/2004 12:22:38 AM
	// Added to facilitate the ability to show/hide columns
	// externally to this class.
	CString GetHiddenColName( int nHiddenCol )
	{
		return m_vHiddenColData[nHiddenCol].strColHeading;
	}

	bool SetColumnName( int nAbsCol, CString& strNewColName );

protected:

	void AddColumnHidingMenu( 
		CMenu*	pPopupMenu, 
		int		nInsertionPoint	= 0
	);

	// OLD
	//----
	// This is a helper to keep hidden columns at the 
	// beginning of the column order after a col drop.
	// void VerifyHiddenOrder( int nCol );

	// This holds previous widths for hidden columns.
	// The hidden columns are stored at the beginning of
	// the column order.  This array holds the widths for
	// the columns at the beginning of the order array.
	// You will need to use the order array to find the 
	// actual column number.  For example, if you have
	// just hidden column 3, then it is the first column
	// in the order array, and m_vlColHiddenPrevWidths[0]
	// equals the width for column 3.
	// LongDeque	m_vlColHiddenPrevWidths;
	//----

	// (NEW versions)
	// COLUMN VARS
	// deque<HiddenColData>	m_vHiddenColData;		// Vector does not persist, but info is copied out
	std::vector<HiddenColData>	m_vHiddenColData;		// Vector does not persist, but info is copied out

	LongVector					m_lvAbsToVisLookup;
	LongVector					m_lvVisToAbsLookup;
	
	void RefreshAbsVisColLookup();

	// This adds default cached col data on column insertions.  
	// It always defaults to start ascending, and empty previous filtering.
	void AddDefaultColCachedData( int nCol );

	std::vector<ColCachedData>	m_vColCachedData;		// vector does not persist, but contains "cached" info

	//
	////////////////////////////////////////////////////////////


	// ==============
	// ITEM FUNCTIONS
	// ==============

public:
	// These are used to turn off painting during insertion
	// of several items.
	// They use reference counting so you can safely nest calls.
	void InsertStart()
	{
		if ( m_nInsertStartCount == 0 )
			SetRedraw( FALSE );
		m_nInsertStartCount++;
	}
	void InsertEnd()
	{
		if ( --m_nInsertStartCount == 0 )
		{
			// Re-enable drawing!
			SetRedraw();

			ResetAndRedraw();
		}

		// Don't call this too many times.
		ASSERT( m_nInsertStartCount >= 0 );
	}

	void ResetAndRedraw()
	{
		// Reset the count.
		if ( m_bUseExternalData )
			SetItemCountEx( m_Data.size() );
		
		// Force a repaint.
		Invalidate();
		UpdateWindow();
	}

	// Note: this function is optional but highly recommended.
	void EstimateItemCount( 
		int nTotalItemCount, 
		int nItemAllocationIncrement	= 50
	);
	
	// Note: this function is optional.  Call it if you can
	// afford processing time but want a smaller memory footprint.
	// It should also be used if your item count estimate was
	// potentially way off.  Be CAREFUL using it if you have
	// TONS of items in your list - an accurate precount for 
	// EstimateItemCount() is always the better option whenever
	// possible.  Also be careful if you have a very dynamic
	// list - if u don't leave enough extra room there will have to
	// be reallocations later.
	bool DoneInserting( int nAdditionalReserve = 0 );

	int InsertItem(
		int		nBeforeItem,
		LPCTSTR	szMainString,
		LPARAM	lParam			= 0
	);

	int InsertItem(
		UINT		nMask,
		int		nItem,
		LPCTSTR	lpszItem,
		UINT		nState		= 0,
		UINT		nStateMask	= 0,
		int		nImage		= 0,
		LPARAM	lParam		= 0
	);

	bool SetItemText( 
		int		nItem,
		int		nColumn,
		LPCTSTR	lpszString
	);
	CString GetItemText( int nItem, int nColumn );
	int GetItemText( int nItem, int nColumn, LPTSTR lpszText, int nLen );
	
	void SetItemData( int nItem, LPARAM lParam );
	LPARAM GetItemData( int nItem );

	int GetItemCount()
	{
		if ( m_bUseExternalData )
			return m_Data.size();
		else
			return inherited::GetItemCount();
	}

	// ==============


	// This turns the filter bar display on/off.
	void ShowFilterBar( bool bShow = true );

	/////////////////////
	// LPARAM HANDLING
	/////////////////////
	//

	// Use this to delete an item, as it will call
	// DeleteLPARAMData(), which is chained through
	// derived classes to properly clean up.
	bool DeleteItem( int nItem, bool bRedraw = true );
	
	// Deletes all items contained in the list.
	// This version handles any custom data allocations,
	// via the overridden DeleteLPARAMData() function.
	// ALWAYS USE THIS INSTEAD OF CListCtrl::DeleteAllItems()!
	void DeleteAllItems();
	
protected:

	// Helper function called by DeleteAll(), and in destructor.
	// You should never call this directly, or you'll end up
	// whacking your items' data but not the items.  Bad.
	void DeleteAllItemData();

	// LPARAM OVERRIDE!
	// This function handles deletion of our data
	// stored via the item lParam.
	// Derived classes should override if they need to
	// delete data allocated and pointed to by the lParam.
	// Note that once you override this function, there will be
	// no need to clean up your allocations when the list is
	// destroyed.  We do that for you in our destructor.
	virtual void DeleteLPARAMData( LPARAM lpData )
	{
	}

	// These two functions handle the extra data associated with an
	// item (attached through the lParam of ListItemData).  
	// Override in derived classes.  These versions just dump the
	// lParam as is.
	virtual void WriteLParam( 
		CArchive*		pArchive,
		LPARAM			lParam 
	) { pArchive->Write( &lParam, sizeof( LPARAM ) ); }
	
	virtual void ReadLParam(	
		CArchive*		pArchive,
		LPARAM*			plParam 
	) { pArchive->Read( plParam, sizeof( LPARAM ) ); }

	//
	/////////////////////
	

	////////////////////////////////////////////////////////
	// Right Click Menu
	//
	
public:

	bool DisplayRightClickMenu(
		CPoint&	point
	);

protected:

	UINT m_NoItemMenuID;
	UINT m_OneItemMenuID;
	UINT m_MultiItemMenuID;

	// This function allows check marks to be set on 
	// the popup menu.  Derived classes should override.
	virtual void PrepareMenu(
		CMenu*	pPopupMenu,
		UINT		uMenuID,
		int		nUnderCursor
	) { /* */ }

	//
	////////////////////////////////////////////////////////


public:

	// [Un]selects all items.
	void SelectAll();
	void UnselectAll();

	// This deletes the current selection.
	void DeleteSelection( bool bRehighlight = true );

	// Is there just 1 item selected?
	bool bSingleItemSelected();

	// For debugging...
	#ifdef _DEBUG
		void TracePrintSelection( LPCTSTR szComment );
		virtual void TraceState();
	#else
		void TracePrintSelection( LPCTSTR szComment ) {}
		void TraceState() {}
	#endif

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MultiColumnSortListCtrl)
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:

	void ResortAllItems();

	// Generated message map functions
	//{{AFX_MSG(MultiColumnSortListCtrl)
	afx_msg void OnDestroy();
	afx_msg void OnRightClickKey();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectAll();
	//}}AFX_MSG

   // TO DO
   // When using external data, THIS ONLY CONSIDERS ITEMS THAT ARE VISIBLE!  Ack!
	afx_msg void OnAutosizeColumns();
   afx_msg void OnAutosizeColumnsAndHeaders();

	// Moved here and made virtual.
	afx_msg virtual void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);

	// For filter notifications - ClassWizard seems filter-braindead
	// as of right now.
	afx_msg void OnFilterChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFilterButtonClicked(NMHDR* pNMHDR, LRESULT* pResult);

	// We post changes to column order.
	afx_msg LRESULT OnSaveColumnOrder(
		WPARAM uID,
		LPARAM lEvent
	);

	/*
	// We post changes to reset width to zero.
	afx_msg LRESULT OnSetZeroColWidth(
		WPARAM uID,
		LPARAM lEvent
	);
	*/

	// We handle requests to hide/unhide columns.  These requests come
	// from our pop-up menus.
	afx_msg void OnToggleColumnHiddenStatus( UINT nID );

	// We moved this out here to force it to be virtual.
	// WATCH THE SPELLING, we went with the Class Wizard default.
	// Note that this is a ON_NOTIFY_EX message, so that it gets passed
	// through to the parent as needed.
	afx_msg virtual BOOL OnItemchanged( NMHDR* pNMHDR, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()


// Operations
public:

	// This clears all previous sorting.
	void ClearSorting();

protected:

	// Override this function if you would like the default settings
	// on first-time use of the tree to include hidden or sorted columns.
	virtual void SetDefaultColumnSortingAndHiding() {}

	void AutoSizeColumn( int iColumn );
	

	////////////////////////////////////////////////////////////////
	// Registry storage of column sorting, order, width, visibility
	////////////////////////////////////////////////////////////////
	//
	// NOTES: 
	//
	// MDM	6/19/00 10:55:19 PM
	// This is being totally revamped to fully REMOVE hidden
	// columns.  We are dealing with lists with LARGE numbers
	// of columns, and the memory and processing to deal with
	// keeping hidden columns around is becoming too great.
	//
	// OLD:
	//	1) The column widths are obviously available from the
	//		list.  However, for hidden columns, we stuff them
	//		to the far left and set the widths to zero.  We 
	//		want to track the pre-hidden width for when the
	//		column is reshown.
	//
	//	2) The column order array is maintained by the 
	//		list itself.  Note that this order is maintained
	//		with the hidden columns first, since we stuff
	//		them to the left.
	//

	
	// REGISTRY STATE STORAGE
	
	// This is used as a prefix to the reg strings that
	// will hold all registry data for this list.  Make
	// sure it is unique across all lists in your project's
	// registry.  Set it in the constructor.
	CString		m_strUniqueName;

	// This flags when the settings have first been loaded.
	// Subsequent calls to InsertColumn() must update the
	// registry via ForceRegistrySave().
	bool m_bRegistrySettingsLoaded;

	// This reloads registry settings when the list is
	// first initialized.  If the current column count 
	// doesn't match the registry, or there are columns
	// in the data that are out of range, the registry is 
	// reset with default values.
	void	RegGetColumnSortOrderWidths();
	
	/*
	// Absolute column count = visible + hidden col count
	void SetAbsoluteColumnCount( Long lCount ) { m_lAbsoluteColCount = lCount; }
	Long GetAbsoluteColumnCount() { return m_lAbsoluteColCount; }
	*/

	// Whenever you DYNAMICALLY add or remove columns, this
	// is used to keep the registry in synch.
	void ForceRegistrySave();

	// The number of columns that were in the table when
	// the registry was last updated is maintained so that
	// when we reload from the registry, we can verify
	// that the data is valid for our list.
	int	RegGetTotalColumnCount();
	void  RegSetTotalColumnCount( int nColCount );

	// The number of sorted and hidden columns must be maintained,
	// as well.
	int	RegGetSortedColumnCount();
	int	RegGetHiddenColCount();
	void	RegSetHiddenColCount();

	// Virtual functions to save settings.
	// These are called automatically for you, and
	// only called as needed.  The default implementations
	// will take care of saving settings to the registry.
	// If you need to perform some additional task,
	// feel free to override, but don't forget to call the
	// base class.
	//
	// NOTE: See OnEndDrag/Track() for potential problems,
	// thanks again, MS.  Should be handled fine, though.
	virtual void SaveColumnWidths();
	virtual void SaveColumnOrder();
	virtual void SaveColumnSorting();

	//
	////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////
	// These set/reset the sorted state of the columns.
	//
	
	bool	bIsColumnSorted( int iItem );
	void	MoveSortedColumnToEnd( int iItem );
	void	UnsetColumnSorted( int iItem );
	void SetColumnSortState( int, SORT_STATE );

	// NOTE: Public access of these 3 is req'd for STOOOOPID sort static function.
public:

	SORT_STATE GetColumnSortState( int );

	int GetSortedColumnCount()
	{ return m_vlSortedColumns.size(); }

	// This returns the n'th sorted column.
	int GetSortedColumn( int n )
	{ return m_vlSortedColumns[n]; }

protected:	
	BitSet		m_bsColSortStates;
	LongVector	m_vlSortedColumns;
	
	//
	////////////////////////////////////////////////////////////

	
	////////////////////////////////////////////////////////////
	// These set/reset the numeric sort status of a column.
	// Numeric sorting handles numbers properly.
	//

public:
	const bool IsColumnNumeric( int nCol ) const;
	void SetColumnNumeric( int nCol );
	void UnsetColumnNumeric( int nCol );
	
protected:
	LongVector						m_vlNumericColumns;

	//
	////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////
	// These set the initial sort direction (upon first click).
	//

	// Columns default to ascending, call this to change.
public:
	void SetColumnStartSortDescending( int nCol );

protected:
	const bool bStartSortAscending( int ) const;

	//
	////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////
	// EXTERNAL DATA HANDLING
	//

	// Data storage
	ListItemDataVector			m_Data;
	ListItemDataVector			m_FilteredOutData;
	
	// Settings.
	bool								m_bUseExternalData;
	bool								m_bPreallocateItemData;
	int								m_nItemAllocationIncrement;
	
	// Reference counter for InsertStart() calls.
	int								m_nInsertStartCount;	

	//
	////////////////////////////////////////////////////////////


	bool SortColumn( int, bool = false );

	CString	GetHeaderFilterInfo( 
		int		nVisibleCol
	);

	// Re-add all previously filtered items.
	void ResetFilter();

	CImageList						m_ilImageList;
	
	CString							m_strColumnWidthSection;
	MultiColumnSortHeaderCtrl	m_ctlHeaderCtrl;	
	bool								m_bAutoSizeOnInsert;
	bool								m_bSorting;

	// Since this is a base class for many other list controls,
	// we may want to turn off some functionality.  These 
	// flags let the caller do so in the constructor.
	bool								m_bAllowSorting;
	bool								m_bShowHeader;

	// This tracks the last clicked column.  We only toggle
	// the sort state when a column is clicked twice in a row.
	int								m_nLastClickedColumn;

	
	////////////////////////////////////////////////////////////
	// FILTERBAR
	//
public:
	bool SetFocusToFilter( int nAbsCol );

	bool SetFilterText( int nAbsCol, LPTSTR szText );

	bool MakeColumnVisible( int nAbsCol, bool bForceShow = true );

	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////
	// COLUMN DESCRIPTIONS
	//

public:
	ustring GetColumnDescription( int nAbsCol )
	{ 
		return m_ctlHeaderCtrl.GetColumnDescription( nAbsCol ); 
	}
	void SetColumnDescription( int nAbsCol, ustring strDesc )
	{ 
		m_ctlHeaderCtrl.SetColumnDescription( nAbsCol, strDesc ); 
	}

	//
	////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////
	// CHECK BOXES
	// Should be pretty self-explanatory.  Pretty nice!

public:

	// Call this in OnInitDialog() after calling the base class version.
	void ShowCheckboxes()
	{
		SetExtendedStyle( 
				GetExtendedStyle()
			|	LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT
		);

		SetCallbackMask(0);
		ASSERT(GetCallbackMask() == 0);

		// CListControl cannot do checkboxes if we request to
		// manage our own string data.  Another score for M$!!
		ASSERT( !m_bUseExternalData );
	}

	// This function sets the item to checked.  Note that derived classes
	// can override to provide an action when a checkmark is added or removed.
	virtual void CheckItem( int nItem, bool bChecked = true )
	{
		SetItemState( nItem, LIST_ITEM_CHECKED( bChecked ), LVIS_STATEIMAGEMASK );
	}

	bool bIsItemChecked( int nItem )
	{
		return GetItemState( nItem, LVIS_STATEIMAGEMASK ) == LIST_ITEM_CHECKED( true );
	}

	//
	////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


//-------------------------------------------------------------------//
#endif	// RC_INVOKED
#endif // !defined(MULTI_COLUMN_SORT_LIST_CTRL_H)
//-------------------------------------------------------------------//
