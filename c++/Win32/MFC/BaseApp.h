#if !defined(BASE_APP_H)
#define BASE_APP_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//
#define IDS_UPDATE_PROGRAM_VERSION      6900		// NOTE: This message prompt can be customized for each release.
#define IDS_UPDATE_FAILED               6901
#define IDS_OLD_VERSION_WARNING         6902
#define IDL_UPDATE_PROGRAM_VERSION      6903
// 
// Ideally, resources would be specified on a class-by-class basis.  Unfortunately,
// Visual Studio does not handle that well.  Different projects have different resource
// files that contain all of the project's resources in one place.  Make sure
// you provide the resources matching the above defines in your resource file.
// You must also include this file in the resource file's "Resource Set Includes"
// ( see "Resource Includes" on the View menu ).
//
// We want to exclude the remainder of the include file when dealing with App Studio.
#ifndef RC_INVOKED
/////////////////////////////////////////////////////////////////////////


#include "WinAppEx.h"								// Our (scary) base class, provided via the IEToolbar project ("SizableRebar" on CodeGuru)

#include "SharedMemory.h"							// For our "IPC" needs, allows one instance, with communication between.

#include "OtherInstanceNotificationThread.h"	// This is our thread that waits for notification from other instances.

class AppRegistryData;								// For program path, version.


/////////////////////////////////////////////////////////////////////////////
// BaseApp:
// This is the base App class for functionality common across multiple
// projects.
//

class BaseApp : public CWinAppEx
{
	typedef CWinAppEx inherited;

public:

	BaseApp();
	~BaseApp();

	// These functions should be overridden to provide support for 
	// shared memory, used across multiple program instances.
	virtual int nSharedMemorySize()	{	return 0;	}
	virtual void SetSharedMemory()	{}

	// Used by processor intensive tasks to allow messages to
	// be processed once in a while.  This will allow for
	// button presses on modeless dialogs & paint refreshes,
	// etc.
	void PurgeMessages();

	bool VerifyInstallation(
		AppRegistryData*		pRegData,
		CCommandLineInfo*		pNewCommandLine,
		CPropertySheet*		pInstallWizard,
		bool						bForceOneInstance	= true
	);

	// Call this once the main window is created, so
	// it can be sent notifications.
	void SetNotificationWindow( HWND hMainWnd )
	{
		m_OtherInstancesThread.SetNotificationWnd( hMainWnd );
	}

protected:

	void CleanUp();

	void RunInstalledVersion(
		bool bUpdate = false
	);

	CString	m_strExeName;
	CString	m_strCurrentPath;
	CString	m_strRegistryPath;
	
	CCommandLineInfo*		pCommandLine;

	// For our "IPC" needs, allows one instance, with communication between.	
	CSharedMemory* m_pMem;
	
	// Again for IPC, this thread detects messages from another process.
	OtherInstanceNotificationThread	m_OtherInstancesThread;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BaseApp)
	public:
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(BaseApp)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

//-------------------------------------------------------------------//
// Globals/Statics																	//
//-------------------------------------------------------------------//
extern BaseApp* pBaseApp;


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif	// RC_INVOKED
#endif	// BASE_APP_H


