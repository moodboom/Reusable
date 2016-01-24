// DatabaseManagerDlg.cpp : implementation file
//

#include "stdafx.h"

#include "EMDBArray.h"

#include "..\DisplayMessage.h"

#include "..\SplashScreenDlg.h"				// Needed when reinitializing trees.

#include "..\MainFrame.h"						// For pointer to parent.

#include "DatabaseManagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DatabaseManagerDlg dialog

// This class uses a DBMgrListControl.  It does most of the work.  This
// class contains the list and passes commands like right-click on to it.

// Consider:
// This class hierarchy can be reorganized so that we use a generic
// "RightClickListControl".  We should provide it with information regarding
// the right click menus to use.  It then merely displays the menus, and
// fires the resulting commands back to the parent.
// A potential problem with this is if we have to customize the menu on
// the fly, we end up having to derive a class from RCLC, and RCLC doesn't
// have a whole lot of functionality.  But it's better than duplicate code,
// so it may be worth it.  
// Perhaps RCLC should be incorporated into the OLE/DBListControl
// hierarchy as well, although in that case we want to handle the commands
// in the derived class, not the parent.  hmmmm, there must be a better way....


//-------------------------------------------------------------------//
// DatabaseManagerDlg()																//
//-------------------------------------------------------------------//
DatabaseManagerDlg::DatabaseManagerDlg() 
: 
	
	// Call base class.
	inherited(
		DatabaseManagerDlg::IDD, 
		theFrame
	),

	bOldListMsg( false )

{
	//{{AFX_DATA_INIT(DatabaseManagerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


//-------------------------------------------------------------------//
// DoDataExchange()																	//
//-------------------------------------------------------------------//
void DatabaseManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DatabaseManagerDlg)
	DDX_Control(pDX, IDC_DATABASE_LIST, DBList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DatabaseManagerDlg, CDialog)
	//{{AFX_MSG_MAP(DatabaseManagerDlg)
	ON_COMMAND(IDC_RIGHT_CLICK_KEY, OnRightClickKey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// DatabaseManagerDlg message handlers


//-------------------------------------------------------------------//
// OnInitDialog()																		//
//-------------------------------------------------------------------//
BOOL DatabaseManagerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Calc the column widths.
	CRect ListRect;
	DBList.GetWindowRect( &ListRect );
	int nNameWidth = ListRect.Width() / 2;				//       1/2
	int nStatusWidth = ListRect.Width() / 6;			//			1/6
	int nFilenameWidth = ListRect.Width() * 5 / 4;	//			5/4

	// Add the list columns.
	DBList.InsertColumn( 0, _T("Database Name"), LVCFMT_LEFT, nNameWidth			);
	DBList.InsertColumn( 1, _T("Status"),			LVCFMT_LEFT, nStatusWidth		);
	DBList.InsertColumn( 2, _T("Filename"),		LVCFMT_LEFT, nFilenameWidth	);

	// Fill the database list using the database manager.
	DBList.Fill();
	
	// Manually attach our accelerator, as explained in MSJ Volume 12 Number 7, C++ Q & A.  
	// Search for "CDialog near accelerator" to find it.
	hAccelerators = ::LoadAccelerators(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE( IDA_DB_MGR_TREE )
	);
	ASSERT( hAccelerators );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

}


//-------------------------------------------------------------------//
// OnOK()																				//
//-------------------------------------------------------------------//
// This function exits the db manager.  In doing so, we may need
// to refresh all the tree contents.
//-------------------------------------------------------------------//
void DatabaseManagerDlg::OnOK() 
{
	
	// Press the button and close the window.
	CDialog::OnOK();

	if ( DBList.bRefreshTrees ) {

		// Get a progress screen.
		SplashScreenDlg FillProgressDlg(
			this,
			IDD_DB_REFRESH_SCREEN
		);

		// Refresh the trees.
		pEMDBArray->FillTrees( &FillProgressDlg );

		// Clean up.
		FillProgressDlg.DestroyWindow();
		
	}
	
}


//-------------------------------------------------------------------//
// OnCmdMsg()																			//
//-------------------------------------------------------------------//
BOOL DatabaseManagerDlg::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{

	// We want to keep the message routing alive until done,
	// so return FALSE by default.
	BOOL bReturn = FALSE;

	if ( !bOldListMsg ) {
	
		bOldListMsg = true;
		
		// We give the message a shot...
		bReturn  = CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

		if (
				!bReturn								// Not handled
			&&	( nID & 0x8000 )					// In command range
			&& ( nID < 0xF000 )					// Not a system command
			&& ( nCode == CN_COMMAND )			// Has command code
			&& ( DBList.GetSafeHwnd() )		// We have a list control
		)

			// Give the list control a shot at it.
			bReturn = DBList.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );

	}
	
	bOldListMsg = false;

	return bReturn;

}


//-------------------------------------------------------------------//
// OnRightClickKey()																	//
//-------------------------------------------------------------------//
void DatabaseManagerDlg::OnRightClickKey() 
{
	DBList.OnRightClickKey();	
}


//-------------------------------------------------------------------//
// PreTranslateMessage()															//
//-------------------------------------------------------------------//
// This was overridden to provide accelerator handling.
// See MSJ Volume 12 Number 7, C++ Q & A.  
// Search for "CDialog near accelerator" in MSDN to find it.
//-------------------------------------------------------------------//
BOOL DatabaseManagerDlg::PreTranslateMessage(MSG* pMsg) 
{
	
	if (
			pMsg->message >= WM_KEYFIRST 
		&& pMsg->message <= WM_KEYLAST
	) {

		// translate using accelerator table
		ASSERT( hAccelerators );
		return ::TranslateAccelerator( m_hWnd, hAccelerators, pMsg );

	}
	
	return CDialog::PreTranslateMessage(pMsg);

}

