// DBAppendDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMDBArray.h"
#include "..\BaseProgressDlg.h"
#include "..\DisplayMessage.h"
#include "DBAppendDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DBAppendDlg dialog


DBAppendDlg::DBAppendDlg(
	EMDatabase*	pNewTargetDatabase,
	CWnd*			pParent
) : 

	// Call base class.
	inherited(
		DBAppendDlg::IDD, 
		pParent
	),

	// Init vars.
	pTargetDatabase( pNewTargetDatabase )

{
	//{{AFX_DATA_INIT(DBAppendDlg)
	strDBFilename = _T("");
	strDBName = _T("");
	//}}AFX_DATA_INIT
}


void DBAppendDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DBAppendDlg)
	DDX_Control(pDX, IDC_APPEND_FROM_LIST, FromList);
	DDX_Text(pDX, IDC_APPEND_TO_DB_FILENAME, strDBFilename);
	DDX_Text(pDX, IDC_APPEND_TO_DB_NAME, strDBName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DBAppendDlg, CDialog)
	//{{AFX_MSG_MAP(DBAppendDlg)
	ON_BN_CLICKED(IDC_APPEND_ADD, OnAdd)
	ON_BN_CLICKED(IDC_APPEND_DEL, OnDel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DBAppendDlg message handlers


//-------------------------------------------------------------------//
// OnInitDialog()																		//
//-------------------------------------------------------------------//
BOOL DBAppendDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the database name text.
	// Here is an example of converting a wchar_t*
	// to a TCHAR*.  We let the temp CString object
	// do the bulk of the work.  It can accept
	// a wchar_t* pointer in its constructor, and
	// does the conversion to TCHAR for us.
	strDBName = CString(
		pTargetDatabase->GetDatabaseName()
	);
	
	// Filename text.
	strDBFilename = *pTargetDatabase->GetFilename();

	// Calc the column widths.
	CRect ListRect;
	FromList.GetWindowRect( &ListRect );
	int nNameWidth = ListRect.Width() / 2;				//       1/2
	int nFilenameWidth = ListRect.Width();				//			1

	// Add the list columns.
	FromList.InsertColumn( 0, _T("Database Name"), LVCFMT_LEFT, nNameWidth );
	FromList.InsertColumn( 1, _T("Filename"), LVCFMT_LEFT, nFilenameWidth );

	// Now update the dlg from our vars.
	UpdateData( FALSE );
			
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//-------------------------------------------------------------------//
// OnAdd()																				//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void DBAppendDlg::OnAdd() 
{

	// Get a filename.
	CString strDBName;
	if ( 
		pEMDBArray->GetFile(
			&strDBName,
			IDS_APPEND_FROM_TITLE,
			this,
			true								// bIncludePublished
		) == IDOK 
	) {

		// This will take a minute to load the request.
		CWaitCursor WaitAMinuteBub;
	
		// TO DO
		// Loop through all selected databases if we
		// convert to a multi-select file dialog.
		// for ( int i = 0; i < nFileCount; i++ ) {

		EMDatabase* pDatabase;
		
		int NewItem;
		
		// Open the database.
		pDatabase = new EMDatabase( &strDBName );
		
		// Verify that the database is valid.
		// Note that we now allow append from published.
		if ( 
				pDatabase->Initialize() == DB_INIT_SUCCESS
		//	&& !pDatabase->IsPublished() 
		) {

			SourceDatabases.Add( pDatabase );

			// Database name text.
			// Here is an example of converting a wchar_t*
			// to a TCHAR*.  We let the temp CString object
			// do the bulk of the work.  It can accept
			// a wchar_t* pointer in its constructor, and
			// does the conversion to TCHAR for us.
			CString TempName(
				pDatabase->GetDatabaseName()
			);
			NewItem = FromList.InsertItem( 
				LVIF_TEXT | LVIF_PARAM,
				0, 
				LPCTSTR( TempName ),
				0,
				0,
				0,
				(LPARAM) pDatabase
			);
			ASSERT( NewItem != -1 );
			
			// Filename text.
			FromList.SetItemText( 
				NewItem, 
				1, 
				LPCTSTR( *pDatabase->GetFilename() ) 
			);

		} else {
		
			DisplayMessage( IDM_DB_APPEND_TO_FAILURE );

			delete pDatabase;
		
		}

	}
		
}


//-------------------------------------------------------------------//
// OnDel()																				//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void DBAppendDlg::OnDel() 
{

	int nCount = FromList.GetItemCount();
	for ( int i = 0; i < nCount; i++ ) 
	{
		// If this is a selected item...
		if ( FromList.GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED )
		{
			// Get the pointer.
			EMDatabase* pDB = (EMDatabase*) FromList.GetItemData( i );

			// Remove it from the db array.
			int nDBCount = SourceDatabases.GetSize();
			for ( int j = 0; j < nDBCount; j++ )
			{
				if ( SourceDatabases[j] == pDB )
				{
					delete pDB;
					SourceDatabases.RemoveAt( j );
					break;
				}
			}

			// Make sure we found the database.
			ASSERT( j < nDBCount );
			
			// Remove it.
			FromList.DeleteItem( i );

		}
	
	}
	
}


//-------------------------------------------------------------------//
// OnOK()																				//
//-------------------------------------------------------------------//
void DBAppendDlg::OnOK() 
{

	int nSourceCount = SourceDatabases.GetSize();

	if ( pTargetDatabase && nSourceCount > 0 ) {
	
		// This will take a while.
		CWaitCursor WaitAMinuteBub;
		
		// Perform the append now.
		// Let the user know of the progress.
		// First, we want to create a modeless dlg with a progress
		// meter (ProgressDlg is made modeless in its 
		// constructor).  Then we can call the database manager to do
		// the append, giving it a pointer to the progress meter 	
		// so it can update the meter as it goes.
		BaseProgressDlg ProgressDlg(
			this,
			IDS_APPEND_IN_PROGRESS
		);

		// We call the target database's Append function.
		pTargetDatabase->Append(
			&SourceDatabases,
			&ProgressDlg
		);

		// Now destroy the progress meter.
		ProgressDlg.DestroyWindow();

		// Delete the database objects.
		delete pTargetDatabase;
		for ( int i = 0; i < nSourceCount; i++ )
			delete SourceDatabases[i];

		CDialog::OnOK();

	} else 

		DisplayMessage( IDM_APPEND_NEED_DB );

}


