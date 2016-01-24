//-------------------------------------------------------------------//
// EMDBArray.c - code file
//
//
//
//-------------------------------------------------------------------//

#include "stdafx.h"									// Required for precompiled header
															
#include <io.h>										// For _taccess
#include <direct.h>									// For _tmkdir
															
#include "..\DisplayMessage.h"					
															
#include "..\UserSettings.h" 						// For user settings
#include "..\UnitsSettings.h"						// "
#include "..\ConversionSet.h"						// "
															
#include "..\RegistryData.h"						// For access to database registry data
															
#include "..\ObjectTypes.h"						// For OBJECT_TYPE definitions.
															
#include "..\SplashScreenDlg.h"					// We update it from here.
															
#include "..\Dongle\Dongle.h"						// Used to get new DBID's, PromptForDongle() during perm file processing, etc..
															
#include "DBAssociations.h"						// Checked upon initialization.

#include "..\Dongle\PermissionsDatabase.h"	// We process these here.

#include "DatabaseManagerDlg.h"

#include "..\EMComponentInit.h"

#include "..\GenericHelpers.h"					// For GetProgramPath()

#include "..\DBTreeControl.h"						// For Begin/EndObjectInsertions() during FillTree calls.

#include "EMDBArray.h"


//-------------------------------------------------------------------//
// Initialize statics/globals.
//-------------------------------------------------------------------//

// The database schema requires exactly one database array object.
// On program startup, it should be created, and this pointer should 
// be pointed to it.
// We already have a pointer to the object in the derived class.  This
// additional pointer can be used to access the additional functionality
// of the derived class.
EMDBArray*	pEMDBArray = 0;

// Establish the name of the registry database section.
static const TCHAR szDBRegSection[] = _T("Databases");

// Establish the names of the default databases.
static const TCHAR tUserSettingsDBName	[] = _T( "UserSettings" );
static const TCHAR tEarthmoverDBName	[] = _T( "Earthmover"	);
static const TCHAR tDemoDBName			[] = _T( "Demo"			);
static const TCHAR tDemoPersonalDBName	[] = _T( "DemoPersonal"	);
static const TCHAR tPersonalDBName		[] = _T( "Personal"		);

//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// EMDBArray()																			//
//-------------------------------------------------------------------//
// We need to delay initialization of databases until the 
// Initialize() function.  See that function (below) for details.
//-------------------------------------------------------------------//
EMDBArray::EMDBArray()
:

	// Init vars.
	bCrossDBOperations( false )

{

	// Assign the global pointer.
	pEMDBArray = this;

	EMDatabase::InitializeStatics();

}


//-------------------------------------------------------------------//
// ~EMDBArray()																		//
//-------------------------------------------------------------------//
EMDBArray::~EMDBArray()
{

	// Save and delete the user settings.
	pUserSettings->ChangeObject();
	delete pUserSettings;

	// Delete the master units object.
	delete pUnitSet;

	// Delete all the database objects.
	int nCount = Databases.GetSize();
	for ( int i = 0; i < nCount; i++ )
		delete Databases[i];

	// Delete the user settings database.
	delete pUserSettingsDB;

}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// Here, we attempt to connect to all known databases.  Connecting
// to a database consists of creating a database object that 
// represents that database.  Its constructor does most of the work.
//
// This initialization is not done in the constructor because the
// app is not yet fully initialized at that time, and we therefore 
// do not have access to app resources, which are required for
// many of the steps performed here.  The Initialization function
// must be called externally after the app is initialized.
//
// There are three required databases: the UserSettings 
// database, the standard Earthmover database, and the current 
// target database.  All other databases are optional.  The
// database types are described next.
// 
// 1) The UserSettings database does not contain standard
//		Earthmover database classes.  Instead, it contains the classes 
//		that are used to store all the allowable user settings/options/
//		preferences.  The classes include:
//
//			UserSettings		
//			CUnitsObj			
//			CUnitsSettings	
//			CSystemFonts		
//			CSpecTable_Settings
//
//		The UserSettings database object is a "UserSettingsDB" class.
//		It is derived from "ObjectStream", and knows about the objects
//		that are contained in it.  It registers all known objects
//		in its constructor.  This allows us to use FirstObject(),
//		a requirement.  We can also reindex this file, but it is
//		more likely easier for the user to delete a corrupt User
//		Settings file; it will be rebuilt with defaults.
//		It is accessed through the pUserSettingsDB pointer.
//
// 2) The master Earthmover database provides the foundation
//		for most Earthmover objects.  It is published regularly by
//		Accelerator.  If it is not found, the user is warned that
//		they will be in "demo" mode.
//
//	3) The target database is where new objects are added.  There
//		should ALWAYS be a target database ready to accept new objects.
//
// 4) The user determines which additional databases he or
//		she wishes to connect to.
//
// There are two ways that databases are located.  We first
// attempt to find the default database directory specified in the
// registry.  If not found, we create a "Databases" directory
// under the program directory and add the dir to the registry.
// From there, we look in it and attempt to load all databases
// we find in it.  We first look for the UserSettings db, creating 
// it if not found.  Then we look for the EM Master db.  
//
// If it is not found, we ask the user if they want to use "demo" mode.
// If so, we look for the demo database.  If it is not found,
// we warn the user and exit, otherwise we switch to demo mode.  
//
// If not in demo mode, we attempt to connect to every other database 
// found in the database directory.  Then, we attempt to connect to 
// additional databases specified in the registry (for databases
// that are not located in the default dir).
//	Note that SetRegistryKey() should have been called in the main 
//	program initialization to initialize the registry.
// As we are loading active databases, we check to see if they
// match the ID of the target specified in user settings.  If so,
// we set the target database pointer.  If we have not found a
// target db by the time we have finished the above, we create
// a new database for the user, setting it to the target db.
// 
// Here is a summary of the functions that are performed as we
// connect to each database:
//
//	Function					Description
// ------------------------------------------------------------------
// File existance			We make sure we can find at least the data 
//								file.  We can recreate the index file as
//								needed if everything else checks out.  If
//								the data file is found but not the index,
//								the index file is created and a rebuild flag 
//								is set.
// Access rights			We test all known database keys in the
//								dongle memory against the data file until
//								we successfully decrypt it.
// Class registration	All Earthmover classes are registered in the
//								index file.  If the correct assignments are 
//								not made, we catch that here.  Note that if
//								the index file needs to be rebuilt, this
//								step adds all class registrations.  If the
//								index file existed, but did not include class
//								registration for all currently known classes, 
//								the new classes are registered.
//	File access				Now that we have a database file, we get
//								the published status.  If the database is
//								working (non-published), we verify that we
//								have write access to it.
//	Tree load				Next, the database's objects are added to 
//								the Earthmover tree controls.  We load a
//								cache containing a tree initialization 
//								object for each object type.  If any of the 
//								tree objects are not found, we catch that here.  
//	
// This function is given a pointer to the splash screen, so it can
// update our progress as we go.  We have been designated
// from 5% to 100% progress.
//-------------------------------------------------------------------//
bool EMDBArray::Initialize(
	SplashScreenDlg*	pSplash
) {

	int nDBCount = 0;
	int i, j;
	CString strTemp;
	CString ProgressString;

	// Make sure we have a wait cursor while loading.
	CWaitCursor Cursor;

	// Clean up any leftover dongle logs.
	MainDongle.DeleteDiagnosticLog();

	// Get the program and database directory.
	CString strEMDir = GetProgramPath();
	CString DBDir;
	GetDatabasePath( &DBDir );
	
	// Perform a passwords calc.  See "int Pass1" in Dongle.h for details.
	// This is the final calc that results in the final password values.
	int temp = Pass1;
	Pass1 = Pass2 / 35817 + 30577;
	Pass2 = temp  / 12122 + 8111;	
	
	// Process any permissions files in the Earthmover directory.
	// Prescan the files so we can process them according to the version in their names.
	CTypedPtrArray <CPtrArray, CString*> PermFiles;
	PermFiles.SetSize( 0, 2 );
	CFileFind DBFileFinder;    
	BOOL bFound = DBFileFinder.FindFile(
		strEMDir + _T("*") + ctPermissionsExt
	);    
	while ( bFound ) 
	{
		// This will actually get us a file.
		bFound = DBFileFinder.FindNextFile();
		
		// Save the file name.
		PermFiles.Add( new CString( DBFileFinder.GetFilePath() ) );

	}
	int nPermFileCount = PermFiles.GetSize();
	if ( nPermFileCount > 0 ) 
	{
		// First step is to sort them.
		// Use the quicksort string algorithm.
		QuickSortStringArray( PermFiles );

		// Since we found a file, we need to have a corresponding
		// dongle.  Look for it here.  If we find it, set 
		// Dongle::bDemoMode to false, so we can call Dongle::GetDongleID
		// successfully.  If it is not found, skip processing of
		// the permissions files; we will end up in demo mode.
		//
		// Note that bDemoMode will be reset below, after the permissions
		// files are processed, in SetProgramMode().  We only set it here
		// temporarily so that GetDongleID() will work.
		bFound = MainDongle.PromptForDongle();
		Dongle::bDemoMode = !bFound;

		i = 0;
		while ( bFound && i < nPermFileCount ) {        
				
			// Attempt to process each file.
			// Any failures are handled within the call to 
			// PermDB::Initialize(), which displays a message.
			PermissionsDatabase* pPermDB = new PermissionsDatabase( PermFiles[i] );
			pPermDB->Initialize();

			// Move the permissions file to the backup directory.
			pPermDB->MoveToBackup();

			// Clean up.
			delete pPermDB;

			// Loop.
			i++;

		}

		// Clean up allocated strings.
		for ( i = 0; i < nPermFileCount; i++ ) 
			delete PermFiles[i];

	}
	
	// Set the target database info to NULL.
	pTargetDB = 0;
	TargetDBID = DatabaseID();
	
	// Create or open the User Settings database.
	Databases.SetSize( 0, 1 );
	pUserSettingsDB = new UserSettingsDatabase( &DBDir );
	VERIFY( pUserSettingsDB->Initialize() == DB_INIT_SUCCESS );

	// Open the UserSettings db during the following loads.
	// It must be write open so we can save new defaults,
	// if none found.
	pUserSettingsDB->Open( true );

	// Create master units object.
	// The constructor handles locating the object.
	pUnitSet = new CConversionSet;


	// -----------------------------------------------------
	// Extract user settings from the UserSettings database.
	// This includes tree sort preferences.
	//

	// Try ObjectID = 1.
	pUserSettings = new UserSettings( 1 );

	// If object not found, look for the first.
	if ( !pUserSettings->ObjectExists() )
		pUserSettings->FirstObject();

	// If still not found, we need to add it.
	if ( !pUserSettings->ObjectExists() ) 
	{
		// Add the object.
		pUserSettings->AddObject();
		pUserSettings->SaveObject();
	}

	// Make sure we made it.
	ASSERT( pUserSettings->ObjectExists() );

	// Get current user units from the database.
	CUnitsSettings::GetUserDefault();

	// Update the progress.
	ProgressString.LoadString( IDS_DB_CONNECTED );
	ProgressString += CString(
		pUserSettingsDB->GetDatabaseName()
	);
	pSplash->AddString( ProgressString );
	pSplash->UpdateProgress( 7 );

	// Close the UserSettings db.
	pUserSettingsDB->Close( true );

	//
	// -----------------------------------------------------


	pSplash->AddString( CString( (LPSTR) IDS_INIT_DONGLE ) );
		
	const int DBInitProgressWidth = 15;
	EMDatabase* pNewDatabase;

	// Set the current "mode" (demo or full) using the dongle.
	MainDongle.SetProgramMode();

	if ( Dongle::bDemoMode ) 
	{
		// Check for expired program.
		if ( theApp.bIsProgramGettingOldAndCrusty() )
			return false;
		
		// Attempt to load the demo database.

		// MDM	7/6/2000 11:52:30 AM
		// Improving access in demo mode.
		// So this comment is OLD:
		// In demo mode, the user will not have rights to any other 
		// databases, and will not be able to create new databases.
		// The demo database is a published database.  Therefore,
		// the user is limited to viewing and printing the objects 
		// in the demo database.

		pNewDatabase = new EMDatabase(
			&( DBDir + tDemoDBName + tPublishedDBExt )
		);
		
		if ( pNewDatabase->Initialize() == DB_INIT_SUCCESS ) 
		{
			// Add the demo database.
			Databases.Add( pNewDatabase );

			// Update the progress.
			ProgressString.LoadString( IDS_DB_CONNECTED );
			ProgressString += CString(
				pNewDatabase->GetDatabaseName()
			);
			pSplash->AddString( ProgressString );

			// MDM	7/6/2000 11:52:30 AM
			// Improving access in demo mode.
			
			// OLD:
			/*
			// We'll need a target database, to create temp objects.
			// We do not want to allow new objects to be saved, however!
			pTargetDB = pNewDatabase;
			*/

			// ALL NEW:

			// We need to locate the user's personal demo database.
			// If we don't find it, we will need to create it.
			pNewDatabase = new EMDatabase( 
				&( DBDir + tDemoPersonalDBName + tWorkingDBExt )
			);

			DB_INIT_RESULT DemoPersonalResult = pNewDatabase->Initialize();
			if ( DemoPersonalResult != DB_INIT_SUCCESS ) 
			{
				// If the Demo Personal db was bad, delete it, we will create a new one.
				if ( DemoPersonalResult != DB_INIT_DATAFILE_NOT_FOUND ) 
					VERIFY( pNewDatabase->MoveToBackup() );

				// Clean up our initial attempt, we will try again by
				// creating a new db.
				delete pNewDatabase;

				// We want to make a new working database to 
				// use as the default target.
				DatabaseID DemoPersonalDBID = DatabaseID();
				pNewDatabase = new EMDatabase( 
					&( DBDir + tDemoPersonalDBName + tWorkingDBExt )
				);
				wstring wsNewDBName;
				CopyCStringToWstring( &CString( tDemoPersonalDBName ), &wsNewDBName );
				if ( 
					pNewDatabase->Initialize(
						false,								//	bFileShouldExist,
						&wsNewDBName,						//	pwsDatabaseName,		= L"DemoPersonal"
						&DemoPersonalDBID,				//	pDBID,					= User 0, DB# 0
						DemoGroup							// Group - use demo group
					) != DB_INIT_SUCCESS
				) {
					// Clean up, you slob.
					pNewDatabase->DeleteFiles();
					delete pNewDatabase;

					DisplayMessage( 
						IDM_DB_ACTIVE_FAILED,
						IDL_WARNING,
						pSplash
					);

					// Exit the program.					
					return false;
				
				}

			}

			// Add the database to the db list.
			Databases.Add( pNewDatabase );

			// Make it the target.
			pTargetDB = pNewDatabase;

		} else 
		{
			delete pNewDatabase;
			pNewDatabase = 0;
			DisplayMessage( 
				IDM_DB_NO_DEMO_DATABASE,
				IDL_WARNING,
				pSplash
			);

			// Exit the program.					
			return false;

		}

	// Dongle mode.
	} else 
	{
		// Attempt to connect to the Master Earthmover
		// database, which should exist in the Databases
		// directory after an installation.
		// The Master DBID is as follows:
		//
		//		DatabaseID(
		//			UserID(
		//				0xc5ae88e3,							// EM User ID
		//				MasterGroup							// EM Group ID
		//			),
		//			0,											// Next DB number
		//			0,											// DBVersion
		//			true										// bPublished
		//		)
		//
		pNewDatabase = new EMDatabase( 
			&( DBDir + tEarthmoverDBName + tPublishedDBExt )
		);

		DB_INIT_RESULT MasterResult = pNewDatabase->Initialize();
		if ( MasterResult == DB_INIT_SUCCESS ) 
		{
			// Add the Earthmover Master database as first in the db list.
			Databases.Add( pNewDatabase );
			
			// Update progress.
			ProgressString.LoadString( IDS_DB_CONNECTED );
			ProgressString += CString(
				pNewDatabase->GetDatabaseName()
			);
			pSplash->AddString( ProgressString );

		// TO DO
		// We would like to let legitimate users continue without
		// the master.  But we need to verify that the file was
		// not found.  This may fail for other reasons, too.
		// We also need to verify the code that checks for failure
		// of the creation of a target database if none found.
	
		// Just limit it to Accel for now...
		#ifdef ACCELERATOR_INHOUSE
		} else if ( MasterResult == DB_INIT_DATAFILE_NOT_FOUND ) 
		{
			// No master, warn the user and continue.
			delete pNewDatabase;
			pNewDatabase = 0;

			// MDM, 7/29/98 12:11:40 PM
			// Removed tedious warning.
			// DisplayMessage( 
			// 	IDM_DB_NO_EM_DATABASE,
			// 	IDL_WARNING,
			// 	pSplash
			// );

		#endif

		// The Master was bad.  Warn and exit.
		} else 
		{		
			delete pNewDatabase;
			pNewDatabase = 0;
			DisplayMessage( 
				IDM_DB_MASTER_BAD,
				IDL_WARNING,
				pSplash
			);
			return false;
		
		}

		pSplash->UpdateProgress( 15 );

		// There are two ways we access additional db's.
		// The first is to get all the databases in the database 
		// directory.  This allows the user to drop new database 
		// files into her database dir and automatically connect 
		// to them on start of Earthmover.
		// The second is to allow the user to specify other db's,
		// and store the path in the registry.
		
		// In order to properly update the progress meter, we
		// want to get a count of the total number of files
		// before connecting to them.
		
		// Count the working db files in the DB dir.
		CFileFind DBFileFinder;    
		bFound = DBFileFinder.FindFile(
			DBDir + "*" + tWorkingDBExt
		);    
		while ( bFound ) 
		{
			bFound = DBFileFinder.FindNextFile();
			nDBCount++;
		}
		
		// Count the published db files in the DB dir.
		bFound = DBFileFinder.FindFile(
			DBDir + "*" + tPublishedDBExt
		);    
		while ( bFound ) 
		{
			bFound = DBFileFinder.FindNextFile();
			nDBCount++;
		}
		
		// Get all the additional databases in the database 
		// section of the registry.
		RegistrySection DBSection( szDBRegSection );
		int nRegCount = DBSection.Values.GetSize();
		nDBCount += nRegCount;
		
		// Now reset for the real file scan.
		bool bProcessingWorking = true;
		bFound = DBFileFinder.FindFile(
			DBDir + "*" + tWorkingDBExt
		);    
		
		DB_INIT_RESULT Result;
		i = 0;
		while ( bFound ) 
		{			
			// This will actually get us a file.
			bFound = DBFileFinder.FindNextFile();

			// If this is not the UserSettings working db or the EM published db...
			if ( 
					( bProcessingWorking		&& DBFileFinder.GetFileTitle() != CString( tUserSettingsDBName	) )
				||	( !bProcessingWorking	&& DBFileFinder.GetFileTitle() != CString( tEarthmoverDBName	) )
			) {
				// Attempt to connect to it.
				pNewDatabase = new EMDatabase( &DBFileFinder.GetFilePath() );
				Result = pNewDatabase->Initialize();
				
				if ( Result == DB_INIT_SUCCESS ) 
				{				
					// Skip demo databases lying around in the main database directory.  
					// They will likely contain duplicates of objects in the 
					// master database.
					if ( pNewDatabase->DBHeader.Group == DemoGroup ) 
					{
						delete pNewDatabase;

					// Note that this function will clean up for us if
					// we are already connected.
					} else if ( !DBAlreadyConnected( pNewDatabase, pSplash ) ) 
					{
						// Add the db to our db list.
						Databases.Add( pNewDatabase );

						// See if this is the target database.
						// MAKE SURE we don't let published db's qualify.
						// If the user had a working db as target, then shut down, then
						// swapped a publish version for the working, we don't want to
						// allow it to be the target anymore.
						if ( pNewDatabase->GetDBID() == TargetDBID && !pNewDatabase->IsPublished() )
							pTargetDB = pNewDatabase;

						// Update the progress status.
						ProgressString.LoadString( IDS_DB_CONNECTED );
						ProgressString += CString(
							pNewDatabase->GetDatabaseName()
						);
						pSplash->AddString( ProgressString );

					}

				} else 
				{

					if ( Result == DB_INIT_UNAUTHORIZED ) 
					{					
						// We were unable to access this database, warn the user.
						// This is important enough to halt with a message.  
						// Include the author data in the message.
						CString strAuthor;
						BuildUserBlock(
							strAuthor,
							(wchar_t*) pNewDatabase->UserName,
							(wchar_t*) pNewDatabase->Company,
							(wchar_t*) pNewDatabase->Contact,
							pNewDatabase->GetDBID().UserNumber,
							false
						);
						ProgressString.Format(
							IDS_DIR_DATABASE_ACCESS_UNAUTHORIZED,
							DBFileFinder.GetFilePath(),
							strAuthor
						);
						DisplayMessage( 
							ProgressString,
							IDL_WARNING,
							pSplash
						);
					
						// Move it to the backup dir.
						pNewDatabase->MoveToBackup();

					} else 
					{
						
						// Unanticipated error.
						ProgressString = DBFileFinder.GetFilePath();
						ProgressString += CString( LPSTR( IDM_DIR_DATABASE_INVALID ) );
						DisplayMessage( 
							ProgressString,
							IDL_WARNING,
							pSplash
						);

						// Move it to the backup dir.
						pNewDatabase->MoveToBackup();

					}

					// Clean it up.
					delete pNewDatabase;

				}

			}

			// If we are done with the working db files,
			// set up to check the published next.
			if ( !bFound && bProcessingWorking ) 
			{
				bProcessingWorking = false;
				bFound = DBFileFinder.FindFile(
					DBDir + "*" + tPublishedDBExt
				);    
			}
			
			// We update our progress regardless of 
			// whether we processed the file or not.
			i++;
			pSplash->UpdateProgress(
				15 + i / ( nDBCount * 23 )
			);

		}

		// Loop through all databases specified in the Registry.
		for ( i = 0; i < nRegCount; i++ ) 
		{
			// Add the next Earthmover database.
			pNewDatabase = new EMDatabase( DBSection.Values[i] );
			Result = pNewDatabase->Initialize();
			
			if ( Result == DB_INIT_SUCCESS ) 
			{
				// Skip demo databases.  They may contain duplicates of objects
				// in the master database.
				if ( pNewDatabase->DBHeader.Group == DemoGroup ) 
				{
					delete pNewDatabase;

				} else if ( !DBAlreadyConnected( pNewDatabase, pSplash ) ) 
				{
					// Add the db to our db list.
					Databases.Add( pNewDatabase );

					// See if this is the target database.
					// Never allow published db's to be the target.
					if ( pNewDatabase->GetDBID() == TargetDBID && !pNewDatabase->IsPublished() )
						pTargetDB = pNewDatabase;

					// Update the progress status.
					ProgressString.LoadString( IDS_DB_CONNECTED );
					ProgressString += CString(
						pNewDatabase->GetDatabaseName()
					);
					pSplash->AddString( ProgressString );

				} else 
				{				
					// Remove this entry from the registry.
					// Note that DBAlreadyConnected() already 
					// cleaned up for us.
					DBSection.Remove( i );

					// We now need to readjust the loop vars.
					i--;	nRegCount--;
					
				}

			} else 
			{			
				// We were unable to access this database;
				// warn the user.
				if ( Result == DB_INIT_UNAUTHORIZED ) 
				{
					// Include the author data in the message.
					CString strAuthor;
					BuildUserBlock(
						strAuthor,
						(wchar_t*) pNewDatabase->UserName,
						(wchar_t*) pNewDatabase->Company,
						(wchar_t*) pNewDatabase->Contact,
						pNewDatabase->GetDBID().UserNumber,
						false
					);
					ProgressString.Format(
						IDS_DATABASE_ACCESS_UNAUTHORIZED,
						DBFileFinder.GetFilePath(),
						strAuthor
					);
					DisplayMessage( 
						ProgressString,
						IDL_WARNING,
						pSplash
					);
				
				} else 
				{
					ProgressString = *( DBSection.Values[i] );
					ProgressString += CString( LPSTR( IDM_DATABASE_INVALID ) );
					DisplayMessage( 
						ProgressString,
						IDL_WARNING,
						pSplash
					);

				}
				
				// Remove it from the registry.
				DBSection.Remove( i );
				
				// We now need to readjust the loop vars.
				i--;	nRegCount--;
					
				// Clean it up.
				delete pNewDatabase;

			}
			
			// We update our progress regardless of 
			// whether we processed the file or not.
			pSplash->UpdateProgress(
				15 + ( i + nDBCount - nRegCount + 1 ) / ( nDBCount * DBInitProgressWidth - 2 )
			);
			
		}
			
	}

	// Update the progress.
	// We are now at 40%.
	pSplash->UpdateProgress( 15 + DBInitProgressWidth );
	
	// Update nCount to represent a count of all successfully 
	// loaded databases.
	nDBCount = Databases.GetSize();

	// MDM	7/6/2000 11:52:30 AM
	// Improving access in demo mode.
	// We now have TWO, not ONE, databases in demo mode.

	// We must have two databases in demo mode or at least one 
	// (the default) in standard mode.
	// Note that we do allow 0 databases in-house; a target
	// will be created momentarily.
	#ifndef ACCELERATOR_INHOUSE
		ASSERT( 
				( !Dongle::bDemoMode		&& nDBCount > 0	)
			|| ( Dongle::bDemoMode		&& nDBCount == 2	) 
		);
	#endif


	/////////////////////////////////////////////////////////////
	//        VERIFY VERSIONS AND ASSOCIATIONS
	/////////////////////////////////////////////////////////////
	// We have all the database objects.  Now we need to make 
	// sure that each is updated to the current file version,
	// and that all required associations are available.
	/////////////////////////////////////////////////////////////
	//

	pSplash->AddString( ResStr( IDS_VERIFYING_DATABASES ) );
	
	for ( i = 0; i < nDBCount; i++ ) 
	{	
		// We want to make sure that each database is updated 
		// to the current file version.
		bool bDBOK = Databases[i]->UpdateDatabaseAsNeeded();
		
		if ( bDBOK )
		{
			// We want to get all the db associations and verify 
			// that we have what we need.  We do it for both published 
			// and working databases.  We assume that working
			// db associations are somewhat up to date.

			ObjectStream* pTemp;
			DBAssociations DBA( Databases[i] );
			int nAssocCount = DBA.Associations.GetSize();
			for ( j = 0; j < nAssocCount; j++ ) 
			{
				if ( 
					!LookUpDatabase( 
						&( DBA.Associations[j]->DBID ),
						&pTemp
					)
				) {
					// Association not found!
					CString strMsg;
					strMsg.Format(
						IDS_DB_ASSOC_NOT_FOUND,
						CString( Databases[i]->GetDatabaseName() )
					);
					DisplayMessage( 
						strMsg,
						IDL_WARNING,
						pSplash
					);

					// Disconnect from this database.
					bDBOK = false;
					break;
				
				}

			}
	
		} 
		
		// If we had a problem, disconnect.
		if ( !bDBOK )
		{
			// We want to remove this database from the array.  However, if
			// this is the target db, we'll need to clear the target pointer
			// so the program can refind the target below.
			if ( Databases[i] == pTargetDB )
				pTargetDB = 0;

			// Remove this database from the array.
			// Note that the db files have already been moved to the
			// backup directory.  DB's that were previously in the Earthmover
			// database will no longer be loaded.  We won't bother to clean up 
			// the entry in the registry for db's not located in the db dir.  The 
			// next time the user runs the program, they will get another
			// notice reminding them that the database is not available.  Two
			// notices seems appropriate.
			delete Databases[i];
			Databases.RemoveAt( i );

			// Now readjust the counters.
			i--;
			nDBCount--;

		}

	}
	
	//
	/////////////////////////////////////////////////////////////
	//         DONE  [ VERIFY VERSIONS AND ASSOCIATIONS ]
	/////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////
	//							SET TARGET DATABASE
	/////////////////////////////////////////////////////////////
	// Here, we make sure we have a target DB.
	//
	// In demo mode, failure of the demo database means we exit.
	if ( Dongle::bDemoMode && !pTargetDB )
		return false;
	if ( !pTargetDB ) 
	{
	
		// If we haven't found a match yet, we'll set the
		// target to the first working in the db's we have
		// connected to.
		for ( 
			i = 0; 
			i < Databases.GetSize() && !pTargetDB; 
			i++ 
		)
			if ( !Databases[i]->IsPublished() )
				pTargetDB = Databases[i];
		
		if ( !pTargetDB ) 
		{
			// We want to make a new working database to 
			// use as the default target.
			DatabaseID NewWorkingDBID = MainDongle.GetNewDBID();

			// If this failed, get on out of here.
			if ( NewWorkingDBID == DatabaseID() ) {

				DisplayMessage( 
					IDM_DB_ACTIVE_FAILED,
					IDL_WARNING,
					pSplash
				);
				return false;

			}
			
			// Get the name and make sure it doesn't exist.
			// If so, move it to the backup dir.
			CString strNewWorkingDBName = DBDir + tPersonalDBName + tWorkingDBExt;
			if ( _taccess( LPCTSTR( strNewWorkingDBName ), 0 ) == 0 )
				MoveDatabaseToBackup( &strNewWorkingDBName );

			// TO DO
			// Loop through db's and make sure that the DBID's do not conflict
			// with the new one we are about to create.  There should also
			// never be a DBID with a common UserID and a lower db number.
			
			pNewDatabase = new EMDatabase( 
				&( DBDir + tPersonalDBName + tWorkingDBExt )
			);
			wstring wsNewDBName;
			CopyCStringToWstring( &CString( tPersonalDBName ), &wsNewDBName );
			if ( 
				pNewDatabase->Initialize(
					false,								//	bFiCleShouldExist,
					&wsNewDBName,						//	pwsDatabaseName,		= L"Personal"
					&NewWorkingDBID,					//	pDBID,
					MainDongle.GetUserNumber()		// Group - use user's personal group
				) == DB_INIT_SUCCESS
			) {
			
				// Add the new database to the db list.
				Databases.Add( pNewDatabase );

				// Make it the new target.
				pTargetDB = pNewDatabase;

				// Update the progress status.
				ProgressString.LoadString( IDS_DB_CONNECTED_TO_NEW );
				ProgressString += CString(
					pNewDatabase->GetDatabaseName()
				);
				pSplash->AddString( ProgressString );

			} else {
			
				// Clean up, you slob.
				pNewDatabase->DeleteFiles();
				delete pNewDatabase;

				DisplayMessage( 
					IDM_DB_ACTIVE_FAILED,
					IDL_WARNING,
					pSplash
				);

				// Exit the program.					
				return false;
			
			}

		}

	}

	// Now that we have a target for sure, make sure 
	// the ID is updated.
	if ( !( TargetDBID == pTargetDB->GetDBID() ) ) {
	
		// Set and save.
		TargetDBID = pTargetDB->GetDBID();
		SaveUserSettings();

	}
	//
	/////////////////////////////////////////////////////////////
	//         DONE  [ SET TARGET DATABASE ]
	/////////////////////////////////////////////////////////////


	// Now that we have created all the database objects,
	// we need to add the objects within each database
	// to the Earthmover tree controls.
	FillTrees(
		pSplash,								// Progress dlg
		15 + DBInitProgressWidth,		// Progress start
		100,									// Progress end
		false									// bClearFirst
	);

	return true;

}


//-------------------------------------------------------------------//
// MoveDatabaseToBackup()															//
//-------------------------------------------------------------------//
// This is used to remove databases that might otherwise conflict
// with databases we create in this module.
//-------------------------------------------------------------------//
bool EMDBArray::MoveDatabaseToBackup( CString* pstrDBName )
{

	CString strBackupDir;
	GetDBBackupPath( &strBackupDir );
	
	CString strBackupName;
	ExtractFileNameFromPath( pstrDBName, &strBackupName );

	// Copy then delete, as opposed to move, so we can overwrite existing files.
	// We just copy the data file, then delete the data file and index file.
	bool bReturn = CopyFile( 
		LPCTSTR( *pstrDBName ),							// Existing file
		LPCTSTR( strBackupDir + strBackupName ),	// New file
		FALSE													// bFailIfExists	(We want to overwrite any previous backup.)
	) == TRUE;

	bReturn &= ( _tremove( LPCTSTR( *pstrDBName											) ) == 0 );
	bReturn &= ( _tremove( LPCTSTR( *StripExtension( pstrDBName ) + tIndexExt	) ) == 0 );
	
	return bReturn;

}


//-------------------------------------------------------------------//
// DBAlreadyConnected()																//
//-------------------------------------------------------------------//
// This function tests for what it says it tests for.
//
// TO DO
// Update this to swap the db's as needed so that the latest version
// is always selected.  If both the latest published and working are
// found, swap to the working.

//-------------------------------------------------------------------//
bool EMDBArray::DBAlreadyConnected( 
	EMDatabase* pDatabase,
	CWnd*			pParent
) {

	int i;
	int nDBCount = Databases.GetSize();
	for ( i = 0; i < nDBCount; i++ )
		if ( Databases[i]->GetDBID() == pDatabase->GetDBID() )
			break;
	bool bResult = ( i < nDBCount );
	
	// Handle the problem here.
	if ( bResult ) {
		DisplayMessage( 
			IDM_DB_ALREADY_CONNECTED,
			IDL_WARNING,
			pParent
		);
		delete pDatabase;
		pDatabase = 0;
	}

	return bResult;

}


//-------------------------------------------------------------------//
// FillTrees()																			//
//-------------------------------------------------------------------//
// This function fills all the db trees from the databases.
// Object relationships are established in ObjectTypes.h.
// They are enforced here as follows.  The tree controls
// are filled from the databases one object type at a
// time, in the order specified in ObjectTypes.h  This
// ensures that, for example, all manufacturers are 
// available when loading sources.
//-------------------------------------------------------------------//
void EMDBArray::FillTrees(
	SplashScreenDlg*	pSplash,
	int					nProgressStart,
	int					nProgressEnd,
	bool					bClearFirst
) {

	bool bBadCache = false;

	// Loop through all Earthmover trees.
	int nProgress;
	CString ProgressString;
	int nDBCount = Databases.GetSize();
	for ( int j = 0; j <= TYPE_TREE_LAST; j++ ) 
	{
		ProgressString.LoadString( IDS_DB_ADDING_OBJECT );
		ProgressString += EMComponentInit::ByType( j )->strDescPlural();
		pSplash->AddString( ProgressString );

		// Loop through all the databases.
		for ( int i = 0; i < nDBCount; i++ ) 
		{
			// Fill each tree from all the databases.
			// The first time we call for each object type,
			// set the flags as appropriate.
			FillTreeResult Result = ( (EMDatabase*)( Databases[i] ) )->FillTree( 
				(OBJECT_TYPE) j,						// eWhich
				pSplash,									// Splash screen
				( ( i == 0 ) && !bClearFirst ),	// bCreate
				( ( i == 0 ) && bClearFirst ),	// bRefresh
				false										// bSort
			);

			if ( Result == FT_PUBLISHED_CACHE_OLD ) 
			{			
				// We had to update a published cache, note in splash box.
				pSplash->AddString( ResStr( IDS_DB_PUBLISHED_CACHE_OLD ) );

			} else if ( Result == FT_BAD_DATABASE ) 
			{			
				// Houston, we have a problem...
				// TO DO
				// We need to remove this database and refill the trees.
				// We'll need to handle the situation if this is the target or
				// master database.  We should probably move the core of
				// DBMgrListControl::OnDisconnect() into a function in 
				// DatabaseArray.
				
				// For now, we remove it and exit the program.
				// Warn the user.
				CString strAuthor;
				BuildUserBlock(
					strAuthor,
					(wchar_t*) ( dynamic_cast<EMDatabase*> ( Databases[i] ) )->UserName,
					(wchar_t*) ( dynamic_cast<EMDatabase*> ( Databases[i] ) )->Company,
					(wchar_t*) ( dynamic_cast<EMDatabase*> ( Databases[i] ) )->Contact,
					Databases[i]->GetDBID().UserNumber,
					false
				);
				CString strWarn;
				strWarn.Format(
					IDS_DATABASE_ACCESS_UNAUTHORIZED_ABORT,
					*( Databases[i]->GetFilename() ),
					strAuthor
				);
				DisplayMessage( 
					strWarn,
					IDL_WARNING,
					pSplash
				);

				// Move it and get outta here.
				Databases[i]->MoveToBackup();
				AfxGetMainWnd()->PostMessage( WM_QUIT );
				return;
			
			}

			// Update progress.
			nProgress = 
					nProgressStart
				+	( nProgressEnd - nProgressStart )
							* ( i + 1 + j * nDBCount					)
							/ ( TYPE_TREE_LAST * (long) nDBCount );
			pSplash->UpdateProgress( nProgress );
		
		}

		// We have added all objects of this type to the tree,
		// we can now sort it.
		// Note that we call BeginOI() in FillTree (when create was true).
		ASSERT( DBTreeControl::GetTreePtr( (OBJECT_TYPE) j ) );
		DBTreeControl::GetTreePtr( (OBJECT_TYPE) j )->EndObjectInsertions();


	}

}


//-------------------------------------------------------------------//
// Manage()																				//
//-------------------------------------------------------------------//
// This function is called to display a dialog allowing the user
// to perform a variety of database management functions.
//-------------------------------------------------------------------//
void EMDBArray::Manage()
{

	DatabaseManagerDlg DBDlg;
	DBDlg.DoModal();
	
}


//-------------------------------------------------------------------//
// Disconnect()																		//
//-------------------------------------------------------------------//
// This is called from DBMgrListControl and Append.
//-------------------------------------------------------------------//
bool EMDBArray::Disconnect( 
	EMDatabase* pDatabase,
	CWnd*			pParent
) {	

	bool bReturn = false;
	
	// Can't disconnect the target, gotta put new objects somewhere!  :>
	if ( pDatabase != pTargetDB ) 
	{
		// Get the db filename.
		CString DBFilename( *pDatabase->GetFilename() );
		
		// Get the db path.
		CString DBPath;
		int nTrunc = DBFilename.ReverseFind( _T('\\') );
		if ( nTrunc == -1 )
			nTrunc = DBFilename.ReverseFind( _T('/' ) );
		DBPath = DBFilename.Left( nTrunc + 1 );
		
		// Get the default db path.
		CString DefaultDBPath;
		GetDatabasePath( &DefaultDBPath );

		// If the database is located in the database dir,
		CString strDBName = pDatabase->GetDatabaseName();
		if ( DBPath.CompareNoCase( LPCTSTR( DefaultDBPath ) ) == 0 ) 
		{			
			// Tell the user that they will be reconnected on
			// start of Earthmover, and get confirmation.
			CString strMessage;
			strMessage.Format(
				CString( (LPSTR) IDM_DB_DEFAULT_DIR_DISCONNECT ), 
				strDBName 
			);
			bReturn = DisplayMessage( 
				strMessage,
				IDL_WARNING, 
				pParent,  
				MB_OKCANCEL
			) == IDOK;

		} else 
		{
			// Verify.
			CString strMessage;
			strMessage.Format(
				CString( (LPSTR) IDM_DB_REGISTRY_DISCONNECT ),
				strDBName 
			);
			bReturn = DisplayMessage( 
				strMessage,
				IDL_WARNING,
				pParent,  
				MB_OKCANCEL
			) == IDOK;
			
			// Remove it from the registry.
			if ( bReturn )
				RemoveDBFromRegistryList( pDatabase );

		}

		if ( bReturn ) 
		{
			// Remove it from the db array.
			int nDBIndex = GetDBIndex( pDatabase );
			RemoveAt( nDBIndex );

		}

	} else 
	{	
		DisplayMessage( IDM_DB_CANT_DISCONNECT_STD );
	}

	return bReturn;

}


//-------------------------------------------------------------------//
// GetFile()																			//
//-------------------------------------------------------------------//
// This is a helper function that calls the common file dialog to
// open a single database (*.RTH) file.
//
// This may be updated to provide additional options.  See the
// notes marked with ** for multi-select considerations.
//
// We use m_ofn to set our specifics.
// Here's the format:
/*

	typedef struct tagOFN { // ofn 
		 DWORD         lStructSize; 
		 HWND          hwndOwner; 
		 HINSTANCE     hInstance; 
		 LPCTSTR       lpstrFilter; 
		 LPTSTR        lpstrCustomFilter; 
		 DWORD         nMaxCustFilter; 
		 DWORD         nFilterIndex; 
		 LPTSTR        lpstrFile; 
		 DWORD         nMaxFile; 
		 LPTSTR        lpstrFileTitle; 
		 DWORD         nMaxFileTitle; 
		 LPCTSTR       lpstrInitialDir; 
		 LPCTSTR       lpstrTitle; 
		 DWORD         Flags; 
		 WORD          nFileOffset; 
		 WORD          nFileExtension; 
		 LPCTSTR       lpstrDefExt; 
		 DWORD         lCustData; 
		 LPOFNHOOKPROC lpfnHook; 
		 LPCTSTR       lpTemplateName; 
	} OPENFILENAME; 

*/
//-------------------------------------------------------------------//
int EMDBArray::GetFile(
	CString* pstrName,
	UINT		TitleID,
	CWnd*		pParent,
	bool		bIncludePublished,
	DWORD		Flags
) {


	///////////////////////////////////////////////////////////////////
	// Set up the filter; see CFileDialog::CFileDialog() for details.
	// Example string for Excel:
	//		"Chart Files (*.xlc)|*.xlc|Worksheet Files (*.xls)|*.xls|Data Files (*.xlc;*.xls)|*.xlc; *.xls|All Files (*.*)|*.*||";
	//
	
	CString strFilter;

	if ( bIncludePublished ) {
		strFilter += _T("All Earthmover databases (*");
		strFilter += tWorkingDBExt;
		strFilter += _T(";*");
		strFilter += tPublishedDBExt;
		strFilter += _T(")|*");
		strFilter += tWorkingDBExt;
		strFilter += _T("; *");
		strFilter += tPublishedDBExt;
		strFilter += _T("|");
	}

	strFilter += _T("Earthmover working databases (*");
	strFilter += tWorkingDBExt;
	strFilter += _T(")|*");
	strFilter += tWorkingDBExt;
	strFilter += _T("|");

	if ( bIncludePublished ) {
		strFilter += _T("Earthmover published databases (*");
		strFilter += tPublishedDBExt;
		strFilter += _T(")|*");
		strFilter += tPublishedDBExt;
		strFilter += _T("|");
	}

	// For convenience, allow all files to be viewed (they might
	// need this to tell where they are if they are lost, etc.).
	// Opening of files that are not valid databases should be handled
	// anyway.
	strFilter += _T("All Files (*.*)|*.*|");

	strFilter += _T("|");

	//
	///////////////////////////////////////////////////////////////////


	// Use the common file dialog to get a file name.	
	CFileDialog FileDlg(
		TRUE,							// Open ( as opposed to SaveAs ) style
		tWorkingDBExt,				// default ext - will be appended to filename
		NULL,							// initial filename
		NULL,							// flags - will be set in m_ofn
		LPCTSTR( strFilter ),	// filter
		pParent						// pParent
	);

	// Use m_ofn to set our specifics.
	CString DBDir;
	GetDatabasePath( &DBDir );
	FileDlg.m_ofn.lpstrInitialDir	=	LPCTSTR( DBDir );
	
	CString strTitle( (LPSTR) TitleID );
	FileDlg.m_ofn.lpstrTitle		=	LPCTSTR( strTitle );
	
	FileDlg.m_ofn.Flags				|= Flags;
	
										// File must exist:
										// |=	OFN_FILEMUSTEXIST;
										
										// ** For multi-select:
										// | OFN_EXPLORER
										// | OFN_LONGNAMES
										// | OFN_ALLOWMULTISELECT;
	
	// Provide _MAX_PATH + 100 chars in single-select buffer.
	// ** Provide more for multi-select.
	TCHAR FilenameBuffer[ _MAX_PATH + 100 ];
	FileDlg.m_ofn.lpstrFile =		FilenameBuffer;
	FileDlg.m_ofn.nMaxFile =		sizeof FilenameBuffer / sizeof TCHAR;

	// Make sure it starts out with a NULL!
	FilenameBuffer[0] = '\0';

	// Now bring it on up.
	int nResult = FileDlg.DoModal();
	
	// Process the result.
	if ( nResult == IDOK ) {

		*pstrName = FileDlg.m_ofn.lpstrFile;

	} else {

		*pstrName = _T("");

	}

	return nResult;

}
		

//-------------------------------------------------------------------//
// BuildUserBlock()																	//
//-------------------------------------------------------------------//
// This function builds a string describing the user.  It includes
// name, company and contact data.  The data is provided in wchar_t
// arrays, and returned as a CString with embedded newlines.
//-------------------------------------------------------------------//
void EMDBArray::BuildUserBlock(
	CString&	strAuthor,							// Result
	wchar_t*	pwcUser,
	wchar_t*	pwcCompany,
	wchar_t*	pwcContact,
	uLong		ulUserNumber,
	bool		bUseIndustrialStrengthDlgControlCRLF
) {

	CString strNewLine;
	if ( bUseIndustrialStrengthDlgControlCRLF )
		strNewLine = _T( "\015\015\012" );
	else
		strNewLine = _T( "\015\012" );

	strAuthor = pwcUser;
	strAuthor += strNewLine + pwcCompany;

	// Just in case pwcContact was not null-terminated, truncate it.
	CString strTemp = pwcContact;
	strTemp = strTemp.Left( nContactLength );

	// Parse each ';' in contact string into a line separator.
	int n;
	while ( ( n = strTemp.Find( _T(';') ) ) != -1 ) {
		strAuthor += strNewLine + strTemp.SpanExcluding( _T(";") );	
		strTemp = strTemp.Right( strTemp.GetLength() - n - 1 );
	}

	strAuthor += strNewLine + strTemp;

	// Add the "subscriber number".
	strTemp.Format( 
		// IDS_USER_BLOCK_SUBSCRIBER_LINE 
		_T("Subscriber #%d"), 
		ulUserNumber 
	);
	strAuthor += strNewLine + strTemp;
}


//-------------------------------------------------------------------//
// GetDisplayDBID()																	//
//-------------------------------------------------------------------//
CString EMDBArray::GetDisplayDBID( DatabaseID* pDBID )
{
	CString strID;
	strID.Format(
		_T("[%i-%i]"), 
		pDBID->UserNumber,
		pDBID->DBNumber
	);
	return strID;
}

	
//-------------------------------------------------------------------//
// GetTempDBName()																	//
//-------------------------------------------------------------------//
// This function builds the name used for temporary databases.
// It will also delete the existing temp db if there is one.
//-------------------------------------------------------------------//
void EMDBArray::GetTempDBName( CString* pstrTempDBName )
{
	GetDBBackupPath( pstrTempDBName );
	*pstrTempDBName += "TempDB";
	*pstrTempDBName += tWorkingDBExt;

	// Delete the db data file and corresponding index file
	// if they exist.
	CString strFilename = *pstrTempDBName;
	StripExtension( &strFilename );
	remove( LPCTSTR( *pstrTempDBName ) );
	remove( LPCTSTR( strFilename + tIndexExt ) );

}


//-------------------------------------------------------------------//
// SaveUserSettings()																//
//-------------------------------------------------------------------//
void EMDBArray::SaveUserSettings()
{
	pUserSettings->ChangeObject();
	pUserSettings->SaveObject();
}


//-------------------------------------------------------------------//
// AddDBToRegistryList()															//
//-------------------------------------------------------------------//
// This function adds the given database to the registry section.
//-------------------------------------------------------------------//
void EMDBArray::AddDBToRegistryList( 
	EMDatabase* pDatabase 
) {

	// Get the database section of the registry
	// and add the db filename.  RegistrySection
	// objects load/save in the con/destructor.
	RegistrySection DBSection( szDBRegSection );
	DBSection.Add( pDatabase->GetFilename() );

}


//-------------------------------------------------------------------//
// RemoveDBFromRegistryList()														//
//-------------------------------------------------------------------//
// This function removes the given database from the registry section.
//-------------------------------------------------------------------//
void EMDBArray::RemoveDBFromRegistryList( 
	EMDatabase* pDatabase 
) {

	// Get the database section of the registry.
	// RegistrySection objects load/save in the con/destructor.
	RegistrySection DBSection( szDBRegSection );

	DBSection.Remove( pDatabase->GetFilename() );

}


//-------------------------------------------------------------------//
// RefreshFromDefaultDir()															//
//-------------------------------------------------------------------//
// This function checks all the files in the database dir and 
// connects to any that are new and disconnects from any that
// have been removed.  It checks the filenames and the DBID's,
// including the version numbers.
//-------------------------------------------------------------------//
void EMDBArray::RefreshFromDefaultDir()
{

	// TO DO
	// Note that MainFrame::OnSetFocus(), which calls this, is 
	// calling this after focus loss from EM child windows, which 
	// it shouldn't.

}


//-------------------------------------------------------------------//
// NewComponent()																		//
//-------------------------------------------------------------------//
// Returns an EMComponent pointer to an object of the requested
// OBJECT_TYPE.  If pRef is NULL, this object is a new one from the
// target DB.  If pRef is passed, then the object returned is from
// the DB within the ObjectReference.
//-------------------------------------------------------------------//
EMComponent * EMDBArray::NewComponent(
	OBJECT_TYPE			eType,
	ObjectReference	* pRef
) {

	if ( pRef ) 
	{
		ObjectStream* pDB;
		LookUpDatabase( &pRef->DBID, &pDB );

		if ( pDB ) 
		{
			return ( (EMDatabase*) pDB )->NewComponent( eType, pRef );

		} else 
		{
			ASSERT( false );	// Bad DBID in pRef?
			return ( (EMDatabase*) pTargetDB )->NewComponent( eType );
		}

	} else 
	{
		return ( (EMDatabase*) pTargetDB )->NewComponent( eType );
	}
}

//-------------------------------------------------------------------//
// Create_DB_Menu()																	//
//-------------------------------------------------------------------//
// This function takes the passed CMenu pointer and creates popup
// menus of all the (active) databases.  If the DatabaseID pointer is
// passed, then the DB with that ID is excluded from the list.
//
// We deal with IDC_COPY_TO, IDC_MOVE_TO and IDC_CREATE_IN menu items.
//
// If the respective menu items aren't found then those menus are
// not created.
//
// Command ranges:
//
//		ID_DB_OBJECT_COMMAND_COPY_FIRST		0x1000
//		ID_DB_OBJECT_COMMAND_MOVE_FIRST		0x1400
//		ID_DB_OBJECT_COMMAND_NEW_FIRST		0x1800
//		ID_DB_OBJECT_COMMAND_NEW_LAST			0x2200
//
//-------------------------------------------------------------------//
void EMDBArray::Create_DB_Menu(
	CMenu			*pMenu,
	bool			bActiveOnly,
	DatabaseID	*pDBID
) {

	ASSERT( pMenu );

	int nCopyPos = 0, nMovePos = 0, nNewPos = 0;
	int nCount = pMenu->GetMenuItemCount();

	// Search for the "Copy To.." menu item IDC_COPY_TO.
	while ( ( IDC_COPY_TO != pMenu->GetMenuItemID( nCopyPos ) ) &&
		( nCopyPos < nCount ) ) nCopyPos++;

	// Search for the "Move To.." menu item IDC_MOVE_TO.
	while ( ( IDC_MOVE_TO != pMenu->GetMenuItemID( nMovePos ) ) &&
		( nMovePos < nCount ) ) nMovePos++;

	// Search for the "Create new in.." menu item IDC_CREATE_NEW_IN.
	while ( ( IDC_CREATE_NEW_IN != pMenu->GetMenuItemID( nNewPos ) ) &&
		( nNewPos < nCount ) ) nNewPos++;

	// If "Copy To.." menu item is grayed then don't touch it.
	if ( ( nCopyPos < nCount ) &&
		( MF_GRAYED & pMenu->GetMenuState( IDC_COPY_TO, MF_BYCOMMAND ) ) )
		nCopyPos = nCount;

	// If "Move To.." menu item is grayed then don't touch it.
	if ( ( nMovePos < nCount ) &&
		( MF_GRAYED & pMenu->GetMenuState( IDC_MOVE_TO, MF_BYCOMMAND ) ) )
		nMovePos = nCount;

	// If "Create new in.." menu item is grayed then don't touch it.
	if ( ( nNewPos < nCount ) &&
		( MF_GRAYED & pMenu->GetMenuState( IDC_CREATE_NEW_IN, MF_BYCOMMAND ) ) )
		nNewPos = nCount;

	// Found no menu? Our work is done.
	if ( 
			( nCopyPos == nCount ) 
		&& ( nMovePos == nCount ) 
		&& ( nNewPos == nCount ) 
	)
		return;

	// Create new menus to build up and add.
	CMenu CopyMenu, MoveMenu, NewMenu;
	if ( nCopyPos < nCount ) CopyMenu.CreatePopupMenu();
	if ( nMovePos < nCount ) MoveMenu.CreatePopupMenu();
	if ( nNewPos < nCount ) NewMenu.CreatePopupMenu();

	bool bAddedItems = false;

	// Loop through all the databases.
	for ( int nA = 0; nA < Databases.GetSize(); nA++ ) {

		// Use a local pointer to save multiple array lookups.
		EMDatabase *pDatabase = dynamic_cast<EMDatabase*> ( Databases[ nA ] );

		// Skip published databases.
		if ( bActiveOnly && pDatabase->IsPublished() )
			continue;

		// Skip database with same DBID as passed DBID ptr.
		if ( pDBID && ( *pDBID == pDatabase->GetDBID() ) )
			continue;

		// Get the DB name.
		CString strName( pDatabase->GetDatabaseName() );

		// We now have at least one item in the menu.
		bAddedItems = true;

		// Add the DB name string to the menus.
		if ( nCopyPos < nCount )
			CopyMenu.AppendMenu( MF_BYPOSITION | MF_STRING,
				nA + ID_DB_OBJECT_COMMAND_COPY_FIRST, strName );

		if ( nMovePos < nCount )
			MoveMenu.AppendMenu( MF_BYPOSITION | MF_STRING,
				nA + ID_DB_OBJECT_COMMAND_MOVE_FIRST, strName );

		if ( nNewPos < nCount )
			NewMenu.AppendMenu( MF_BYPOSITION | MF_STRING,
				nA + ID_DB_OBJECT_COMMAND_NEW_FIRST, strName );

	}

	CString strMenu;

	if ( nCopyPos < nCount ) {

		// Get the menu string, delete the menu item.
		pMenu->GetMenuString( IDC_COPY_TO, strMenu, MF_BYCOMMAND );
		pMenu->RemoveMenu( IDC_COPY_TO, MF_BYCOMMAND );

		// Add the popup menu in its place.
		if ( bAddedItems ) {
			pMenu->InsertMenu( nCopyPos, MF_POPUP | MF_BYPOSITION, (UINT)CopyMenu.m_hMenu, strMenu );
			CopyMenu.Detach();
		}

	}

	if ( nMovePos < nCount ) {

		// Get the menu string, delete the menu item.
		pMenu->GetMenuString( IDC_MOVE_TO, strMenu, MF_BYCOMMAND );
		pMenu->RemoveMenu( IDC_MOVE_TO, MF_BYCOMMAND );

		// Add the popup menu in its place.
		if ( bAddedItems ) {
			pMenu->InsertMenu( nMovePos, MF_POPUP | MF_BYPOSITION, (UINT)MoveMenu.m_hMenu, strMenu );
			MoveMenu.Detach();
		}

	}

	if ( nNewPos < nCount ) {

		// Get the menu string, delete the menu item.
		pMenu->GetMenuString( IDC_CREATE_NEW_IN, strMenu, MF_BYCOMMAND );
		pMenu->RemoveMenu( IDC_CREATE_NEW_IN, MF_BYCOMMAND );

		// Add the popup menu in its place.
		if ( bAddedItems ) {
			pMenu->InsertMenu( nNewPos, MF_POPUP | MF_BYPOSITION, (UINT)NewMenu.m_hMenu, strMenu );
			NewMenu.Detach();
		}

	}

}


//-------------------------------------------------------------------//
// UpdateAllCaches()																	//
//-------------------------------------------------------------------//
// Here, we update all caches for all Earthmover databases that
// we are currently connected to.  We typically do this when
//	we are about to exit the program, so we are ready to go the next
// time we start the program.
//-------------------------------------------------------------------//
void EMDBArray::UpdateAllCaches()
{

	// This MAY take a minute...
	CWaitCursor WheresTheFireBuddy;

	int nCount = Databases.GetSize();
	for ( int i = 0; i < nCount; i++ )
		if ( !Databases[i]->IsPublished() )
			( dynamic_cast<EMDatabase*> ( Databases[i] ) )->UpdateCaches();

}


