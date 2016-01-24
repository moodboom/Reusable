// SuperProgressDlg.cpp : implementation file
//

#include "stdafx.h"

// DEBUG
// TO DO
// Update this class to allow bitmaps to be specified
// on-the-fly.
#include "..\HangTheDJ\resource.h"

#include "SuperProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-------------------------------------------------------------------//
// SuperProgressDlg()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
SuperProgressDlg::SuperProgressDlg( 
	CWnd* pParent,
	UINT	TextID,
	UINT	DialogID,
	int	nRespondTimeInMS,
	UINT	IconID,
	bool	bCalledFromDerivedClass
) :

	// Call base class
	BaseProgressDlg(
		pParent,               
		TextID,                
		DialogID,              
		nRespondTimeInMS,      
		IconID,                
		true						// bCalledFromDerivedClass
	)

	// Init vars.

{
	//{{AFX_DATA_INIT(SuperProgressDlg)
	//}}AFX_DATA_INIT

	//////////////////////////////////////////////////////////////
	// DERIVED CLASS NOTE
	//////////////////////////////////////////////////////////////
	// Create a modeless dlg now, if there is no derived class.
	//
	// NOTE: If there is a class derived from this one, it must 
	// make the call to Create(), since we don't know about its
	// members here, and they therefore won't get initialized
	// in Create() if we call it here.
	//
	// This block of code, along with the bCalledFromDerivedClass
	// parameter, (or something similar) must be used in all classes 
	// derived from this one, if you want to save the user of the
	// class from having to make a separate Create() call.
	//
	if ( !bCalledFromDerivedClass )
	{
		// Create!  This actually creates the controls as well, so
		// don't access them unless we call this first.
		Create( 
			DialogID? DialogID : IDD_PROGRESS_STD, 
			pParent 
		);

		// Set the text if requested.
		if ( TextID != 0 )
			SetText( TextID );
		
		// Make sure the progress ctrl is at zero.
		ProgressMeter.SetPos( 0 );
	}
	//
	//////////////////////////////////////////////////////////////

}


//-------------------------------------------------------------------//
// OnInitDialog()																		//
//-------------------------------------------------------------------//
// Here, we need to set up the super progress ctrl.
//-------------------------------------------------------------------//
BOOL SuperProgressDlg::OnInitDialog() 
{
	inherited::OnInitDialog();
	
	CSuperProgressCtrl::RegisterClass();

	m_SuperCtrl.Create(
		this, 
		10, 10, 
		LoadBitmap(
			AfxGetInstanceHandle(),
			MAKEINTRESOURCE(
				IDB_PROGRESS1
			)
		), 
		0
	);
	m_SuperCtrl.SetFillStyle( SP_FILL_HORZGRAD );
	m_SuperCtrl.SetColours(
		RGB(0,0,255), 
		RGB(255,0,0)
	);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//-------------------------------------------------------------------//
// ~SuperProgressDlg()																//
//-------------------------------------------------------------------//
SuperProgressDlg::~SuperProgressDlg()
{
}


//-------------------------------------------------------------------//
// UpdateProgress()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void SuperProgressDlg::UpdateProgress( int nNewProgress )
{
	// The base class sets the position and checks for msgs.
	// Let the progress ctrl do its checking too.
	inherited::UpdateProgress( nNewProgress );
	m_SuperCtrl.SetPos( nNewProgress );
	m_SuperCtrl.MessageLoop();
}


//-------------------------------------------------------------------//
// DoDataExchange()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void SuperProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	inherited::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SuperProgressDlg)
	//}}AFX_DATA_MAP
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(SuperProgressDlg, inherited)
	//{{AFX_MSG_MAP(SuperProgressDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SuperProgressDlg message handlers

