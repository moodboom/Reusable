//-------------------------------------------------------------------//
// OtherInstanceNotificationThread.h interface								//
//-------------------------------------------------------------------//
// This class wraps the functionality of our notification code.
// We will notify the app if another instance has been run and
// we need to do something as a result.
//
// GENERAL MFC "WORKER"-THREAD COMMENTS:
// Note that we use the AfxBeginFunction() to create a "worker"
// thread.  This means that ABF() creates its own CWinThread class,
// and all we supply is a function.  Pretty un-OO.  So we wrap
// the function AND the AfxBeginFunction() call into this class.
//-------------------------------------------------------------------//

#ifndef OTHER_INSTANCE_NOTIFICATION_THREAD_H
#define OTHER_INSTANCE_NOTIFICATION_THREAD_H

class BaseApp;						// Our owner


/////////////////////////////////////////////////////////////////
// Globals Statics Constants												//
/////////////////////////////////////////////////////////////////
//

// These define the user messages that we send to 
// our owner.  Make sure you call SetOwner() 
// after creating the OINT object, typically using
// your mainframe or main dlg window.  Also make sure
// that your message pump for the owner handles these 
// messages.
// NOTE: I HATE to use this sneaky method of selecting
// the define value, but it is more important to make 
// it work EASILY in 99% of cases than to obsess about
// the 1% where we might have an WM_USER range collision.
#define WM_USER_OTHER_INSTANCE_SAYS_QUIT	WM_USER + 998
#define WM_USER_OTHER_INSTANCE_WAS_RUN		WM_USER + 999

typedef enum
{
	OT_OTHER_INSTANCE_SAYS_QUIT,
	OT_OTHER_INSTANCE_WAS_RUN,
	OT_DIE,

	OT_COUNT
} OT_EVENTS;

//
/////////////////////////////////////////////////////////////////

class OtherInstanceNotificationThread
{
public:

	// Our constructor.
	OtherInstanceNotificationThread();

	// Destructor.
	~OtherInstanceNotificationThread();

	// This MUST be done first thing.  It is not a constructor
	// param because you can't use "this" in the owner's
	// constructor (without generating a warning).
	void SetNotificationWnd( HWND	hOwner )
	{ 
		// Set up the static pointer to our owner.
		// Make sure we don't create this thread more than once.
		ASSERT( m_hOwner == 0 );
		m_hOwner = hOwner; 
	}

	// This starts the thread up, waiting for events.
	// You should call this for the FIRST legitimate
	// instance of the application.  Subsequent instances
	// should call the appropriate event-firing functions,
	// next.
	void Start();

	// Call these to fire our events across processes.
	void NotifyOtherInstance();
	void ForceOtherInstanceShutdown();

protected:

	bool SetEvent( OT_EVENTS EventID )
	{
		ASSERT( EventID >= 0 && EventID < OT_COUNT );
		ASSERT( m_hEvents[ EventID ] != NULL );
		bool bReturn =	( ::SetEvent( m_hEvents[ EventID ] ) != FALSE );
		ASSERT( bReturn );
		return bReturn;
	}

	// Our "guts", must use the function prototype expected by AfxBeginThread().
	static UINT Main(LPVOID pParam);

	// Keep track of the thread created in the constructor 
	// so we can end it in the destructor.
	HANDLE			m_hThread;

	// Events signalled in the thread.
	HANDLE			m_hEvents[OT_COUNT];

	// Our owner, we call its virtual functions on events.
	static HWND	m_hOwner;

};


#endif	// OTHER_INSTANCE_NOTIFICATION_THREAD_H
