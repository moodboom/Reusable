//-------------------------------------------------------------------//
// OtherInstanceNotificationThread.cpp
//-------------------------------------------------------------------//

#include "stdafx.h"
#include <process.h>								// For _beginthreadex()

#include "PragmaMessages.h"					// For _TODO() etc.

#include "BaseApp.h"								// Owner

#include "DisplayMessage.h"

#include "TimeHelpers.h"						// For sleep()

#include "OtherInstanceNotificationThread.h"


//-------------------------------------------------------------------//
// Globals / Statics / Constants
//-------------------------------------------------------------------//
HWND OtherInstanceNotificationThread::m_hOwner = 0;

const TCHAR	szQuitEventName[]		= _T("D752048C-7833-4a86-89C0-39537C0F8BEB-QuitEvent"		);		// GUID created 8/11/2000, MDM.
const TCHAR	szNotifyEventName[]	= _T("D752048C-7833-4a86-89C0-39537C0F8BEB-NotifyEvent"	);		// GUID created 8/11/2000, MDM.
//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// construction																		//
//-------------------------------------------------------------------//
OtherInstanceNotificationThread::OtherInstanceNotificationThread() 
:

	// Init vars.
	m_hThread	(	0	)

{
}


//-------------------------------------------------------------------//
// Start()																				//
//-------------------------------------------------------------------//
// Call this function to begin the thread processing.  It will
// immediately return.
//-------------------------------------------------------------------//
void OtherInstanceNotificationThread::Start()
{
	// Get it going.
	// This thread is to always be low priority.  It is OK if it
	// takes a little time for the notification to get through,
	// and in tradeoff it will have little impact on the rest of
	// the program.
	CWinThread* pThread = AfxBeginThread( 
		OtherInstanceNotificationThread::Main,
		this,
		THREAD_PRIORITY_IDLE									// bWin2000OrGreater()? THREAD_PRIORITY_BELOW_NORMAL : THREAD_PRIORITY_NORMAL
	);
	m_hThread = pThread->m_hThread;
}


//-------------------------------------------------------------------//
// ~OtherInstanceNotificationThread()											//
//-------------------------------------------------------------------//
OtherInstanceNotificationThread::~OtherInstanceNotificationThread()
{
	// Shut down the thread if it is still running.
	if ( m_hThread )
	{
		// Set the die event, then give it a litle time to succeed.  
		SetEvent( OT_DIE );

		// Yawn.  This sucks, we're waiting for it to die.
		const int cnMaxTries = 8;
		const int cnTimeBetweenTriesMS = 100;
		int nTry;
		for ( nTry = 0; nTry < cnMaxTries; ++nTry )
		{
			DWORD nCode;
			if ( !GetExitCodeThread( m_hThread, &nCode ) ) 
				break;
			if ( nCode != STILL_ACTIVE ) 
				break;

			Sleep( cnTimeBetweenTriesMS );
		}
		
		if ( nTry == cnMaxTries )
		{
			// This sucks, we have to shoot the thread with our elephant gun.
			ASSERT( FALSE );
			TerminateThread( m_hThread, 0 );
			Sleep( cnTimeBetweenTriesMS );
		}

		// Finish up.
		m_hThread = 0;
	}
}


//-------------------------------------------------------------------//
// Main()																				//
//-------------------------------------------------------------------//
// Here, we run the thread, waiting for any of our possible events to 
// be "signaled".  When they are, we notify our parent, little
// snitches that we are.
//-------------------------------------------------------------------//
UINT OtherInstanceNotificationThread::Main(LPVOID pParam)
{
	// Our ugly little C-to-C++ bridge.
	OtherInstanceNotificationThread* pThis = (OtherInstanceNotificationThread*)pParam;
	ASSERT( dynamic_cast<OtherInstanceNotificationThread*>( pThis ) );

	DWORD i, dwEvent; 

	// Create event objects.
	for ( i = 0; i < OT_COUNT; i++ )
	{ 
		pThis->m_hEvents[i] = CreateEvent(
			NULL,								// no security attributes
			FALSE,							// auto-reset event object
			FALSE,							// initial state is nonsignaled
												// named object
				( i == 0 )?	szQuitEventName
			:	( i == 1 )? szNotifyEventName
			:	NULL							// not used, we track the handle!

		);
		 
		if ( pThis->m_hEvents[i] == NULL )
		{ 
			printf("CreateEvent error: %d\n", GetLastError() ); 
			return 1;
		} 
	} 

	bool bTerminate = false;
	while ( !bTerminate )
	{
		// The creating thread waits for other threads or processes
		// to signal the event objects.
		dwEvent = WaitForMultipleObjects( 
			OT_COUNT,			// number of objects in array
			pThis->m_hEvents,	// array of objects
			FALSE,				// wait for any
			INFINITE				// indefinite wait
		);

		// Return value indicates which event is signaled.
		switch (dwEvent) 
		{ 
			// QuitEvent
			case WAIT_OBJECT_0 + OT_OTHER_INSTANCE_SAYS_QUIT:
			{
				
				// Have you called SetOwner() in your BaseApp-derived class?
				// WARNING: There MAY be situations (multiple fast runs of the
				// program, e.g.) where this is not yet set.  Test and fix, as needed.
				ASSERT( m_hOwner != 0 );

				::SendMessage( m_hOwner, WM_USER_OTHER_INSTANCE_SAYS_QUIT, 0, 0L );
				bTerminate = true;
			}
			break; 

			// NotifyEvent
			case WAIT_OBJECT_0 + OT_OTHER_INSTANCE_WAS_RUN: 
			{

				// Have you called SetOwner() in your BaseApp-derived class?
				// WARNING: There MAY be situations (multiple fast runs of the
				// program, e.g.) where this is not yet set.  Test and fix, as needed.
				ASSERT( m_hOwner != 0 );

				::SendMessage( m_hOwner, WM_USER_OTHER_INSTANCE_WAS_RUN, 0, 0L );
			}
			break; 

			case WAIT_OBJECT_0 + OT_DIE:
			{				
				bTerminate = true;
			}
			break; 

			// Return value is invalid.
			default: 				
				ASSERT( FALSE );
				return 2;
		}

	}

	// The thread completed successfully.
	return 0;   

}


//-------------------------------------------------------------------//
// NotifyOtherInstance()															//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void OtherInstanceNotificationThread::NotifyOtherInstance()
{
	// Get the other instance's notify event (the thread should already exist).
	HANDLE hNotifyEvent = OpenEvent( 
		EVENT_ALL_ACCESS | EVENT_MODIFY_STATE,
		TRUE,
		szNotifyEventName
	);
	ASSERT( hNotifyEvent != NULL );

	VERIFY(
		::SetEvent(
			hNotifyEvent
		) != FALSE
	);
}


//-------------------------------------------------------------------//
// ForceOtherInstanceShutdown()													//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void OtherInstanceNotificationThread::ForceOtherInstanceShutdown()
{
	// Get the other instance's shutdown event (the thread should already exist).
	HANDLE hQuitEvent = OpenEvent( 
		EVENT_ALL_ACCESS | EVENT_MODIFY_STATE,
		TRUE,
		szQuitEventName
	);
	ASSERT( hQuitEvent != NULL );

	VERIFY(
		::SetEvent(
			hQuitEvent
		) != FALSE
	);

	// Now we want to wait for it to notify US of successful shutdown.
	// ( Hint: we need a third event! :>  Copy Main(), pretty much. )
	#pragma _TODO("This should be an event not a timer...although this hack seems to work for now...")
	sleep( 5000 );

}


