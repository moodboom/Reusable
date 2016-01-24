//-------------------------------------------------------------------//
// ObjectStream.h
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef OBJECT_STREAM_H
	#define OBJECT_STREAM_H


/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module:
//

#define IDS_DB_REPAIRING                12001
#define IDS_DB_UPDATE_PUBLISHED_PROBLEM 12002
#define IDS_DB_UPDATE_UNKNOWN_PROBLEM   12003
#define IDM_UNKNOWN_HEADER              12004

// 
// Ideally, resources would be specified on a class-by-class basis.  Unfortunately,
// Visual Studio does not handle that well.  Different projects have different resource
// files that contain all of the project's resources in one place.  Make sure
// you provide the resources matching the above defines in your resource file.
// You must also include this file in the resource file's "Read-only symbol
// directives" ( see "Resource Includes" on the View menu ).
//
// We want to exclude the remainder of the include file when dealing with App Studio.
#ifndef RC_INVOKED
/////////////////////////////////////////////////////////////////////////


#include <fstream>
#include <typeinfo>
#include <cstring>									// For wstring, etc.

// TO DO
// Replace with STL version.
#include "linklist.h"

#include "Exceptions.h"
#include "Persistent.h"
#include "NodeFile.h"

#include "..\Crypto\Blowfish\BlowFish.h"		// We do encryption!

#include "..\PragmaMessages.h"					// For [#pragma DEBUG_CODE()] etc

class BaseProgressDlg;								// For Compact, etc.
class DBPropertiesDlg;								// Displays properties.


// Static database file extensions are defined in the *.cpp.
extern	const TCHAR		tIndexExt[];
extern	const TCHAR		tWorkingDBExt[];
extern	const TCHAR		tPublishedDBExt[];


//-------------------------------------------//
// Silly version functions to beautify code.
// Avoids having to type things like 0x0000000100000005.
//-------------------------------------------//
inline uHuge Version( uLong x, uLong y )			{ return ( ( ( (uHuge) x ) << 32 ) + y );						}
inline uLong GetMajorVersion( uHuge uhV	)		{ return (uLong) ( ( uhV & 0xFFFFFFFF00000000 ) >> 32 ); }
inline uLong GetMinorVersion( uHuge uhV )			{ return (uLong) (   uhV & 0x00000000FFFFFFFF         ); }


// This defines the return types for Initialize();
typedef enum {
	DB_INIT_SUCCESS,
	DB_INIT_DATAFILE_NOT_FOUND,
	DB_INIT_BAD_GRANULARITY,
	DB_INIT_UNAUTHORIZED,
	DB_INIT_REINDEX_FAILED,
	DB_INIT_REGISTER_FAILED,
} DB_INIT_RESULT;


class ObjectStreamHeader{

public:

	// This class is encrypted, we want it to fill
	// an even number of 8-byte blocks.								Bytes

	// The full name of the database.
	wchar_t		DBName[43];												//  86

	uByte			Unused[2];												//	  2
	
	
	///////////////////////////////////////////////////////////////
	// VERSION 1.14 UPDATE
	///////////////////////////////////////////////////////////////
	//


	// OLD:

	// Identifies this db; see ObjectReference.h.
	// DatabaseID	DBID;													//	 16	

	
	// NEW:

	// Identifies this db; see ObjectReference.h.
	DatabaseID		DBID;													//	  8

	// Flag to specify published or working.
	unsigned int	bPublished:1;										//	  1 bit		

	// Version #, incremented after Publish() succeeds.
	unsigned int	DBVersion:31;										//	 31 bits		

	// Unique group number, generated in-house
	uLong				Group;												//	  4			
	
	//
	///////////////////////////////////////////////////////////////


	// Identifies the file version of the program
	// executable that was used to originally create 
	// the database.
	uHuge			FileVersionCreated;									//	  8

																				// ---
																				// 112 / 8 = 14 blocks

	// Clear data upon creation.
	// DatabaseID's constructor will clear itself.
	ObjectStreamHeader() 
	:
	
		// Init vars.
		FileVersionCreated( 0 )
	{ 

		memset( DBName, 0, sizeof DBName );
		memset( Unused, 0, sizeof Unused );

	}

	void SetPublishedStatus( bool bNewStatus )
	{
		bPublished = bNewStatus? 1 : 0;
	}

};


// ============================
// the ObjectStream database
// ============================
class ObjectStream	{

public:
	
	// Our constructor.
	ObjectStream( 
		CString*	pstrFileName, 
		int		nNewClassCount
	);

	~ObjectStream();

	DB_INIT_RESULT Initialize(
		bool				bFileShouldExist	= true,
		const wstring*	pwsDatabaseName	= NULL,
		DatabaseID*		pDBID					= NULL,
		uLong				Group					= 0,
		uLong				DBVersion			= 0,
		bool				bPublished			= false,
		bool				bForceReindex		= false,
		short int		nDataGranularity	= 128,
		short int		nIndexGranularity	= 72			// 72 fits 4/6 ObID keys EXACTLY!
	);

protected:
	friend Persistent;

	NodeFile datafile;	 			// the object datafile
	NodeFile indexfile; 				// the b-tree file

	LinkedList<Class> classes;		// registered classes
	LinkedList<PdyBtree> btrees;	// btrees in the database
	
	bool FindClass(Class *cls, NodeNbr *nd = 0);
	
	void RegisterIndexes(Class *cls, const Persistent& pcls)
			throw (ZeroLengthKey);
	
	void RegisterClass(
		const Persistent& cls,
		ClassID*				pResultID
	);

	void AddClassToIndex(
		Class	*cls
	);

	streamoff GetObjectHeaderAddress( Node* pNode ) {
		return (streamoff)
		(
					(fpos_t) pNode->NodeAddress()		// start of node
				+	(fpos_t) sizeof NodeNbr				// node header
				+	(fpos_t) classnamesize				// classname
		);
	}

	ClassID GetClassID( const Persistent* pObject );

	Class* GetClass( ClassID CID );

	// This function should be overridden in derived classes.  See 
	// RegisterClasses() for details.
	//
	// We call this within RebuildIndexFile() and RegisterClasses().
	virtual Persistent* NewObject( ClassID classid ) = 0;
	
	// Note that we can't call RegisterClasses() in the base class 
	// constructor; C++ can't handle finding a derived function in 
	// a base class constructor.  We handle this by using an
	// Initialize() function, and call RegisterClasses() there.
	bool RegisterClasses();

	// The derived class should provide the key used to encrypt
	// both the header and object data.
	virtual bool GetHeaderKey( uByte** ppKey, uShort* pKeyBytes ) = 0;		// abstract
	virtual bool GetObjectKey( uByte** ppKey, uShort* pKeyBytes ) = 0;		// abstract

	/*
	virtual bool GetHeaderKey( uByte** ppKey, uShort* pKeyBytes )
	{
      static uByte sbKey[] = { 0x10, 0x10, 0x10, 0x10, 0x10 };
      *ppKey = &sbKey[0];
      *pKeyBytes = sizeof sbKey * sizeof uByte;
      return true;
	}
	virtual bool GetObjectKey( uByte** ppKey, uShort* pKeyBytes )
	{
		return GetHeaderKey( ppKey, pKeyBytes );
	}
	*/
	
	
	
	////////////////////////////////
	// DATABASE HEADER FUNCTIONS
	//
	// Database headers can be expanded by derived classes.
	//
	// There are three steps to reading the header:
	//
	//		1) Read the unencrypted portion of the header;
	//		2) Get the key used for the encrypted portion; and
	//		3) Read the encrypted portion and decrypt it.
	//
	// Each of these three steps can be overridden by derived
	// classes.
	//
	virtual void ReadPlainHeader();
	virtual bool ReadEncryptedHeader();

	// Writing is similar.
	virtual void WritePlainHeader();
	virtual void WriteEncryptedHeader();

	// The header size must also be provided so we can adjust for it 
	// when calculating node data offsets.
	// The vars define the existing header sizes as read from
	// the database.
	uShort uObjectStreamPlainHeaderSize;
	uShort uObjectStreamEncryptedHeaderSize;

public:
	virtual int GetHeaderSize() 
		{ return uObjectStreamPlainHeaderSize + uObjectStreamEncryptedHeaderSize; }

protected:
	BlowFish*		pHeaderEncryptor;

	// Plain Header vars

	// This identifies the file version of the database.
	// We check this number against the current file version 
	// in Initialize().  If we have an older version number, 
	// we attempt to update the database before opening it
	// for the user.
	uHuge			FileVersionLastModified;//	  8

	// Encrypted Header vars

public:
	ObjectStreamHeader	DBHeader;

	// These give public access to reading the entire header.
	// This is used by NodeFile::Initialize().
	bool ReadHeader();
	void WriteHeader();

	//
	/////////////////////////////////

	
	////////////////////////////////
	// VERSIONING 
	////////////////////////////////
	// See ObjectStream.txt for versioning details.
	//

protected:
	
	// The next two functions are called for a db that is found to be 
	// out-of-date.  The first function makes any necessary
	// updates to the database strucure (i.e., header structure).
	// The second loops through all objects, allowing each to
	// update itself as needed.  The base class COULD loop through 
	// all objects on a ClassID basis, but derived classes will often 
	// have a required order of loading the objects, so we leave 
	// the task up to the derived class.  
	// ( Think of OBJECT_TYPE in EMC. :> )
	virtual bool	UpdateDatabaseFormat() { return true; }
	virtual bool	UpdateOldObjects() = 0;
	
public:

	// This function is called at the appropriate time to verify
	// that the database is updated to the current version.  It
	// calls UpdateOldObjects() as needed.
	bool UpdateDatabaseAsNeeded();

	// Provide public access to versions.
	
	// Derived classes should override to provide their specific
	// current file version.  Each derived database class will use its
	// own version numbers, of course.
	virtual uHuge	GetCurrentFileVersion()				= 0;

	uHuge	GetDBVersion()	{ return FileVersionLastModified; }

	//
	/////////////////////////////////


public:

	BlowFish blow;

	CString* GetFilename()
	{
		return datafile.GetFilename();
	}

	wchar_t* GetDatabaseName()
	{
		return &( DBHeader.DBName[0] );
	}

	void SetProgressTitle(
		BaseProgressDlg* pDlg,
		UINT StartID
	);

	int GetPercentCompacted();

	bool IsPublished()
	{ return ( DBHeader.bPublished > 0 ); }

	// The database ID uniquely identifies the database.
	DatabaseID GetDBID()
	{ return DBHeader.DBID; }
	void SetDBID( DatabaseID NewDBID )
	{ DBHeader.DBID = NewDBID; }

	// We need to know if index is new so we can register 
	// all classes.
	bool IsIndexNew() {
		return indexfile.IsNewFile();
	}

	bool Open( bool bWritable );
	void Close( bool bWritable );

	// This rebuilds the index file.
	// Supply an overridden version of NewObject() to be able 
	// to use this function in derived classes.
	void RebuildIndexFile( 
		CProgressCtrl*	pProgressMeter		= 0,
		int				nStartProgress		= 0,
		int				nEndProgress		= 100,
		bool				bLog					= false 
	);

	bool Backup( bool bAddVersion = false );

	// This makes sure the database files are writable, so 
	// that write attempts are not made to read-only database files.
	bool VerifyFilesAreWritable( 
		bool	bNoPrompt = true,
		UINT	MessageID = 0
	);

	// Provide access to nClassCount for checking for out-of-range values.
	int GetClassCount() { return m_nClassCount; }

	// This kills our files if they exist, use with caution!
	void DeleteFiles()
	{
		ASSERT( datafile.IsClosed() && indexfile.IsClosed() );
		_tremove( LPCTSTR( *datafile.GetFilename() ) );
		_tremove( LPCTSTR( *indexfile.GetFilename() ) );
	}

	bool MoveToBackup()
	{
		bool bReturn = Backup();
		if ( bReturn )
			DeleteFiles();
		return bReturn;
	}

	// Provide a function to make a nice, pretty ASCII db header
	// for log files.
	void GetDatabaseHeader( CString* pHeader );

protected:

	// For registration/index rebuilds.
	// Also used when doing appends to loop through all objects.
	int m_nClassCount;						

	// Make this a friend so it can get to all the data.
	friend class DBPropertiesDlg;

};


#endif		// RC_INVOKED

#endif	// OBJECT_STREAM_H

