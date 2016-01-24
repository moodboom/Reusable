//-------------------------------------------------------------------//
// OleListCtrl.cpp : implementation file
//
// This class provides an OLE drag-and-drop-enabled list control.  
// It is designed to be a base class that allows fast development of
// user-defined list controls.
//
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include <afxadv.h>								// For archive stuff

#include "..\..\CStringHelpers.h"			// For ResStr()

// #include "..\ColorBitmap.h"				// For image lists (not yet implemented)

#include "OleListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-------------------------------------------------------------------//
// Globals Statics Constants														//
//-------------------------------------------------------------------//

const TCHAR ctColumnSeparator = _T('\013');

//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// OleListCtrl()																	//
//-------------------------------------------------------------------//
// All initialization that is not dependent on the HWND being
// available should be done here.  Initialization that is dependent
// upon the creation of the window MAY be able to use PreSubclassWindow(),
// but most likely will have to be placed in an initialization function
// called externally after Create().  This same problem applies to other
// common controls, with the exception of CPropertySheet, for which
// InitDialog() can be used to init internally after HWND creation.
//-------------------------------------------------------------------//
OleListCtrl::OleListCtrl(
	CString		strUniqueRegistryName,
	bool			bUseExternalData		,
	UINT			NoItemMenuID			,
	UINT			OneItemMenuID			,
	UINT			MultiItemMenuID		,
	int			nNewDropXMargin		
) :
	
	// Call base class.
	inherited(
		strUniqueRegistryName	,
		bUseExternalData			,
		250							,	// TO DO
		50								,	// Make these two into params...
		NoItemMenuID				,
		OneItemMenuID				,
		MultiItemMenuID	
	),

	// Init vars.
	nDropXMargin			( nNewDropXMargin ),
	bAutoArrange			( false				),
	m_hAccel					( 0					),
	m_nLastUnderCursor	( -1					),
	m_bDeselectOnNoDrag	( false				),
	bDraggingFromThisList( false				),
	m_bDrawInsertionLine	( false				),
	m_nItemAboveLine		( -1					)
{

	// Register our own unique clipboard format.
	// ClipFormat = CF_OLE_DRAG_DATA;
	ClipFormat = RegisterClipboardFormat( cszOleDragData );
	
}

//-------------------------------------------------------------------//
// ~OleListCtrl()																	//
//-------------------------------------------------------------------//
OleListCtrl::~OleListCtrl()
{
}


BEGIN_MESSAGE_MAP(OleListCtrl, OleListCtrl::inherited)
   //{{AFX_MSG_MAP(OleListCtrl)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	ON_COMMAND(IDC_COPY, OnCopy)
	ON_COMMAND(IDC_CUT, OnCut)
	ON_COMMAND(IDC_PASTE, OnPaste)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(IDC_PASTE, OnUpdatePaste)
	//}}AFX_MSG_MAP

	ON_COMMAND_RANGE( IDC_LARGE_ICONS, IDC_DETAILS_ICONS, OnListStyle)

END_MESSAGE_MAP()

//-------------------------------------------------------------------//
// PreSubclassWindow()																//
//-------------------------------------------------------------------//
void OleListCtrl::PreSubclassWindow()
{

	// Register the drop target now.
	// This is an OleControl function.
	Register( this );

	// Call the base class.
	inherited::PreSubclassWindow();

}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// Similar to the tree control situation, we were not able to find a 
// built-in init function, so we were forced to add one of our own, 
// which must be called manually by the object containing this one.
// This function sets up the image list.  Note that we can compile 
// with this code in PreSubclassWindow(), but the images are not 
// loaded.  Therefore, we need an Initialize() function.
//-------------------------------------------------------------------//
void OleListCtrl::Initialize() 
{

	// Call the base class to set up the correct styles.
	inherited::Initialize();

	// Add images.
	AddImages();

	// TO DO
	// If you want to be able to do this, you should add dwInitialDisplayStyle
	// as a constructor param, so we can still use a generic Initialize()
	// function that carries through base and derived classes.
	/*
	// Set the requested style type.
	if ( dwInitialDisplayStyle != 0 )
	{
		ModifyStyle(
			LVS_SMALLICON | LVS_REPORT | LVS_ICON | LVS_LIST | LVS_SHOWSELALWAYS,
			dwInitialDisplayStyle
		);
	}
	*/
	
   // Load accelerators.  See MSJ July 97 C++ Q&A.  Thanks again Paul.
	m_hAccel = ::LoadAccelerators(
		AfxGetInstanceHandle(),
      MAKEINTRESOURCE(
			IDA_OLE_LIST_ACCELERATORS
		)
	);
	ASSERT( m_hAccel );

	UpdateAfterDrop();

}


// This function is an example of preloading images.
/*
//-------------------------------------------------------------------//
// AddImages()																			//
//-------------------------------------------------------------------//
// This default version loads up an image for each object type.
//-------------------------------------------------------------------//
void OleListCtrl::AddImages()
{

	/////////////////////////////////////
	// Add Large Icon images
	// 

	// Create a full-color image list.
	ObjectImages.Create( 48, 48, ILC_COLORDDB, 3, 1 );
	
	// Add images.
	for ( int i = 0; i < cnImageCount; i++ ) {
		ColorBitmap SingleImage(
			uiIconID[ i ],
			false					
		);
		ASSERT( SingleImage.m_hObject != NULL );
		ObjectImages.Add( 
			&SingleImage, 
			
			// Color in bitmap to make transparent = black
			RGB( 0, 0, 0 ) 	

		);
	}

	// Set the images into the list control.
	SetImageList( &ObjectImages, LVSIL_NORMAL );

	//
	//////////////////////////////////////


	////////////////////////////////////////////
	//  Add Small Icon images, in same manner
	//

	//
	//////////////////////////////////////


}
*/


//-------------------------------------------------------------------//
// RefreshImages()																	//
//-------------------------------------------------------------------//
// This is called during palette changes to reload images.
//-------------------------------------------------------------------//
void OleListCtrl::RefreshImages()
{

	RemoveImageList( LVSIL_NORMAL );
	ObjectImages.DeleteImageList();
	RemoveImageList( LVSIL_SMALL );
	SmallObjectImages.DeleteImageList();
	AddImages();
	Invalidate();

}


/////////////////////////////////
// Drag and drop functionality.
//

//-------------------------------------------------------------------//
// OnDragOver()																		//
//-------------------------------------------------------------------//
// This function takes a drag over request, determines if we are over
// an item, and passes the request to WillAcceptDrop.
//-------------------------------------------------------------------//
DROPEFFECT OleListCtrl::OnDragOver(
	CWnd* pWnd,
	COleDataObject* pDataObject,
	DWORD dwKeyState,
	CPoint point
) {

	DROPEFFECT deReturn = DROPEFFECT_NONE;
	bool bInsertAfter = false;
	int nUnderCursor = 0;

	UINT nHitFlags;
	if ( pDataObject->IsDataAvailable( ClipFormat ) ) 
	{
		// Get item under cursor.
		nUnderCursor = HitTest( point, &nHitFlags );

		// Adjust based on where point is.
		// If we're in the top half of the item, insert before.
		CRect ItemRect;
		GetItemRect( nUnderCursor, &ItemRect, FALSE );
		ItemRect.DeflateRect( 0, 0, 0, ItemRect.Height()/2 );
		bInsertAfter = ( ItemRect.PtInRect( point ) == FALSE );

		deReturn = WillAcceptDrop(
			nUnderCursor,
			bInsertAfter,
			nHitFlags,
			pDataObject,
			dwKeyState,
			true
		);

	// We don't currently handle text drops.
	// } else if ( pDataObject->IsDataAvailable( CF_TEXT ) ) {

	}

	// TO DO
	// Note that this next section is tailored to lists in 
	// REPORT mode.  Other modes may still need further work.

	// Convert to always insert AFTER nUnderCursor.
	// Note that first we adjust nUnderCursor for drags BELOW 
	// the bottom item.
	if ( nHitFlags & LVHT_NOWHERE )
	{
		nUnderCursor = GetItemCount() - 1;
	}
	else if ( !bInsertAfter ) 
		nUnderCursor--;
	
	// Indicate where the drop would be inserted.
	if ( 
			(
					deReturn == DROPEFFECT_COPY
				|| deReturn == DROPEFFECT_MOVE
				|| deReturn == DROPEFFECT_LINK
			)
		
		&& nUnderCursor > -1
	) 
		DrawInsertionLine( nUnderCursor );
	
	else
	{
		if ( m_nItemAboveLine > -1 )
		{
			// Erase the old line.
			RedrawItems( m_nItemAboveLine, m_nItemAboveLine );
			UpdateWindow();
			m_nItemAboveLine = -1;
		}
	}

	return deReturn;

}


//-------------------------------------------------------------------//
// DrawInsertionLine()																//
//-------------------------------------------------------------------//
// Here, we draw an insertion line UNDER the specified item.
//
// Actually, all the redrawing is expected to be done in a derived
// class.  Here, we merely set m_bDrawInsertionLine to true, and
// force a redraw of the target item.  If you want this to have
// any effect, you'll need to override OnDrawItem() in a derived
// class (Cheat: see HangTheDJ::SongList.cpp for an example).
//-------------------------------------------------------------------//
void OleListCtrl::DrawInsertionLine( int nItemAboveLine )
{
	// If no change, just return.
	if ( m_nItemAboveLine == nItemAboveLine )
		return;

	// Erase the old line.
	if ( m_nItemAboveLine > -1 )
	{
		RedrawItems( m_nItemAboveLine, m_nItemAboveLine );
		UpdateWindow();
	}

	// Draw the new line.
	m_bDrawInsertionLine = true;
	RedrawItems( nItemAboveLine, nItemAboveLine );
	UpdateWindow();
	m_bDrawInsertionLine = false;

	// Save the item we drew the line under.
	m_nItemAboveLine = nItemAboveLine;

}


//-------------------------------------------------------------------//
// WillAcceptDrop()																	//
//-------------------------------------------------------------------//
// Does the actual drop test work.
// This version tests the basic acceptance requirements (in this
// case, that means making sure we are not over the item that 
// started the drag), and then applying any keys pressed by the
// user to the drop effect.
// Derived classes should override this function if they have any
// requirement to test the drop data for validity, etc.  The
// derived function version can call this base version, and then
// do its testing.
//-------------------------------------------------------------------//
DROPEFFECT OleListCtrl::WillAcceptDrop(
	int nUnderCursor,
	bool bInsertAfter,
	UINT nHitFlags,
	COleDataObject* pDataObject,
	DWORD dwKeyState,
	bool bFullDropFormat
) {

	DROPEFFECT deReturn = DROPEFFECT_NONE;

	// Test for basic requirements first.
	if ( bDraggingFromThisList ) 
	{
		if ( nUnderCursor != nListItemBeingDragged ) 
		{
			// This is the default within-window operation.
			// Note that we only allow operations if we are
			// not on top of the item being dragged.
			deReturn = DROPEFFECT_MOVE;

		}
	} else 
	{
		// This is the default cross-window operation.
		// deReturn = DROPEFFECT_COPY;
		deReturn = DROPEFFECT_MOVE;
	}

	// If we have a legit operation, allow the user
	// to override it with the keys.
	if ( deReturn != DROPEFFECT_NONE && dwKeyState ) 
	{
		// Link, then copy, then move has priority.
		if ((dwKeyState & (MK_CONTROL|MK_SHIFT)) == (MK_CONTROL|MK_SHIFT))
			deReturn = DROPEFFECT_LINK;
		else if ((dwKeyState & MK_CONTROL) == MK_CONTROL)
			deReturn = DROPEFFECT_COPY;
		else if ((dwKeyState & MK_SHIFT) == MK_SHIFT)
			deReturn = DROPEFFECT_MOVE;
	}

	return deReturn;

}


//-------------------------------------------------------------------//
// OnDrop()																				//
//-------------------------------------------------------------------//
// This function prepares a drop request and passes it on
// to AcceptDrop.
//-------------------------------------------------------------------//
BOOL OleListCtrl::OnDrop(
	CWnd* pWnd,
	COleDataObject* pDataObject,
	DROPEFFECT dropEffect,
	CPoint point
) {

	// Erase the last drop line.
	RedrawItems( m_nItemAboveLine, m_nItemAboveLine );
	UpdateWindow();

	BOOL bReturn = FALSE;

	// Get item under cursor.
	UINT nHitFlags;
	int nUnderCursor = HitTest( point, &nHitFlags );

	// Adjust based on where point is.
	// If we're in the top half of the item, insert before.
	CRect ItemRect;
	GetItemRect( nUnderCursor, &ItemRect, FALSE );
	ItemRect.DeflateRect( 0, 0, 0, ItemRect.Height()/2 );
	bool bInsertAfter = ( ItemRect.PtInRect( point ) == FALSE );

	// Now adjust based on being BELOW all items.
	// Note: this is biased toward REPORT mode.
	if ( nHitFlags & LVHT_NOWHERE )
	{
		nUnderCursor = GetItemCount() - 1;
		bInsertAfter = true;
	}
	
	// Handle all supplied variants of OleDragData with AcceptDrop.
	if ( pDataObject->IsDataAvailable( ClipFormat ) ) {

		bReturn = AcceptDrop(
			nUnderCursor,
			bInsertAfter,
			nHitFlags,
			point,
			pDataObject,
			dropEffect,
			true
		);

	// We don't currently handle text drops.
	// } else if ( pDataObject->IsDataAvailable( CF_TEXT ) ) {

	}

	// Default is to return false if no CF found.
	return bReturn;

}


//-------------------------------------------------------------------//
// AcceptDrop()																		//
//-------------------------------------------------------------------//
// Does the actual drop accept.
//-------------------------------------------------------------------//
BOOL OleListCtrl::AcceptDrop(
	int nUnderCursor,
	bool bInsertAfter,
	UINT nHitFlags,
	CPoint DropPoint,
	COleDataObject* pDataObject,
	DROPEFFECT dropEffect,
	bool bFullDropFormat
) {

	// There isn't currently any condition that we should
	// return FALSE.  We must return TRUE for the drop
	// effect to get properly passed back up the chain to us.
	BOOL bReturn = TRUE;

	// Unarchive the data into the drag data array.
	ExtractDropData( 
		pDataObject
	);

	// This allows us to manipulate the drag data 
	// before we add it as new list items.
	ProcessNewDragData( 
		dropEffect
	);

	// If we don't actually add anything, return FALSE, and 
	// the drag source won't process the drop effect.
	bReturn = AddItemsFromDragData( 
		nUnderCursor,
		bInsertAfter
	);

	// If we added anything...
	if ( bReturn )
	{
		// This updates the items displayed after the drop.
		// It can be ovverridden to e.g. update the caption and icon, 
		// mark the list "dirty", etc.
		UpdateAfterDrop( nUnderCursor + ( bInsertAfter? 1 : 0 ), true );
	
		// Turn off sorting, now that we have stuffed new items
		// into specific locations.
		ClearSorting();
		
		// Save the change, too.
		// Well, we WOULD, but this is screwing up
		// the combo box, as it calls Playlist::UpdateCB()
		// while we are in the middle of shifting items
		// around.  This has been moved to 
		// PlayList::FinishDragAndDrop().
		// SaveColumnSorting();

	}
	
	// Clear the drag contents, now that we have added them.
	Items.RemoveAll();

	return bReturn;

}


//-------------------------------------------------------------------//
// AddItemsFromDragData()															//
//-------------------------------------------------------------------//
// Does the actual work of ripping items from the drag data and
// adding them to the list control.
//-------------------------------------------------------------------//
bool OleListCtrl::AddItemsFromDragData( 
	int				nUnderCursor,
	bool				bInsertAfter,
	CPoint			DropPoint
) {
	
	InsertStart();

	// Loop variables.
	CRect ItemRect;
	bool bFirstItem = true;

	// If nUnderCursor is not valid, we should be at the bottom of
	// the list.
	if ( nUnderCursor < 0 )
		nUnderCursor = GetItemCount();

	// Adjust nUnderCursor to always be before.
	if ( bInsertAfter )
		nUnderCursor++;

	// Add the items to the list.
	int i;
	for ( i = 0; i < Items.GetSize(); i++ ) {

		// Add it to the list.
		
		// The string contains all column text.  Break it
		// back down.
		CString* pstrFull = &Items[i].m_strName;
		int nStartPos = pstrFull->Find( ctColumnSeparator );
		CString strText = pstrFull->Left( nStartPos );

		// We need to include the text and
		// the data stored in the lParam.
		int nNewIndex = InsertItem(
			LVIF_TEXT | LVIF_PARAM,		//	UINT nMask,
			nUnderCursor + i,				//	int nItem,
			LPCTSTR( strText ),			//	LPCTSTR lpszItem,
			0,									//	UINT nState,
			0,									//	UINT nStateMask,
			0,									//	int nImage,
			Items[i].m_lParam				//	LPARAM lParam
		);
		ASSERT( nNewIndex == nUnderCursor + i );

		// Set the checked state.
		CheckItem( nNewIndex, Items[i].m_bChecked );

	   // Mark all the dropped items as selected.
		// WARNING: Don't try to do this until later, so we don't 
		// end up adding to the original moved selection that is
		// to be deleted.
		m_vnNewlyDroppedItems.push_back( nNewIndex );
		
		// Set the text of the subitems, if any.
		int nStopPos;
		int nCol;
		for ( nCol = 1; nCol < GetAbsoluteColCount(); nCol++ )
		{
			// Get the next stop position.
			nStopPos = pstrFull->Find( ctColumnSeparator, nStartPos + 1 );

			// Extract the col string.
			if ( nStopPos == -1 )
				strText = pstrFull->Mid( nStartPos + 1 );
			else
				strText = pstrFull->Mid( nStartPos + 1, nStopPos - nStartPos - 1 );

			// Do the deed.
			SetItemText( nUnderCursor + i, nCol, strText );

			// Set up for next col.
			nStartPos = nStopPos;
		}

		// Set the position of the item based on the drop
		// point and the number in the RefItems array.
		// The first item needs a different calc.  We didn't do it
		// outside the loop because it was easier to get the size
		// of the item bitmap here.
		if ( bFirstItem ) 
		{
			// Calculate the starting drop point.
			bFirstItem = false;
			GetItemRect( 0, &ItemRect, LVIR_ICON );
			DropPoint.x -= ( ItemRect.Width() / 2 );
			DropPoint.y -= ( ItemRect.Height() / 2 );

		} else {

			// This aligns multiple items out to the right.
			DropPoint.x += ItemRect.Width() + nDropXMargin;

		}

		// Now set it, if available.
		if ( (GetStyle() & LVS_OWNERDATA)==0 )
			SetItemPosition( nUnderCursor + i, DropPoint );

		// Provide a post-insert-item virtual function.
		OnAddDroppedItem( nUnderCursor + i );

	}

	InsertEnd();

	// We want to return true only if we processed items.
	// bFirstItem can tell us this.
	return !bFirstItem;

}

//
////////////////////////////////


//////////////////////////////////
// Cut, Copy and Paste operations
//////////////////////////////////
// These do what you would expect, via the OLE archiving mechanism.
//


//-------------------------------------------------------------------//
// Cut()																					//
//-------------------------------------------------------------------//
void OleListCtrl::Cut()
{

	Copy();

	// Now delete the current selection.
	DeleteSelection( false );

}


//-------------------------------------------------------------------//
// Copy()																				//
//-------------------------------------------------------------------//
void OleListCtrl::Copy()
{
	// We provide a ClipFormat, so make sure ClipFormat is ready to go.
	ASSERT( ClipFormat != NULL );

	// Dump the current selection into drag data array.
	ExtractItemsToDragData( 
		true,
		-1				/// nUnderCursor
	);

	// To drag, we need a COleDataSource that is loaded with our data.
	// Get a new one.
	// From "Using the OLE Clipboard Mechanism" in MSDN:
	// Create an OLE data source on the heap
	// COleDataSource DataSource;
	COleDataSource* pSource = new COleDataSource;

	// Put the drag data into the DS.
	// This renders to the clipboard.
	ArchiveAndRenderClipFormats( pSource );

	// The Clipboard now owns the allocated memory
	// and will delete this data object
	// when new data is put on the Clipboard
	pSource->SetClipboard();

	// Clear the drag contents, now that we have copied them to the clipboard.
	Items.RemoveAll();

}


//-------------------------------------------------------------------//
// Paste()																				//
//-------------------------------------------------------------------//
void OleListCtrl::Paste( 
	PASTEMODE ePasteMode 
) {
	COleDataObject DataObject;

	if ( DataObject.AttachClipboard() )
	{
		// Unarchive the data into the DragData array.
		ExtractDropData( 
			&DataObject
		);

		// This allows us to manipulate the drag data 
		// before we add it as new list items.
		ProcessNewDragData( 
			DROPEFFECT_MOVE	// dropEffect
		);

		// PASTE_REPLACE_ALL, PASTE_TOP default.
		int nUnderCursor = 0;

		switch( ePasteMode )
		{
			case PASTE_BOTTOM:
				nUnderCursor = GetItemCount();
				break;
			case PASTE_REPLACE_ALL:
				DeleteAllItems();
				break;
			case PASTE_AT_SELECTION:
				nUnderCursor = GetNextItem( -1, LVNI_FOCUSED );
				if ( nUnderCursor == -1 )
					nUnderCursor = GetItemCount();
				break;
			default:
				break;
		}

		if (
			AddItemsFromDragData( 
				nUnderCursor,
				false				// bInsertAfter
			)
		) {
			// If we added anything we need to redraw.  This may also
			// e.g. update the caption and icon and mark the list "dirty"
			// in derived classes.
			UpdateAfterDrop( nUnderCursor, true );
		}
		
		// Clear the drag contents, now that we have added them.
		// Don't worry, the clipboard hasn't changed, in case we
		// wanted to add the items again!  :>
		Items.RemoveAll();

	}
}


//-------------------------------------------------------------------//
// UpdateAfterDrop()																	//
//-------------------------------------------------------------------//
// Here, we redraw after a drop.  Derived classes can do more, e.g.,
// mark a document as dirty, change a caption, whatever.
//-------------------------------------------------------------------//
void OleListCtrl::UpdateAfterDrop( int nFirstInsertedItem, bool bMarkAsDirty )
{
	if ( bMarkAsDirty )
	{
		// Now that we are handling our own data,
		// we need to force a redraw on drop, as
		// all the item placements may have changed.
		RedrawItems( nFirstInsertedItem, GetItemCount() - 1 );
		UpdateWindow();
	}
}


//-------------------------------------------------------------------//
// OnCopy|Cut|Paste()																//
//-------------------------------------------------------------------//
// Message handlers for cut/copy/paste are built into the base
// class.  All you have to do is allow them in the list control's
// right-click menu.  Use IDC_CUT, IDC_COPY, and IDC_PASTE.
// We also handle keyboard accelerators, as defined in
// IDA_OLE_LIST_ACCELERATORS.
//-------------------------------------------------------------------//
void OleListCtrl::OnCopy() 
{
	Copy();
}

void OleListCtrl::OnCut() 
{
	Cut();
}

void OleListCtrl::OnPaste() 
{
	Paste( PASTE_AT_SELECTION );
}


//-------------------------------------------------------------------//
// OnUpdatePaste()																	//
//-------------------------------------------------------------------//
// Disable paste if nothing's available.
// Nice of MultiColumnSortListCtrl::DisplayRightClickMenu() to
// force checks for ON_UPDATE_COMMAND_UI's.
//-------------------------------------------------------------------//
void OleListCtrl::OnUpdatePaste(CCmdUI* pCmdUI) 
{
	bool bHaveData = false;

	COleDataObject DataObject;
	if ( DataObject.AttachClipboard() )
	{
		// Do we have OLE data in the DragData array?
		bHaveData = bHaveValidDropData( &DataObject );
	}

	pCmdUI->Enable( bHaveData );
}


//-------------------------------------------------------------------//
// PreTranslateMessage()															//
//-------------------------------------------------------------------//
// List control accelerators are handled here.  Cool!
//-------------------------------------------------------------------//
BOOL OleListCtrl::PreTranslateMessage( MSG* pMsg )
{
	// We use MS's instructions for translating accelerators.  The
	// instructions in MSJ July 97 C++ Q&A were not as complete.
	// First, attempt to handle this msg via our accelerators.
	if ( !( m_hAccel && ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) ) )
		
		// If we weren't able to translate the msg via our 
		// accelerators, send it on.
		return OleListCtrl::inherited::PreTranslateMessage( pMsg );
	
	else
	
		// We handled it, return TRUE.
		return TRUE;
}


//
//////////////////////////////////



//-------------------------------------------------------------------//
// OnListStyle()																		//
//-------------------------------------------------------------------//
// This function manages the setting of the list style on the fly 
// by the user.
// The following commands are handled by this function.  Make sure they
// are contiguous, and not shared with any other commands.  See the
// note about required resources at the top of the header file.
//
//		IDC_LARGE_ICONS    
//		IDC_MEDIUM_ICONS   
//		IDC_SMALL_ICONS    
//		IDC_LIST_ICONS     
//		IDC_DETAILS_ICONS  
//
//-------------------------------------------------------------------//
void OleListCtrl::OnListStyle( UINT nID ) 
{

	int nNewStyle;

	switch ( nID ) {
	
		case IDC_LARGE_ICONS		:	nNewStyle = LVS_ICON;		break;
		case IDC_MEDIUM_ICONS	:	nNewStyle = LVS_ICON;		break;
		case IDC_SMALL_ICONS		:	nNewStyle = LVS_SMALLICON;	break;
		case IDC_LIST_ICONS		:	nNewStyle = LVS_LIST;		break;
		case IDC_DETAILS_ICONS	:	nNewStyle = LVS_REPORT;		break;
		default						:	ASSERT( false );
	
	}

	// For now, don't allow details mode.
	if ( nNewStyle == LVS_REPORT )
		nNewStyle = LVS_LIST;
	
	// Set the requested mode.
	ModifyStyle(
		LVS_SMALLICON | LVS_REPORT | LVS_ICON | LVS_LIST,
		nNewStyle
	);
	
}


/*
//-------------------------------------------------------------------//
// Icon Arrange functions
//-------------------------------------------------------------------//
// These functions manage the arrangement of icons in icon and
// small icon views.
//-------------------------------------------------------------------//
void OleListCtrl::OnAutoArrange() 
{

	bAutoArrange = !bAutoArrange;

	if ( bAutoArrange ) {
		ModifyStyle( 0, LVS_AUTOARRANGE );
		Arrange( LVA_DEFAULT );
	} else
		ModifyStyle( LVS_AUTOARRANGE, 0 );
	
}

void OleListCtrl::OnUpdateAutoArrange(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( bAutoArrange );
}
*/


//-------------------------------------------------------------------//
// ExtractItemsToDragData()														//
//-------------------------------------------------------------------//
// Dump the current selection into drag data array.
//-------------------------------------------------------------------//
void OleListCtrl::ExtractItemsToDragData( 
	bool bSelectedOnly,
	int nUnderCursor
) {
	// Any previous drag contents should have been cleared at this time.
	// Remember, the drag data item array is only for temporary use, as
	// the clipboard is already handling the persistence of the drag data.
	ASSERT( Items.GetSize() == 0 );

	// Loop through items.
	int nLoop = GetNextItem( -1, bSelectedOnly? LVNI_SELECTED : LVNI_ALL );
	while ( nLoop != -1 ) 
	{
		// Get a drag data object to add to the drag archive.
		OleDragItem Temp;

		// Get the item's lParam.
		Temp.m_lParam	= GetItemData( nLoop );

		// Get the item's checked status.
		Temp.m_bChecked = bIsItemChecked( nLoop );

		// Add all the text from each column, separated by
		// the column separator.
		int nA;
		Temp.m_strName.Empty();
		for ( nA = 0; nA < GetAbsoluteColCount(); nA++ )
		{
			if ( nA != 0 )
				Temp.m_strName += ctColumnSeparator;
			Temp.m_strName += GetItemText( nLoop, nA );
		}
		
		// Currently we do not need any state data from the list control.
		// We may need state information at some point.
		// The flag variable can be used to hold this information.
		// There are states that are specific to tree controls,
		// list controls, etc.; we will add state flags only
		// as needed.  For example, if, at some point in time,
		// we need to know whether an item was marked to be cut,
		// we can test the LVIS_CUT flag here in the list control,
		// and set the EM_STATE_MARKED_FOR_CUT flag accordingly.
		// UINT uListState = GetItemState(
		// 	nUnderCursor,
		// 	LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED
		// );
		// if ( uListState & LVIS_CUT ) uNodeState |= EM_STATE_MARKED_FOR_CUT;
		// etc.
		
		// No thanks to this functionality.
		/*
		// If we are on the item under the cursor, make sure it
		// is moved to the top of the list, as needed.
		if (		nLoop == nUnderCursor 
				&& pData->Items.GetSize() > 0 ) {
		
			// Swap our new item with item 0.
			OleDragItem Temp2 = pData->Items[0];
			pData->Items[0] = Temp;
			Temp = Temp2;

		}
		*/
		
		// Add it to the list.
		Items.Add( Temp );

		// Try for the next one.
		nLoop = GetNextItem( nLoop, bSelectedOnly? LVNI_SELECTED : LVNI_ALL );

	}

}
	

//-------------------------------------------------------------------//
// BeginDrag()
//-------------------------------------------------------------------//
// This function attempts a drag when the left button has been
// pressed on an item.  It returns true if a drag was made.
//-------------------------------------------------------------------//
BOOL OleListCtrl::BeginDrag(
	int	nUnderCursor,
	UINT	nKeyFlags
) {
	// Attempt a drag.

	/////////////////////////////////////////////////////
	// Next, we provide a ClipFormat holding the item reference.

	// Make sure ClipFormat is ready to go.
	ASSERT( ClipFormat != NULL );

	// Dump the current selection into drag data array.
	ExtractItemsToDragData( 
		true,
		nUnderCursor
	);

	// To drag, we need a COleDataSource that is loaded with our data.
	// Get a new one.
	COleDataSource DataSource;

	// Put the drag data into the DS.
	ArchiveAndRenderClipFormats( &DataSource );
	
	// Now we can clear the previous drag contents.
	Items.RemoveAll();

	//	
	/////////////////////////////////////////////////////

	// Get the rectangle of the list item.
	// This will help us avoid a drag unless the user really means it.
	CRect ItemRect;
	GetItemRect( nUnderCursor, &ItemRect, LVIR_BOUNDS );
	ClientToScreen( &ItemRect );
		
	// Set flags to indicate that we are dragging.
	nListItemBeingDragged = nUnderCursor;
	bDraggingFromThisList = true;

	// Now drag it.
	DROPEFFECT deResult = DataSource.DoDragDrop(
		DROPEFFECT_COPY | DROPEFFECT_MOVE,	// | DROPEFFECT_LINK,
		ItemRect
	);

	InsertStart();

	// Indicate that we are no longer dragging.
	bDraggingFromThisList = false;
	nListItemBeingDragged = -1;

	// If it was a move, remove the item(s).
	if ( deResult == DROPEFFECT_MOVE )
	{
		// Delete the current selection, then
		// highlight the dragged items.
		DeleteOldAndHighlightNew();
	}

	// Successful drag and drop operation.
	if ( deResult != DROPEFFECT_NONE )
	{
		// Allow derived classes to do any additional clean up steps.
		FinishDragAndDrop();
	}

	InsertEnd();

	return ( deResult != DROPEFFECT_NONE );

}


// This is a little tricky.  We need to delete the current selection,
// while highlighting the newly dropped items, without letting them
// screw each other's indices up.
// We know that the newly dropped items are in a contiguous block,
// which helps.
void OleListCtrl::DeleteOldAndHighlightNew()
{
	// First, get a reverse-order list of selected items,
	// which will be deleted.
	CArray <int, int> anSelectedItems;
	anSelectedItems.SetSize( 0, 50 );
	int nA = -1;
	while ( ( nA = GetNextItem( nA, LVNI_SELECTED ) ) != -1 ) 
	{
		anSelectedItems.Add( nA );
	}
	UnselectAll();

	// Now select the dropped items.
	std::vector<int>::iterator it;
	for ( it = m_vnNewlyDroppedItems.begin(); it != m_vnNewlyDroppedItems.end(); ++it )
	{
		SetItemState( (*it), LVIS_SELECTED, LVIS_SELECTED );
	}

	// Now step through the deleted items, deleting.
	// We track the first newly dropped item as we go.
	int nFirst = m_vnNewlyDroppedItems[0];
	for ( nA = anSelectedItems.GetUpperBound(); nA >= 0; nA-- )
	{
		// There should be no overlap here!
		ASSERT( anSelectedItems[nA] != m_vnNewlyDroppedItems[0] );

		DeleteItem( anSelectedItems[nA], false );

		// Track the first slot as it slides up.
		if ( anSelectedItems[nA] < nFirst )
			nFirst--;
	}

	if ( nFirst >= 0 )
	{
		SetItemState( nFirst, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}

	m_vnNewlyDroppedItems.clear();
}


//-------------------------------------------------------------------//
// OnBegindrag()																		//
//-------------------------------------------------------------------//
// Catches LVN_BEGINDRAG messages, so we can let MS tell us when
// to begin dragging.
//-------------------------------------------------------------------//
void OleListCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_bDeselectOnNoDrag &= !BeginDrag( m_nLastUnderCursor, 0 );
	*pResult = 0;
}

//-------------------------------------------------------------------//
// OnLButtonDown()																	//
//-------------------------------------------------------------------//
void OleListCtrl::OnLButtonDown( UINT nFlags, CPoint point )
{
	UINT nHitFlags;
	int nUnderCursor = HitTest( point, &nHitFlags );

	m_bDeselectOnNoDrag = false;
	bool bCtrlMode = false;

	// Test if we are on an item.
	if (
			( nUnderCursor >= 0 )
		&&	( nHitFlags & LVHT_ONITEM )
	){
		// Save which item was actually clicked upon for later.
		m_nLastUnderCursor = nUnderCursor;

		// Ctrl + drag is a special case.  In that case, we want to make sure
		// the current item is ALWAYS selected when attempting the drag.  Then
		// if a drag does not occur, we will toggle the item's select state.
		
		// If control was pressed...
		if ( nFlags & MK_CONTROL )
		{
			// Set some flags for later.
			bCtrlMode = true;
			m_bDeselectOnNoDrag =
				( ( GetItemState( nUnderCursor, LVIS_SELECTED ) & LVIS_SELECTED ) != 0 );

			// Select it.  We may unselect it if no drag is made.
			SetItemState( nUnderCursor, LVIS_SELECTED, LVIS_SELECTED );
		}

		// Call derived classes to do any special handling 
		// associated with a left click on a list item.
		OnLButtonDownOnListItem( nUnderCursor, nFlags );
	}

	// MFC may not return from this function until it has
	// determined whether a drag has been attempted or not.
	// That's why we set the item state above and again below.
	inherited::OnLButtonDown( nFlags, point );

	// If the control key was down, implement the item toggling.
	// Note that m_bDeselectOnNoDrag will have been cleared
	// from within OnBeginDrag() if a drag did occur.
	if ( bCtrlMode )
	{
		if ( m_bDeselectOnNoDrag )
			SetItemState( nUnderCursor, 0, LVIS_SELECTED );
		else
			SetItemState( nUnderCursor, LVIS_SELECTED, LVIS_SELECTED );
	}

}


//-------------------------------------------------------------------//
// ExtractDropData()																	//
//-------------------------------------------------------------------//
// This routine takes the Ole drop data, provided by the Ole 
// framework upon a drop, creates a memory file and archive to
// process the data, and dumps the data into the drag data item 
// array.  The data includes a count of the number of items, and 
// an array called Items that contains the name and associated 
// lParam data for each of the items that are incorporated into 
// the drag data.
//-------------------------------------------------------------------//
void OleListCtrl::ExtractDropData(
	COleDataObject*	pDropData
) {

	// Get the standard OleDragData CF.
	CLIPFORMAT ClipFormat = RegisterClipboardFormat( cszOleDragData );

	// Get file referring to clipboard data.
	CFile *pFile = pDropData->GetFileData( ClipFormat );

	// If that didn't take, we don't have the format we want.
	// Get the heck outta here!
	if ( pFile == NULL )
		return;
	
	// Connect the file to the archive.
	CArchive DataArchive( pFile, CArchive::load );			
		
	// Load up the array.
	// Note that the drag data array should be empty.  Usually, the
	// OleDragData object has just been created prior to calling
	// this function, so the array is empty.  Make sure.
	ASSERT( Items.GetSize() == 0 );
	int nCount;
	DataArchive.Read( &nCount, sizeof( nCount ) );

	OleDragItem Temp;
	for ( int i = 0; i < nCount; i++ )
	{
		// Read the next item.		
		// This MUST MATCH the order in the write function.

		// First, we read the column string names.
		DataArchive >> Temp.m_strName;
		
		// Set the item's checked state.
		DataArchive >> (BYTE&)Temp.m_bChecked;

		// Now we read and stuff any data maintained by the lParam
		// pointer.  Derived classes should override this function
		// if they maintain any data.
		ReadLParam( &DataArchive, &Temp.m_lParam );

		// Add the item
		Items.Add( Temp );
	}

	// Finish up.
	DataArchive.Close();
	delete pFile;
}


//-------------------------------------------------------------------//
// bHaveValidDropData()																//
//-------------------------------------------------------------------//
// This routine peeks at the Ole drop data to see if it contains
// any recognizable OLE data.  Useful in OnUpdatePaste()-type
// handlers.
//-------------------------------------------------------------------//
bool OleListCtrl::bHaveValidDropData(
	COleDataObject*	pDropData
) {
	bool bFoundData = false;

	// Get the standard OleDragData CF.
	CLIPFORMAT ClipFormat = RegisterClipboardFormat( cszOleDragData );

	// Get file referring to clipboard data.
	CFile *pFile = pDropData->GetFileData( ClipFormat );

	// If that didn't take, we don't have the format we want.
	// Get the heck outta here!
	if ( pFile != NULL )
	{
		// Connect the file to the archive.
		CArchive DataArchive( pFile, CArchive::load );			
			
		// Note that the drag data array should be empty.
		ASSERT( Items.GetSize() == 0 );

		// Load up the array.
		int nCount;
		DataArchive.Read( &nCount, sizeof( nCount ) );

		bFoundData = ( nCount > 0 );

		// Finish up.
		DataArchive.Close();
		delete pFile;
	
	}

	return bFoundData;
}


//-------------------------------------------------------------------//
// ArchiveAndRenderClipFormats()													//
//-------------------------------------------------------------------//
// This routine takes the drag data, provided within the members of 
// this object by the object that owns this one, creates a memory 
// file and archive to process the data, and renders the data to the 
// clipboard using the supplied data source.  The data will then
// be available for drop targets to extract it.
// The clipboard formats associated with the data include the
// CF_OLE_DRAG_DATA type, which identifies the data as a
// OleDragData object.  The extraction always uses the 
// CF_OLE_DRAG_DATA type.
//-------------------------------------------------------------------//
void OleListCtrl::ArchiveAndRenderClipFormats(
	COleDataSource* pSource
) {
	
	// Prepare to archive the data.
	// Create a shared file and associate a CArchive with it
	CSharedFile DataMemFile;
	
	// TO DO
	// BoundsChecker is telling us, the first time we
	// hit this code and close the archive, that the 
	// flags are illegal.  What flags is it referring to?
	// The store flag here?  Research.
	CArchive DataArchive( &DataMemFile, CArchive::store );

	// Archive the count.
	int nCount = Items.GetSize();
	DataArchive.Write( &nCount, sizeof( nCount ) );

	// Loop through the items array and archive.
	int i;
	OleDragItem* pItem;
	for ( i = 0; i < nCount; i++ ) {

		pItem = &( Items[i] );
		
		// Write the next item.		
		// This MUST MATCH the order in the read function.

		// First, write the column names.
		DataArchive << pItem->m_strName;

		// Set the item's checked state.
		DataArchive << (BYTE)pItem->m_bChecked;

		// Now dump any data that is attached via the lParam.
		// This is a virtual function that is overridden in derived
		// classes so that we dump the actual contents of the data
		// pointed to by the lParam, if any.
		WriteLParam( &DataArchive, pItem->m_lParam );

	}

	// We need to close the archive before caching the data.
	DataArchive.Close();

	// We now render a clip format that contains the entire
	// drag data array.  This format is common to all classes
	// that use OleDragData.
	// CLIPFORMAT ClipFormat = CF_OLE_DRAG_DATA;
	CLIPFORMAT ClipFormat = RegisterClipboardFormat( cszOleDragData );
	
	// This immediately renders the data.
	// DelayRenderData is an alternative that delays rendering of the 
	// data until needed.
	pSource->CacheGlobalData( ClipFormat, DataMemFile.Detach() );

	// Note: Don't free the global allocation, that will be done by the 
	// data source.
	
	// TO DO
	// TEXT CLIPFORMAT CF_TEXT

	// Clear the items array, now that we have archived it.
	Items.RemoveAll();

}


