#ifndef PERSISTENT_H
	#define PERSISTENT_H

#include <fstream>
#include <typeinfo>
#include <cstring>					// For wstring, etc.

// Include the underlying database class.
// Note that we COULD save this include for the *.cpp implementation
// file, as we do not directly access it here in the interface file.
// However, every class derived from Persistent (and there are many)
// would have to include this file in its implementation file.  Putting
// it here saves that step.  
// BTW, There doesn't seem to be any way to avoid a rebuild of
// every Persistent-derived class with a change in BaseDatabase.h anyway.
// #include "BaseDatabase.h"					

// TO DO
// Replace with STL version.
#include "linklist.h"

#include "Class.h"

#include "ObjectReference.h"		// Defines the Ref structure


// ============================
// Key Controls
// ============================
typedef short int IndexNo;
typedef short int KeyLength;

#include "btree.h"
#include "TNode.h"

// ============================
// Object Address
// ============================
struct ObjAddr	{
	NodeNbr oa;
	ObjAddr(NodeNbr nd = 0) : oa(nd)
		{ /* ... */ }
	operator NodeNbr() const
		{ return oa; }
};

// ============================
// Persistent Object Header Rcd
// ============================
struct ObjectHeader	{
	
	// class identification
	ClassID classid;	

	// Indicates first node in an object's linked list of nodes.
	bool	bFirstNode;

	ObjectHeader() : classid(-1), bFirstNode( true )
		{ /* ... */ }
};

// MDM This was moved up here because we use it for the
// primary ObjectID key.  It was previously located at the
// bottom of this file.
#include "key.h"

// =====================================
// Persistent object abstract base class
// =====================================
class Persistent	{

private:

	ObjectHeader objhdr;
	ObjAddr objectaddress;	// Node address for this object
	short int indexcount;	// number of keys in the object
	Node *node;					// current node for reading/writing

	// We use these in the derived class for subobject handling.
protected:
	BaseDatabase* pDatabase;		// database for this object
	bool changed;				// true if user changed the object
	bool deleted;				// true if user deleted the object
	bool newobject;		   // true if user is adding the object

private:	

	streampos filepos;	   // for saving file position

	// These are used to make sure derived classes call
	// Load/SaveObject in the con/destructor.  You will
	// get an ASSERTion in the object destructor in
	// debug mode if you did not.  The variables and the
	// checks will not be a part of a release build.
	#ifdef _DEBUG
		bool loaded;			// true if LoadObject called
		bool saved;				// true if SaveObject called
	#endif

public:
	
	// Static pointer that allows access to this object
	// from locations that otherwise don't have access to it.
	// This includes the PdyKey constructor and ReadObject().
	static Persistent* pTargetObject;

private:

	LinkedList<PdyKey> keys;
	LinkedList<PdyKey> orgkeys; // original keys in the object

	// ---- methods used from within Persistent class
	void RegisterKey(PdyKey *key)
		{ keys.AppendEntry(key); }

	void PositionNode() throw (BadObjAddr);
	void AddIndexes();
	void DeleteIndexes();
	void RemoveObject();
	void RemoveOrgKeys();

	void UpdateIndexes();
	void RecordObject();

	void ObjectIn( bool bReindexDataOnly = false );
	void GetObjectHeader();
	void DecryptObjectData();

	void ObjectOut();
	void PutObjectHeader();
	void EncryptObjectData();

	bool SearchIndex(PdyKey* key);
	PdyBtree* FindIndex(PdyKey* key);

	// These are only for use with keyless objects, 
	// which we never use.  They were just commented 
	// out, so they could be used as example code.
	/*
	void ScanForward(NodeNbr nd);
	void ScanBackward(NodeNbr nd);
	*/

public:
	// --- These are public members because template functions
	//     cannot be friends or member functions, and some
	//     template functions need to call them.
	//     Users should not call these member functions.
	void PdyReadObject(void *buf, unsigned int length);
	void PdyWriteObject(const void *buf, unsigned int length);
	void ReadStrObject(string& str);
	void WriteStrObject(const string& str);

	// Added for wstring's.
	void ReadWStrObject(wstring& str);
	void WriteWStrObject(const wstring& str);

protected:

	// Our constructor.
	// The pRef tells us the database AND the object.
	// If we specify the database directly, it overrides
	// the Ref.
	Persistent(
		const ObjectReference* pRef	= NULL,
		BaseDatabase*	pNewDB					= NULL
	);

	// --- provided by derived class
	// These babies do all the work.
	virtual void ReadKeys();
	virtual void WriteKeys();
	virtual void Read() = 0;
	virtual void Write() = 0;

	// ---- called from derived class's constructor
	virtual void LoadObject(ObjAddr nd = 0);

	// Resolves save action request conflicts (newobject, changed, deleted).
	void VerifySaveRequest();

public:

	// Public - we directly delete persistent objects.
	virtual ~Persistent()
		throw (NotLoaded, NotSaved, MustDestroy);

	// ---- called from derived class's destructor
	//      or by user to force output to database
	// MDM Added return value.  It is false if neither AddObject,
	// ChangeObject nor DeleteObject were previously called.
	virtual bool SaveObject() throw (NoDatabase);

	// This specifies if the read was successful, from a data integrity
	// verification standpoint.  All versions of Read() should set this
	// accordingly.
	bool bReadOK;

	// --- class interface methods for modifying database
	void AddObject()		{ newobject	= true; }
	void ChangeObject()	{ changed	= true; }
	void DeleteObject()	{ deleted	= true; }	
	
	bool ObjectExists() const	{ return objectaddress != 0; }
	bool ObjectExistsAndReadOK() const
	{
		return ( ( objectaddress != 0 ) && bReadOK ); 
	}

	// ---- class interface methods for searching database
	// Derived classes may need to perform cleanup, etc.,
	// so these are virtual.  Make sure you call the base class!
	virtual Persistent& FindObject(PdyKey *key);
	virtual Persistent& CurrentObject(PdyKey *key = 0);
	virtual Persistent& FirstObject(PdyKey *key = 0);
	virtual Persistent& LastObject(PdyKey *key = 0);
	virtual Persistent& NextObject(PdyKey *key = 0);
	virtual Persistent& PreviousObject(PdyKey *key = 0);

	// ---- return the object identification
	ObjAddr ObjectAddress() const
		{ return objectaddress; }

	// This is only used when rebuilding an
	// entire index file.
	void RebuildIndex( 
		NodeNbr	nd,
		bool		bReindexDataOnly = true
	);

	// This is only used by Persistent when reindexing with output to
	// a log file.  EMComponent-derived classes all provide an override.
	virtual void GetDisplayName( CString* pstrName ) { *pstrName = _T(""); } 

	// Provide on-the-fly access to the object type.
	ClassID GetClassID() { return objhdr.classid; }

	////////////////////// Versioning //////////////////////////////
	//
	// This function returns the lowest version number for which this object
	// does not need to be updated.  Derived classes should override this
	// and provide the version number during which the last change to the 
	// database structure of the object occurred.
	virtual uHuge GetLowestSafeVersion()
		{ return 0; }
	
protected:

	// Most versioning will typically be handled within Read().  However,
	// some versioning steps require that the object is fully loaded first.
	// Sometimes a "subobject" may need to be saved, etc.  This overridable
	// function provides a place to do that type of updating.
	// This will be called within LoadObject() whenever an old object is 
	// encountered.
	virtual bool	UpdateObjectAfterLoad() { return true; }
	
	//
	////////////////////////////////////////////////////////////////
	
	////////////////////// ObjectID Primary Key //////////////////////////////
	// MDM Added.
	// Use ObjectID's for indexed access to this database's objects.
	// This should be the primary key for all objects, regardless of type.
	// BaseDatabase uses the first key that is assigned as the primary.
	// The ID is set automatically by GenerateID() in SaveObject().
protected:
	Key<ObjectID> ID;

	// This checks the object's ID and automatically assigns the next
	// available ID to the object, and increments the value stored
	// in the class header, if the current ID is zero.
	ObjectID GenerateID();

	// These manipulate the highest ID, kept in the class header.
	ObjectID GetHighestObjectID();
	void SetHighestObjectID(
		ObjectID	ID
	);

public:

	// MDM Allow access to ID.
	ObjectID GetID()
		{ return ID.KeyValue(); }

	// This should be public only because we need to be
	// able to set the ID when doing a find.
	void SetID( const ObjectID NewID )
		{ ID.SetKeyValue( NewID ); }

	// This returns the ID key.
	Key<ObjectID>* GetIDKey() { return &ID; }

	ObjectReference GetRef();

	//////////////////// End ObjectID Primary Key /////////////////////////////

	// This is used to look up the db array index,
	// which is stored in an item's sort data.
	BaseDatabase* GetDatabasePtr() { return pDatabase; }

	void LogBTree( int nKeyNumber = 0 );

	// This function gives us a new object of our object type.
	Persistent* NewObject();
	
	friend class BaseDatabase;
	friend class PdyKey;

};


//-------------------------------------------------------------------//
// VerifySaveRequest()																//
//-------------------------------------------------------------------//
// This ensures that the user has not requested incompatible save 
// actions (newobject, changed, deleted).  It will correct the 
// action flags and ASSERT() if so.
// 
// Derived classes may want to call this function before doing any
// processing, if they do not call the base class version first.
//-------------------------------------------------------------------//
inline void Persistent::VerifySaveRequest()
{

	if ( deleted && changed )
	{
		ASSERT( false );
		changed = false;
	}
	if ( deleted && newobject )
	{

		// Note that in this case, the object probably does not
		// yet exist in the database.  We considered turning off the 
		// deleted flag to make sure that nonexistant objects were
		// not attempted to be deleted.  But SaveObject() only
		// deletes the object if it exists.  So we leave the deleted
		// flag alone.  If the object exists, it will then be deleted.
		// If not, nothing will be done.

		ASSERT( false );
		newobject = false;

	}

}


template <class T>
void ReadObject(T& t)
{
	Persistent::pTargetObject->PdyReadObject(&t, sizeof(T));
}

template <class T>
void WriteObject(const T& t)
{
	Persistent::pTargetObject->PdyWriteObject(&t, sizeof(T));
}

inline void ReadObject(string& s)
{
	Persistent::pTargetObject->ReadStrObject(s);
}

inline void WriteObject(const string& s)
{
	Persistent::pTargetObject->WriteStrObject( s );
}

// MDM Added two functions for Unicode wstring's.
inline void ReadObject(wstring& s)
{
	Persistent::pTargetObject->ReadWStrObject( s );
}

inline void WriteObject(const wstring& s)
{
	Persistent::pTargetObject->WriteWStrObject( s );
}


// MDM Temporarily added this override.
// It is being used to update object refs from
// old style to new style.
extern bool bReadOldStyle;
void ReadObject( ObjectReference& Ref );

#endif	// PERSISTENT_H
