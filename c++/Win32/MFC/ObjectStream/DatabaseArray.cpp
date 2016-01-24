
#include "stdafx.h"						// Required for precompiled header

#include <direct.h>						// For _tmkdir

#include "BaseDatabase.h"

#include <AppHelpers.h>					// For GetProgramPath()

#include "DatabaseArray.h"


//-------------------------------------------------------------------//
// Initialize statics/globals.
//-------------------------------------------------------------------//

// The database schema requires exactly one database array object.
// On program startup, it should be created, and this pointer should 
// be pointed to it.
// We use a pointer and not an actual object because a derived class
// may be created.  If so, we create the derived object and set
// this pointer to it.
DatabaseArray*	pDBArray = 0;

//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// DatabaseArray()																	//
//-------------------------------------------------------------------//
DatabaseArray::DatabaseArray()
{
	// A program should only have one db array object.  Here, we make
	// sure the global pointer has not yet been set, then set it to
	// point to this object.
	ASSERT( pDBArray == 0 );
	pDBArray = this;

}


//-------------------------------------------------------------------//
// ~DatabaseArray()																//
//-------------------------------------------------------------------//
DatabaseArray::~DatabaseArray()
{
}


//-------------------------------------------------------------------//
// LookUpDatabase()																	//
//-------------------------------------------------------------------//
// This function uses the given database ID to find the matching
// database in the known databases list.  The pointer to the 
// database is returned through the use of a pointer to a pointer
// to a database.
// The caller can specify NULL for pDBID, in which case the target
// database is used.  The target database is the one which receives
// all new objects.  Its pointer is always located in slot[1]
// of the Databases array.
// If a DBID is provided, but it is not found in the DB list, we
// set the DB pointer to the target DB and return false.
//-------------------------------------------------------------------//
bool DatabaseArray::LookUpDatabase( 
	const DatabaseID*	pDBID,
	BaseDatabase**				ppDatabase				// Pointer to a pointer
) {

	bool bReturn = true;

	// Init to NULL.
	*ppDatabase = 0;
	
	// If pDBID is NULL or blank, use the target database.
	if ( !pDBID || *pDBID == DatabaseID() ) {

		*ppDatabase = pTargetDB;

	} else {
	
		// Search for the DBID in the database list.
		// Return is false until found.
		bReturn = false;
		int nCount = Databases.GetSize();
		for ( 
			int i = 0; 
			i < nCount && !bReturn; 
			i ++ 
		) {

			if ( bReturn = ( *pDBID == Databases[i]->GetDBID() ) ) 

				// We found it, set the database pointer.
				*ppDatabase = Databases[i];

		}

		// We should always return a valid database pointer
		// from this function.  If the provided DBID did not
		// resolve, use the target db. 
		if ( !bReturn )
			*ppDatabase = pTargetDB;

	}
	
	return bReturn;

}


//-------------------------------------------------------------------//
// GetDatabasePath()																	//
//-------------------------------------------------------------------//
void DatabaseArray::GetDatabasePath( 
	CString* pstrDBPath
) {

	/*
	TCHAR tDBDirRegValue[] = _T("Database Directory");

	bool bFoundDBDir = ( 
		GetRegistryString(
			tDBDirRegValue,
			pstrDBPath
		)
	);

	// If we find the directory in the registry...
	if ( bFoundDBDir ) {

		// Make sure the dir ends with a backslash.
		if ( pstrDBPath->Right( 1 ) != _T("\\") )
			*pstrDBPath += _T("\\");

		// Make sure the dir exists.
		// Check the error result and set bFoundDBDir to false
		// if this doesn't work.
		errno = 0;
		if ( _tmkdir( LPCTSTR( *pstrDBPath ) ) == -1 )
			bFoundDBDir = !( errno == ENOENT );

	}

	// If we didn't find anything or couldn't create the dir...
	if ( !bFoundDBDir ) {
	
		// Build the default database path.
		*pstrDBPath = GetProgramPath() + _T("Databases\\");

		// Create it.
		_tmkdir( LPCTSTR( *pstrDBPath ) );

		// Place it in the registry.
		VERIFY( 
			SetRegistryString(
				tDBDirRegValue,
				pstrDBPath
			)
		);

	}
	*/

	ASSERT( false );
	*pstrDBPath = _T("c:\\");

}
	

//-------------------------------------------------------------------//
// GetDBBackupPath()																	//
//-------------------------------------------------------------------//
void DatabaseArray::GetDBBackupPath( 
	CString* pstrPath
) {

	GetDatabasePath( pstrPath );
	*pstrPath += _T("Backup\\");

	// Make sure it exists.
	_tmkdir( LPCTSTR( *pstrPath ) );


}


