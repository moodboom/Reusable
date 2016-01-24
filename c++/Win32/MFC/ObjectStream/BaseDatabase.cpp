//-------------------------------------------------------------------//
// BaseDatabase.cpp
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include <io.h>						// For remove(), rename()

#include <new>							// STL versions, no '.h'
#include <cstdlib>
#include <string>

#include <stdio.h>					// For rand
#include <time.h>						// For rand

// #include <hasp.h>					// For HASP security key

#include <CStringHelpers.h>		// For StripExtension, ExtractFileNameFromPath

#include <CFileHelpers.h>			// For VerifyFileIsWritable

#include "DatabaseArray.h"			// For data file handler functions

// #include "..\Dongle\Dongle.h"		// To ascertain group access.

#include <DisplayMessage.h>
#include <BaseProgressDlg.h>

#include "BaseDatabase.h"


///////////////////////
// Initialize statics.
///////////////////////


// We use this var to specify the expected size of the plain header.
const uShort uExpectedPHSize = 
		sizeof uShort						// for uObjectStreamPlainHeaderSize
	+	sizeof uHuge;						// for FileVersionLastModified;

// We use this var to specify the expected size of the encrypted header.
const uShort uExpectedEHSize = 
		sizeof uShort						// for uObjectStreamEncryptedHeaderSize
	+	sizeof BaseDatabaseHeader;		// for DBHeader;

// Establish the database file name extensions.
static const TCHAR tIndexExt[]			= _T(".rtx");
static const TCHAR tWorkingDBExt[]		= _T(".rtw");
static const TCHAR tPublishedDBExt[]	= _T(".rth");


//-------------------------------------------------------------------//
// BaseDatabase()																				//
//-------------------------------------------------------------------//
// The constructor is just used to initialize data.
// Initialization occurs in a separate Initialize() function.  
// This is done for two MAJOR reasons that apply to constructors
// in general: 
//
//		1) The first is simply because the constructor cannot return 
//			a result value.  We would have to either add a return value
//			param, or use exception handling to provide a result value.  
//		2) The second is because functions overridden in derived 
//			classes cannot be resolved within the constructor in C++.  
//			Any call to an overridden function here would only
//			execute the base version.
//
// See Initialize().
//-------------------------------------------------------------------//
BaseDatabase::BaseDatabase( 
	CString* pstrFileName, 
	int		nNewClassCount
) :

	// Init vars.
	datafile( pstrFileName ),
	indexfile( &( StripExtension( *pstrFileName ) + tIndexExt ) ),
	m_nClassCount( nNewClassCount ),
	FileVersionLastModified( 0 ),
	
	// Set these to the current size.
	uObjectStreamPlainHeaderSize( uExpectedPHSize ),
	uObjectStreamEncryptedHeaderSize( uExpectedEHSize )

{
}


//-------------------------------------------------------------------//
// ~BaseDatabase()																	//
//-------------------------------------------------------------------//
// ---------- close the BaseDatabase database
//-------------------------------------------------------------------//
BaseDatabase::~BaseDatabase()
{
	
	PdyBtree *bt = btrees.FirstEntry();
	while (bt != 0)	{
		delete bt;
		bt = btrees.NextEntry();
	}

	Class *cls = classes.FirstEntry();
	while (cls != 0)	{

		delete [] cls->classname;
		
		delete cls;
		cls = classes.NextEntry();
	}

}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// If the file is expected to exist, only the filename is required.
// If the file is not expected to exist, all the parameters that
// define the database should be supplied.  The granularity is
// always optional; the default parameters should usually suffice.
//
// The return value is one of the following:
//
//		DB_INIT_SUCCESS*				Initialization succeeded.
//
//		DB_INIT_BAD_DATAFILE			The data file was not found 
//											when it should have been.
//
//		DB_INIT_BAD_GRANULARITY		The granularity was invalid.
//
//		DB_INIT_REINDEX_FAILED*		The index file was not found,
//											or a reindex was requested,
//											and the reindex failed.
// 
//		DB_INIT_REGISTER_FAILED*	Class registration failed, even
//											after a reindex.
//
//	(*)	Note that the encryption engine will be initialized for 
//			these cases.
//
// TO DO
// Add return values for file open errors, etc.
//
//-------------------------------------------------------------------//
DB_INIT_RESULT BaseDatabase::Initialize(
	bool				bFileShouldExist,
	const wstring*	pwsDatabaseName,
	DatabaseID*		pDBID,
	uLong				Group,
	uLong				DBVersion,
	bool				bPublished,
	bool				bForceReindex,
	short int		nDataGranularity,
	short int		nIndexGranularity
) {

	// We are about to initialize a data file.  If bFileShouldExist is false,
	// we assume that the file does not exist, and we assign any passed params.
	// Then, if the file exists, the param data will be overwritten.  If it does 
	// not, our data will be used in a new header for the file.
	if ( !bFileShouldExist ) {

		// Param data should be available.
		ASSERT( pDBID );
		ASSERT( pwsDatabaseName );

		// Set up the header as requested.
		DBHeader.DBID = *pDBID;
		DBHeader.Group = Group;
		DBHeader.SetPublishedStatus( bPublished );
		DBHeader.DBVersion = DBVersion;

		// Here's how you do a strncpy from a wstring to a wchar_t* pointer (which
		// happens to point to a static wchar_t array).  Remember, this will pad the target to
		// the end with NULL, unless the source is longer than the target.  This is acceptable here.
		wcsncpy( 
			DBHeader.DBName, 
			pwsDatabaseName->c_str(),
			sizeof DBHeader.DBName / sizeof ( wchar_t )
		);
		
		// Set the database version to the application's db version.
		DBHeader.FileVersionCreated = GetCurrentFileVersion();
		FileVersionLastModified = GetCurrentFileVersion();

	}

	// Init the data file.
	int nDataInitResult = datafile.Initialize( 
		nDataGranularity,
		!bFileShouldExist,		// bCreateAsNeeded
		this							// BaseDatabase pointer, provides extended header functions
	);

	if ( nDataInitResult == NFI_ENCRYPTION_FAILURE )
		return DB_INIT_UNAUTHORIZED;
	else if ( nDataInitResult == NFI_FILE_NOT_FOUND )
		return DB_INIT_DATAFILE_NOT_FOUND;

	// Make sure the amount of node space available
	// for storing an encrypted object is an even
	// multiple of 8.
	if ( ( datafile.GetNodeDataLength() - sizeof ObjectHeader ) % 8 != 0 )
		return DB_INIT_BAD_GRANULARITY;
	

	// We have read the header.  Before setting up for encryption, do any
	// versioning update on the header.  We will likely be using header data
	// when setting up the encryption.
	UpdateDatabaseFormat();
	
	// ---------------------------------------------------------
	// Set up for en/decryption.
	// This is required before we attempt a reindex.
	// ---------------------------------------------------------
	uByte* pObjectKey;
	uShort ObjectKeyBytes;
	if ( 
		!GetObjectKey( 
			&pObjectKey, 
			&ObjectKeyBytes
		)
	)
		return DB_INIT_UNAUTHORIZED;

	blow.InitializeSubKeys( 
		pObjectKey, 
		ObjectKeyBytes
	);

	// Clean up.
	delete pObjectKey;

	// ---------------------------------------------------------

	
	// Init the index file.  If not found, try to create it.
	// If a reindex was requested, jump in here only after
	// calling initialize to set the granularity.
	if ( 
			indexfile.Initialize( 
				nIndexGranularity, 
				!bFileShouldExist				// bCreateAsNeeded
			) != NFI_SUCCESS
		||	bForceReindex
	) {
	
		// Use a progress dialog.
		CWaitCursor WaitAMinuteBub;
		BaseProgressDlg ProgressDlg;
		SetProgressTitle(
			&ProgressDlg,
			IDS_DB_REPAIRING
		);
		
		RebuildIndexFile(
			&ProgressDlg.ProgressMeter,
			0,
			100,
			bForceReindex							// bLog = true if forcing a reindex
		);
	
		// Clean up.
		ProgressDlg.DestroyWindow();

		// Try again.
		if ( 
			indexfile.Initialize( 
				nIndexGranularity, 
				!bFileShouldExist				// bCreateAsNeeded
			) != NFI_SUCCESS
		)
			return DB_INIT_REINDEX_FAILED;

	}


	// Make sure that all classes are registered.
	// This is required for us to be able to rebuild
	// the index file, and to use functions such as
	// FirstObject(), which expect objects to be
	// registered.
	// If it fails, we can try a reindex to fix it.
	if ( !RegisterClasses() ) {
	
		// Use a progress dialog.
		CWaitCursor WaitAMinuteBub;
		BaseProgressDlg ProgressDlg;
		SetProgressTitle(
			&ProgressDlg,
			IDS_DB_REPAIRING
		);
		
		RebuildIndexFile(
			&ProgressDlg.ProgressMeter
		);
	
		// Try again.
		if ( !RegisterClasses() )
			return DB_INIT_REGISTER_FAILED;
	
	}

	return DB_INIT_SUCCESS;

}


//-------------------------------------------------------------------//
// UpdateDatabaseAsNeeded()														//
//-------------------------------------------------------------------//
// This function is called externally at the appropriate time to
// make sure the database has been updated to the current version.
// If it has not, an update is attempted via a call to 
// UpdateOldObjects().
//
// Originally, this functionality was contained in Initialize().
// But we found it was required to call the update at a specific
// time after the database was initialized.  ( We needed EMDBArray
// to be set up before doing the updating.)  So it was moved to
// a separate function.
//-------------------------------------------------------------------//
bool BaseDatabase::UpdateDatabaseAsNeeded()
{

	// Before we actually load any objects, we check the 
	// version number of the database.  If the version number
	// is old, we want to update objects.
	if ( GetCurrentFileVersion() > FileVersionLastModified ) {
	
		// Back up the database, with the version number 
		// appended to the name.
		Backup( true );

		// Attempt a global update.
		if ( UpdateOldObjects() ) 
		{
		
			FileVersionLastModified = GetCurrentFileVersion();
			datafile.Open( true );
			WriteHeader();
			datafile.Close( true );
			return true;

		}
		else
		{
		
			// Tell the user that the update failed.
			CString strMsg;
			strMsg.Format(
				IsPublished() ?
						IDS_DB_UPDATE_PUBLISHED_PROBLEM
					:	IDS_DB_UPDATE_UNKNOWN_PROBLEM, 
				CString( GetDatabaseName() )
			);

			DisplayMessage( 
				strMsg,
				IDL_WARNING, 
				0, 
				MB_OK
			);

			// Remove the database files.  
			// (The current files are trashed, and there already is a backup.)
			DeleteFiles();

			return false;
	
		}
		
	} else
		return true;

}


//-------------------------------------------------------------------//
// Open()																				//
//-------------------------------------------------------------------//
// This opens the file.
//-------------------------------------------------------------------//
bool BaseDatabase::Open( bool bWritable )
{ 
		
	// TO DO
	// error checking

	// TO DO
	// Consider rebuilding the index file if it is new
	// but the database file is not.
	
	indexfile.Open( bWritable );
	datafile.Open( bWritable );

	return true;

}


//-------------------------------------------------------------------//
// Close()																				//
//-------------------------------------------------------------------//
void BaseDatabase::Close( bool bWritable )
{
	
	datafile.Close( bWritable );
	indexfile.Close( bWritable );
	
}


//-------------------------------------------------------------------//
// ReadHeader()																		//
//-------------------------------------------------------------------//
// This function reads the data file header, which can be expanded
// by derived classes.
//-------------------------------------------------------------------//
bool BaseDatabase::ReadHeader()
{ 
	
	ReadPlainHeader();
	
	uByte* pHeaderKey;
	uShort uHeaderKeyBytes;
	GetHeaderKey(
		&pHeaderKey,
		&uHeaderKeyBytes
	);

	// Initialize a new encryption engine 
	// with the header key.
	pHeaderEncryptor = new BlowFish;
	pHeaderEncryptor->InitializeSubKeys(
		(uByte*) pHeaderKey,
		uHeaderKeyBytes
	);

	// Clean up the header key now that
	// we are done with it.
	delete pHeaderKey;

	bool bReturn = ReadEncryptedHeader();

	// Clean up the encryptor object.
	delete pHeaderEncryptor;

	return bReturn;

}


//-------------------------------------------------------------------//
// ReadPlainHeader()																	//
//-------------------------------------------------------------------//
// This function reads the unencrypted portion of the header.
// It may be overridden by derived classes that require their own
// unencrypted data.
// Update the static uExpectedPHSize as needed when adding a new
// header version to this base class.
//-------------------------------------------------------------------//
void BaseDatabase::ReadPlainHeader()
{

	// Read the size of the BaseDatabase plain header.
	datafile.ReadData(
		&uObjectStreamPlainHeaderSize,
		sizeof uObjectStreamPlainHeaderSize,
		sizeof FileHeader						// offset
	);
	
	if ( uObjectStreamPlainHeaderSize == uExpectedPHSize ) {

		// Read the last modified file version.
		datafile.ReadData(
			&FileVersionLastModified,			// buffer
			sizeof FileVersionLastModified	// bytes to read
		);

	// Handle old versions here.  We don't have any right now.
	// } else if ( uObjectStreamPlainHeaderSize == uOldPHSize1 ) {
	
	} else {

		// This database has an unknown format!
		DisplayMessage( IDM_UNKNOWN_HEADER );
	
	}

}


//-------------------------------------------------------------------//
// ReadEncryptedHeader()															//
//-------------------------------------------------------------------//
// This function reads the encrypted portion of the header.
// It may be overridden by derived classes that require their own
// encrypted data.
// Update the static uExpectedEHSize as needed when adding a new
// header version.
//-------------------------------------------------------------------//
bool BaseDatabase::ReadEncryptedHeader()
{

	bool bReturn = true;

	// Read the size of the BaseDatabase encrypted header.
	datafile.ReadData(
		&uObjectStreamEncryptedHeaderSize,
		sizeof uObjectStreamEncryptedHeaderSize
	);
	
	// We'll hit this if we are reading a database that was not created
	// with the current version of the app.  It's OK if the size we read is
	// smaller than expected; it should not be bigger.
	if ( uObjectStreamEncryptedHeaderSize == uExpectedEHSize ) {
	
		// Read the encrypted header block.  We are already at the
		// correct file offset.
		datafile.ReadData(
			&DBHeader,			// buffer
			sizeof DBHeader	// bytes to read
		);

		// Decrypt the header.
		VERIFY( 
			pHeaderEncryptor->StreamDecrypt(
				&DBHeader, 
				sizeof DBHeader
			)
		);

	// Handle old versions here.  We don't have any right now.
	// } else if ( uObjectStreamEncryptedHeaderSize == uOldEHSize1 ) {
	
	} else {

		// This database has an unknown format!
		// This may be the first time that we encounter a problem with
		// the database.  We should pass back the error.
		// DisplayMessage( IDM_UNKNOWN_HEADER );
		bReturn = false;
	
	}

	return bReturn;

}


//-------------------------------------------------------------------//
// WriteHeader()																		//
//-------------------------------------------------------------------//
// This function writes the data file header, which can be expanded
// by derived classes.
//-------------------------------------------------------------------//
void BaseDatabase::WriteHeader()
{ 
	
	WritePlainHeader();
	
	uByte* pHeaderKey;
	uShort uHeaderKeyBytes;
	GetHeaderKey(
		&pHeaderKey,
		&uHeaderKeyBytes
	);

	// Initialize a new encryption engine 
	// with the header key.
	pHeaderEncryptor = new BlowFish;
	pHeaderEncryptor->InitializeSubKeys(
		(uByte*) pHeaderKey,
		uHeaderKeyBytes
	);

	// Clean up the header key now that
	// we are done with it.
	delete pHeaderKey;

	WriteEncryptedHeader();

	// Clean up the encryptor object.
	delete pHeaderEncryptor;

}


//-------------------------------------------------------------------//
// WritePlainHeader()																//
//-------------------------------------------------------------------//
// This function writes the unencrypted portion of the header.
// It may be overridden by derived classes that require their own
// unencrypted data.
//-------------------------------------------------------------------//
void BaseDatabase::WritePlainHeader()
{

	if ( uObjectStreamPlainHeaderSize == uExpectedPHSize ) {

		// Write the size of the header.
		datafile.WriteData(
			&uObjectStreamPlainHeaderSize,			// buffer
			sizeof uObjectStreamPlainHeaderSize,	// bytes to write
			sizeof FileHeader						// offset
		);

		// Write the last modified file version.
		datafile.WriteData(
			&FileVersionLastModified,			// buffer
			sizeof FileVersionLastModified	// bytes to write
		);

	// Handle conversions here.
	// } else if ( uObjectStreamPlainHeaderSize == uOldPHSize1 ) {

	} else {
	
		// This database has an unknown format!
		DisplayMessage( IDM_UNKNOWN_HEADER );
	
	}

}


//-------------------------------------------------------------------//
// WriteEncryptedHeader()															//
//-------------------------------------------------------------------//
// This function writes the encrypted portion of the header.
// It may be overridden by derived classes that require their own
// encrypted data.
//-------------------------------------------------------------------//
void BaseDatabase::WriteEncryptedHeader()
{

	if ( uObjectStreamEncryptedHeaderSize == uExpectedEHSize ) {

		// Write the size of the header.
		datafile.WriteData(
			&uObjectStreamEncryptedHeaderSize,			// buffer
			sizeof uObjectStreamEncryptedHeaderSize	// bytes to write
		);

		// Make a copy of the header; we don't want to trash the original.
		BaseDatabaseHeader EncryptedHeader;
		memcpy(
			&EncryptedHeader,
			&DBHeader,
			sizeof DBHeader
		);
		
		// Encrypt the header.
		VERIFY( 
			pHeaderEncryptor->StreamEncrypt(
				&EncryptedHeader, 
				sizeof EncryptedHeader
			)
		);

		// Write the encrypted header block.  We are already at the
		// correct file offset.
		datafile.WriteData(
			&EncryptedHeader,			// buffer
			sizeof EncryptedHeader	// bytes to read
		);

	// Handle conversions here.
	// } else if ( uObjectStreamEncryptedHeaderSize == uOldEHSize1 ) {

	} else {
	
		// This database has an unknown format!
		DisplayMessage( IDM_UNKNOWN_HEADER );
	
	}

}


//-------------------------------------------------------------------//
// Get/SetCacheStatus()																//
//-------------------------------------------------------------------//
// OLD CLASS HEADER ACCESS FUNCTIONS
// These functions were used to access a bool in the class header.
// The bool was moved to within an object.  The storage space is
// still available in the class header.  However, it is recommended
// that data be maintained within objects whenever practical.
//
// These manipulate the CacheStatus, kept in the class header.
// See GetHighestObjectID() for class header details.
//-------------------------------------------------------------------//
/* 
bool BaseDatabase::GetCacheStatus( ClassID CID )
{
		
	bool bStatus = false;
	Class* pClass = GetClass( CID );

	// Get the latest status directly out of the 
	// database, if available.
	if ( pClass )

		indexfile.ReadData( 
			&bStatus, 
			sizeof bool, 
				pClass->headeraddr 
			+	(streampos) sizeof ObjectID
		);

	return bStatus;

}
void BaseDatabase::SetCacheStatus( 
	ClassID	CID,
	bool		bNewAvailableStatus
) {
		
	// Save it, if class is available.
	// ASSERT if not.
	Class* pClass = GetClass( CID );
	ASSERT( pClass );
	if ( pClass )

		indexfile.WriteData(
			&bNewAvailableStatus, 
			sizeof bool, 
				pClass->headeraddr 
			+	(streampos) sizeof ObjectID
		);

}
*/

//-------------------------------------------------------------------//
// GetCurrentFileVersion()															//
//-------------------------------------------------------------------//
// This function did not work.  We're better off using our own 
// version numbers that have specific purposes, anyway.
// Here are the original comments:
//
// All this hacky code is just to get the version out of the standard
// Version resource object, geesh.  
// We also have to link "version.lib" into the project.
// It was pieced together from MSJ v12#2 "Under The Hood" and
// some Visual Basic sample code.  MS's documentation is lacking.
// Next time, we use our own dang number.  :>
//-------------------------------------------------------------------//
/*
uHuge BaseDatabase::GetCurrentFileVersion() 
{

	// The version will be zeroed unless the extraction succeeds.
	uHuge uhVersion = 0;

	DWORD dwTemp;
	unsigned int nTemp;
	const int nMaxPath = 350;
	TCHAR* pszEMPath = new TCHAR[ nMaxPath ];
	GetModuleFileName(
		0,							// NULL = current
		pszEMPath,
		nMaxPath
	);
	DWORD dwVerSize = GetFileVersionInfoSize(
		pszEMPath,	// pointer to filename string
		&dwTemp		// pointer to variable to receive zero
	);

	if ( dwVerSize ) {

		// Allocate space to hold the info
		PBYTE pVerInfo = new BYTE[dwVerSize];

		if ( 
			GetFileVersionInfo(
				pszEMPath,	// pointer to filename string
				0,				// ignored
				dwVerSize,	// size of buffer
				pVerInfo		// pointer to buffer to receive file-version info.
			)
		) {
	
			// TO DO
			// Here's a note from some Visual Basic sample code.  We're supposed to
			// do some sort of multilingual code lookups here.
			// 'now we change the order of the language id and code page
			// 'and convert it into a string representation.
			// 'For example, it may look like 040904E4    
			// 'Or to pull it all apart:
			// '04------        = SUBLANG_ENGLISH_USA    
			// '--09----        = LANG_ENGLISH
			// '----04E4 = 1252 = Codepage for Windows:Multilingual				
			PSTR pszVerRetVal, pszStrEnd;
			VerQueryValue(
				pVerInfo, 
				TEXT("\\StringFileInfo\\040904E4\\FileVersion"), 
				(LPVOID *)&pszVerRetVal,               
				&nTemp
			); 

			uhVersion = (uHuge) strtod( 
				pszVerRetVal,
				&pszStrEnd
			);

		}

		delete []pVerInfo;

	}

	delete pszEMPath;


	return uhVersion;

}
*/
		

//-------------------------------------------------------------------//
// GetClassID()																		//
//-------------------------------------------------------------------//
// This function returns the class id for the given persistent
// object.  We assume that the class has been pre-registered, so
// the lookup will be successful.  If it fails for any reason, we
// return -1.
//-------------------------------------------------------------------//
ClassID BaseDatabase::GetClassID( const Persistent* pObject )
{

	// Get the actual name of our class.
	const char* ty = typeid( *pObject ).name();
	
	// MDM Don't bother checking the "class " that
	// preceeds each class name returned from typeid.
	ty += 6;
	
	Class *cls = classes.FirstEntry();
	while (cls != 0)	{
		
		if (strcmp(cls->classname, ty) == 0)
			break;
		cls = classes.NextEntry();
	}

	return cls? cls->classid: -1;

}


//-------------------------------------------------------------------//
// GetClass()																			//
//-------------------------------------------------------------------//
// This function returns the class object for the given ID.
// It is used for example to get the class header offset when 
// obtaining the cache status.
//-------------------------------------------------------------------//
Class* BaseDatabase::GetClass( ClassID CID )
{

	// Find the class.
	Class* cls = classes.FirstEntry();
	while (cls != 0) {
		if ( cls->classid == CID )
			break;
		cls = classes.NextEntry();
	}

	return cls;

}


//-------------------------------------------------------------------//
// FindClass()																			//
//-------------------------------------------------------------------//
// TO DO 
// Break this into two functions, AddClass and GetClassNode.
// AddClass should be used by RegisterClass, and GetClassNode
// should be used everywhere else.  It should just loop through
// the class nodes a fixed number of times, depending on the
// class.
//-------------------------------------------------------------------//
bool BaseDatabase::FindClass(Class *cls, NodeNbr *nd)
{
	char classname[classnamesize];
	ClassID cid = 0;
	if (!indexfile.IsNewFile())	{
		
		Node* pLoopNode;
		
		NodeNbr nx = 1;
		// ------- locate the class header
		while (nx != 0)	{

			// MDM This node exists, use default third parameter.
			// Note that we would be in danger here if we assumed 
			// that any existing file has a valid node 1.  If the 
			// database file is first created, then it is closed 
			// without registering any classes, then it is reopened, 
			// we would look for node 1 and not find it.
			// We handle this situation in NodeFile::Initialize() by 
			// setting newfile to true if the file does not have
			// node 1 yet.
			// Note that we only read the class name, not the whole node.
			pLoopNode = new Node(
				&indexfile,
				nx,
				NM_SCAN,
				classnamesize
			);
			
			pLoopNode->GetFromBuffer(
				classname, 
				classnamesize
			);

			if ( strcmp( classname, cls->classname ) == 0 ) {
				
				// Calc the class header address.
				cls->headeraddr = GetObjectHeaderAddress( pLoopNode );
				
				// Get the class ID.
				cls->classid = cid;
				
				// Clean up and return.
				delete pLoopNode;
				return true;

			}

			// --- this node is not the class header
			cid++;
			nx = pLoopNode->NextNode();
		
			// If this is the last node, we want to save it
			// so we can update its next node when adding
			// a new one.  Otherwise, delete and loop.
			if ( nx )
				delete pLoopNode;

		}

		if (nd != 0)	{
			
			// We now create a brand new node to hold 
			// the class registration.
			Node ClassNode(
				&indexfile,
				0,					// New node, use 0
				NM_CREATE
			);

			// Copy the new node number.
			*nd = ClassNode.GetNodeNbr();
			
			// Place the new class node after the last in the list.
			// TO DO
			// This will save the entire node.  All we need to
			// save is the new next node.  Optimize.  See above.
			pLoopNode->SetNextNode(*nd);
			pLoopNode->MarkNodeChanged();

		}

		// Now we can clean up.
		delete pLoopNode;

	}
	cls->classid = cid;
	return false;
}


//-------------------------------------------------------------------//
// AddClassToIndex()																	//
//-------------------------------------------------------------------//
void BaseDatabase::AddClassToIndex(
	Class *cls
) {

	NodeNbr nd = 0;

	bool bFoundClass = FindClass(cls, &nd );

	// FindClass fails under two conditions:
	//
	//		1) we have a new file; or
	//		2) we didn't find the class in existing nodes, so
	//			we created a new one.
	//
	// nd was updated with a valid node only for condition 2.
	if ( bFoundClass == false ) {

		// We no longer have a new index file, we
		// are adding a node.
		indexfile.ResetNewFile();
		
		// ------- build the class header for new class
		// This uses the node we found in the call to FindClass, if any, but
		// disregards its previous contents.  This is fine, as we are re-creating
		// all the contents.
		Node tmpnode(
			&indexfile, 
			nd,					
			NM_CREATE
		);

		// Calc the class header address.
		cls->headeraddr = GetObjectHeaderAddress( &tmpnode );

		
		// The rest of the buffer contains the "class header".

		// CLASS HEADER DETAILS
		// --------------------
		//
		// The format of the class header node is as follows:
		//
		//		4	NextNode				(not really in "class header", considered in "node header")
		//	  16	ClassName
		//		4	HighestObjectID
		//		1	Unused
		//		4	Primary key btree start nodenbr
		//		2	Length of key
		//		2	(fill)
		//		4	Key 2 btree start nodenbr
		//		2	Length of key
		//		2	(fill)
		//		4	Key 3 btree start nodenbr
		//		2	Length of key
		//		2	(fill)
		//		etc.
		//
		// If you change the format of the header, make sure you update PdyBtree::HdrPos().
		//
		// HighestObjectID should be set to zero.  If we are dealing with a reindex, 
		// HighestObjectID will be updated towards the end of the reindex.
		//
		// The next byte is currently unused.  (It was previously for bCacheAvailable.)
		//
		// The key data should be initialized to zero. When keys are registered, if the key data is
		// not zero, the value is validated.  If the key data is zero, it is updated.
		//

		int residual = indexfile.GetNodeDataLength();
		
		// Put the class name into the node buffer.
		tmpnode.PutToBuffer( cls->classname, classnamesize );
		residual -= classnamesize;

		// Zero out the HighestObjectID.
		ObjectID HID = 0;
		tmpnode.PutToBuffer( &HID, sizeof HID );
		residual -= sizeof HID;

		// The next byte is available for future use.
		bool bUnused = true;
		tmpnode.PutToBuffer( &bUnused, sizeof bUnused );
		residual -= sizeof bUnused;
		
		// Watch out for very small granularity.
		ASSERT( residual >= 8 );
		
		char *residue = new char[residual];
		memset(residue, 0, residual);
		tmpnode.PutToBuffer( 
			residue, 
			residual
		);

		// delete residue;
		delete [] residue;

		// Mark the node changed, it'll be saved when it
		// goes out of scope next.
		tmpnode.MarkNodeChanged();

	}

}


//-------------------------------------------------------------------//
// RegisterIndexes()																	//
//-------------------------------------------------------------------//
// ----- register a class's indexes with the database manager
//-------------------------------------------------------------------//
void BaseDatabase::RegisterIndexes(Class *cls, const Persistent& pcls)
			throw (ZeroLengthKey)
{
	
	// Cast off const.
	Persistent& cl = const_cast<Persistent&>(pcls);
	
	// Loop through all the keys for this persistent class.
	PdyKey *key = cl.keys.FirstEntry();
	while (key != 0)	{
		
		// Make sure we have a valid key.
		if (key->GetKeyLength() == 0)
			throw ZeroLengthKey();
		
		// Create a NEW btree in the index file.
		PdyBtree *bt = new PdyBtree(indexfile, cls, key);
		bt->SetClassIndexed(cls);
		
		// Add the btree to our list.
		btrees.AppendEntry(bt);

		// Get the next key for the class.
		key = cl.keys.NextEntry();

	}
}


//-------------------------------------------------------------------//
// RegisterClass()																	//
//-------------------------------------------------------------------//
// ---- register a persistent class with the database manager
// This function calls AddClassToIndex(), which checks the index 
// file to see if the class has been registered there.  If it was 
// not found, it registers a new class.
//-------------------------------------------------------------------//
void BaseDatabase::RegisterClass(
	const Persistent&	pcls,
	ClassID*				pResultID
) {

	// Turn off the stupid target object pointer,
	// so the new keys we create aren't erroneously
	// registered as object keys under it.
	Persistent::pTargetObject = 0;

	// We need to attempt to create a new class.
	// Get a new object, and the object name.
	Class* cls = new Class;
	const char* cn = typeid(pcls).name();

	// MDM Don't bother checking the "class ..." that
	// preceeds each class name returned from typeid.
	cn += 6;
	
	// MDM We can save the classname length here.
	// We will need it twice below.
	// cls->classname = new char[strlen(cn)+1];
	size_t ActualClassNameLength = strlen( cn );
	cls->classname = new char[ ActualClassNameLength +1 ];
	
	
	// Make sure we don't exceed classnamesize limit.  
	if ( ActualClassNameLength >= classnamesize ) {
	
		// Plug a NULL at limit.
		// Note that the classnamesize limit is exceeded;
		// it must be assured by the programmer that the
		// first 15 char's of all classnames must be unique.
		(char)( *( cn + classnamesize - 1 ) ) = '\0';
	
	}
	
	// DO NOT USE strncpy HERE!  It pads with NULLs, which will
	// cause overrun in any strings shorter than classnamesize.
	strcpy( cls->classname, cn );

	// ---- search the index file for the class
	// This will create the class header node as needed.
	AddClassToIndex( 
		cls
	);

	// ---- register the indexes
	RegisterIndexes(cls, pcls);

	classes.AppendEntry(cls);

	*pResultID = cls->classid;

}


//-------------------------------------------------------------------//
// RegisterClasses()																	//
//-------------------------------------------------------------------//
// Here, we add class headers to the index file for
// all known database classes.  The ClassID's for all known classes
// should start at 0 and increment upwards.
//
// This function is called as the first step in creating or
// connecting to a database.   It has two purposes: 
//
//		1) It provides consistent ClassID's.  This allows us to
//			recreate the index file using the ClassID's in the 
//			database file.
//		2) It pre-registers all classes so that navigation functions 
//			such as FirstObject() and NextObject() are available.
//
// If we are unable to register the ClassID's with the expected
// values, then we return false.  We can't use the database, as
// it has objects registered that we do not know about, or the
// registration was done out of order.
//
// Note that if you wish to permanently remove a class from the 
// database, you MAY remove it from the list below, but you will
// need to add the capability to increment the next classid so
// the old classid is skipped and NEVER reused.
//-------------------------------------------------------------------//
bool BaseDatabase::RegisterClasses()
{

	bool bReturn = ( m_nClassCount > 0 );

	// We are about to force the index file to be opened for writing.
	// We'll force it to be writable.  
	// Published data files can remain read-only.
	VerifyFileIsWritable( 
		LPCTSTR( indexfile.strFilename	)
	);

	// Open the database files, the data file for reading
	// and the index file for writing.
	indexfile.Open( true );
	datafile.Open( false );

	// Register the classes using temp objects.
	// We verify the registration result via bReturn.
	Persistent* pTemp;
	ClassID		ResultID;

	for ( ClassID i = 0; i < m_nClassCount; i++ ) {

		pTemp = NewObject( i );
		RegisterClass( *pTemp, &ResultID );
		bReturn &= ( ResultID == i ); 
		delete pTemp;

	}

	// Close the database files.
	datafile.Close( false );
	indexfile.Close( true );

	return bReturn;

}


//-------------------------------------------------------------------//
// GetPercentCompacted()															//
//-------------------------------------------------------------------//
// This function determines the percentage of the data file that is
// not deleted.
//-------------------------------------------------------------------//
int BaseDatabase::GetPercentCompacted()
{

	int DeleteCount = 0;

	datafile.Open( false );
	datafile.ReadHeader();

	NodeNbr nd = datafile.DeletedNode();
	
	while ( nd ) {

		DeleteCount++;
		
		// We scan the deleted node.  This will just read the next node number.
		Node* pTemp = new Node( 
			&datafile, 
			nd,
			NM_SCAN
		);

		nd = pTemp->NextNode();

		delete pTemp;

	}

	datafile.Close( false );

	int nTotalCount = datafile.HighestNode();
	return ( nTotalCount? 100 * ( nTotalCount - DeleteCount ) / nTotalCount : 100 );

}


//-------------------------------------------------------------------//
// RebuildIndexFile()																//
//-------------------------------------------------------------------//
// This function rebuilds the index file for a db class derived from
// BaseDatabase that has defined the abstract NewObject() function.
//-------------------------------------------------------------------//
void BaseDatabase::RebuildIndexFile( 
	CProgressCtrl*	pProgressMeter,
	int				nStartProgress,
	int				nEndProgress,
	bool				bLog
) {

	CWaitCursor YouGotAntsInYourPants;

	// We need to kill the btrees and classes lists.  
	// We will be rebuilding them.
	PdyBtree *bt = btrees.FirstEntry();
	while (bt != 0)	{
		delete bt;
		bt = btrees.NextEntry();
	}
	Class *cls = classes.FirstEntry();
	while (cls != 0)	{
		delete [] cls->classname;
		delete cls;
		cls = classes.NextEntry();
	}

	// Empty the lists.
	btrees.ClearList();
	classes.ClearList();
	
	// Create a log file if requested.
	fstream logfile;
	CString LogBuffer;
	CString DisplayName;
	if ( bLog ) {

		LogBuffer = *GetFilename();
		LogBuffer = LogBuffer.SpanExcluding( _T(".") );
		LogBuffer += _T(".IndexLog");
		logfile.open(
			LPCSTR( LogBuffer ), 
			ios::out | ios_base::trunc 
		);
	
		// Start log.
		LogBuffer = "Reindex initialized.\n\n";
		logfile.write(
			LPCSTR( LogBuffer ), 
			LogBuffer.GetLength()
		);

		// Write database ID.
		GetDatabaseHeader( &LogBuffer );
		logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );
	
		// Write header for objects.
		LogBuffer = "ClassID   ObjectID   Node  Name\n";
		logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );
		LogBuffer = "---------------------------------------------\n";
		logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );

	}

	// Clear out the index file.  Note that the third param of
	// true means we want to wipe the file's current contents.
	VERIFY(
		indexfile.Initialize( 
			indexfile.GetNodeLength(),			// Preserve current granularity
			true,										// Create as needed
			0,											// No extended header data
			true										// bTruncate
		) == NFI_SUCCESS
	);

	// Open the database files, the data file for reading
	// and the index file for writing.
	indexfile.Open( true );
	datafile.Open( false );
	
	// Read open does not read the header, because it is not
	// typically needed.  We will need it to know the last
	// node when we loop through all nodes.
	datafile.ReadHeader();
   
	ObjectHeader objhdr;

	// We need to add class headers to the index file for
	// all known database classes.  The resulting ClassID's
	// should be set in stone across all databases.  This is
	// the only way that we can assure that we can rebuild
	// the index file for any database.
	// We will be using the resulting ClassID's to determine
	// the class types when scanning the database file.  	
	// This should not fail, as we are dealing with a new
	// index file.
	if ( RegisterClasses() ) {

		// ------- scan object nodes
		NodeNbr end = datafile.HighestNode();
		long lProgress;
		for ( NodeNbr nd = 1; nd <= end; nd++ ) {
   
			// --- read the object header for this node
			// All we need to read is the object header; the node will
			// be re-read if it turns out to be the start of an object.
			Node Temp( 
				&datafile, 
				nd,
				NM_SCAN,
				sizeof ObjectHeader
			);
			Temp.GetFromBuffer( &objhdr, sizeof ObjectHeader );

			// Check for first node of object.  bFirstNode
			// will be true.  If we have a first node and
			// it has not been deleted, then we should
			// process the object.
			if ( objhdr.bFirstNode ) {
			
				if ( objhdr.classid == -1 ) {

					// Log the deleted node.
					if ( bLog ) {
		
						LogBuffer.Format( "Skipped deleted object.\n" );
						logfile.write(
							LPCTSTR( LogBuffer ), 
							LogBuffer.GetLength()
						);
					
					}
								

				} else {
      
					// ----- rebuild depending on class type
					// The derived class provides this function,
					// because it knows about all the classes contained
					// in it, as well as the established class-classid
					// relationships.
					Persistent* pTemp = NewObject( objhdr.classid );

					// Skip unknown types.
					if ( pTemp ) {

						// Do the deed.
						// We want to load the entire object if we are
						// creating a log, so we can get the object name.
						pTemp->RebuildIndex( nd, !bLog );

						// Log the reindex.
						if ( bLog ) {
			
							// Header is of the form:
							// "ClassID   ObjectID   Node  Name"
							LogBuffer.Format( 
								"%7i   %8i   %4i  ", 
								objhdr.classid, 
								pTemp->GetID(),
								nd
							);
							
							pTemp->GetDisplayName( &DisplayName );
							LogBuffer += DisplayName;
							LogBuffer += "\n";
							logfile.write(
								LPCTSTR( LogBuffer ), 
								LogBuffer.GetLength()
							);

						}

						// Clean up.
						delete pTemp;
						pTemp = NULL;

					} else {
					
						// Log the skip.
						if ( bLog ) {
			
							LogBuffer.Format( "Skipped unknown class %3d object.\n", objhdr.classid );
							logfile.write(
								LPCTSTR( LogBuffer ), 
								LogBuffer.GetLength()
							);
						
						}
									
					}	// skip test

				}	// deleted test

			} // first node test

   		// Update progress meter.
			if ( pProgressMeter ) {
			
				lProgress = nStartProgress + ( nEndProgress - nStartProgress ) * nd / end;
				pProgressMeter->SetPos( (int) lProgress );

			}

		}

	}	// RegisterClasses()

	// Close the database files.
	datafile.Close( false );
	indexfile.Close( true );

	// Close the log file.
	if ( bLog )
		logfile.close();

}


//-------------------------------------------------------------------//
// VerifyFilesAreWritable()														//
//-------------------------------------------------------------------//
// This function makes sure that the database files are not 
// read-only.  This is important because write attempts to read-
// only database files will cause an unhandled exception.
// 
// If bNoPrompt is true, this function will attempt to update 
// read-only files automatically.  If bNoPrompt is false, the 
// provided MessageID will be used to display a Yes/No message to
// the user.  If they select Yes, the files will be made writable.
//
// Make sure that the data file is closed before calling this, or 
// it will ASSERT.
//
//	The return value will be true if the files are writable, false
// if not.
//-------------------------------------------------------------------//
bool BaseDatabase::VerifyFilesAreWritable( 
	bool	bNoPrompt,
	UINT	MessageID 
) {
	
	ASSERT( datafile.IsClosed() );

	bool bResult =	VerifyFileIsWritable( 
		LPCTSTR( datafile.strFilename	),
		bNoPrompt,
		MessageID
	);

	// Note that we just force the index file to follow the data file.
	// If the data file is writable but the index is not, no prompting
	// will occur.  No big deal.
	if ( bResult )
		VerifyFileIsWritable( LPCTSTR( indexfile.strFilename	) );

	return bResult;

}


//-------------------------------------------------------------------//
// Backup()																				//
//-------------------------------------------------------------------//
// This function backs up the data file.  
//
// Make sure that the data file is closed before calling this, or 
// it will ASSERT.
//
// We place the backup in the backup dir, under the databases dir.
// See EMDBArray::GetDBBackupPath() for details.
//-------------------------------------------------------------------//
bool BaseDatabase::Backup( bool bAddVersion )
{

	bool bReturn;
	
	// The files should always be closed at this point.
	ASSERT( datafile.IsClosed() );

	CString strBackupDir;
	pDBArray->GetDBBackupPath( &strBackupDir );
	
	CString strBackupName;
	ExtractFileNameFromPath( &datafile.strFilename, &strBackupName );

	if ( bAddVersion )
	{
	
		int nExt = strBackupName.ReverseFind( _T('.') );
		CString strTemp;
		strTemp.Format(
			_T("%s-v%d_%02d%s"),
			strBackupName.Left( nExt ),
			GetMajorVersion( FileVersionLastModified ),
			GetMinorVersion( FileVersionLastModified ),
			strBackupName.Right( strBackupName.GetLength() - nExt )
		);
		strBackupName = strTemp;
	
	}

	bReturn = CopyFile( 
		LPCTSTR( datafile.strFilename ),				// Existing file
		LPCTSTR( strBackupDir + strBackupName ),	// New file
		FALSE													// bFailIfExists	(We want to overwrite any previous backup.)
	) != FALSE;

	
	// We want to know immediately if the backup fails for any reason.
	ASSERT( bReturn );
	
	return bReturn;

}


//-------------------------------------------------------------------//
// GetDatabaseHeader()																//
//-------------------------------------------------------------------//
// This function creates a multi-line header defining the database.
// It is useful when creating log files.
//-------------------------------------------------------------------//
void BaseDatabase::GetDatabaseHeader( CString* pHeader )
{

	// Convert wchar_t[] to CString.
	CString DatabaseName( DBHeader.DBName );

	DatabaseID DBID = GetDBID();
	pHeader->Format( 
		"Database [ %s ]\n"
		"(%s)\n"
		" User Number 0x%8X\n"
		" Group       0x%8X\n"
		" DB Number     %8i\n"
		" DB Version    %8i\n"
		" ",
		DatabaseName,
		*datafile.GetFilename(),
		DBHeader.DBID.UserNumber,
		DBHeader.Group,
		DBHeader.DBID.DBNumber,
		DBHeader.DBVersion
	);
	if ( IsPublished() )
		*pHeader += _T( "Published\n\n" );
	else
		*pHeader += _T( "Active\n\n" );

}


//-------------------------------------------------------------------//
// SetProgressTitle																	//
//-------------------------------------------------------------------//
void BaseDatabase::SetProgressTitle(
	BaseProgressDlg* pDlg,
	UINT StartID
) {

	CString strProgressTitle( (LPSTR) StartID );
	strProgressTitle += _T("\"");
	strProgressTitle += CString( GetDatabaseName() );
	strProgressTitle += _T("\"...");
	pDlg->SetText( strProgressTitle );

}

