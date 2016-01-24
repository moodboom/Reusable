#include "stdafx.h"

#include "EMDBArray.h"
#include "UserSettingsDatabase.h"

// ---------------------------------------------------------------
// We have to include EVERY SINGLE PERSISTENT OBJECT in this file
// that is potentially included in the UserSettings database.
// ---------------------------------------------------------------
#include "..\SpecTable_Settings.h"
#include "..\SystemFonts.h"
#include "..\UnitsObj.h"			 
#include "..\UnitsSettings.h"		 
#include "..\UserSettings.h"		 

// TO DO
// #include "..\GlobalCostVariables.h"		 

// ---------------------------------------------------------------


// constructor
UserSettingsDatabase::UserSettingsDatabase(
	CString* pExecutablePath
) :

	// Base class.
	ObjectStream(
		&( *pExecutablePath + tUserSettingsDBName +	tWorkingDBExt ),	// FileName,              
		5					//  *UPDATE AS NEEDED*									// nClassCount,
	)

{
}


//-------------------------------------------------------------------//
// ~UserSettingsDatabase()																			//
//-------------------------------------------------------------------//
UserSettingsDatabase::~UserSettingsDatabase()
{
}


int UserSettingsDatabase::Initialize()
{

	wstring wsDBName;
	CopyCStringToWstring( &CString( tUserSettingsDBName ), &wsDBName );
	return ObjectStream::Initialize(
		false,								//	bFileShouldExist,
		&wsDBName,							//	pwsDatabaseName,       
		
		// These shouldn't matter.
		&DatabaseID(),						// DatabaseID
		1,										// Group,
		0,										// DBVersion,
		0,										// bPublished,

		false,								// bForceReindex,
		
		// TO DO
		// Optimize this number.
		48, 													//	nDataGranularity,      

		72														//	nIndexGranularity     
	);

}


//-------------------------------------------------------------------//
// NewObject()																			//
//-------------------------------------------------------------------//
// This function is called by RegisterClasses() and 
// RebuildIndexFile().
//-------------------------------------------------------------------//
Persistent* UserSettingsDatabase::NewObject( 
	ClassID classid 
) {

	Persistent* pTemp = NULL;

																				// Specify the db
	if			( classid ==   0 ) pTemp = new UserSettings			( 0, this );		 
	else if	( classid ==   1 ) pTemp = new CUnitsObj				( SI_NULL, this );			 
	else if	( classid ==   2 ) pTemp = new CUnitsSettings		( SI_NULL, this );
	else if	( classid ==   3 ) pTemp = new CSystemFonts			( NULL, NULL, 0, FONT_SET_NULL, this );
	else if	( classid ==   4 ) pTemp = new CSpecTable_Settings	( 0, -1, this );
	
	// TO DO
	// else if	( classid ==   5 ) pTemp = new GlobalCostVariables	( 0, -1, this );

	// WARNING!  You may add to this list, but don't delete or 
	// reorder items in it.  See function comments for details.
	// ** Make sure that you update the param in RegisterClasses()
	// in the constructor if you add to the list. **
	
	// Unknown type found.
	else ASSERT( false );

	return pTemp;

}


//-------------------------------------------------------------------//
// GetHeaderKey()																		//
//-------------------------------------------------------------------//
// The user settings databases always use the same key.  It is hardcoded
// within the program, as there will not always be a dongle available to
// store it.
//-------------------------------------------------------------------//
bool UserSettingsDatabase::GetHeaderKey( 
	uByte** ppKey, 
	uShort* pKeyBytes 
) {

	// Allocate the key buffer.
	uHuge* pKey = new uHuge;

	// Point the param to our buffer.
	*ppKey = (uByte*) pKey;

	// Assign the size.
	*pKeyBytes = sizeof uHuge;

	// Assign the value.
	*pKey = 0x325bdb59799ef6b7;

	return true;

}


//-------------------------------------------------------------------//
// GetObjectKey()																		//
//-------------------------------------------------------------------//
// The user settings databases always use the same key.  It is hardcoded
// within the program, as there will not always be a dongle available to
// store it.
//-------------------------------------------------------------------//
bool UserSettingsDatabase::GetObjectKey( 
	uByte**	ppKey, 
	uShort*	pKeyBytes 
) {

	// Allocate the key buffer.
	uHuge* pKey = new uHuge[2];

	// Point the param to our buffer.
	*ppKey = (uByte*) pKey;

	// Assign the size.
	*pKeyBytes = sizeof uHuge * 2;

	// Assign the value.
	*pKey = 0xf3b0e5606a2e4b9a;
	*( pKey + 1 ) = 0x9336cb83304bca60;

	return true;

}
