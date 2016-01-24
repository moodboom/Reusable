#ifndef EM_DATABASE_H
	#define EM_DATABASE_H

#include <afxtempl.h>					// For CTypedPtrArray
#include <vector>

// #include "..\EMArray.h"					// Custom CArray classes

#include "ObjectStream.h"						// Base class
#include "TypedObjectReference.h"	// Used as a CopyAndGather() param, etc.

#include "..\ObjectTypes.h"			// For OBJECT_TYPE definition.
												
#include "..\SplashScreenDlg.h"		// Needed for FillTree(), BaseProgressDlg needed for Append, VerifyTestData.
												
#include "..\Dongle\Dongle.h"			// Needed for header string length constants.
												
class MainFrame;							// Needed for FillTrees()
												
class EMComponent;						// Needed for UpdateObjectReferences()
												
class EMDatabase;							// For DBPointerArray.


//-------------------------------------------------------------------//
// Globals, Statics, Typedefs
//-------------------------------------------------------------------//
//

// This var is incremented each time a new version of the Earthmover program
// is released that contains a new version of any object.
// See ObjectStream.txt for versioning details.
extern const uHuge uhCurrentEMFileVersion;

// This var defines the oldest version number processed by the program.
// We should definitely avoid raising this unless absolutely neccessary.
// It was initially used while removing versioning code in versions
// before the first official program ship.
extern const uHuge uhOldestHandledEMFileVersion;

extern const COleDateTime dtDemoExpirationDate;

// This defines an array of EMDatabase pointers.
typedef CTypedPtrArray<CPtrArray, EMDatabase*> DBPointerArray;

// This defines an ObjectID pair array.
// It is used by Append().
typedef struct {
	ObjectID	OldID;
	ObjectID	NewID;
} IDPair;

// We are going to use an STL container class.
//
//		Container	Provides
//	----------------------------------------------------------
//		vector		push_back, pop_back, at
//		list			push_back, pop_back, push_front, pop_front
//		deque			push_back, pop_back, push_front, pop_front, at
//		set			insert(), find(), uses value as key, 
//							requires key_comp() function
//		multiset		same as set except duplicate keys allowed
//		map			insert(), find(), uses value AND a key, 
//							requires key_comp() function
//		multimap		same as map except duplicate keys allowed
//		
typedef vector<IDPair> IDArray;

//
//-------------------------------------------------------------------//


class EMDBHeader{

public:

	// This class is encrypted, we want to 
	// fill an even number of 8-byte blocks.					Bytes

	// This random number is generated when this database
	// is created.  It is used as a seed to the HASP key;
	// the return code is XOR'ed with the group key to 
	// give us the database key.
	uShort		ObjectSeed;											//	  2
	
	uByte			Unused[6];											//   6


	// Clear data upon creation.
	// DatabaseID's constructor will clear itself.
	EMDBHeader() { 

		ObjectSeed = 0;

	}

};

typedef enum {
	FT_BAD_DATABASE,
	FT_PUBLISHED_CACHE_OLD,
	FT_SUCCESS
} FillTreeResult;


//-------------------------------------------------------------------//
// class EMDatabase																	//
//-------------------------------------------------------------------//
// This is the big one.  It can handle all EMComponent-derived
// classes.
//-------------------------------------------------------------------//
class EMDatabase: public ObjectStream 
{
public:

	// Our constructor.
	EMDatabase(
		CString*	pstrFileName
	);

	~EMDatabase();

	DB_INIT_RESULT Initialize(
		bool				bFileShouldExist	= true,
		const wstring*	pwsDatabaseName	= 0,
		DatabaseID*		pDBID					= 0,
		uLong				Group					= 0,
		uLong				DBVersion			= 0,
		bool				bPublished			= false,
		uShort*			pObjectSeed			= NULL,
		bool				bForceReindex		= false,
		short int		nDataGranularity	= 128,
		short int		nIndexGranularity	= 72			// 72 fits 4/6 ObID keys EXACTLY!
	);

protected:
	friend Persistent;

public:

	/////////////////////////////////////////////////////////////
	//
	//	NewObject() functions
	//

	// This is the main NewObject function that does all the
	// actual work - see the cpp file.
	Persistent * NewObject(
		ClassID				classid,
		ObjectReference	* pRef
	);

	// This version is derived from abstract ObjectStream.h version.
	// This is required for class registration/reindexing.
	Persistent * NewObject(
		ClassID				classid
	) {
		return NewObject( classid, NULL );
	}

	// This version returns an EMComponent pointer instead of
	// a Persistent pointer, hence the "Component" in the name
	// instead of the "Object".  This version also specifies
	// the type as an OBJECT_TYPE, not ClassID.  This is by
	// far the most commonly used way of obtaining a new
	// Earthmover database object.
	EMComponent * NewComponent(
		OBJECT_TYPE			eType,
		ObjectReference	* pRef	= NULL
	) {
		return (EMComponent *) NewObject( GetClassID( eType ), pRef );
	}

	// These convert types for us.
	static ClassID GetClassID( OBJECT_TYPE eType );
	static OBJECT_TYPE GetObjectType( ClassID CID );

private:

	// Used by the above for quick lookup.
	static OBJECT_TYPE arObType [ OBJECT_LIST_LEN ];
	static ClassID arCID [ OBJECT_LIST_LEN ];

public:

	// Initializes the above.
	static void InitializeStatics();

	//
	/////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////
	// GATHERING
	//

public:

	// Gather.  The default params gather all working-database
	// objects.  Change as needed.
	void Gather( 
		CWnd*			pParent,
		bool			bIncludePublished	= false,
		EMDatabase* pSourceDB			= 0
	);

	// Copy all.  This takes all the objects in the source
	// database and copies them over.  Make sure you
	// specify the bGather flag whenever you want to avoid
	// duplicate objects!
	void CopyAll( 
		CWnd*			pParent,
		EMDatabase* pSourceDB,
		bool			bGather		= false
	);

	// This clears the previously-gathered objects array, GatheredRefs.
	void InitGatherSession();


	// Gathered object manipulators.
	bool FindAndReplaceGatheredRef( 
		ObjectReference&	OldRef,
		OBJECT_TYPE			eType
	);
	void AddGatheredObject( 
		TypedRef&	trOld,
		ObjectID		NewID
	);

protected:
		
	// This function takes the source object, specified by TypedRef,
	// and determines if it has been previously gathered.  If so, we return.
	// If not, we stuff a new object in our database, copy the source's
	// guts, and gather it up.
	bool CopyAndGather(
		TypedRef&			trSource,
		bool					bIncludePublished	= false,
		EMDatabase*			pGatheringFromDB	= 0,    
		fstream*				pLogFile				= 0,    
		int					nLogIndents			= 0
	);

	// Typedef for the gathered Ref array.
	typedef struct {
		ObjectReference	OldRef;
		ObjectID				NewID;
	} GatherPair;
	typedef vector<GatherPair> GatherArray;

	// Here, we keep track of previously gathered objects.  Information about
	// any objects that have been copied during the session is maintained 
	// within the array.  Objects in the array will not be re-duplicated.
	GatherArray GatheredRefs[ OBJECT_LIST_LEN ];
		
	//
	///////////////////////////////////////////////////////////////////////

public:

	// This adds objects in the database to the
	// associated tree control.
	FillTreeResult FillTree(
		OBJECT_TYPE			eWhich,
		SplashScreenDlg*	pSplash		= 0,
		bool					bCreate		= false,
		bool					bRefresh		= false,
		bool					bSort			= true
	);

	bool Compact( 
		BaseProgressDlg*	pProgressDlg,
		int					nStartProgress	= 0,
		int					nEndProgress	= 100,
		bool					bLog				= true
	);

	void UpdateCaches(
		BaseProgressDlg*	pProgressDlg	= 0,
		int					nStartProgress	= 0,
		int					nEndProgress	= 100
	);

	int UpdateAssociations(
		BaseProgressDlg*	pProgressDlg,
		int					nStartProgress			= 0,
		int					nEndProgress			= 100,
		bool					bVerifyCompleteness	= false
	);

	void Publish( 
		CWnd*	pParent					= NULL,
		bool	bUnpublishing			= false
	);

	// Given the type and reference of an object, this function will
	// create a new one in the database that is an exact copy of that
	// object.  The appropriate tree is updated with the new object's
	// information.  Returns the ObjectReference of the new object.
	ObjectReference DuplicateDBObject(
		TypedRef&		trSrc,
		bool				bUpdateTree		= true,
		SwapRefArray	*parSwap			= NULL	// Used internally by EMComponent to update linked refs.
	);

	// Verifies that every object in the database is fully
	// complete and calculated.  Puts up a detailed error
	// log for the user, which he can save for reopening later.
	// Passing a valid eSingleType will cause this function to
	// check only that object type.
	bool VerifyContents( int nStartType = 0, bool bContinueAfter = true );

	void DisplayProperties( CWnd* pParent );

	void ClearCache();

public:


	// The name of the user that created the database.
	wchar_t		UserName[nUserNameLength];					//  64

	// The company of the user that created the db.
	wchar_t		Company[nCompanyLength];					//  64

	// The contact info (telephone #, email, etc.)
	// of the user that created the db.
	wchar_t		Contact[nContactLength];					// 128


protected:
	
	////////////////////////////////
	// DATABASE HEADER FUNCTIONS
	//
	void ReadPlainHeader();
	bool GetHeaderKey( uByte** ppKey, uShort* pKeyBytes );
	bool ReadEncryptedHeader();
	
	void WritePlainHeader();
	void WriteEncryptedHeader();
	
	uShort uEMPlainHeaderSize;
	uShort uEMEncryptedHeaderSize;

public:
	int GetHeaderSize() 
	{
		return 
				ObjectStream::GetHeaderSize()
			+	uEMPlainHeaderSize
			+	uEMEncryptedHeaderSize; 
	}

protected:

	// Plain Header vars

	// This random number is generated when this database
	// is created.  It is used as a seed to the HASP key
	// to give us the key to en/decrypt the encrypted 
	// portion of the header.
	uShort		HeaderSeed;					//   2
	
	// Encrypted Header vars

	// We interrupt this program for...
	// ------------------------------------------------------
	// My Public/Protected Member Policy
	// ------------------------------------------------------
	// We could have made this protected and provided member
	// functions to access it, but we would be providing Get/Set
	// member functions for EVERY header member, so it would
	// be against my policy.  If you really need access to
	// a member, then GIVE ACCESS.  Providing both "GetXxx"
	// and "SetXxx" member functions is NO different, just
	// adds a few steps.  
	// It could be argued that if things change, the Get/Set
	// functions provide some buffering.  But it would be a very
	// rare situation with simple member variables where the 
	// member functions themselves would not also change.
	// They typically would have to change the returned type,
	// and then you have more work to do, not less.
	// ------------------------------------------------------
public:
	EMDBHeader EMHeader;

	//
	// End DATABASE HEADER FUNCTIONS
	///////////////////////////////////


	////////////////////////////////
	// VERSIONING FUNCTIONS
	//
	// See ObjectStream.txt for versioning details.

protected:
	virtual bool	UpdateDatabaseFormat();
	virtual bool	UpdateOldObjects();
	uHuge	GetCurrentFileVersion()	{ return uhCurrentEMFileVersion; }

	//
	// End VERSIONING FUNCTIONS
	///////////////////////////////////


protected:

	bool GetObjectKey( 
		uByte** ppKey, 
		uShort* pKeyBytes 
	);

	// These functions manipulate the cache "freshness" status,
	// maintained in memory and in the database header.
	bool IsCacheUpToDate( OBJECT_TYPE eWhich );
	void CacheIsOutOfDate( OBJECT_TYPE eWhich );

	// These two functions are Append() helpers.
	bool UpdateObjectReferences(
		EMComponent*		pObject,						// The object to update
		IDArray**			pSubIDs,						// The arrays used for updating
		DBPointerArray*	pSourceDatabases
	);
	bool UpdateObjectRef(
		ObjectReference*	pRef,							// The subobject Ref to update
		OBJECT_TYPE			Type,							// The subobject type
		IDArray**			pSubIDs,						// The arrays used for updating
		DBPointerArray*	pSourceDatabases
	);

public:
	bool CreateReferenceLog(
		CProgressCtrl*	pProgressMeter,
		CString strLogName  = _T("")
	);


	// ---------------------------------------------------------------
	// TEST DATA FUNCTIONS
	// ---------------------------------------------------------------
	bool StuffTestData(
		CProgressCtrl*	pProgressMeter,
		int				nObjectCount		= 10
	);

	bool ChangeTestData(
		CProgressCtrl* pProgressMeter,
		int				nObjectCount		= 10
	);

	bool DeleteTestData(
		CProgressCtrl*	pProgressMeter,
		int				nObjectCount		= 10
	);

	bool VerifyTestData(
		BaseProgressDlg* pProgressDlg
	);

	void SetObjectData(
		OBJECT_TYPE	eType,
		int			nObjectNum,
		int			nTestValue
	);

	bool VerifyObject(
		OBJECT_TYPE	eType,
		int			nObjectNum,
		int			nTestValue
	);

	void DeleteObject(
		OBJECT_TYPE	eType,
		int			nObjectNum
	);

	void NewTestObject(
		OBJECT_TYPE	eType
	);

	// ---------------------------------------------------------------

};


#endif	// EM_DATABASE_H

