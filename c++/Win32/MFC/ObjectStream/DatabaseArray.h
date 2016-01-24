#ifndef DATABASE_ARRAY_H
	#define DATABASE_ARRAY_H

#include <afxtempl.h>					// For CTypedPtrArray

#include "BaseDatabase.h"
// class BaseDatabase;


class DatabaseArray {

public:

	DatabaseArray();

	~DatabaseArray();

	bool LookUpDatabase( 
		const DatabaseID*			pDBID,
		BaseDatabase**						ppDatabase
	);

	int GetSize() { return Databases.GetSize(); }

	void Add( BaseDatabase* pNewDB ) { Databases.Add( pNewDB ); }

	void RemoveAt( int nIndex )
	{
		delete Databases[ nIndex ];
		Databases.RemoveAt( nIndex );
	}
				

	void GetDatabasePath( 
		CString* pstrDBPath
	);

	void GetDBBackupPath( 
		CString* pstrPath
	);

	
	// The target working database to which new objects are saved.
	BaseDatabase*			pTargetDB;
	DatabaseID		TargetDBID;

protected:

	// The core list of databases.
	CTypedPtrArray<CPtrArray, BaseDatabase*> Databases;

};


//-------------------------------------------------------------------//
// Global variables
//-------------------------------------------------------------------//

// The database schema requires exactly one database array object.
// This pointer should be set to the array on program startup.
extern DatabaseArray*	pDBArray;

//-------------------------------------------------------------------//


#endif		// DATABASE_ARRAY_H
