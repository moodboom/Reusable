//-------------------------------------------------------------------//
// user_msg_nt.cpp
//-------------------------------------------------------------------//
//	Abstracts the display of user messages across platforms.
//	This is the Windows C++ implementation.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "user_msg.h"

#include <DisplayMessage.h>

#include <BaseProgressDlg.h>


const int cnMaxMsgSize = 512;


//-------------------------------------------------------------------//
// user_msg()																			//
//-------------------------------------------------------------------//
// Displays a simple message in a dlg with an OK button.
//-------------------------------------------------------------------//
WINEXPORT void WINMANGLE user_msg( TCHAR* szMsg, ... )
{
	TCHAR szBuffer[cnMaxMsgSize];
	
	va_list args;
	va_start(args, szMsg );
		
	_vsntprintf( 
		szBuffer, 
		cnMaxMsgSize, 
		szMsg,
		args
	);
	
	DisplayMessage( szBuffer, IDS_MSG_LABEL );
}


//-------------------------------------------------------------------//
// user_msg_yes_no()																	//
//-------------------------------------------------------------------//
// Displays a simple message in a dlg with YES and NO buttons.
// Returns 1 for YES, 0 for NO.
//-------------------------------------------------------------------//
WINEXPORT int WINMANGLE user_msg_yes_no( TCHAR* szMsg, ... )
{
	TCHAR szBuffer[cnMaxMsgSize];
	
	va_list args;
	va_start(args, szMsg );
		
	_vsntprintf( 
		szBuffer, 
		cnMaxMsgSize, 
		szMsg,
		args
	);
	
	return ( DisplayMessage( szBuffer, IDS_YES_NO_LABEL, NULL, MB_YESNO ) == IDYES ) ? 1 : 0;
}


//-------------------------------------------------------------------//
// update_progress()																	//
//-------------------------------------------------------------------//
// Provide progress feedback.
//-------------------------------------------------------------------//
BaseProgressDlg* gpProgress = 0;
// bool gbProgressRegistered = false;
WINEXPORT void WINMANGLE update_progress( int pct, TCHAR* task, ... )
{
	if ( !gpProgress )
	{
		gpProgress = new BaseProgressDlg(
			NULL,
			AfxGetApp()
		);
		gpProgress->SetMaxPos( 1000 );
	}

	// Build the display string.
	TCHAR szBuffer[cnMaxMsgSize];
	va_list args;
	va_start(args, task );
	_vsntprintf( 
		szBuffer, 
		cnMaxMsgSize, 
		task,
		args
	);
	
	gpProgress->UpdateProgress( pct );
	gpProgress->SetText( szBuffer );

}


//-------------------------------------------------------------------//
// progress_complete()																//
//-------------------------------------------------------------------//
// Use this when finished with progress.
//-------------------------------------------------------------------//
WINEXPORT void WINMANGLE progress_complete()
{
	if ( gpProgress )
	{
		gpProgress->DestroyWindow();
		delete gpProgress;
		gpProgress = 0;
	}
}

