// DatabaseCreateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMDBArray.h"
#include "DatabaseCreateDlg.h"
#include "..\FileHelpers.h"				// For bFileExists()
#include "..\DisplayMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DatabaseCreateDlg dialog


//-------------------------------------------------------------------//
// DatabaseCreateDlg()																//
//-------------------------------------------------------------------//
DatabaseCreateDlg::DatabaseCreateDlg(
	CWnd*					pParent
) : 
	// Call base class.
	inherited(DatabaseCreateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(DatabaseCreateDlg)
	strFilename = _T("");
	strDatabaseName = _T("");
	//}}AFX_DATA_INIT
}


//-------------------------------------------------------------------//
// DoDataExchange()																	//
//-------------------------------------------------------------------//
void DatabaseCreateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DatabaseCreateDlg)
	DDX_Text(pDX, IDC_DB_FILENAME, strFilename);
	DDX_Text(pDX, IDC_DB_NAME, strDatabaseName);
	DDV_MaxChars(pDX, strDatabaseName, 42);
	//}}AFX_DATA_MAP
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(DatabaseCreateDlg, CDialog)
	//{{AFX_MSG_MAP(DatabaseCreateDlg)
	ON_BN_CLICKED(IDC_DB_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DatabaseCreateDlg message handlers


//-------------------------------------------------------------------//
// OnBrowse()																			//
//-------------------------------------------------------------------//
void DatabaseCreateDlg::OnBrowse() 
{
	// Get a filename.
	CString strNewFilename;
	if ( 
		pEMDBArray->GetFile(
			&strNewFilename,			// Result
			IDS_DB_CREATE_TITLE,		// Title ID
			this,							// pParent
			false,						// bIncludePublished
			0								// Do not specify OFN_FILEMUSTEXIST.
		) == IDOK 
	) {
		// Copy the filename and refresh.
		strFilename = strNewFilename;
		UpdateData( FALSE );					// Update the dialog.
	}
}


//-------------------------------------------------------------------//
// OnOK()																				//
//-------------------------------------------------------------------//
void DatabaseCreateDlg::OnOK() 
{
	// Update our variables.
	UpdateData( TRUE );
	
	// Strip off any extension added by the user
	// and punch it back out to the dlg.
	StripExtension( &strFilename );
	UpdateData( FALSE );
	
	// Get the database path.
	CString strDBPath;
	pEMDBArray->GetDatabasePath( &strDBPath );

	// Make sure a path doesn't exist on the filename.
	if ( ExtractFileNameFromPath( &strFilename, &strFilename ) ) 
	{
		// Put up a message.
		DisplayMessage( IDM_DB_CREATE_NO_PATH, IDL_WARNING, this );

		// Repost the filename without the path.
		UpdateData( FALSE );

	// If ANY of these exist, there WILL be problems, eventually...
	} else if ( 
			bFileExists( strDBPath + strFilename + tWorkingDBExt		)
		||	bFileExists( strDBPath + strFilename + tIndexExt			)
		||	bFileExists( strDBPath + strFilename + tPublishedDBExt	)
	) {
		// Try again.
		DisplayMessage( IDM_DB_CREATE_NAME_EXISTS, IDL_WARNING, this );

	// Make sure we have a filename and database name.
	} else if (
			strFilename.IsEmpty()
		|| strDatabaseName.IsEmpty()
	
	) {
		// Try again.
		DisplayMessage( IDM_DB_NO_NAME, IDL_WARNING, this );
	
	} else
	{
		// This will fill the vars from the dialog.
		CDialog::OnOK();

		// Build up the final filename.
		strFilename = strDBPath + strFilename + tWorkingDBExt;
	}
}
