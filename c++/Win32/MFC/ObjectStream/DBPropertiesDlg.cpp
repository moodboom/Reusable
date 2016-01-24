// DBPropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\earthmover.h"

#include "..\DisplayMessage.h"

#include "..\UnicodeConversions.h"	// For CString->wstring.

#include "EMDatabase.h"					// For uhCurrentEMFileVersion

#include "EMDBArray.h"				// For GetDisplayDBID()

#include "DBAssociations.h"		// We display them here.

#include "DBPropertiesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DBPropertiesDlg dialog


DBPropertiesDlg::DBPropertiesDlg(
	EMDatabase*	pNewDatabase,
	CWnd*			pParent
) :

	// Call base class.
	inherited(DBPropertiesDlg::IDD, pParent),

	// Init vars.
	pDatabase( pNewDatabase )

{
	//{{AFX_DATA_INIT(DBPropertiesDlg)
	strFilename = _T("");
	strName = _T("");
	strStatus = _T("");
	strVersion = _T("");
	strAuthor = _T("");
	strGroupOrCompacted = _T("");
	//}}AFX_DATA_INIT

	/////////////////////////////////////
	// DEBUG
	//
	/*
	#ifdef ACCELERATOR_INHOUSE

		// Fix olly's header if both shift and ctrl are pressed.
		// Make sure it isn't read-only!
		if (		GetKeyState( VK_SHIFT   ) < 0
				&&	GetKeyState( VK_CONTROL ) < 0	) {

			pDatabase->Open(true);

			// This was used by itself, since all we wanted to do was patch
			// the user number to match the one used in the database refs.
			// Olly's user number got changed on her machine during fixing
			// of her dongle, oops.
			pDatabase->DBHeader.DBID.User.UserNumber =	0xecfb46e4;
			*/

			/*
			wcscpy( pDatabase->UserName,	L"Olivia Pugh" );
			wcscpy( pDatabase->Company,	L"Accelerator, Inc." );
			wcscpy( pDatabase->Contact,	L"(941) 277-5177;accel@gate.net" );
			wcscpy( pDatabase->DBHeader.DBName,				L"Master" );
			pDatabase->DBHeader.DBID.User.Group =			MasterGroup;
			pDatabase->DBHeader.DBID.DBNumber =				0;
			
			// Update this as needed with the lastest version.
			pDatabase->DBHeader.DBID.DBVersion =			4;
			*/

			/*
			// DON'T CHANGE THIS unless you reaally need to - all objects
			// are encrypted using this seed.
			// pDatabase->EMHeader.ObjectSeed =				0x77ff;

			pDatabase->WriteHeader();
			pDatabase->Close(true);

		}


	#endif
	*/
	//
	/////////////////////////////////////

	
	// Init vars.

	// All databases have names, an author, and
	// the same executable-database version message, if needed.
	
	// DB Name.
	strFilename = *pDatabase->GetFilename();
	strName = pDatabase->GetDatabaseName();
	
	// Author.
	pEMDBArray->BuildUserBlock(
		strAuthor,
		(wchar_t*) pDatabase->UserName,
		(wchar_t*) pDatabase->Company,
		(wchar_t*) pDatabase->Contact,
		pDatabase->GetDBID().UserNumber
	);

	// Executable-db version message.
	uHuge uhDBVersion;
	uhDBVersion = max(
		pDatabase->DBHeader.FileVersionCreated,
		pDatabase->FileVersionLastModified
	);
	if ( uhDBVersion < uhCurrentEMFileVersion )
		strStatus.LoadString( IDS_DB_OLD_VERSION );
	else if ( uhDBVersion > uhCurrentEMFileVersion )
		strStatus.LoadString( IDS_DB_NEW_VERSION );
	else
		strStatus = _T("");


	// The remainder of the property text is publish-status-dependant.
	if ( pDatabase->IsPublished() ) {

		strVersion.Format(
			IDS_DB_PUBLISHED_STATUS,
			pDatabase->DBHeader.DBID.DBNumber,
			pDatabase->DBHeader.DBVersion
		);

		MainDongle.GetGroupName(
			pDatabase->DBHeader.Group,
			&strGroupOrCompacted
		);
		
	} else {

		if ( pDatabase->DBHeader.DBVersion == 0 )
			strVersion.Format( 
				IDS_DB_WORKING_STATUS_NO_PUB,
				pDatabase->DBHeader.DBID.DBNumber
			);
		else
			strVersion.Format(
				IDS_DB_WORKING_STATUS,
				pDatabase->DBHeader.DBID.DBNumber,
				pDatabase->DBHeader.DBVersion
			);

		// Compaction.
		strGroupOrCompacted.Format( 
			_T("%d%% compacted."), 
			pDatabase->GetPercentCompacted() 
		);

	}

}


void DBPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DBPropertiesDlg)
	DDX_Control(pDX, IDC_ASSOCIATED_DBS, DBAssocList);
	DDX_Text(pDX, IDC_DB_FILENAME, strFilename);
	DDX_Text(pDX, IDC_DB_NAME, strName);
	DDX_Text(pDX, IDC_DB_STATUS, strStatus);
	DDX_Text(pDX, IDC_DB_VERSION, strVersion);
	DDX_Text(pDX, IDC_DB_AUTHOR, strAuthor);
	DDX_Text(pDX, IDC_DB_GROUP_OR_COMPACTED, strGroupOrCompacted);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DBPropertiesDlg, CDialog)
	//{{AFX_MSG_MAP(DBPropertiesDlg)
	ON_BN_CLICKED(IDC_GET_ASSOC, OnGetAssoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DBPropertiesDlg message handlers

//-------------------------------------------------------------------//
// OnInitDialog()																		//
//-------------------------------------------------------------------//
// We set up the label strings and other dlg variables in the
// constructor, before they were displayed.  Here, we set up
// the controls now that they are created.
//-------------------------------------------------------------------//
BOOL DBPropertiesDlg::OnInitDialog() 
{
	
	CDialog::OnInitDialog();
	
	if ( pDatabase->IsPublished() ) {

		// Initialize the associated databases list.
		FillAssocList();

	} else {

		// Determine if this is the author of the db; 
		// turn off read-only on the db name if so.
		if ( pDatabase->DBHeader.DBID.UserNumber == MainDongle.GetUserNumber() ) {
			CEdit* pName = (CEdit*) GetDlgItem( IDC_DB_NAME );
			pName->SetReadOnly( TRUE );
		}		

		// Reset the visible controls.
		CWnd* pControl = GetDlgItem( IDC_GROUP_LABEL );
		pControl->ShowWindow( SW_HIDE );
		pControl = GetDlgItem( IDC_ASSOCIATED_DBS );
		pControl->ShowWindow( SW_HIDE );
		pControl = GetDlgItem( IDC_ASSOCIATED_DBS_LABEL );
		pControl->ShowWindow( SW_HIDE );
		pControl = GetDlgItem( IDC_GET_ASSOC );
		pControl->ShowWindow( SW_SHOW );

	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//-------------------------------------------------------------------//
// OnGetAssoc()																		//
//-------------------------------------------------------------------//
// This function requests the associations for the database from
// the database itself.  The results are then displayed in the 
// list control.
//-------------------------------------------------------------------//
void DBPropertiesDlg::OnGetAssoc() 
{

	// Get associations list for this database.
	DBAssociations* pNewAssoc = new DBAssociations( 
		pDatabase
	);
	BaseProgressDlg ProgressDlg( this );
	pNewAssoc->Extract( &ProgressDlg );
	ProgressDlg.DestroyWindow();
	delete pNewAssoc;

	// Reset the visible controls and resize.
	CWnd* pControl = GetDlgItem( IDC_GROUP_LABEL );
	pControl->ShowWindow( SW_SHOW );
	pControl = GetDlgItem( IDC_ASSOCIATED_DBS );
	pControl->ShowWindow( SW_SHOW );
	pControl = GetDlgItem( IDC_ASSOCIATED_DBS_LABEL );
	pControl->ShowWindow( SW_SHOW );
	pControl = GetDlgItem( IDC_GET_ASSOC );
	pControl->ShowWindow( SW_HIDE );

	// Fill the list.
	FillAssocList();

}


//-------------------------------------------------------------------//
// FillAssocList()																	//
//-------------------------------------------------------------------//
// This function fills the associations list using the database.
// It is called right away for published db's, and on request for
// working db's.
//-------------------------------------------------------------------//
void DBPropertiesDlg::FillAssocList()
{

	CRect ListRect;
	DBAssocList.GetWindowRect( &ListRect );
	int nNameWidth = ListRect.Width()		* 30 / 100;
	int nUserWidth = ListRect.Width()		* 25 / 100;
	int nCompanyWidth = ListRect.Width()	* 16 / 100;
	int nPhoneWidth = ListRect.Width()		* 20 / 100;
	int nPercentageWidth = ListRect.Width()*  8 / 100;
	int nIDWidth = ListRect.Width()			* 20 / 100;

	DBAssocList.InsertColumn( 0, _T("Name"),		LVCFMT_LEFT, nNameWidth			);
	DBAssocList.InsertColumn( 1, _T("Author"),	LVCFMT_LEFT, nUserWidth			);
	DBAssocList.InsertColumn( 2, _T("Company"),	LVCFMT_LEFT, nCompanyWidth		);
	DBAssocList.InsertColumn( 3, _T("Contact"),	LVCFMT_LEFT, nPhoneWidth		);
	DBAssocList.InsertColumn( 4, _T("%"),			LVCFMT_LEFT, nPercentageWidth	);
	DBAssocList.InsertColumn( 5, _T("ID"),			LVCFMT_LEFT, nIDWidth			);

	// Now loop through all known associations.
	DBAssociations DBA( pDatabase );
	int nAssocCount = 0;
	if ( DBA.ObjectExists() ) {
	
		nAssocCount = DBA.Associations.GetSize();

		for ( int i = 0; i < nAssocCount; i++ ) {
	
			CString strTemp;
			
			// Add the database to the list.
			int NewItem = DBAssocList.InsertItem( 
				0, 
				CString( DBA.Associations[i]->DBName )
			);
			ASSERT( NewItem != -1 );
			
			// Author text.
			DBAssocList.SetItemText( 
				NewItem, 
				1, 
				CString( DBA.Associations[i]->UserName )
			);

			// Company text.
			DBAssocList.SetItemText( 
				NewItem, 
				2, 
				CString( DBA.Associations[i]->Company )
			);

			// Contact text.
			DBAssocList.SetItemText( 
				NewItem, 
				3, 
				CString( DBA.Associations[i]->Contact )
			);

			// Percentage text.
			strTemp.Format(
				"%d",
				DBA.Associations[i]->nPercentage
			);
			DBAssocList.SetItemText( 
				NewItem, 
				4, 
				strTemp
			);
	
			// ID text.
			strTemp = pEMDBArray->GetDisplayDBID(
				&( DBA.Associations[i]->DBID )
			);
			DBAssocList.SetItemText( 
				NewItem, 
				5, 
				strTemp
			);

		}

	}

	// Add "none" string if none found.
	if ( nAssocCount == 0 )
		DBAssocList.InsertItem( 0, CString( (LPSTR) IDS_DB_ASSOC_NONE_FOUND ) );


}


//-------------------------------------------------------------------//
// OnOK()																				//
//-------------------------------------------------------------------//
// This function processes any requested name change.
//-------------------------------------------------------------------//
void DBPropertiesDlg::OnOK() 
{

	CDialog::OnOK();

	// We may have a new name, process it here.
	CString strOrigName = pDatabase->GetDatabaseName();
	if ( strOrigName != strName ) {
	
		// Place a warning message about changing the database name.
		if ( DisplayMessage( IDM_DB_RENAME_VERIFY ) == IDYES ) {
		
			// Update the name.
			wstring wsNewName;
			CopyCStringToWstring(
				&strName,
				&wsNewName
			);
			wcsncpy(
				pDatabase->DBHeader.DBName,
				wsNewName.c_str(),
				sizeof pDatabase->DBHeader.DBName / sizeof wchar_t
			);
			pDatabase->Open(true);
			pDatabase->WriteHeader();
			pDatabase->Close(true);
		
		}
	
	}

}
