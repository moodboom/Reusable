//-------------------------------------------------------------------//
// OleListCtrl.h : header file
//
// This class provides an OLE drag-and-drop-enabled list control.  
// It is derived from the MultiColumnSortListCtrl, so it has
// column sorting capability as well.  
// It is also derived from OleControl, which provides the basic d/d 
// functions that we override.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef OLE_LIST_CTRL_H
   #define OLE_LIST_CTRL_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define	IDC_LARGE_ICONS				7001
#define	IDC_MEDIUM_ICONS				7002
#define	IDC_SMALL_ICONS    			7003
#define	IDC_LIST_ICONS     			7004
#define	IDC_DETAILS_ICONS  			7005

// Accelerator table
#define	IDA_OLE_LIST_ACCELERATORS	7006
	#define	IDC_CUT						7007
	#define	IDC_COPY						7008
	#define	IDC_PASTE					7009
	// Also, IDC_RIGHT_CLICK_KEY (defined in base class)

// 
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


#include <afxtempl.h>																// For CArray
#include "..\MultiColumnSortListCtrl\MultiColumnSortListCtrl.h"		// Base class
#include "OleControl.h"																// Base class


//-------------------------------------------------------------------//
// Globals Statics Constants														//
//-------------------------------------------------------------------//

typedef enum
{
	PASTE_AT_SELECTION,
	PASTE_TOP,
	PASTE_BOTTOM,
	PASTE_REPLACE_ALL,

	PASTE_MODE_COUNT,

} PASTEMODE;


// Clipboard formats can either be registered for public use, or
// we can use application-private CF's instead.  
// Private CF's have values between CF_PRIVATEFIRST
// and CF_PRIVATELAST.  They must be managed so
// they do not overlap.

// Note that any time we create an OLE server, the associated 
// CF's should be registered publicly, using resource strings.

// I am adding a public CF, since I am having trouble with
// the private CF on the (public?) clipboard.
const TCHAR cszOleDragData[] = _T("MDM_OleDragData");

typedef enum {

	CF_OLE_DRAG_DATA				= ( CF_PRIVATEFIRST + 1 ),
	// CF_XXX,

} APP_CFs;

// Note: Make sure that the last CF does not go past our allowed 
// range (not necessarily easy to detect!).


//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// class OleDragItem																	//
//-------------------------------------------------------------------//
// This class designates the contents of the data maintained for 
// each item being dragged.  We maintain an array of these during 
// drags and drops.
//-------------------------------------------------------------------//
class OleDragItem {

public:

	// Constructors
	OleDragItem()
	:
		// Init vars.
		m_lParam( 0 )
	{}

	OleDragItem(
		LPARAM lParam
	) :
		// Init vars.
		m_lParam( lParam )
	{}

	OleDragItem(
		CString&	strName,
		LPARAM	lParam,
		bool		bChecked
	) :
		m_strName	( strName	),
		m_lParam		( lParam		),
		m_bChecked	( bChecked	)
	{}

	// Data
	// Allow direct access, since we need to set and get these.
	CString	m_strName;
	LPARAM	m_lParam;
	bool		m_bChecked;

};


//-------------------------------------------------------------------//
// class OleListCtrl																	//
//-------------------------------------------------------------------//
// OleListCtrl window
//-------------------------------------------------------------------//
class OleListCtrl 
	: 
		public MultiColumnSortListCtrl,
		public OleControl
{
	typedef MultiColumnSortListCtrl inherited;

// Construction
public:

	// All initialization that is not dependent on the HWND being
	// available should be done here.
   OleListCtrl(
		CString	strUniqueRegistryName,
		bool		bUseExternalData			= false,
		UINT		NoItemMenuID				= 0,
		UINT		OneItemMenuID				= 0,
		UINT		MultiItemMenuID			= 0,
		int		nNewDropXMargin			= 8
	);

	// Virtual destructors are required to bypass a known bug with proper 
	// constructor/destructor matching across DLL boundaries in the VC++ 
	// compiler.  Always provide one.  See MSDN Q122675, resolution #4.
   virtual ~OleListCtrl();

// Attributes
public:

// Operations
public:

	// Similar to the tree control situation, we were not able to find a 
	// built-in init function, so we were forced to add one of our own, 
	// which must be called manually by the object containing this one.
	// This function sets up the image list.  Note that we can compile 
	// with this code in PreSubclassWindow(), but the images are not 
	// loaded.  Therefore, we need an Initialize() function.
	virtual void Initialize();

// Overrides
	// ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(OleListCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:

	////////////////////////////////////////////////////////
	// Drag/drop functionality.
	//
	// This function takes a drag over request, determines if we are over
	// an item, and passes the request to WillAcceptDropOnTree.
	// Accessed by DBListDropTarget.
	virtual DROPEFFECT OnDragOver(
		CWnd* pWnd,
		COleDataObject* pDataObject,
		DWORD dwKeyState,
		CPoint point
	);

	// This function prepares a drop request and passes it on
	// to AcceptDrop.  Accessed by DBListDropTarget.
	virtual BOOL OnDrop(
		CWnd* pWnd,
		COleDataObject* pDataObject,
		DROPEFFECT dropEffect,
		CPoint point
	);

	// This function archives the data in the list control, in
	// order to save it to or restore it from a file.
	void Archive(
		CArchive& ar
	);

	// This function is called following an archive read.  The archive
	// read obtains the archive data.  Then, Create() and Initialize()
	// are called.  Finally, this function is called to fill the list
	// with the data extracted in Archive().
	void FillFromArchive();
	
	// These functions handle cut, copy and paste operations.
	// They do what you would expect, using the same archiving
	// used by drag and drop.
	// Note that you MAY override these if you need to take an
	// action on these events, but you will have more control
	// by overriding the AcceptDrop functions below.
	virtual void Cut();
	virtual void Copy();
	virtual void Paste( 	
		PASTEMODE ePasteMode 
	);

protected:

	// Does the actual drop accept.
	virtual BOOL AcceptDrop(
		int nUnderCursor,
		bool bInsertAfter,
		UINT nHitFlags,
		CPoint DropPoint,
		COleDataObject* pDataObject,
		DROPEFFECT dropEffect,
		bool bFullDropFormat
	);

	// Does the actual drop test work.
	virtual DROPEFFECT WillAcceptDrop(
		int nUnderCursor,
		bool bInsertAfter,
		UINT nHitFlags,
		COleDataObject* pDataObject,
		DWORD dwKeyState,
		bool bFullDropFormat
	);

	// Allows derived classes to do any special handling as a
	// result of a left click upon a list control item.
	virtual void OnLButtonDownOnListItem(
		int	nUnderCursor,
		UINT	nKeyFlags
	) { }

	// This function attempts a drag when the left button has been
	// pressed on an item.  It returns true if a drag was made.
	virtual BOOL BeginDrag(
		int	nUnderCursor,
		UINT	nKeyFlags
	);

	// Derived classes should perform more intense
	// data validation that is impractical during
	// WillAcceptDrop() here.
	virtual void ProcessNewDragData( 
		DROPEFFECT		DropEffect
	) { }

	// Dump the current selection into drag data array.
	void ExtractItemsToDragData( 
		bool bSelectedOnly = false,
		int nUnderCursor = -1
	);

	// Does the actual work of ripping items from the drag data and
	// adding them to the list control.
	virtual bool AddItemsFromDragData( 
		int				nUnderCursor	= 0,
		bool				bInsertAfter	= false,
		CPoint			DropPoint		= CPoint( 0, 0 )
	);

	// Override this to do any additional processing
	// upon insertion of a dropped item.
	virtual void OnAddDroppedItem( int nItem ) {}

	// Do we have valid data on the clipboard?
	// Useful in OnUpdatePaste()-type handlers.
	// Override if you are doing something really
	// weird and hacked-up ( seriously, if
	// you are handling more than one type of 
	// CLIPFORMAT).
	virtual bool bHaveValidDropData(
		COleDataObject*	pDropData
	);

	void DrawInsertionLine( int nItemAboveLine );

	// Successful drag and drop operations - allow derived
	// classes to do any additional clean up steps.
	virtual void FinishDragAndDrop() {}

	friend class DBListDropTarget;

	int	m_nLastUnderCursor;
	bool	m_bDeselectOnNoDrag;
	bool	m_bDrawInsertionLine;
	int	m_nItemAboveLine;

	
	////////////////////////////////////////////////
	// OleDragItem manipulators
	////////////////////////////////////////////////
	
	// This group of functions and members are
	// the guts of the data handling.

	CArray < OleDragItem, OleDragItem& > Items;

	// Extract data from the clipboard to our Items array.
	void ExtractDropData(
		COleDataObject*	pDropData
	);

	// Put data in our Items array to the clipboard.
	void ArchiveAndRenderClipFormats(
		COleDataSource* pSource
	);
		
	//
	////////////////////////////////////////////////

	//
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// Image list

public:

	// This is called during palette changes to reload images.
	void RefreshImages();

protected:

	// This default version does nothing.
	// Derived versions can load up the image lists.
	void AddImages() {};

	CImageList ObjectImages;
	CImageList SmallObjectImages;
	//
	////////////////////////////////////////////////////////

	// These help us track the current drag operation.
	int	nListItemBeingDragged;
	bool	bDraggingFromThisList;

	int	nDropXMargin;
	bool	bAutoArrange;

protected:

	// Override in derived classes as needed.  For example,
	// This could update the caption of our frame, or, if we are
	// within a doc-view architecture, update the doc "title" and
	// icon and mark the doc dirty if requested.
	virtual void UpdateAfterDrop( int nFirstInsertedItem = 0, bool bMarkAsDirty = false );

   // List accelerators
	HACCEL m_hAccel;

	// MDM 10/14/2005 back...
	// Let's track dropped items so we can highlight at the 
	// far side of a "move" after we delete the source items.
	void DeleteOldAndHighlightNew();
	std::vector<int> m_vnNewlyDroppedItems;

	// Generated message map functions
protected:
   //{{AFX_MSG(OleListCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCopy();
	afx_msg void OnCut();
	afx_msg void OnPaste();
	afx_msg void OnUpdatePaste(CCmdUI* pCmdUI);
	//}}AFX_MSG

	afx_msg void OnListStyle( UINT nID );

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.


//-------------------------------------------------------------------//
#endif	// RC_INVOKED
#endif	// OLE_LIST_CTRL_H
//-------------------------------------------------------------------//
