//-------------------------------------------------------------------//
// EMDBArray.h - header file
//
//
//
//-------------------------------------------------------------------//
#ifndef EM_DB_ARRAY_H
	#define EM_DB_ARRAY_H

#include "EMDatabase.h"
#include "UserSettingsDatabase.h"

#include "..\StringHelpers.h"			// For StripExtension

#include "DatabaseArray.h"				// Base class

// Since EMDBArray.h already has ObjectTypes.h and ObjectReference.h
// included, it costs nothing to include this as well and make
// TypedRef versions of NewComponent().
#include "TypedObjectReference.h"

//-------------------------------------------------------------------//
// Forward declarations																//
//-------------------------------------------------------------------//

class UserSettings;

class SplashScreenDlg;					// We update it from here.


//-------------------------------------------------------------------//
// class EMDBArray																	//
//-------------------------------------------------------------------//
class EMDBArray : public DatabaseArray {

public:

	EMDBArray();

	~EMDBArray();

	bool Initialize(
		SplashScreenDlg*	pSplash
	);

	bool DBAlreadyConnected(
		EMDatabase* pDatabase,
		CWnd*			pParent
	);

	void AddDBToRegistryList( 
		EMDatabase* pDatabase 
	);

	void RemoveDBFromRegistryList( 
		EMDatabase* pDatabase 
	);
	
	void RefreshFromDefaultDir();

	// This initiates a DatabaseManagerDlg.
	void Manage();

	bool Disconnect( 
		EMDatabase* pDatabase,
		CWnd*			pParent			= NULL
	);

	int GetFile(
		CString* pstrName,
		UINT		TitleID,
		CWnd*		pParent,
		bool		bIncludePublished = true,
		DWORD		Flags = OFN_FILEMUSTEXIST
	);

	// ---------------
	// Sorting helpers
	// ---------------
	// These are used by tree sorting to look up database
	// names based on the database array index.
	//

	void GetDBName( 
		CString* pstrDatabaseName,
		int nIndex
	) {

		// Use the following format for database names:
		//		DBName  [ Filename ]
		*pstrDatabaseName = Databases[nIndex]->GetDatabaseName();
		*pstrDatabaseName += 
				_T("  [ ") 
			+	*Databases[nIndex]->GetFilename()
			+	_T(" ]");

	}

	// Finds the index of the passed pointer within the
	// Databases array.  Returns -1 if not found.
	int GetDBIndex( ObjectStream* pDB )
	{
		bool bFound = false;
		int nDBCount = Databases.GetSize();
		for ( int i = 0; i < nDBCount && !bFound; i++ ) 
			bFound = ( Databases[i] == pDB );
		return bFound? i - 1 : -1;
	} 

	//
	// ---------------
	

	void BuildUserBlock(
		CString&	strAuthor,							// Result
		wchar_t*	pwcUser,
		wchar_t*	pwcCompany,
		wchar_t*	pwcContact,
		uLong		ulUserNumber,
		bool		bUseIndustrialStrengthDlgControlCRLF = true
	);
	
	CString GetDisplayDBID( DatabaseID* pDBID );

	void SaveUserSettings();

	void FillTrees(
		SplashScreenDlg*	pSplash,
		int					nProgressStart		= 0,
		int					nProgressEnd		= 100,
		bool					bClearFirst			= true
	);

	// Returns an EMComponent pointer to an object of the requested
	// OBJECT_TYPE.  If pRef is NULL, this object is a new one from the
	// active DB.  If pRef is passed, then the object returned is from
	// the DB within the ObjectReference.
	EMComponent *NewComponent(
		OBJECT_TYPE			eType,
		ObjectReference	* pRef	= NULL
	);

	// Typed ObjectReference version
	inline EMComponent *NewComponent( TypedRef &TRef )
	{
		return NewComponent( TRef, &TRef.Ref );
	}

	void UpdateAllCaches();

	// This function takes the passed CMenu pointer and creates a popup
	// menu of all the active databases.  If the DatabaseID pointer is
	// passed, then the DB with that ID is excluded from the list.
	void Create_DB_Menu(
		CMenu			*pMenu,
		bool			bActiveOnly		= true,
		DatabaseID	*pDBID			= NULL
	);

	
	//-------------------------------------------------------------------//
	// pGetDBPtr()																			//
	//-------------------------------------------------------------------//
	// Returns a database ptr according to the passed index.
	// We make sure we have a valid index, and that the cast to 
	// EMDatabase is successful.  
	// If not, we assert and return the target DB.
	//-------------------------------------------------------------------//
	EMDatabase * pGetDBPtr( int nIndex )
	{
		
		ObjectStream* pDB;

		if ( nIndex >= 0 && nIndex < Databases.GetSize() ) {
		
			// We have a valid index.
			pDB = Databases[ nIndex ];
		
		} else {
		
			// Invalid index!  Assert and return the target db.
			ASSERT( false );
			pDB = pTargetDB;

		}

		return dynamic_cast<EMDatabase*> ( pDB );

	}


	bool MoveDatabaseToBackup( CString* pstrDBName );

	void BackupTargetDB()
	{
		pTargetDB->Backup();
	}

	void GetTempDBName( CString* pstrTempDBName );

	// This is a user setting that determines if the user
	// can make new, copy or move to any working db on the fly, or
	// if the target is always the default db.
	// The default value, set in the constructor, is false.
	bool	bCrossDBOperations;

	// The user settings database.
	UserSettingsDatabase* pUserSettingsDB;

	// Global user settings.
	UserSettings*	pUserSettings;

};


//-------------------------------------------------------------------//
// Global variables
//-------------------------------------------------------------------//

// The database schema requires exactly one database array object.
extern EMDBArray*			pEMDBArray;

// This is used by UserSettingsDatabase.cpp.
extern const TCHAR		tUserSettingsDBName[];

// Permanent group numbers.
const	uLong					DemoGroup						= 0xb2c2c5df;

//-------------------------------------------------------------------//
// Global functions
//-------------------------------------------------------------------//

// Created these global versions of NewComponent that automatically
// use the pEMDBArray pointer.
inline EMComponent *NewComponent( OBJECT_TYPE eType, ObjectReference *pRef = NULL )
{
	return pEMDBArray->NewComponent( eType, pRef );
}

inline EMComponent *NewComponent( TypedRef &TRef )
{
	return pEMDBArray->NewComponent( TRef, &TRef.Ref );
}




#endif		// EM_DB_ARRAY_H
