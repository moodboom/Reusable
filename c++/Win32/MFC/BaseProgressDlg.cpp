//-------------------------------------------------------------------//
// BaseProgressDlg.cpp : implementation file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

// #include "BaseApp.h"					// For pBaseApp->PurgeMessages()

#include "BaseProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////
// Globals Statics Constants												//
//

// Make sure all timer IDs are non-zero and unique.
#define TIMER_PURGE_MSGS				1

//
/////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------//
// BaseProgressDlg()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BaseProgressDlg::BaseProgressDlg( 
	CWnd*		pParent,
	CWinApp*	pParentApp,
	UINT		TextID,
	UINT		DialogID,
	int		nRespondTimeInMS,
	UINT		IconID,
	bool		bCalledFromDerivedClass
) :

	// Call base class.
	inherited( DialogID ),

	// Init vars.
	m_nRespondTimeInMS	( nRespondTimeInMS	),
	m_nTextUpdateStepSize( 1						),
	m_hIcon					( 0						),
	m_pParentApp			( pParentApp			)

{
	//{{AFX_DATA_INIT(BaseProgressDlg)
	strProgressText = _T("");
	//}}AFX_DATA_INIT

	// Load any specified icon.
	// This allows us to display the icon if the progress is 
	// a main window under the desktop.
	if ( IconID )
		m_hIcon = AfxGetApp()->LoadIcon( IconID );

	
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
			// DialogID? DialogID : IDD_PROGRESS_STD,		// MDM DO NOT USE with FontDlg
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
// Overridden to set the program icon.
// We fire up the msg purge timer here, too.
//-------------------------------------------------------------------//
BOOL BaseProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ProgressMeter.SetTextFormat( _T("%d%%"), PBS_SHOW_PERCENT);  // also can use SetShowPercent()
	
	COLORREF clrStart	= RGB( 0, 0, 128 );		// Dark blue
	COLORREF clrEnd	= RGB( 0, 0, 255 );		// Blue

	ProgressMeter.SetGradientColors(clrStart, clrEnd);
	ProgressMeter.Invalidate();


	if ( m_hIcon )
	{
		// Set the icon for this dialog.  The framework does this automatically
		// when the application's main window is not a dialog.
		SetIcon( m_hIcon, TRUE	);		// Set big icon
		SetIcon( m_hIcon, FALSE );		// Set small icon
	}
	
	// Set a timer that will process paint messages.
	// NOTE: We figured that now that the dlg is created, it should be 
	// safe to start this up.  If you have troubles, perhaps
	// move this to OnPaint() or something...
	SetTimer( TIMER_PURGE_MSGS, m_nRespondTimeInMS, NULL );
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//-------------------------------------------------------------------//
// DestroyWindow()																	//
//-------------------------------------------------------------------//
// Clean up here.
//-------------------------------------------------------------------//
BOOL BaseProgressDlg::DestroyWindow() 
{
	// Kill our message-processing timer.
	KillTimer( TIMER_PURGE_MSGS );
	
	return CDialog::DestroyWindow();
}


//-------------------------------------------------------------------//
// ~BaseProgressDlg()																//
//-------------------------------------------------------------------//
BaseProgressDlg::~BaseProgressDlg()
{
}


//-------------------------------------------------------------------//
// SetText()																			//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void BaseProgressDlg::SetText( CString& strText )
{
	strProgressText = strText;
	UpdateData( FALSE );					// Update the dialog.
}


//-------------------------------------------------------------------//
// UpdateProgress()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void BaseProgressDlg::UpdateProgress( int nNewProgress )
{
	ProgressMeter.SetPos( nNewProgress );
	PurgeMessages();
}


//-------------------------------------------------------------------//
// UpdateProgressAndDisplayAsText()												//
//-------------------------------------------------------------------//
// This function allows us to do message strings that include 
// the progress number.  It displays the current progress value
// in the text string using the format specified in m_strFormat.
//-------------------------------------------------------------------//
void BaseProgressDlg::UpdateProgressAndDisplayAsText( int nNewProgress )
{
	// Make sure you specified a format string with at least "%d" in it.
	ASSERT( m_strFormat.GetLength() > 1 );

	if ( nNewProgress % m_nTextUpdateStepSize == 0 )
	{
		CString strText;
		strText.Format( m_strFormat, nNewProgress );
		SetText( strText );
	}

	UpdateProgress( nNewProgress );
}


//-------------------------------------------------------------------//
// DoDataExchange()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void BaseProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BaseProgressDlg)
	DDX_Control(pDX, IDC_PROGRESS_METER, ProgressMeter);
	DDX_Text(pDX, IDC_PROGRESS_TEXT, strProgressText);
	//}}AFX_DATA_MAP
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(BaseProgressDlg, CDialog)
	//{{AFX_MSG_MAP(BaseProgressDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BaseProgressDlg message handlers


//-------------------------------------------------------------------//
// OnCancel()																			//
//-------------------------------------------------------------------//
// Standard cancel (via ESC, for example) is not enabled
// by default for this dlg.  To allow Cancel, just override
// this function in the derived class, call CDialog::OnCancel(),
// and perform your cancel steps.
//-------------------------------------------------------------------//
void BaseProgressDlg::OnCancel() 
{
	MessageBeep( MB_ICONASTERISK );
}


//-------------------------------------------------------------------//
// OnTimer()																			//
//-------------------------------------------------------------------//
// We want to be able to display the progress dlg during long 
// crunch sessions.  We do this by handling a timer event that
// processes pending messages.  The typical message will be a 
// request for a repaint.
//-------------------------------------------------------------------//
void BaseProgressDlg::OnTimer(UINT nIDEvent) 
{
	
	switch ( nIDEvent ) 
	{
		case TIMER_PURGE_MSGS:
		{
			PurgeMessages();
		}
	}

	CDialog::OnTimer(nIDEvent);
}


//-------------------------------------------------------------------//
// PurgeMessages()																	//
//-------------------------------------------------------------------//
// Used by processor intensive tasks to allow messages to
// be processed once in a while.  This will allow for
// button presses on modeless dialogs & paint refreshes.
//-------------------------------------------------------------------//
void BaseProgressDlg::PurgeMessages()
{
	if ( m_pParentApp )
	{	
		MSG msg;
		
		while ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{ 
			if ( !m_pParentApp->PumpMessage() )        
			{ 
				::PostQuitMessage( 0 );
				break;         
			}
		} 
	}
}