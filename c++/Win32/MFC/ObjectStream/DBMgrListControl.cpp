// DBMgrListControl.cpp : implementation file
//

#include "stdafx.h"

#include "..\DisplayMessage.h"

#include "DatabaseCreateDlg.h"

#include "..\BaseProgressDlg.h"

#include "..\UnicodeConversions.h"

#include "..\EMComponent.h"						// For btree log creation.

#include "EMDBArray.h"

#include "..\Dongle\Dongle.h"

#include "..\ObjectTypeSelectDlg.h"				// We select object type during a btree log.

#include "DBMgrListControl.h"

#include "..\EMComponentInit.h"

#include "..\DBTreeCache.h"						// We invalidate caches here.

#include "..\Dongle\PermissionsDatabase.h"	// We use a debug function here to create them.

#include "..\DBTreeControl.h"						// Needed for OnVerifyPrivateOwnership().

#include "DBAssociations.h"						// Checked upon connect/disconnect.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// DBMgrListControl

//-------------------------------------------------------------------//
// DBMgrListControl()																	//
//-------------------------------------------------------------------//
// Constructor
//-------------------------------------------------------------------//
DBMgrListControl::DBMgrListControl()
:

	// Init vars.
	bRefreshTrees( false )

{
}

//-------------------------------------------------------------------//
// ~DBMgrListControl()																	//
//-------------------------------------------------------------------//
DBMgrListControl::~DBMgrListControl()
{
}


//-------------------------------------------------------------------//
// Fill()																				//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void DBMgrListControl::Fill()
{

	DeleteAllItems();

	int nCount = pEMDBArray->GetSize();
	for ( int i = 0; i < nCount; i++ )
		InsertDBIntoList( i );

}


//-------------------------------------------------------------------//
// InsertDBIntoList																	//
//-------------------------------------------------------------------//
// Inserts a database into the dlg list, given an index into the
// DB Manager's list of databases.
//-------------------------------------------------------------------//
void DBMgrListControl::InsertDBIntoList(
	int nDBIndex
) {

	EMDatabase* pDatabase = pEMDBArray->pGetDBPtr( nDBIndex );

	// Database name text.
	// Here is an example of converting a wchar_t*
	// to a TCHAR*.  We let the temp CString object
	// do the bulk of the work.  It can accept
	// a wchar_t* pointer in its constructor, and
	// does the conversion to TCHAR for us.
	CString TempName(
		pDatabase->GetDatabaseName()
	);

	// Insert the item.
	// The param tells us the index into DB Manager::Databases.
	int NewItem = InsertItem(
		LVIF_TEXT | LVIF_PARAM,
		0,
		LPCTSTR( TempName ),
		0,
		0,
		0,
		(LPARAM) nDBIndex
	);
	ASSERT( NewItem != -1 );

	// Status text.
	UINT nStatus;
	if ( pDatabase->IsPublished() )
		nStatus = IDS_PUBLISHED_STATUS;
	else if ( pDatabase == pEMDBArray->pTargetDB )
		nStatus = IDS_DEFAULT_STATUS;
	else
		nStatus = IDS_WORKING_STATUS;
	SetItemText(
		NewItem,
		1,
		CString( (LPSTR) nStatus )
	);

	// Filename text.
	SetItemText(
		NewItem,
		2,
		LPCTSTR( *pDatabase->GetFilename() )
	);

}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(DBMgrListControl, CListCtrl)
	//{{AFX_MSG_MAP(DBMgrListControl)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(IDC_DB_SET_DEFAULT, OnSetDefault)
	ON_COMMAND(IDC_DB_PUBLISH, OnPublish)
	ON_COMMAND(IDC_DB_UNPUBLISH, OnUnpublish)
	ON_COMMAND(IDC_DB_DISCONNECT, OnDisconnect)
	ON_COMMAND(IDC_DB_INFO, OnProperties)
	ON_COMMAND(IDC_DB_CONNECT, OnConnect)
	ON_COMMAND(IDC_DB_CREATE, OnDBCreate)
	ON_COMMAND(IDC_DEFRAG, OnDefrag)
	ON_COMMAND(IDC_DB_DEFRAG, OnDBDefrag)
	ON_COMMAND(IDC_DB_REPAIR, OnRepair)
	ON_COMMAND(IDC_DB_APPEND, OnAppend)
	ON_COMMAND(IDC_CLEAR_CACHE, OnClearCache)
	ON_COMMAND(IDC_DB_CLEAR_CACHE, OnDBClearCache)
	ON_COMMAND(IDC_DB_UPDATE_ASSOCIATIONS, OnUpdateAssociations)
	ON_COMMAND(IDC_DB_VERIFY, OnVerify)
	ON_COMMAND(IDC_KEY_LOG, OnKeyLog)
	ON_COMMAND(IDC_RIGHT_CLICK_KEY, OnRightClickKey)
	ON_COMMAND(IDC_DB_GATHER, OnGather)
	ON_COMMAND(IDC_DB_GATHER_INCL_PUBLISHED, OnGatherIncludingPublished)
	//}}AFX_MSG_MAP

	ON_COMMAND_RANGE( ID_FIRST_OBJ_TYPE_MENU_COMMAND				, ID_LAST_OBJ_TYPE_MENU_COMMAND				, OnVerifyByType		)
	ON_COMMAND_RANGE( ID_FIRST_OBJ_TYPE_MENU_COMMAND_2				, ID_LAST_OBJ_TYPE_MENU_COMMAND_2			, OnVerifyFromType	)
	ON_COMMAND_RANGE( ID_FIRST_GATHER_TO_MENU_COMMAND				, ID_LAST_GATHER_TO_MENU_COMMAND				, OnGatherTo			)
	ON_COMMAND_RANGE( ID_FIRST_COPY_TO_MENU_COMMAND					, ID_LAST_COPY_TO_MENU_COMMAND				, OnCopyTo				)
	ON_COMMAND_RANGE( ID_FIRST_COPY_AND_GATHER_TO_MENU_COMMAND	, ID_LAST_COPY_AND_GATHER_TO_MENU_COMMAND	, OnCopyAndGatherTo	)

	// Limit access to in-house functions.
	// Coordinate this with PrepareMenu().
	#ifdef ACCELERATOR_INHOUSE
		ON_COMMAND(IDC_REF_LOG, OnRefLog)
		ON_COMMAND(IDC_DB_REF_LOG, OnDBRefLog)
		ON_COMMAND(IDC_BTREE_LOG, OnBtreeLog)
		ON_COMMAND(IDC_DB_BTREE_LOG, OnDBBtreeLog)
		ON_COMMAND(IDC_DB_VERIFY_PRIVATE_OWNERSHIP, OnVerifyPrivateOwnership)
	#endif

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// DBMgrListControl message handlers


//-------------------------------------------------------------------//
// OnRButtonDown()																	//
//-------------------------------------------------------------------//
void DBMgrListControl::OnRButtonDown(UINT nFlags, CPoint point)
{

	CListCtrl::OnRButtonDown(nFlags, point);

	// This bullshit is more trouble than it's worth.
	/*
	// Get the item under the cursor.
	UINT nHitFlags;
	int nUnderCursor = HitTest( point, &nHitFlags );
	if ( ( nHitFlags & LVHT_ONITEM ) == 0 )
		nUnderCursor = -1;

	// Upon a right click in the Windows 98/NT Explorer, the current selection is
	// preserved if the item under the cursor is a part of the selection, or if
	// the Ctrl or Shift keys are pressed.  If neither of these conditions are
	// true, the current selection is unselected and the item under the cursor 
	// is selected before displaying the right-click menu.  We will emulate this.
	if ( nUnderCursor != -1 && GetItemState( nUnderCursor, LVIS_SELECTED ) == LVIS_SELECTED )
	{
		// Just give it the focus rectangle.
		SetItemState( nUnderCursor, LVIS_FOCUSED, LVIS_FOCUSED );
	
	// If the selection keys are pressed...
	} else if (
			( GetKeyState( VK_SHIFT   ) < 0	)
		||	( GetKeyState( VK_CONTROL ) < 0	)
	) {
	
		// We need to make sure nUnderCursor is set if there
		// is any selection.  A click in a blank region of
		// the list will get us here with nUnderCursor = -1.
		nUnderCursor = GetNextItem( -1, LVIS_SELECTED );
	
	} else if ( nUnderCursor != -1 )
	{
		// Select our new item exclusively.
		UnselectAll();
		SetItemState( nUnderCursor, LVIS_SELECTED, LVIS_SELECTED );	
	}
	*/

	// Try to display the right click menu.
	DisplayRightClickMenu( point );

}


//-------------------------------------------------------------------//
// OnRightClickKey()																	//
//-------------------------------------------------------------------//
// This function processes presses of the VK_APPS key on the
// MS natural keyboard, etc.
//-------------------------------------------------------------------//
void DBMgrListControl::OnRightClickKey()
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
		CPoint( ItemRect.left, ItemRect.bottom )
	);

}


//-------------------------------------------------------------------//
// DisplayRightClickMenu()															//
//-------------------------------------------------------------------//
// This function does the work in popping up the right-click menu.
// It is called whenever the mouse is used, or the VK_APPS right-
// click-key (on MS natural keyboard, etc.) is pressed.
//-------------------------------------------------------------------//
void DBMgrListControl::DisplayRightClickMenu(
	CPoint		point
) {

	// Get the appropriate menu ID.
	UINT MenuID = ( GetNextItem( -1, LVNI_SELECTED ) == -1 ) ? IDM_DB_MGR_NO_ITEM_RIGHTCLICK : IDM_DB_MGR_ITEM_RIGHTCLICK;

	// Convert coordinates for TPM.
	ClientToScreen( &point );

	// Get popup menu.
	CMenu Menu;
	VERIFY( Menu.LoadMenu( MenuID ) );
	CMenu* pPopupMenu = Menu.GetSubMenu(0);

	// Prepare it, by e.g. adding checks where needed.
	PrepareMenu( pPopupMenu );

	// Show popup menu.
	pPopupMenu->TrackPopupMenu(
		TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		point.x,
		point.y,
		this
	);

}


//-------------------------------------------------------------------//
// [Un]SelectAll()																	//
//-------------------------------------------------------------------//
void DBMgrListControl::SelectAll()
{
	int nItem = GetNextItem( -1, LVNI_ALL );
	while ( nItem != -1 ) 
	{		
		SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
		nItem = GetNextItem( nItem, LVNI_ALL );
	}
}
void DBMgrListControl::UnselectAll()
{
	int nItem = GetNextItem( -1, LVNI_ALL );
	while ( nItem != -1 ) 
	{		
		SetItemState( nItem, LVIS_SELECTED, 0 );
		nItem = GetNextItem( nItem, LVNI_ALL );
	}
}


//-------------------------------------------------------------------//
// PrepareMenu()																		//
//-------------------------------------------------------------------//
// This function preps the menu.  In this case, there are three
// categories of menu items.  Some are always available.  We want to
// remove other items if the Ctrl-Shift keys are not pressed.  And
// others should only be available if the user is with Accelerator.
// The functions that are limited to use by Accelerator should be
// wrapped with ACCELERATOR_INHOUSE in the ClassWizard maps.
//-------------------------------------------------------------------//
void DBMgrListControl::PrepareMenu(
	CMenu*	pPopupMenu
) {
	int nA;
	
	// Get the selection count.  We need to determine which menu
	// is displayed, and we need to know when just one item is
	// selected.
	int nSelCount = GetSelectedCount();
	
	CMenu* pAdvancedMenu;
	CMenu* pInHouseMenu;
	int nAdvancedMenuPos;
	int nInHouseMenuPos;
	if ( nSelCount > 0 ) 
	{
		nAdvancedMenuPos = 3;
		nInHouseMenuPos = 4;
	} else
	{
		nAdvancedMenuPos = 2;
		nInHouseMenuPos = 3;
	}
	pAdvancedMenu	= pPopupMenu->GetSubMenu( nAdvancedMenuPos );
	pInHouseMenu	= pPopupMenu->GetSubMenu( nInHouseMenuPos );
		
	// If an item is selected, we need to determine its status
	// to know what to allow on the menu.
	EMDatabase* pDB;
	if ( nSelCount > 0 ) 
	{
		pDB = pEMDBArray->pGetDBPtr( (int) GetItemData( GetNextItem( -1, LVNI_SELECTED ) ) );

		// Disable the items that can't be applied to published db's.
		if ( pDB->IsPublished() ) 
		{
			pPopupMenu		->EnableMenuItem	( IDC_DB_PUBLISH,					MF_BYCOMMAND | MF_GRAYED	);
			pPopupMenu		->EnableMenuItem	( IDC_DB_SET_DEFAULT,			MF_BYCOMMAND | MF_GRAYED	);
			pAdvancedMenu	->EnableMenuItem	( IDC_DB_APPEND,					MF_BYCOMMAND | MF_GRAYED	);
		}

		// Disable the items that can't be applied to the target database.
		if ( pDB == pEMDBArray->pTargetDB ) 
		{
			pPopupMenu->EnableMenuItem	( IDC_DB_DISCONNECT	,	MF_BYCOMMAND | MF_GRAYED );
			pPopupMenu->CheckMenuItem	( IDC_DB_SET_DEFAULT	,	MF_BYCOMMAND | MF_CHECKED	);
			pPopupMenu->EnableMenuItem	( IDC_DB_SET_DEFAULT	,	MF_BYCOMMAND | MF_GRAYED );
		}

		int nSelectedDBIndex = GetNextItem( -1, LVNI_SELECTED );

		// Add an advanced menu for entire-db copying, if any are selected.
		int nGatherPos;
		int nAdvCount = pAdvancedMenu->GetMenuItemCount();
		if ( 
				nSelectedDBIndex != -1
			&& GetItemCount() > 1
		) {
			nGatherPos = 0;

			// Search for the "Gather" menu item.
			while (
					( IDC_DB_GATHER != pAdvancedMenu->GetMenuItemID( nGatherPos ) )
				&&	( nGatherPos < nAdvCount ) 
			)
				nGatherPos++;

			// Now go just past "Gather".
			nGatherPos++;

			// Ensure that nothing is goofy.
			ASSERT( nGatherPos < nAdvCount );
			if ( nGatherPos < nAdvCount )
			{
				// Create a new menu to build up and add as the object popup menu.
				CMenu ObjMenu;
				ObjMenu.CreatePopupMenu();

				int nDBCount = pEMDBArray->GetSize();
				for ( nA = 0; nA < nDBCount; nA++ )
				{
					// Don't add the selected item.
					if ( nA == nSelectedDBIndex )
						continue;

					// Get the db pointer.
					int nDBIndex = (int) GetItemData( nA );
					EMDatabase* pDatabase = pEMDBArray->pGetDBPtr( nDBIndex );

					// Add an item for the db.
					ObjMenu.AppendMenu(
						MF_STRING | ( pDatabase->IsPublished() ? MF_GRAYED : MF_ENABLED ),
						nA + ID_FIRST_COPY_TO_MENU_COMMAND,
						CString( pDatabase->GetDatabaseName() )
					);
				}

				pAdvancedMenu->InsertMenu(
					nGatherPos + 1, 
					MF_POPUP | MF_BYPOSITION,
					(UINT)ObjMenu.m_hMenu, 
					ResStr( IDS_DB_COPY_TO ) 
				);

				ObjMenu.Detach();
			}
		}

		// Add an advanced menu for db-specific CopyAndGatherTo, if any are selected.
		if ( 
				nSelectedDBIndex != -1
			&& GetItemCount() > 1
		) {
			// Ensure that nothing is goofy.
			ASSERT( nGatherPos < nAdvCount );
			if ( nGatherPos < nAdvCount )
			{
				// Create a new menu to build up and add as the object popup menu.
				CMenu ObjMenu;
				ObjMenu.CreatePopupMenu();

				int nDBCount = pEMDBArray->GetSize();
				for ( nA = 0; nA < nDBCount; nA++ )
				{
					// Don't add the selected item.
					if ( nA == nSelectedDBIndex )
						continue;

					// Get the db pointer.
					int nDBIndex = (int) GetItemData( nA );
					EMDatabase* pDatabase = pEMDBArray->pGetDBPtr( nDBIndex );

					// Add an item for the db.
					ObjMenu.AppendMenu(
						MF_STRING | ( pDatabase->IsPublished() ? MF_GRAYED : MF_ENABLED ),
						nA + ID_FIRST_COPY_AND_GATHER_TO_MENU_COMMAND,
						CString( pDatabase->GetDatabaseName() )
					);
				}

				pAdvancedMenu->InsertMenu(
					nGatherPos + 1, 
					MF_POPUP | MF_BYPOSITION,
					(UINT)ObjMenu.m_hMenu, 
					ResStr( IDS_DB_COPY_AND_GATHER_TO ) 
				);

				ObjMenu.Detach();
			}
		}

		// Add an advanced menu for db-specific gathering, if any are selected.
		if ( 
				nSelectedDBIndex != -1
			&& GetItemCount() > 1
		) {
			// Ensure that nothing is goofy.
			ASSERT( nGatherPos < nAdvCount );
			if ( nGatherPos < nAdvCount )
			{
				// Create a new menu to build up and add as the object popup menu.
				CMenu ObjMenu;
				ObjMenu.CreatePopupMenu();

				int nDBCount = pEMDBArray->GetSize();
				for ( nA = 0; nA < nDBCount; nA++ )
				{
					// Don't add the selected item.
					if ( nA == nSelectedDBIndex )
						continue;

					// Get the db pointer.
					int nDBIndex = (int) GetItemData( nA );
					EMDatabase* pDatabase = pEMDBArray->pGetDBPtr( nDBIndex );

					// Add an item for the db.
					ObjMenu.AppendMenu(
						MF_STRING | ( pDatabase->IsPublished() ? MF_GRAYED : MF_ENABLED ),
						nA + ID_FIRST_GATHER_TO_MENU_COMMAND,
						CString( pDatabase->GetDatabaseName() )
					);
				}

				pAdvancedMenu->InsertMenu(
					nGatherPos + 1, 
					MF_POPUP | MF_BYPOSITION,
					(UINT)ObjMenu.m_hMenu, 
					ResStr( IDS_DB_GATHER_TO ) 
				);

				ObjMenu.Detach();
			}
		}

		// Add a popup menu for verification by OBJECT_TYPE
		#ifdef ACCELERATOR_INHOUSE
		{
			int nVerifyPos = 0;
			int nCount = pInHouseMenu->GetMenuItemCount();

			// Search for the "Create Index log" menu item.
			while (
					( IDC_BTREE_LOG != pInHouseMenu->GetMenuItemID( nVerifyPos ) )
				&&	( nVerifyPos < nCount ) 
			){
				nVerifyPos++;
			}

			// Ensure that nothing is goofy.
			if ( nVerifyPos < nCount )
			{
				// Create a new menu to build up and add as the object popup menu.
				CMenu ObjMenu, ObjMenu2;
				ObjMenu.CreatePopupMenu();
				ObjMenu2.CreatePopupMenu();

				for ( nA = 0; nA < TYPE_TREE_LAST; nA++ )
				{
					ObjMenu.AppendMenu(
						MF_STRING | MF_ENABLED,
						nA + ID_FIRST_OBJ_TYPE_MENU_COMMAND,
						EMComponentInit::ByType( nA )->strDescPlural()
					);
					ObjMenu2.AppendMenu(
						MF_STRING | MF_ENABLED,
						nA + ID_FIRST_OBJ_TYPE_MENU_COMMAND_2,
						EMComponentInit::ByType( nA )->strDescPlural()
					);
				}

				pInHouseMenu->InsertMenu(
					nVerifyPos + 1, 
					MF_POPUP | MF_BYPOSITION,
					(UINT)ObjMenu.m_hMenu, 
					ResStr( IDS_VERIFY_BY_TYPE ) 
				);
				pInHouseMenu->InsertMenu(
					nVerifyPos + 2, 
					MF_POPUP | MF_BYPOSITION,
					(UINT)ObjMenu2.m_hMenu, 
					ResStr( IDS_VERIFY_STARTING_AT ) 
				);

				ObjMenu.Detach();
				ObjMenu2.Detach();
			}
		}
		#endif // ACCELERATOR_INHOUSE

	}

	// If we are not in-house...
	#ifndef ACCELERATOR_INHOUSE

		// Remove the entire inhouse menu.
		pPopupMenu->DeleteMenu( nInHouseMenuPos, MF_BYPOSITION );

		// If Ctrl and Shift are not both pressed...
		if (		GetKeyState( VK_SHIFT   ) >= 0
				||	GetKeyState( VK_CONTROL ) >= 0	) 
		{
			// Remove the entire advanced menu.
			pPopupMenu->DeleteMenu( nAdvancedMenuPos, MF_BYPOSITION );
		}

		// Remove the items that can't be applied by the end user to published db's.
		if ( pDB->IsPublished() ) 
		{
			pAdvancedMenu->DeleteMenu(	IDC_CLEAR_CACHE,						MF_BYCOMMAND );
			pAdvancedMenu->DeleteMenu(	IDC_DB_CLEAR_CACHE,					MF_BYCOMMAND );
			pAdvancedMenu->DeleteMenu(	IDC_DB_VERIFY,							MF_BYCOMMAND );
		}
		
	#endif

}


//-------------------------------------------------------------------//
// NewDatabaseFromFile()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
EMDatabase* DBMgrListControl::NewDatabaseFromFile(
	UINT	FileDlgTitle,
	bool	bIncludePublished
) {

	EMDatabase* pDatabase = 0;

	// Get a filename.
	CString strDBName;
	if (
		pEMDBArray->GetFile(
			&strDBName,
			FileDlgTitle,
			this,
			bIncludePublished
		) == IDOK
	) {

		CWaitCursor WaitAMinuteBub;

		// Open the database.
		pDatabase = new EMDatabase( &strDBName );
		if ( pDatabase->Initialize() != DB_INIT_SUCCESS ) {

			delete pDatabase;
			pDatabase = 0;

		}

	}

	return pDatabase;

}



// The following functions are for in-house use.
#ifdef ACCELERATOR_INHOUSE

//-------------------------------------------------------------------//
// OnRefLog()																			//
//-------------------------------------------------------------------//
// This function creates a detailed report for all currently 
// selected databases.
//-------------------------------------------------------------------//
void DBMgrListControl::OnRefLog()
{

	CWaitCursor WaitAMinuteBub;

	// Loop through list items until we find a selection.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ ) {

		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED ) {

			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			// Get the db pointer.
			EMDatabase* pDatabase = pEMDBArray->pGetDBPtr( nDBIndex );

			BaseProgressDlg ProgressDlg(
				this
			);
			pDatabase->SetProgressTitle(
				&ProgressDlg,
				IDS_CREATING_REF_LOG
			);

			// Do it.
			pDatabase->CreateReferenceLog(
				&ProgressDlg.ProgressMeter
			);

			// Clean up.
			ProgressDlg.DestroyWindow();

			break;

		}

	}

}


//-------------------------------------------------------------------//
// OnDBRefLog()																		//
//-------------------------------------------------------------------//
// This function gets a db file and creates a detailed report for it.
//-------------------------------------------------------------------//
void DBMgrListControl::OnDBRefLog()
{

	EMDatabase* pDatabase = NewDatabaseFromFile( IDS_DB_LOG_TITLE );

	if ( pDatabase ) {

		CWaitCursor WaitAMinuteBub;


		BaseProgressDlg ProgressDlg(
			this
		);
		pDatabase->SetProgressTitle(
			&ProgressDlg,
			IDS_CREATING_REF_LOG
		);

		// Do it.
		pDatabase->CreateReferenceLog(
			&ProgressDlg.ProgressMeter
		);

		// Clean up.
		ProgressDlg.DestroyWindow();

		delete pDatabase;

	}

}


#endif


//-------------------------------------------------------------------//
// OnProperties()																		//
//-------------------------------------------------------------------//
// This function displays the properties for the database.  The
// name can be changed.
//-------------------------------------------------------------------//
void DBMgrListControl::OnProperties()
{

	// Loop through list items until we find a selection.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ ) {

		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED ) {

			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			// Display properties.
			pEMDBArray->pGetDBPtr( nDBIndex )->DisplayProperties( GetParent() );

			break;

		}

	}

}


//-------------------------------------------------------------------//
// OnConnect()																			//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void DBMgrListControl::OnConnect()
{
	EMDatabase* pDatabase = NewDatabaseFromFile( IDS_DB_CONNECT_TITLE );

	if ( pDatabase ) 
	{
		CWaitCursor Stop_HammerTime;

		// We have a new database, make sure it is updated to the 
		// latest version.
		if ( pDatabase->UpdateDatabaseAsNeeded() )
		{

			// If we are not already connected to the db...
			// Note that this function will clean up for us if
			// we are already connected.
			if ( !pEMDBArray->DBAlreadyConnected( pDatabase, this ) ) 
			{
				// Don't normally allow connect to demo databases.
				// They will likely contain duplicates of objects in the 
				// master database.
				#ifndef ACCELERATOR_INHOUSE
				
					if ( pDatabase->DBHeader.Group == DemoGroup ) 
					{
						CString strMsg;
						strMsg.Format( 
							IDS_CONNECT_FAILED_DEMO_DATABASE, 
							CString( pDatabase->GetDatabaseName() )
						);
						DisplayMessage( strMsg, IDL_WARNING, this );
						delete pDatabase;

					} else
				
				#endif
				{
					// Check the associations of this database.  If there
					// is a missing associated db, don't allow the connect.
					if ( !pDatabase->IsPublished() )
					{
						BaseProgressDlg ProgressDlg( this );
						pDatabase->UpdateAssociations( &ProgressDlg );
						ProgressDlg.DestroyWindow();
					}
					DBAssociations DBA( pDatabase );
					int nAssocCount = DBA.Associations.GetSize();
					ObjectStream* pDBAssn;
					int nB;
					for ( nB = 0; nB < nAssocCount; nB++ )
						if (
							!pEMDBArray->LookUpDatabase( 
								&( DBA.Associations[nB]->DBID ),
								&pDBAssn
							)
						)
							break;

					if ( nB < nAssocCount )
					{
						CString strMsg;
						strMsg.Format( 
							IDS_CONNECT_FAILED_MISSING_ASSOC, 
							CString( pDatabase->GetDatabaseName()		),
							CString( DBA.Associations[nB]->DBName		),
							CString( DBA.Associations[nB]->UserName	),
							CString( DBA.Associations[nB]->Company		),
							CString( DBA.Associations[nB]->Contact		)
						);
						DisplayMessage( strMsg, IDL_WARNING, this );
						delete pDatabase;

					} else
					{

						bool bConnectOK = true;

						// Add it to the databases array.  We have to do this now
						// because its ID may be grabbed below in FillTree.
						pEMDBArray->Add( pDatabase );

						// If we aren't currently planning on doing a refresh anyway...
						if ( !bRefreshTrees ) 
						{
							// Fill the trees using the database.
							BaseProgressDlg ProgressDlg(
								this
							);
							pDatabase->SetProgressTitle(
								&ProgressDlg,
								IDS_DB_CONNECTED
							);

							for ( int j = 0; j <= TYPE_TREE_LAST && bConnectOK; j++ ) {

								DBTreeControl* pTree = DBTreeControl::GetTreePtr( (OBJECT_TYPE) j );
								pTree->BeginObjectInsertions();

								// We're OK if we had to update a published cache, this happens 
								// whenever we add a new object type.
								bConnectOK &= (
									(
										pDatabase->FillTree(
											(OBJECT_TYPE) j,						// eWhich
											0,											// Splash screen
											false,									// bCreate
											false										// bRefresh
										) & ( FT_SUCCESS | FT_PUBLISHED_CACHE_OLD ) 
									) != 0
								);

								pTree->EndObjectInsertions();

								ProgressDlg.UpdateProgress( 100 * ( j + 1 ) / ( TYPE_TREE_LAST + 1 ) );

							}

							if ( !bConnectOK ) {

								// Warn if the db had trouble.
								DisplayMessage( IDM_DB_BAD_INIT );
								bRefreshTrees = true;

							}

							// Clean up.
							ProgressDlg.DestroyWindow();

						}

						// If everything went okay, add the db.
						if ( bConnectOK ) 
						{
							// Add it to the display list.
							InsertDBIntoList(
								pEMDBArray->GetSize() - 1
							);

							// Add it to the db registry list.
							pEMDBArray->AddDBToRegistryList( pDatabase );

						}
						// Otherwise, remove the one we added above.
						else
						{
							int nNewDBIdx = pEMDBArray->GetSize() - 1;
							pEMDBArray->RemoveAt( nNewDBIdx );
						}
					}	// All associations found
				}	// Not demo
			}	// Not already connected
		
		// If we couldn't update the database, delete it.
		} else
		{
			delete pDatabase;
		}
	}

}


//-------------------------------------------------------------------//
// OnDisconnect()																		//
//-------------------------------------------------------------------//
void DBMgrListControl::OnDisconnect()
{
	CWaitCursor GoDogGo;
	int nA, nB, nC;
	int nDBIndex;

	// Loop through list items and gather up the 
	// db pointers into selected and unselected groups.
	vector<EMDatabase*> vpSelectedDB;
	vector<EMDatabase*> vpUnselectedDB;
	
	int nCount = GetItemCount();
	for ( nA = 0; nA < nCount; nA++ ) 
	{
		// Get the database.
		nDBIndex = (int) GetItemData( nA );
		EMDatabase* pDB = pEMDBArray->pGetDBPtr( nDBIndex );

		// Update associations for working databases.
		if ( !pDB->IsPublished() )
		{
			BaseProgressDlg ProgressDlg( this );
			pDB->UpdateAssociations( &ProgressDlg );
			ProgressDlg.DestroyWindow();
		}

		// Stuff it into the appropriate array.
		if ( GetItemState( nA, LVIS_SELECTED ) == LVIS_SELECTED ) 
			vpSelectedDB.push_back( pDB );
		else 
			vpUnselectedDB.push_back( pDB );
	}
	
	// Now determine if any unselected db's need any selected db's.
	bool bDisconnectOK = true;
	CString strDisconnectProblems, strTemp;
	
	// Loop through unselected db's.
	for ( nA = 0; nA < vpUnselectedDB.size(); nA++ )
	{
		// Loop through all associations for this db.
		DBAssociations DBA( vpUnselectedDB[nA] );
		int nAssocCount = DBA.Associations.GetSize();
		for ( nB = 0; nB < nAssocCount; nB++ )
		{
			ObjectStream* pDBAssn;
			if (
				!pEMDBArray->LookUpDatabase( 
					&( DBA.Associations[nB]->DBID ),
					&pDBAssn
				)
			) {
				// We won't make this problem force failure of the
				// disconnect, as it is unrelated.  We just let the
				// user know there is a problem.
				strTemp.Format( 
					IDS_DISCONNECT_PROBLEM_UNKNOWN_ASSOC, 
					CString( vpUnselectedDB[nA]->GetDatabaseName() )
				);
				DisplayMessage( strTemp, IDL_WARNING, this );
				continue;
			}

			// Is this association one of the selected items?
			for ( nC = 0; nC < vpSelectedDB.size(); nC++ )
				if ( vpSelectedDB[nC] == (EMDatabase*) pDBAssn )
					break;
			if ( nC < vpSelectedDB.size() )
			{
				bDisconnectOK = false;
				strTemp.Format( 
					IDS_DISCONNECT_PROBLEM_OUTSTANDING_ASSOC, 
					CString( vpUnselectedDB[nA]->GetDatabaseName() ),
					CString( vpSelectedDB  [nC]->GetDatabaseName() )
				);
				strDisconnectProblems += strTemp;
					
			}
		}
	}

	if ( !bDisconnectOK )
	{
		strTemp.Format( IDS_DISCONNECT_PROBLEM_SUMMARY, strDisconnectProblems );
		DisplayMessage( strTemp, IDL_WARNING, this );
		return;
	}

	// Now we can disconnect.
	for ( nA = 0; nA < nCount; nA++ ) 
	{
		// If this is a selected item...
		if ( GetItemState( nA, LVIS_SELECTED ) == LVIS_SELECTED ) 
		{
			// Get the db pointer.
			int nDBIndex = (int) GetItemData( nA );
			EMDatabase* pDatabase = pEMDBArray->pGetDBPtr( nDBIndex );

			// Disconnect.
			if ( pEMDBArray->Disconnect( pDatabase, GetParent() ) ) 
			{
				// Remove it from the db list.
				// Also, subtract 1 from nA and nCount to make up for deletion.
				DeleteItem( nA );
				nA--; nCount--;

				// We'll need to decrease other indexes, now that we removed
				// this one.
				for ( nB = 0; nB < nCount; nB++ ) {
					int nNewIndex = (int) GetItemData( nB );
					if ( nNewIndex > nDBIndex )
						SetItemData(
							nB,
							(LPARAM) ( nNewIndex - 1 )
						);
				}

				bRefreshTrees = true;

			}
		}
	}
}


//-------------------------------------------------------------------//
// OnPublish()																			//
//-------------------------------------------------------------------//
void DBMgrListControl::OnPublish()
{

	// Get the target database.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ ) {

		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED ) {

			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			// See if both Ctrl and Shift are pressed.  If so, confirm that the user
			// wants to do an "unsafe quick" publish.
			bool bBypassPreparation = false;
			if (		GetKeyState( VK_SHIFT   ) < 0
					&&	GetKeyState( VK_CONTROL ) < 0	)

				if (
					DisplayMessage(
						IDM_DB_VERIFY_QUICK_PUBLISH,							// Message ID
						IDL_WARNING,												// Label ID
						0,																// pParent
						MB_YESNO | MB_ICONINFORMATION | MB_APPLMODAL		// nType
					) == IDYES
				)
					bBypassPreparation = true;

			// Publish it.
			pEMDBArray->pGetDBPtr( nDBIndex )->Publish(
				GetParent(),
				bBypassPreparation
			);

			break;

		}

	}

}


// OLD
// Instead, perform "Create DB" then "CopyAndGatherTo".
//-------------------------------------------------------------------//
// OnUnpublish()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void DBMgrListControl::OnUnpublish()
{
	// Get the target database.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ )
	{
		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED )
		{
			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			pEMDBArray->
				pGetDBPtr( nDBIndex )->
					Publish(
						GetParent(),
						true						// Unpublish.
					);

			break;
		}
	}
}


//-------------------------------------------------------------------//
// Gather()	etc.																		//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void DBMgrListControl::OnGather() 
{
	Gather();	
}
void DBMgrListControl::OnGatherIncludingPublished() 
{
	Gather( true );	
}
void DBMgrListControl::Gather( bool bIncludePublished ) 
{
	// Loop through current selection and gather as requested.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ )
	{
		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED )
		{
			int nDBIndex = (int) GetItemData( i );
			pEMDBArray->
				pGetDBPtr( nDBIndex )->
					Gather( this, bIncludePublished );
		}
	}
}


//-------------------------------------------------------------------//
// OnCopyTo()																			//
//-------------------------------------------------------------------//
// Copies all objects in all selected databases to the specified db.
//-------------------------------------------------------------------//
void DBMgrListControl::OnCopyTo( UINT nID )
{
	// Get the database to which we will be copying.
	int nDBIndex = (int) GetItemData( nID - ID_FIRST_COPY_TO_MENU_COMMAND );
	EMDatabase* pToDB = pEMDBArray->pGetDBPtr( nDBIndex );

	// Get the first selected db.
	int nSelectedItem = GetNextItem( -1, LVNI_SELECTED );
	ASSERT( nSelectedItem != -1 );

	while ( nSelectedItem != -1 )
	{
		nDBIndex = (int) GetItemData( nSelectedItem );
		EMDatabase* pFromDB = pEMDBArray->pGetDBPtr( nDBIndex );

		// Now do it.
		pToDB->CopyAll( 
			this,
			pFromDB
		);

		//	Loop.
		nSelectedItem = GetNextItem( nSelectedItem, LVNI_SELECTED );
	}
}


//-------------------------------------------------------------------//
// OnCopyAndGatherTo()																//
//-------------------------------------------------------------------//
// Copies and gathers all objects in all selected databases to 
// the specified db.
//-------------------------------------------------------------------//
void DBMgrListControl::OnCopyAndGatherTo( UINT nID )
{
	// Get the database to which we will be copying.
	int nDBIndex = (int) GetItemData( nID - ID_FIRST_COPY_AND_GATHER_TO_MENU_COMMAND );
	EMDatabase* pToDB = pEMDBArray->pGetDBPtr( nDBIndex );

	// Get the first selected db.
	int nSelectedItem = GetNextItem( -1, LVNI_SELECTED );
	ASSERT( nSelectedItem != -1 );

	while ( nSelectedItem != -1 )
	{
		nDBIndex = (int) GetItemData( nSelectedItem );
		EMDatabase* pFromDB = pEMDBArray->pGetDBPtr( nDBIndex );

		// Now do it.
		pToDB->CopyAll(
			this,
			pFromDB,
			true
		);

		//	Loop.
		nSelectedItem = GetNextItem( nSelectedItem, LVNI_SELECTED );
	}
}


//-------------------------------------------------------------------//
// OnGatherTo()																		//
//-------------------------------------------------------------------//
// Gathers each selected db to the specified db.
//-------------------------------------------------------------------//
void DBMgrListControl::OnGatherTo( UINT nID )
{
	// Get the database to which we will be gathering.
	int nDBIndex = (int) GetItemData( nID - ID_FIRST_GATHER_TO_MENU_COMMAND );
	EMDatabase* pToDB = pEMDBArray->pGetDBPtr( nDBIndex );

	// Get the first selected db.
	int nSelectedItem = GetNextItem( -1, LVNI_SELECTED );
	ASSERT( nSelectedItem != -1 );

	while ( nSelectedItem != -1 )
	{
		nDBIndex = (int) GetItemData( nSelectedItem );
		EMDatabase* pFromDB = pEMDBArray->pGetDBPtr( nDBIndex );

		// Now do it.
		pToDB->Gather( 
			this,
			false,
			pFromDB
		);

		//	Loop.
		nSelectedItem = GetNextItem( nSelectedItem, LVNI_SELECTED );
	}

}


//-------------------------------------------------------------------//
// OnVerify()																			//
//-------------------------------------------------------------------//
void DBMgrListControl::OnVerify()
{
	// Get the target database.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ )
	{
		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED )
		{
			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			pEMDBArray->
				pGetDBPtr( nDBIndex )->
					VerifyContents();
		}
	}
}


//-------------------------------------------------------------------//
// OnVerifyByType()																	//
//-------------------------------------------------------------------//
// Verifies the selected OBJECT_TYPE within the selected DBs.
//-------------------------------------------------------------------//
void DBMgrListControl::OnVerifyByType( UINT nID )
{
	// Get the target database.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ )
	{
		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED )
		{
			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			pEMDBArray->
				pGetDBPtr( nDBIndex )->
					VerifyContents( nID - ID_FIRST_OBJ_TYPE_MENU_COMMAND, false );
		}
	}
}


//-------------------------------------------------------------------//
// OnVerifyFromType()																//
//-------------------------------------------------------------------//
// Verifies starting at the selected OBJECT_TYPE 
// within the selected DBs.
//-------------------------------------------------------------------//
void DBMgrListControl::OnVerifyFromType( UINT nID )
{
	// Get the target database.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ )
	{
		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED )
		{
			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			pEMDBArray->
				pGetDBPtr( nDBIndex )->
					VerifyContents( nID - ID_FIRST_OBJ_TYPE_MENU_COMMAND_2, true );
		}
	}
}





//-------------------------------------------------------------------//
// OnDBCreate()																		//
//-------------------------------------------------------------------//
void DBMgrListControl::OnDBCreate()
{

	// Display a dialog requesting the new database
	// and file name.  Provide the db manager pointer.
	DatabaseCreateDlg CreateDlg( this );
	if ( !Dongle::bDemoMode && CreateDlg.DoModal() == IDOK ) 
	{
		CWaitCursor SimmerDown;

		// Create a new working database object as requested.
		EMDatabase* pDatabase = new EMDatabase( &CreateDlg.strFilename );

		// Convert the database name.
		wstring wsDBName;
		CopyCStringToWstring(
			&CreateDlg.strDatabaseName,
			&wsDBName
		);

		if (
			pDatabase->Initialize(
				false,									// bFileShouldExist
				&wsDBName,								// pwsDatabaseName
				&( MainDongle.GetNewDBID() ),		// pDBID - We want to make a new working database ID.
				MainDongle.GetUserNumber()			// Group - new db's use the user's personal group,
															// which is equivalent to their user number.
			) == DB_INIT_SUCCESS
		) {

			// Add it to the databases array.
			pEMDBArray->Add( pDatabase );

			// Add it to the display list.
			InsertDBIntoList(
				pEMDBArray->GetSize() - 1
			);

		} else {

			DisplayMessage( IDS_DB_CREATION_FAILED );
			pDatabase->DeleteFiles();
			delete pDatabase;

		}

	}

}


//-------------------------------------------------------------------//
// OnDBDefrag																				//
//-------------------------------------------------------------------//
// This function defrags the database by rewriting all valid nodes
// in the data file and then reindexing.
//
// This version handles a non-connected database.  Connected 
// databases must be explicitly defragged using IDC_DEFRAG/OnDefrag().
//-------------------------------------------------------------------//
void DBMgrListControl::OnDBDefrag()
{

	EMDatabase* pDatabase = NewDatabaseFromFile( IDS_DB_DEFRAG_TITLE, false );

	if ( pDatabase ) {

		CWaitCursor Stop_HammerTime;

		// If we are not already connected to the db...
		// Note that this function will clean up for us if
		// we are already connected.
		if ( !pEMDBArray->DBAlreadyConnected( pDatabase, this ) ) {

			// Set up a progress dlg.
			BaseProgressDlg ProgressDlg(
				this
			);

			// Compact.
			pDatabase->Compact(
				&ProgressDlg
			);

			// Clean up.
			ProgressDlg.DestroyWindow();

			// Clean up.
			delete pDatabase;

		} else {

			// We don't want to change db's out from under the DB manager.
			// Connected db's must be explicitly defragged using the existing
			// list item message command.
			DisplayMessage( IDM_DB_DEFRAG_ALREADY_CONNECTED, IDL_WARNING, this );

		}

	}

}


//-------------------------------------------------------------------//
// OnDefrag																				//
//-------------------------------------------------------------------//
// This defrag version defrags all selected db's.
//-------------------------------------------------------------------//
void DBMgrListControl::OnDefrag()
{

	// Get the selected databases.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ )
	{
		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED )
		{

			CWaitCursor Stop_HammerTime;

			// Get the database.
			EMDatabase* pDatabase = pEMDBArray->pGetDBPtr( (int) GetItemData( i ) );

			// Set up a progress dlg.
			BaseProgressDlg ProgressDlg(
				this
			);

			// Compact.
			pDatabase->Compact(
				&ProgressDlg
			);

			// Clean up.
			ProgressDlg.DestroyWindow();

		}
	}
}


//-------------------------------------------------------------------//
// OnRepair()																			//
//-------------------------------------------------------------------//
// This function handles repair of the selected database files.
// If the reindexing fails, the user is notified.  They should
// not connect to the database, as it is irrepairably damaged.
// They will have to re-obtain the database from the publisher.
// Repairs are to be done "automatically"; this is provided
// temporarily as a "back door" for now.
//-------------------------------------------------------------------//
void DBMgrListControl::OnRepair()
{

	// Get a filename.
	// We don't use NewDBFromFile() because we want to
	// specify Init() params.
	CString strDBName;
	if (
		pEMDBArray->GetFile(
			&strDBName,
			IDS_DB_REPAIR_TITLE,
			this,
			false								// bIncludePublished
		) == IDOK
	) {

		// Open the database.
		EMDatabase* pDatabase = new EMDatabase( &strDBName );
		if(
			pDatabase->Initialize(
				true,						// bbFileShouldExist
				0,							// pwsDatabaseName
				0,							// pDBID
				0,							// pObjectSeed
				true						// bForceReindex
			) != DB_INIT_SUCCESS
		)
			DisplayMessage( IDS_REPAIR_FAILED );

		// Clean up.
		delete pDatabase;

	}

}


//-------------------------------------------------------------------//
// OnSetDefault()																		//
//-------------------------------------------------------------------//
void DBMgrListControl::OnSetDefault()
{

	ObjectStream* pOldTarget = pEMDBArray->pTargetDB;

	// Loop through list items until we find
	// an acceptable target db.
	int nCount = GetItemCount();
	bool bFound = false;
	for ( int i = 0; i < nCount && !bFound; i++ ) {

		// If this is a selected item and we haven't found
		// a candidate yet...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED ) {

			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			// If this is not a published db, set default.
			if ( !pEMDBArray->pGetDBPtr( nDBIndex )->IsPublished() ) {

				bFound = true;

				// Update the new default db status.
				SetItemText(
					i,
					1,
					CString( (LPSTR) IDS_DEFAULT_STATUS )
				);

				// Update the pTargetDB pointer.
				pEMDBArray->pTargetDB = pEMDBArray->pGetDBPtr( nDBIndex );

				// Set and save the new TargetDBID.
				pEMDBArray->TargetDBID = pEMDBArray->pGetDBPtr( nDBIndex )->GetDBID();
				pEMDBArray->SaveUserSettings();

			}

		}

	}

	if ( bFound ) {

		// Now we want to remove the target status from
		// the previous target, if it changed.
		int nDBIndex;
		bFound = false;
		if ( pOldTarget != pEMDBArray->pTargetDB )

			for ( i = 0; i < nCount && !bFound; i++ ) {

				// Get the index into Databases.
				nDBIndex = (int) GetItemData( i );

				if ( pEMDBArray->pGetDBPtr( nDBIndex ) == pOldTarget ) {

					SetItemText(
						i,
						1,
						CString( (LPSTR) IDS_WORKING_STATUS )
					);

					bFound = true;
				}

			}

	} else {

		// Whoops, no target found.
		DisplayMessage(
			IDM_TARGET_READONLY
		);

	}

}


//-------------------------------------------------------------------//
// OnAppend()																			//
//-------------------------------------------------------------------//
// This function allows moving of all the objects within
// a database to another in one step.
//
// See "Management of Objects across Earthmover Databases.doc" for
// an overview.
//-------------------------------------------------------------------//
void DBMgrListControl::OnAppend()
{

	// OLD FUNCTION!  See Gather() / CopyAndGather()
	/*
	// Get the target database.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ )
	{
		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED )
		{
			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			if ( !pEMDBArray->pGetDBPtr( nDBIndex )->IsPublished() )
			{

				// Set up an appending dlg.
				DBAppendDlg AppendDlg(
					pEMDBArray->pGetDBPtr( nDBIndex ),
					GetParent()
				);

				if ( AppendDlg.DoModal() == IDOK ) 
				{
					bRefreshTrees = true;

					// We have appended, so there are probably some db's that were
					// disconnected.  Refresh the list.
					Fill();
				}

			} else
				DisplayMessage( IDM_DB_APPEND_TO_INVALID );

			break;

		}

	}
	*/

}


//-------------------------------------------------------------------//
// OnClearCache()																		//
//-------------------------------------------------------------------//
// This function sets the status of all the caches in the selected
// db to be out of date.  The next time the db is loaded, all caches
// will be recreated.  We force a refresh of the trees so that the
// cache will be forced to be reloaded on closing of the db mgr dlg.
//-------------------------------------------------------------------//
void DBMgrListControl::OnClearCache()
{
	// Get the first selected db.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ ) {

		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED ) {

			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			pEMDBArray->pGetDBPtr( nDBIndex )->ClearCache();
			
			break;

		}

	}

	// Cause the cache to be reloaded on close of dlg.
	bRefreshTrees = true;
}


//-------------------------------------------------------------------//
// OnDBClearCache()																	//
//-------------------------------------------------------------------//
// This function sets the status of all the caches to be out of date.
// The next time the db is loaded, all caches will be recreated.  
//-------------------------------------------------------------------//
void DBMgrListControl::OnDBClearCache()
{
	EMDatabase* pDatabase = NewDatabaseFromFile( IDS_DB_CLEAR_CACHE_TITLE, false );

	if ( pDatabase ) 
	{
		CWaitCursor Stop_HammerTime;

		// If we are not already connected to the db...
		// Note that this function will clean up for us if
		// we are already connected.
		if ( !pEMDBArray->DBAlreadyConnected( pDatabase, this ) ) {

			// We can only do this for non-published databases, since it
			// will require write access.  ClearDBCache() should have been removed
			// from the menu if the db is published, ASSERT here to make sure.
			ASSERT( !pDatabase->IsPublished() );

			pDatabase->ClearCache();

			delete pDatabase;

		} else {

			DisplayMessage( IDS_REPAIR_FAILED );

		}
	}
}


//-------------------------------------------------------------------//
// OnUpdateAssociations()															//
//-------------------------------------------------------------------//
// This function updates the list of databases associated with the
// selected db's.
//-------------------------------------------------------------------//
void DBMgrListControl::OnUpdateAssociations()
{

	EMDatabase* pDB;

	// Get the first selected db.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ ) {

		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED ) {

			// Get the index into Databases.
			int nDBIndex = (int) GetItemData( i );

			pDB = pEMDBArray->pGetDBPtr( nDBIndex );

			// We can only do this for non-published databases, since it
			// will require write access.  ASSERT that it was not on the
			// menu for published db's.
			ASSERT( !pDB->IsPublished() );

			// Set up a progress dlg.
			BaseProgressDlg ProgressDlg(
				this
			);

			// Update the associations.
			pDB->UpdateAssociations( &ProgressDlg );

			// Clean up.
			ProgressDlg.DestroyWindow();

		}

	}

}



// BTree logging is for in-house use.
#ifdef ACCELERATOR_INHOUSE

//-------------------------------------------------------------------//
// OnDBBtreeLog																		//
//-------------------------------------------------------------------//
// This function creates a log file for the btree for a single object
// type in the selected database.  The type is requested from the 
// user via ObjectTypeSelectDlg.
//-------------------------------------------------------------------//
void DBMgrListControl::OnDBBtreeLog()
{

	// Get a filename.
	CString strDBName;
	if (
		pEMDBArray->GetFile(
			&strDBName,
			IDS_DB_LOG_TITLE,
			this
		) == IDOK
	) {

		CWaitCursor HoldOn;

		// Open the database.
		EMDatabase* pDatabase = new EMDatabase( &strDBName );
		if( pDatabase->Initialize() == DB_INIT_SUCCESS ) {

			// Get the desired object type.
			// These include some Persistent-derived and some EMComponent-derived objects.
			// Use a Persistent pointer to handle both cases.
			ObjectTypeSelectDlg	TypeDlg;
			TypeDlg.DoModal();

			// Get a temp object, used to create the btree log.
			EMComponent* pObject = pDatabase->NewComponent( TypeDlg.ResultType );

			// Log the mfr btree.
			pObject->LogBTree();

			delete pObject;

		} else
			DisplayMessage( IDS_LOG_FAILED );

		delete pDatabase;

	}

}


//-------------------------------------------------------------------//
// OnBtreeLog()																		//
//-------------------------------------------------------------------//
// This version creates a log for each selected db.
//-------------------------------------------------------------------//
void DBMgrListControl::OnBtreeLog()
{

	ObjectTypeSelectDlg	TypeDlg;
	bool bTypeSelected = false;

	// Get the selected databases.
	int nCount = GetItemCount();
	for ( int i = 0; i < nCount; i++ )
	{
		// If this is a selected item...
		if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED )
		{

			// Get the desired object type.
			// Be careful, the resulting type may not be EMComponent-derived.
			if ( !bTypeSelected )
				TypeDlg.DoModal();

			CWaitCursor StopStaringAtTheScreen;

			// Get the database.
			EMDatabase* pDatabase = pEMDBArray->pGetDBPtr( (int) GetItemData( i ) );

			// Get a temp object, used to create the btree log.
			// There are some Persistent-derived and some EMComponent-derived objects.
			// Use a Persistent pointer to handle both cases.
			Persistent* pObject = pDatabase->NewObject( EMDatabase::GetClassID( TypeDlg.ResultType ) );

			// Log the mfr btree.
			pObject->LogBTree();

			delete pObject;

		}
	}
}

#endif


//-------------------------------------------------------------------//
// OnKeyLog()																			//
//-------------------------------------------------------------------//
// This function dumps some diagnostic key data to a text file.
// Since it will be used for tech support over the phone/email,
// we need end users to be able to access it.
//-------------------------------------------------------------------//
void DBMgrListControl::OnKeyLog() 
{
	MainDongle.CreateDiagnosticLog();
}


#ifdef ACCELERATOR_INHOUSE

// These were needed for component-specific defines.
#include "..\Engine\Engine.h"
#include "..\DataEnumTypes.h"						// For Trans enum
#include "..\Transmission\Transmission.h"
#include "..\Axle\Axle.h"
#include "..\Tire\Tire.h"
#include "..\Chassis\RubberTiredChassis.h"
#include "..\Cab\Cab.h"
#include "..\Bucket\Bucket.h"

//-------------------------------------------------------------------//
// OnVerifyPrivateOwnership()														//
//-------------------------------------------------------------------//
// Used "in house" to verify that there are no private objects that
// do not have owners.  Such an occurance would point to something
// going wrong with the private object handling in EMComponent,
// ConfiguratorTreeControl, DBListControl, or DBTreeControl.
//
// If such an object is found, this function prompts you whether
// you want to delete it.  Go slap JWC in the back of the head if
// one of these turns up.
//-------------------------------------------------------------------//
void DBMgrListControl::OnVerifyPrivateOwnership()
{

	// DBTreeControl::VerifyPrivateOwnership( true );

	// DEBUG
	// You can slap in manual manipulations of the database here,
	// temporarily commenting out the above line.
	// Here are examples.

	/////////////////////////////
	// File-level common code.

	// Get a filename.
	CString strDBName;
	if (
		pEMDBArray->GetFile(
			&strDBName,
			IDS_DB_STUFF_TITLE,
			this
		) == IDOK
	) {

		CWaitCursor WaitAMinuteBub;

		// Open the database.
		EMDatabase* pDatabase = new EMDatabase( &strDBName );
		pDatabase->Initialize();

		BaseProgressDlg ProgressDlg( this );


	/////////////////////////////

		/*
		// --------------------------------------------------------------------------------
		// This section loops through all objects in the database, reading and writing each.
		//
		// It was used to change the encryption of a database.  The new encryption scheme
		// was coded, then Persistent::DecryptObjectData() was patched to use the old
		// encryption key.
		//

		// Loop through all the object types in the database needing conversion.
		// These include some Persistent-derived and some EMComponent-derived objects.
		// Use a Persistent pointer to handle both cases.
		// In addition, internal objects are handled by their parents; make sure we
		// skip them.
		Persistent* pTemp;
		pDatabase->Open( true );

		int nClassCount = pDatabase->GetClassCount();
		for ( int i = 0; i < nClassCount; i++ ) {

			// Skip tire deflection; it is internal, handled within tire.
			if ( i != 8 ) {

				// Update progress text.
				ProgressDlg.SetText(
					EMComponentInit::ByType(
						pDatabase->GetObjectType( i )
					)->uiDescSingular()
				);

				// Loop through all the objects of this type.
				// Get a temp object of the current type in the source database.
				pTemp = pDatabase->NewObject( i );

				// Loop through all objects.
				pTemp->FirstObject();
				while ( pTemp->ObjectExists() ) {

					pTemp->ChangeObject();
					pTemp->SaveObject();

					pTemp->NextObject();

				}

				delete pTemp;

			}

			// Update progress bar.
			ProgressDlg.UpdateProgress( 100 * ( i + 1 ) / nClassCount );

		}
		pDatabase->Close( true );

		//
		// ------------------------------------------------------------------
		*/


		/*
		// ------------------------------------------------------------------
		// This section can be used to delete a group of obsolete objects
		// from the database.
		//

		bool bResult = true;
		EMComponent* pCompound;
		for ( int ID = 102; ID < 204; ID++ ) {
			pCompound = pDatabase->NewComponent(
				OBJECT_TIRE_COMPOUND,
				&ObjectReference(
					DatabaseID(),
					ID
				)
			);

			if ( pCompound->ObjectExists() )
				pCompound->DeleteObject();
			delete pCompound;
		}

		// Clean up.
		delete pCompound;

		//
		// ------------------------------------------------------------------
		*/


		/*
		// ------------------------------------------------------------------
		// This section was used to loop through tire objects.
		// We wanted to fix damaged deflection objects.  They should have been blank.
		// We also used it to compact deflection objects, which had ten lines in each.
		// Also, to force recalcs.

		bool bResult = true;
		EMComponent* pTire;

		pTire = pDatabase->NewComponent( OBJECT_TIRE );
		pTire->FirstObject();

		ProgressDlg.SetText(
			&CString( _T("Recalc'ing and rewriting tires...") )
		);

		int n = 0;
		CString strTemp;
		while ( pTire->ObjectExists() ) {

			// Fix deflection object.
			// This will clear out any unneeded rows in tire deflection.
			// pTire->GetObjPtr( 4 )->Clear();

			// Force recalcs.
			((Tire*)pTire)->UpdateContactArea();
			pTire->bIsComplete = pTire->IsDataComplete();
			if ( pTire->bIsComplete )
				pTire->DoCalcOtherData();

			// Save it and get the next one.
			pTire->ChangeObject();
			pTire->SaveObject();
			pTire->NextObject();

			// Put up progress.
			ProgressDlg.UpdateProgress( n/10 );		// Assuming ~1000 tires.
			pTire->GetDisplayName( &strTemp );
			ProgressDlg.SetText( &strTemp );
			n++;

		}

		// Clean up.
		delete pTire;

		//
		// ------------------------------------------------------------------
		*/


		/*
		// ------------------------------------------------------------------
		// This section was used to loop through all component objects.
		// We wanted to change the format of EMComponent.  Note that internal objects
		// were not specifically included in the loop; each is loaded with its parent
		// object.
		//

		bool bResult = true;
		EMComponent* pObject;

		for ( int i = 0; i <= TYPE_TREE_LAST; i++ ) {

			pObject = pDatabase->NewComponent( (OBJECT_TYPE) i );
			pObject->FirstObject();

			while ( pObject->ObjectExists() ) {

				pObject->ChangeObject();
				pObject->SaveObject();

				pObject->NextObject();

			}

			// Clean up.
			delete pObject;

		}

		//
		// ------------------------------------------------------------------
		*/


		/*
		// ------------------------------------------------------------------
		// This section was used to delete all objects of a given type.
		// We moved Tire Deflections data into Tire, and wanted to remove
		// them (we were eliminating all internal objects and mechanisms).

		bool bResult = true;
		EMComponent* pObject;

		pObject = pDatabase->NewComponent( OBJECT_TIRE_DEFLECTION );
		pObject->FirstObject();

		while ( pObject->ObjectExists() )
		{
			pObject->DeleteObject();
			pObject->SaveObject();

			pObject->NextObject();
		}

		// Clean up.
		delete pObject;

		pObject = pDatabase->NewComponent( OBJECT_OPTION );
		pObject->FirstObject();

		while ( pObject->ObjectExists() )
		{
			pObject->DeleteObject();
			pObject->SaveObject();

			pObject->NextObject();
		}

		// Clean up.
		delete pObject;

		//
		// ------------------------------------------------------------------
		*/


		/*
		// ------------------------------------------------------------------
		// This section was used to update all material objects.
		// This section includes feedback in the progress dlg.

		// Materials needed to have DoCalcOtherData() run on each one, to recalc
		// the bank wet densities.  Rock-type materials were the actual
		// target of this change, but no harm in recalculating ALL materials.

		bool bResult = true;
		EMComponent* pObject;

		pObject = pDatabase->NewComponent( OBJECT_MATERIAL );
		pObject->FirstObject();
		int nProgress = 0;

		while ( pObject->ObjectExists() )
		{
			// Update progress.
			// We are REAL SLOPPY LIKE with the progress, just
			// incrementing by one for each object until we
			// finish or run outta progress juice.  WHO CARES!!??  woohooo!!
			ProgressDlg.SetText( pObject->strDisplayName() );
			ProgressDlg.UpdateProgress( _MIN( nProgress++, 100 ) );
		
			// Recalc bank wet densities.
			pObject->DoCalcOtherData();		
			
			pObject->ChangeObject();
			pObject->SaveObject();

			pObject->NextObject();
		}

		// Clean up.
		delete pObject;

		//
		// ------------------------------------------------------------------
		*/


		// ------------------------------------------------------------------
		// This section was used to loop through a subset of component objects.
		// We wanted to add component cost subobjects where needed.
		//

		bool bResult = true;
		EMComponent* pObject;

		int nSO, nSOStd, nSOList;
		for ( int i = 0; i <= TYPE_TREE_LAST; i++ ) 
		{
			switch ( i )
			{
				case OBJECT_ENGINE:			
					nSO		= Engine_ComponentCost;			
					nSOStd	= Engine_Std_ComponentCost;			
					nSOList	= Engine_Avail_ComponentCosts;
					break;
				case OBJECT_TRANSMISSION:
					nSO		= Trans_ComponentCost;			
					nSOStd	= Trans_Std_ComponentCost;			
					nSOList	= Trans_Avail_ComponentCosts;
					break;
				case OBJECT_AXLE:
					nSO		= Axle_ComponentCost;			
					nSOStd	= Axle_Std_ComponentCost;			
					nSOList	= Axle_Avail_ComponentCosts;
					break;
				case OBJECT_TIRE:
					nSO		= Tire_ComponentCost;			
					nSOStd	= Tire_Std_ComponentCost;			
					nSOList	= Tire_Avail_ComponentCosts;
					break;
				case OBJECT_HAULER_CHASSIS:
				case OBJECT_LOADER_CHASSIS:
					nSO		= RTC_ComponentCost;			
					nSOStd	= RTC_Std_ComponentCost;			
					nSOList	= RTC_Avail_ComponentCosts;
					break;
				case OBJECT_CAB:
					nSO		= Cab_ComponentCost;			
					nSOStd	= Cab_Std_ComponentCost;			
					nSOList	= Cab_Avail_ComponentCosts;
					break;
				case OBJECT_BUCKET:
					nSO		= Bucket_ComponentCost;			
					nSOStd	= Bucket_Std_ComponentCost;			
					nSOList	= Bucket_Avail_ComponentCosts;
					break;
				default: continue;
			}

			pObject = pDatabase->NewComponent( (OBJECT_TYPE) i );
			pObject->FirstObject();

			while ( pObject->ObjectExists() ) 
			{

				if ( pObject->GetListSize( nSOList ) <= 0 )
				{
					// Create a new cost subobject in the target database.
					EMComponent* pCost = pDatabase->NewComponent( OBJECT_COMPONENT_COST );
					pCost->SetPrivate( true );
					pCost->AddObject();
					pCost->SaveObject();

					// Now stuff it as a subobject.
					pObject->SubObjList_AddRef( nSOList, &pCost->GetRef(), true );

					// Now select it for the current and standard slots.
					pObject->SetSubObjRef(
						nSO,
						&pCost->GetRef(),
						true
					);
					pObject->SetSubObjRef(
						nSOStd,
						&pCost->GetRef(),
						true
					);

					pObject->ChangeObject();
					
					// This does not work with non-connected databases, it seems.
					/*
					pObject->ValidateAndSave(
						false,
						false,
						NULL,
						false
					);
					*/

					pObject->SaveObject();

					pObject->NextObject();

				}
			}

			// Clean up.
			delete pObject;

		}

		//
		// ------------------------------------------------------------------


	////////////////////////////////
	// File-level common code.

		// Clean up.
		delete pDatabase;
		ProgressDlg.DestroyWindow();

	}

	bRefreshTrees = false;

	//
	////////////////////////////////


	// ------------------------------------------------------------------

	// This creates a permissions file with the specified permissions.
	// MainDongle.WritePermissionsFile();

	// ------------------------------------------------------------------

}



#endif	// ACCELERATOR_INHOUSE

