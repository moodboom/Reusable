//-------------------------------------------------------------------//
// AppInstaller.cpp
//
// This class handles installation of your program and maintenance
// of that installation as newer versions become available.
//
// To use it, create an object of its type, and call 
// VerifyInstallation() in the program's InitInstance() function.
//
// If VerifyInstallation() returns true, continue execution of
// the program.  If false, exit.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "AppInstaller.h"

#include "AppRegistryData.h"						// This gives us App path and version.
#include "AppHelpers.h"								// For GetProgramPath(), GetProgramFilename(), GetShortFilename().
#include "DisplayMessage.h"						// For displaying warnings and choices.


//-------------------------------------------------------------------//
// construction																		//
//-------------------------------------------------------------------//
AppInstaller::AppInstaller() 
:

	// Init vars.
	m_pOtherInstancesThread	(	0	),
	m_pMem						(	0	)

{
	// Init strings.
	// Note: according to the docs, "this name cannot contain blanks".
	// Therefore, we don't worry about finding the "short" version.
	m_strExeName = AfxGetApp()->m_pszExeName;
	m_strExeName += _T(".exe");
	
	m_strCurrentPath = GetProgramPath();
}


//-------------------------------------------------------------------//
// ~AppInstaller()																	//
//-------------------------------------------------------------------//
AppInstaller::~AppInstaller() 
{
	CleanUp();
}


//-------------------------------------------------------------------//
// CleanUp()																			//
//-------------------------------------------------------------------//
void AppInstaller::CleanUp()
{
	// Did we get to the point where we created the detect thread?
	if ( m_pOtherInstancesThread )
	{
		delete m_pOtherInstancesThread;
		m_pOtherInstancesThread = 0;
	}

	// Did we get to the point where we allocate shared memory?
	if ( m_pMem )
	{
		delete m_pMem;
		m_pMem = 0;
	}
}


//-------------------------------------------------------------------//
// VerifyInstallation()																//
//-------------------------------------------------------------------//
// VerifyInstallation() first determines if the program has been 
// installed.  If not, it calls a user-defined installation wizard, 
// starts the installed program, and returns false.
//
// If so, it determines if the current executable matches the
// installation path.  If so, we return true.
//
// If not, it determines the version numbers of the installation
// and of the current executable.  If the current is more recent,
// the current executable is copied to the installation path.  
//
// Next, the installation path program is started, and we return
// false.
//-------------------------------------------------------------------//
bool AppInstaller::VerifyInstallation(
	AppRegistryData*		pRegData,
	CCommandLineInfo*		pNewCommandLine,
	CPropertySheet*		pInstallWizard,
	bool						bForceOneInstance
) {

	// Init strings.
	// Note: according to the docs, "this name cannot contain blanks".
	// Therefore, we don't worry about finding the "short" version.
	m_strExeName = m_pszExeName;
	m_strExeName += _T(".exe");
	
	m_strCurrentPath = GetProgramPath();

	// Place information about the AutoRun directory in the registry.
	pRegData->SetAutoRunPath( &m_strCurrentPath );

	// Save the command line param pointer.
	pCommandLine = pNewCommandLine;

	// Set up the shared memory object.  It not only allows us to 
	// share memory between processes, it will also tell us if this
	// is the first process for our program or if another is already
	// running.
	if ( bForceOneInstance )
	{
		// Create a shared memory object.
		try {
			m_pMem = new CSharedMemory (
				nSharedMemorySize(),
				LPCTSTR( 
						GetProgramName()
					+	_T("3938E-7CB5-4bc0-AB23-087FD9EAB3EA")		// GUID created by MDM on July 25, 2000
				)
			);
		}
		catch ( CMemoryException* e )
		{
			DisplayMessage( CString( _T("Unable to allocate shared memory, man what a downer!") ) );
			e->Delete ();

			// Exit the app.
			return false;
		}

	}

	// If we can't get the installation path from the registry,
	// or the installed program file no longer exists...
	bool bInstallOK = pRegData->GetAppPath( &m_strRegistryPath );
	if ( bInstallOK )
	{
		// Test for existance of the executable.
		CString strRegistryFilename = AddTrailingBackslash( m_strRegistryPath ) + m_strExeName;
		bInstallOK = bFileExists( strRegistryFilename );
	}
	if ( !bInstallOK ) 
	{
		ASSERT( !bForceOneInstance || m_pMem->bFirstInstantiation() );

		// Start the installation.
		if ( pInstallWizard->DoModal() == ID_WIZFINISH )
		{
			// Wishlist
			// Create uninstall profile for "Add-Remove Programs".

			// Get the newly installed program path.
			if ( pRegData->GetAppPath( &m_strRegistryPath ) )
			{
				// Before we leave, set the new version number into the Registry,
				// since we are as sure as we will ever be that we will have
				// a new version now.
				pRegData->SetAppVersion();

				// If the new install path matches our path...
				if ( GetShortFilename( m_strRegistryPath ) == GetShortFilename( m_strCurrentPath ) )
				{
					// We're all set, we can continue immediately.
					return true;
				}

				// The install wizard does everything short of copying the EXE.
				// We handle that task here, since it's a bit tricky to copy
				// a currently running program.
				RunInstalledVersion( true );

			}
			else
			{
				// The install wizard did not set the App Path!
				ASSERT( false );
			}

		}

		// Return false, so this instance exits.
		return false;

	}
	
	// Let's get the version numbers.
	DWORD dwThisVersion = pRegData->GetCurrentVersion();
	DWORD dwInstallVersion = pRegData->GetAppVersion();

	// Do things look ripe for a run?
	if ( !bForceOneInstance || m_pMem->bFirstInstantiation() )
	{
		// If the install path matches our path...
		if ( GetShortFilename( m_strRegistryPath ) == GetShortFilename( m_strCurrentPath ) )
		{
			// We should be in good shape.  However, if the executable was manually
			// replaced with a newer version, we want to check for the need to update.
			if ( dwThisVersion > dwInstallVersion )
			{
				VERIFY(
					pRegData->UpdateFromOldVersion(
						dwInstallVersion,
						dwThisVersion
					)
				);
			
			// If the user is somehow attempting to run an older
			// version of the program than they have previously run, 
			// give 'em a gentle warning.  Maybe they like the older
			// version, so they replaced the new with old?
			} else if ( dwThisVersion < dwInstallVersion )
				DisplayMessage(
					IDS_OLD_VERSION_WARNING,
					IDL_WARNING,
					AfxGetMainWnd()
				);
			
			// Do we need to set up for preventing future instances?
			if ( bForceOneInstance )
			{
				// Set up the shared memory to reflect this instance of the program.
				SetSharedMemory();

				// Create the instance notification thread.
				m_pOtherInstancesThread = new OtherInstanceNotificationThread( this );
			}

			// We're all set, let's go.
			return true;
		}

		// If the install version is older than our version...
		if ( dwThisVersion > dwInstallVersion )
		{
		
			// If the user wants an update...
			if (
				DisplayMessage(
					IDS_UPDATE_PROGRAM_VERSION, 
					IDL_UPDATE_PROGRAM_VERSION,
					AfxGetMainWnd(),
					MB_OKCANCEL 
				) == IDOK 
			) {

				// Give the app's data a chance to update itself.
				// If it succeeds...
				if ( 
					pRegData->UpdateFromOldVersion(
						dwInstallVersion,
						dwThisVersion
					)
				) {

					// Copy our exe over the installed version.
					// This will also run the new version.
					RunInstalledVersion( true );

				}

				return false;

			}
				
		}
			
		// No update is required.
		// Execute the installed version of the program.
		RunInstalledVersion();

	// A Second Instance!  How fun!  What we gonna do?
	} else
	{
		// If the install path does not match our path...
		if ( GetShortFilename( m_strRegistryPath ) != GetShortFilename( m_strCurrentPath ) )
		{
			// If the install version is older than our version...
			if ( dwThisVersion > dwInstallVersion )
			{
			
				// If the user wants an update...
				if (
					DisplayMessage(
						IDS_UPDATE_PROGRAM_VERSION, 
						IDL_UPDATE_PROGRAM_VERSION,
						AfxGetMainWnd(),
						MB_OKCANCEL 
					) == IDOK 
				) {
					// TO DO
					// Get the other instances thread (it should already exist).
					// Tell it to force a shutdown.
					// Wait for it to finish before going on...
			
					// Give the app's data a chance to update itself.
					// If it succeeds...
					if ( 
						pRegData->UpdateFromOldVersion(
							dwInstallVersion,
							dwThisVersion
						)
					) {
						// Copy our exe over the installed version.
						// This will also run the new version.
						RunInstalledVersion( true );
					}

					return false;

				}
					
			}
				
			// Update shared memory
			SetSharedMemory();

			// TO DO
			// Get the other instances thread (it should already exist).
			// Send deferred notification.

		}		

	}

	// Either we executed the installed version, our paths did not match, 
	// and we figured out what to do, or paths did match, and we just want 
	// to get out of this second instance.
	// Return false, so this instance exits.
	return false;

}


//-------------------------------------------------------------------//
// RunInstalledVersion()															//
//-------------------------------------------------------------------//
// This function is called if an EXE was run other than the one in
// the install path.  It will set up to run the installed version.
// After returning from this function, the caller should quickly
// exit the currently running program.
//
// This function can update the installed version of the program with
// the one currently running, if bUpdate is true.
//
// This is not a straightforward task, because we cannot copy
// ourselves while we are running - Windows has the file exclusively
// locked.
//
// Instead, we fire off a hidden batch file.  The batch file polls
// for a successful copy of this executable over the installed
// version.  It will have to keep trying up until the point where
// this executable exits.  So once we call this function, we want
// to exit the app as quickly as possible, so the whole machine 
// does not grind to a halt.  Note that the batch file's process is
// manhandled by this function to minimize its impact.
//
// An article in MSDN ( MSJ, Win Q & A, Deleting an executable ) has
// a comprehensive discussion of the problem of operating on the
// file of a running executable.  The concept and original code were
// taken from there.
//-------------------------------------------------------------------//
void AppInstaller::RunInstalledVersion(
	bool bUpdate
) {
	// Delete the shared memory, so the installed version can create it
	// properly on its own.
	CleanUp();

	// We want to get the short filename for the installation directory.
	// Make sure we create it first, or GetShortFilename() returns a blank string!!
	VERIFY( MakeDir( m_strRegistryPath ) );
	
	CString strRegistryFilename = AddTrailingBackslash( GetShortFilename( m_strRegistryPath ) ) + m_strExeName;

	if ( !bUpdate )
	{
		ShellExecute(
			NULL,
			_T("open"),
			LPCTSTR( m_strExeName ),
			LPCTSTR( pCommandLine->m_strFileName ),	// pointer to string that specifies executable-file parameters.
			LPCTSTR( m_strRegistryPath ),					// pointer to string that specifies default directory.
			SW_SHOW				  								// whether file is shown when opened.
		);
	}
	else 
	{
		// Get the current filename.  Convert it to a short name, 
		// so the batch file works.
		CString strCurrentFilename = GetShortFilename( GetProgramFilename() );

		// We now replace the installed version of the EXE using
		// a batch file.

		// We need to do some prepping.
		// We need to make sure the installed file is deleted before we
		// call the batch file.  It polls for existance of the file, and
		// we don't want it to find it until we successfully copied it.
		// We could let the batch file do this, but minimizing the batch
		// file's requirements helps us minimize its impact.  Besides,
		// this may not succeed.  Typically, this fails if the executable is
		// already running.  If so, the user is prolly farting around.  We just exit.
		bool bInstallIsDeleted = !bFileExists( strRegistryFilename );
		if ( !bInstallIsDeleted )
			bInstallIsDeleted = ( DeleteFile( strRegistryFilename ) != FALSE );

		if ( bInstallIsDeleted )
		{

			// Create a temporary batch file.
			// We create it in c:\.
			// TO DO
			// Consider using %TEMP%.
			#define BATCH_FILE_NAME _T("c:\\Temp.bat")
			#define BATCH_FILE_PATH _T("c:\\")

			HANDLE hfile;
			STARTUPINFO si;
			PROCESS_INFORMATION pi;

			hfile = CreateFile(
				BATCH_FILE_NAME, 
				GENERIC_WRITE, 
				0, 
				NULL, 
				CREATE_ALWAYS,                             
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 
				NULL
			);
			if ( hfile != INVALID_HANDLE_VALUE ) 
			{
				// Construct the lines for the batch file.
				CString strBatFile;

				// This fills the batch file in one fell swoop.  Unfortunately, the batch file
				// ends up getting truncated somewhere in this mess.
				/*
				strBatFile.Format(
					_T(":Repeat"									)		_T("\r\n")		// Set up the loop point.
					_T("copy \"%s\" \"%s\""						)		_T("\r\n")		// Attempt to copy this file to install file.
					_T("if not exist \"%s\" goto Repeat"	)		_T("\r\n")		// If it did not yet succeed, loop.
					_T("Start \"%s\""								)		_T("\r\n")		// Run the install file.
					_T("del \"%s\""								)		_T("\r\n"),		// Delete the batch file.

																									// Command:
					LPCTSTR( strCurrentFilename				),							// copy from
					LPCTSTR( strRegistryFilename				), 						//	copy to
					LPCTSTR( strRegistryFilename				),							// if	not exist
					LPCTSTR( strRegistryFilename				),							// Start
					BATCH_FILE_NAME															// del
				);
				*/

				// We'll do the batch file one line at a time.
				// NOTE: When we use "Start" to fire up the program here, Win2000 gags.  I believe we
				// can nix the "Start" prefix and just run the proggie directly.  TEST ON 9x!
				CString strBatLine;
				strBatLine.Format( _T(":Repeat"									)																						); strBatFile += strBatLine + _T("\r\n");	// Set up the loop point.
				strBatLine.Format( _T("copy \"%s\" \"%s\""					), LPCTSTR( strCurrentFilename ), LPCTSTR( strRegistryFilename	)	); strBatFile += strBatLine + _T("\r\n");	// Attempt to copy this file to install file.
				strBatLine.Format( _T("if not exist \"%s\" goto Repeat"	), LPCTSTR( strRegistryFilename	)											); strBatFile += strBatLine + _T("\r\n");	// If it did not yet succeed, loop.
				strBatLine.Format( _T("\"%s\""									), LPCTSTR( strRegistryFilename	)											); strBatFile += strBatLine + _T("\r\n");	// Run the install file.
				strBatLine.Format( _T("del \"%s\""								), BATCH_FILE_NAME																); strBatFile += strBatLine + _T("\r\n");	// Delete the batch file.
				strBatLine.Format( _T("exit"										), BATCH_FILE_NAME																); strBatFile += strBatLine + _T("\r\n");	// Exit - this prolly won't work, since we just deleted the batch file!  :>

				DWORD dwNumberOfBytesWritten = strBatFile.GetLength();

				// Write the batch file and close it.
				WriteFile(
					hfile, 
					LPCTSTR( strBatFile ), 
					strBatFile.GetLength(),
					&dwNumberOfBytesWritten, 
					NULL
				);
				CloseHandle(hfile);
				ASSERT( dwNumberOfBytesWritten == (DWORD) strBatFile.GetLength() );

				// Get ready to spawn the batch file we just created.
				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);

				// We want its console window to be invisible to the user.
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;
				
				// DEBUG
				// For debugging purposes, show the window.
				// si.wShowWindow = SW_SHOW;

				// Spawn the batch file with low-priority.
				if (
					CreateProcess(
						NULL, 
						BATCH_FILE_NAME, 
						NULL, 
						NULL, 
						FALSE,
						IDLE_PRIORITY_CLASS, 	// CREATE_SUSPENDED | IDLE_PRIORITY_CLASS, 
						NULL, 
						NULL,							// LPCTSTR( m_strRegistryPath ), 
						&si, 
						&pi
					)
				) {

					// Lower the batch file's priority even more.
					// I removed this, because I was worried it might not ever
					// run.  For example, I have been running a background process
					// that sucks up ALL idle time.
					/*
					SetThreadPriority(
						pi.hThread, 
						THREAD_PRIORITY_IDLE
					);
					*/

					// Raise our priority so that we terminate as quickly as possible.
					SetThreadPriority(
						GetCurrentThread(), 
						THREAD_PRIORITY_TIME_CRITICAL
					);
					SetPriorityClass(
						GetCurrentProcess(), 
						HIGH_PRIORITY_CLASS
					);

					// Allow the batch file to run and clean up our handles.
					CloseHandle( pi.hProcess );
					ResumeThread( pi.hThread );
         
					// We want to terminate right away now so that we can be deleted.
					CloseHandle( pi.hThread );

				}
			}
		}
	}
}