#ifndef USER_SETTINGS_DATABASE_H
	#define USER_SETTINGS_DATABASE_H

#include "ObjectStream.h"											// Base class


class UserSettingsDatabase: public ObjectStream {

public:
	
	// Our constructor.
	UserSettingsDatabase(
		CString* pExecutablePath
	);
	~UserSettingsDatabase();

	int Initialize();
	
	wchar_t* GetDatabaseName()
	{
		return ( L"User settings database" );
	}

	////////////////////////////////
	// VERSIONING FUNCTIONS
	//
	// This database class does not currently implement
	// versioning.
protected:
	bool	UpdateOldObjects() { return true; }
	uHuge	GetCurrentFileVersion()	{ return 1; }
	//
	////////////////////////////////

protected:
	friend Persistent;
	
	// This is required for class registration/reindexing.
	Persistent* NewObject( 
		ClassID classid 
	);

	bool GetHeaderKey(
		uByte** ppKey, 
		uShort* pKeyBytes 
	);
	bool GetObjectKey( 
		uByte** ppKey, 
		uShort* pKeyBytes 
	);

};


#endif	// USER_SETTINGS_DATABASE_H

