#include "stdafx.h"

#include <new>			// STL versions, no '.h'
#include <cstdlib>
#include <string>

#include "BaseDatabase.h"
#include "Persistent.h"

#include "DatabaseArray.h"

//-------------------------------------------------------------------//
// Initialize static vars.
//-------------------------------------------------------------------//
Persistent*			Persistent::pTargetObject	= 0;

// This is for a temp version 1, 14 change.
bool	bReadOldStyle = false;
//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// Persistent()																		//
//-------------------------------------------------------------------//
// constructor 
// We use the optional database pointer if it is provided.  
//	Otherwise, we use the DBID of the provided Ref to find 
// the associated database.  We use the ObID of the Ref to
// assign the object's ID before loading.
//-------------------------------------------------------------------//
Persistent::Persistent(
	const ObjectReference*	pRef,
	BaseDatabase*						pNewDB
) :

	// Construct the key with the ID from the Ref.
	ID( pRef? pRef->ObID : 0),

	// Init vars.
	bReadOK( false )

{

	// Make sure the database array object was correctly
	// created and our global pointer was assigned to it.
	ASSERT( pDBArray );
	
	// If the database pointer was provided directly, use it.
	if ( pNewDB )
		pDatabase = pNewDB;

	// Look up our database pointer using the database ID.
	// This also verifies that the database is ready to go.
	else if (
		pDBArray->LookUpDatabase( 
			pRef? &( pRef->DBID ) : NULL, 
			&pDatabase 
		) == false
	) {

		// The DBID was provided, but does not resolve to a known
		// database.  The pDatabase pointer was set to the target
		// database.  Set the ObjectID to zero.  This is now a 
		// temp object.
		SetID( 0 );
	
	}

	changed = false;
	deleted = false;
	newobject = false;
	node = 0;
	objectaddress = 0;

	// These are used to make sure the derived class
	// called Load/SaveObject in its con/destructor.
	#ifdef _DEBUG
		loaded = false;
		saved = false;
	#endif

	// Set the target object now so the object's keys can use it.
	// We need indexcount for the same reason.
	// Here is the sequence of events:
	//
	//		1) We set the target object pointer here.
	//		2) We register the ID key here.
	//		3) We exit from this function into the derived class's
	//			constructor.
	//		4) The derived class constructs all its keys before
	//			entering the constructor code.
	//		5) The keys use the target object pointer to register
	//			themselves.
	//		6) The target object pointer gets turned off in 
	//			Persistent::LoadObject.
	//		7) The target object pointer gets turned off or on
	//			during subsequent operations.  It gets turned 
	//			on	for Read/WriteObject(), then off when done.  It is
	//			always turned off for RegisterClass().
	//
	pTargetObject = this;
	indexcount = 0;

	////////////////////// ObjectID Primary Key //////////////////////////////
	
	// We need to explicitly register the ObjectID key here.  
	// The key is constructed before we enter the Persistent 
	// constructor.  Because we couldn't set pTargetObject 
	// before the key was constructed, it was not registered.
	
	// Register the ID key.
	RegisterKey( &ID );
	
	// Assign index number based on position in object.
	// This one should always be first.
	ID.indexno = indexcount++;

	//////////////////// End ObjectID Primary Key /////////////////////////////

}


//-------------------------------------------------------------------//
// ~Persistent()																		//
//-------------------------------------------------------------------//
// ------ destructor
//-------------------------------------------------------------------//
Persistent::~Persistent()
	throw (NotLoaded, NotSaved, MustDestroy)
{

	RemoveObject();
	keys.ClearList();
	delete node;

// ASSERT can now appear within inhouse release builds.
// But "loaded" and "saved" are debug-only data members.
#ifdef _DEBUG

	// You forgot to put LoadObject() in your constructor.
	ASSERT( loaded );
	
	// You forgot to put SaveObject() in your destructor.
	ASSERT( saved );
	
#endif
}


//-------------------------------------------------------------------//
// NewObject()																			//
//-------------------------------------------------------------------//
// This function gives us a new object of our object type.
// With the currently mangled BaseDatabase header files, we can't make 
// this inline.
//-------------------------------------------------------------------//
Persistent* Persistent::NewObject()
{ 
	return pDatabase->NewObject( objhdr.classid ); 
}


//-------------------------------------------------------------------//
// RemoveObject()																		//
//-------------------------------------------------------------------//
//  ---- remove the record of the object's state
//-------------------------------------------------------------------//
void Persistent::RemoveObject()
{
	// --- remove copies of the original keys
	RemoveOrgKeys();
}


//-------------------------------------------------------------------//
// RemoveOrgKeys()																	//
//-------------------------------------------------------------------//
// ----- remove copies of the original keys
//-------------------------------------------------------------------//
void Persistent::RemoveOrgKeys()
{
	PdyKey *ky = orgkeys.FirstEntry();
	while (ky != 0)	{
		delete ky;
		ky = orgkeys.NextEntry();
	}
	orgkeys.ClearList();
}


//-------------------------------------------------------------------//
// GenerateID()																		//
//-------------------------------------------------------------------//
// This checks the object's ID and automatically assigns the next 
// available ID to the object, and increments the value stored 
// in the header, if the current ID is zero.
// This function is called within SaveObject if the object is
// actually going to be saved.  Note that it can also be called 
// at any time to ensure that the object has a
// valid ID.  However, it should only be called if the object is
// going to be saved into the database.  Temporary objects should
// never call it.  Then, as long as the temp object does not call
// AddObject or ChangeObject, no new ID will be generated.  This
// is important so that we do not increment the highest ObjectID
// in the header unnecessarily.
//-------------------------------------------------------------------//
ObjectID Persistent::GenerateID()
{

	// Start with the object's current ID.
	ObjectID idResult = GetID();
	
	// We only generate if current ID is zero.
	if ( idResult == 0 ) {
	
		// The BaseDatabase system keeps the file headers in memory.
		// Access the highest Object ID.
		idResult = GetHighestObjectID();
		
		if ( idResult == 0xFFFFFFFF ) {
		
			// We are out of ID's!
			idResult = 0;

			// TO DO
			// We should notify the user that we are out of ID's.
			// They should be told that the database is full, and
			// then allowed to create a new database.
			ASSERT( false );
			
		} else {

			// Increment the ID.
			idResult++;

			// Save the new highest ID directly to database.
			SetHighestObjectID( idResult );
			
		}

		SetID( idResult );

	}

	return idResult;

}


//-------------------------------------------------------------------//
// Get/SetHighestObjectID()														//
//-------------------------------------------------------------------//
// These manipulate the highest ID, kept in the class header.
// Every class has a header that is maintained in a node in the 
// index file.  
//
// The location of the node is maintained within the
// Class structure's headeraddr variable.  The class variable, in turn,
// is maintained within each of the class's btrees.  So we get the
// header node address by getting the first btree, getting the Class
// structure within the btree, and getting the headeraddr within
// class.  
// See BaseDatabase::AddClassToIndex() for details.
//-------------------------------------------------------------------//
ObjectID Persistent::GetHighestObjectID()
{
		
	// Get the first key, which should ALWAYS 
	// be the ObjectID.  Then get the associated
	// tree.
	PdyKey* key = keys.FirstEntry();
	PdyBtree* bt = FindIndex( key );
		
	// Get the latest ID directly out of the database.
	ObjectID ID;
	pDatabase->indexfile.ReadData( 
		&ID, 
		sizeof( ObjectID ), 
		bt->classindexed->headeraddr
	);

	return ID;

}
void Persistent::SetHighestObjectID( 
	ObjectID ID
) {
		
	// Get the first key, which should ALWAYS 
	// be the ObjectID.  Then get the associated
	// tree.
	PdyKey* key = keys.FirstEntry();
	PdyBtree* bt = FindIndex( key );
		
	// Now save it.
	pDatabase->indexfile.WriteData(
		&ID,
		sizeof( ObjectID ), 
		bt->classindexed->headeraddr
	);

}


//-------------------------------------------------------------------//
// GetRef()																				//
//-------------------------------------------------------------------//
// This gives us access to the whole object reference.
// It is built from the DB ID in the associated database
// object and the Object ID in this persistent object.
//-------------------------------------------------------------------//
ObjectReference Persistent::GetRef()
{

	ObjectReference ReturnRef;
	
	// Set the DB ID.
	ReturnRef.DBID = pDatabase->GetDBID();
	
	// Set the Object ID.
	ReturnRef.ObID = GetID();
	
	// Return a copy of our newly built ref.
	return ReturnRef;

}


//-------------------------------------------------------------------//
// RecordObject()																		//
//-------------------------------------------------------------------//
//  ---------------- record the object's state
//-------------------------------------------------------------------//
void Persistent::RecordObject()
{
	// --- remove copies of the original keys
	RemoveOrgKeys();
	// ---- make copies of the original keys for later update
	PdyKey *key = keys.FirstEntry();
	while (key != 0)	{
		
		// This caused an exception with a bad database.
		PdyKey *ky = key->MakeKey();
		
		*ky = *key;
		orgkeys.AppendEntry(ky);
		// --- instantiate the index b-tree (if not already)
		FindIndex(ky);
		key = keys.NextEntry();
	}
}


//-------------------------------------------------------------------//
// LoadObject()																		//
//-------------------------------------------------------------------//
// -- called from derived constructor after all construction
//-------------------------------------------------------------------//
void Persistent::LoadObject(ObjAddr nd)
{

	// This is used to make sure the derived class
	// called LoadObject in its constructor.
	#ifdef _DEBUG
		loaded = true;
	#endif

	// Make sure we have a valid database.
	ASSERT( pDatabase );

	// Now that we have registered all our class keys, we 
	// turn off the target pointer.  See BuildObject()
	// for details.
	pTargetObject = 0;
	
	// We'll need to get the classid for this class.
	// This sets us up to be able to provide the object
	// type on the fly, with Persistent::GetClassID().
	// It will also be needed when we save the headers for
	// newly added objects and when we call navigation
	// functions like FirstObject().
	objhdr.classid = pDatabase->GetClassID( this );

	// There are two situations where we actually
	// perform a load here: 1) we specified a valid
	// ID in the constructor; or 2) we have previously
	// found the ObjAddr, via FirstObject(), etc.
	// We don't need to bother loading if both
	// the ID and the ObjAddr are zero.
	if ( GetID() != 0 || nd != 0 ) {

		// The class should have been registered as
		// soon as the database was created.
		// ASSERT if not.
		ASSERT( objhdr.classid != -1 );

		// Open the files, read-only.
		pDatabase->Open( false );
		
		objectaddress = nd;
		
		if (objectaddress == 0)
			// --- position at object's node
			SearchIndex(keys.FirstEntry());

		ObjectIn();
		
		// Close the database files, read-only.
		pDatabase->Close( false );

		// Now that the object has finished loading, allow
		// derived classes a chance to update as needed.
		if ( bReadOK && ( GetLowestSafeVersion() > pDatabase->GetDBVersion() ) )
			bReadOK &= UpdateObjectAfterLoad();

	}

}


//-------------------------------------------------------------------//
// ObjectIn()																			//
//-------------------------------------------------------------------//
// ------- read an object's data members
//-------------------------------------------------------------------//
void Persistent::ObjectIn( bool bReindexDataOnly )
{
	if (objectaddress != 0)	{

		try {

			// Load our root node.
			// This also loads the node header and the object header.
			PositionNode();

			// Decrypt.  
			// We're then ready to read the data.
			DecryptObjectData();

			// Let the template functions have access to our pointer.
			pTargetObject = this;

			// --- tell object to read its data members
			ReadKeys();
			if ( !bReindexDataOnly )
			{
				// This is the beginning of the reading of data for Persistent
				// and derived classes.  Set bReadOK to true, and we will change 
				// it to false on any error.
				bReadOK = true;
				Read();

			}

			// Turn off the pointer hack.
			pTargetObject = 0;

			delete node;
			node = 0;

			// --- post object instantiated and
			//     put secondary keys in table
			RecordObject();

		}

		catch( ... )
		{
			// Bad database!  Likely causes:
			//		1) Mismatched *.rtx - try deleting it
			//		2) Corrupt file
			ASSERT( false );
			bReadOK = false;
			pTargetObject = 0;
			delete node;
			node = 0;
			objectaddress = 0;
			return;
		}

	}

}


//-------------------------------------------------------------------//
// GetObjectHeader()																	//
//-------------------------------------------------------------------//
// Get the object's node header.
//-------------------------------------------------------------------//
void Persistent::GetObjectHeader()
{
	node->GetFromBuffer( &objhdr, sizeof ObjectHeader );
}


//-------------------------------------------------------------------//
// DecryptObjectData()																//
//-------------------------------------------------------------------//
// This function decrypts the object data in the node.
// We assume that the buffer is loaded and ready to go.
//-------------------------------------------------------------------//
void Persistent::DecryptObjectData()
{

	///////////////////////////////////////////////////
	/*
	// DEBUG
	// This section was used to modify the decryption of an existing
	// database.  The new encryption mechanism was put into place.
	// Then, the old database was converted using DBMgrListControl::OnOtherTest(),
	// which read and wrote all objects.  This section ensured that 
	// during the read, the objects were decrypted with the OLD
	// encryption mechanism.
	//

	if ( 
			pDatabase->GetDBID().User.UserNumber == 0xecfb46e4		// 0xecfb46e4	=	Olly's orig. #
		&&	pDatabase->GetDBID().User.Group == MasterGroup
	) {

		static BlowFish OldDecryptor;
		static bool bOldDecryptorInited = false;

		if ( !bOldDecryptorInited ) {
			bOldDecryptorInited = true;
			uHuge DBKey = 0xc23f92fef4f23d33;
			OldDecryptor.InitializeSubKeys( 
				(unsigned char*)&DBKey, 
				sizeof DBKey
			);
		}

		// Decrypt with the old.
		OldDecryptor.StreamDecrypt(
			node->pBuffer + node->NodeHeaderSize() + sizeof ObjectHeader,
			pDatabase->datafile.GetNodeDataLength() - sizeof ObjectHeader
		);

		return;
	
	}

	//
	*/
	///////////////////////////////////////////////////


	pDatabase->blow.StreamDecrypt(
		node->pBuffer + node->NodeHeaderSize() + sizeof ObjectHeader,
		pDatabase->datafile.GetNodeDataLength() - sizeof ObjectHeader
	);

}


//-------------------------------------------------------------------//
// ReadKeys()																			//
//-------------------------------------------------------------------//
// This function reads all key data.  The key data is read separately
// from the rest of the object data to facilitate rebuilding of the
// index file.  In that case, only the key data is read.  See
// ObjectIn() for details.
// If derived classes have additional keys, they should override this
// function, call this base class version, and then process their
// additional key data.
// While it is not a necessity at this time, writing of keys was 
// separated into the WriteKeys() function for consistency 
// purposes.
//-------------------------------------------------------------------//
void Persistent::ReadKeys() 
{

	/////////////////////////////////////////
	// All objects always start with the ID.
	ObjectID OID;
	ReadObject( OID );
	ID.SetKeyValue( OID );
	/////////////////////////////////////////

}


//-------------------------------------------------------------------//
// PdyReadObject()																	//
//-------------------------------------------------------------------//
// --- read one data member of the object from the database
//
// This function manages reads across database nodes.  
// It first determines if we are at the end of the node, and, 
// if so, loads the next node.
// Then we read to the end of the requested length or the end of
// the node, whichever comes first.
// Then, if we hit the end of the node, we go to the next and
// continue reading.
//-------------------------------------------------------------------//
void Persistent::PdyReadObject(void *buf, unsigned int length)
{
	
	while (node != 0 && length > 0)	{

		// If we hit the end of the node...
		if ( node->BufferPosition == pDatabase->datafile.GetNodeLength() ) {

			// Get the next one.
			NodeNbr nx = node->NextNode();
			delete node;
			
			// If we found one, load it up.
			if ( nx ) {

				node = new Node(
					&( pDatabase->datafile ), 
					nx
				);

				// Get the object header.	
				GetObjectHeader();
			
				// Decrypt the node's object data.
				DecryptObjectData();

			// If we didn't find a node, set the pointer to 0.
			} else {

				node = 0;

			}
			
		}
		if (node != 0)	{
			
			// The length of the next data block is the shorter 
			// of the remaining length of the data and the 
			// remaining length of the node.
			unsigned int len = min(
				length,
				(
						(unsigned int) pDatabase->datafile.GetNodeLength() 
					-	(unsigned int) node->BufferPosition
				)
			);
			
			// Get the data block from the buffer.
			node->GetFromBuffer( buf, len );

			buf = reinterpret_cast<char*>(buf) + len;
			length -= len;

		}
	}
}


//-------------------------------------------------------------------//
// SaveObject()																		//
//-------------------------------------------------------------------//
// This function saves the object, whether it is new or it
// has changed.
//-------------------------------------------------------------------//
bool Persistent::SaveObject()
		throw (NoDatabase)
{
	// Note that we may need to open the file for writing
	// in order to handle the subobjects.  Therefore, we
	// wrap everything in our own open/close group.  That
	// way, we nest the open/closes, and only one occurs.
	// Note that we only open if needed.  We need to go
	// through SaveObject even if we don't save, to reset
	// variables, but we should avoid opening the file.
	// This bypass will cascade through the subobjects.
	bool bOpen = ( changed || newobject || deleted );
	if ( bOpen )
		pDatabase->Open( true );

	// This is used to make sure the derived class
	// called SaveObject in its destructor.
	#ifdef _DEBUG
		saved = true;
	#endif
	
	// Requests to add, change, and delete objects are all
	// handled here.  More than one request may have been
	// made, perhaps unintentionally.  This will ASSERT
	// on incompatible combinations, and correct them.
	VerifySaveRequest();

	bool bReturn = true;
	
	// New object?
	if ( newobject )	
	{
		ASSERT( !deleted );
		ASSERT( objectaddress == 0 );

		// Open the files, writable.
		pDatabase->Open( true );
			
		delete node;  // (just in case)

		// This node does not exist, use true third parameter.
		// This will create our node and stuff the node header
		// into the node buffer.
		node = new Node(
			&( pDatabase->datafile ), 
			0,										// We want a new node, use 0
			NM_CREATE
		);
		
		objectaddress = node->GetNodeNbr();

		// We are now going to save the header.  
		// We looked up the class ID in LoadObject(),
		// now we need to update the bFirstNode status.
		// This is the first node, mark it as so.
		objhdr.bFirstNode = true;

		PutObjectHeader();
		
		// Now that the header is written, additional headers 
		// are not the first.
		objhdr.bFirstNode = false;

		ASSERT( ObjectExists() );

		// MDM Make sure we have generated the ID.
		GenerateID();

		AddIndexes();
		PositionNode();
		ObjectOut();
		RecordObject();

		// Close the files, writable.
		pDatabase->Close( true );

		// Note that bReadOK was never set true, since this is a newly
		// created object.  Now that we know the object is correctly
		// stored in the database, mark bReadOK true.  This helps
		// derived classes be able to verify objects more consistently.
		bReadOK = true;

	// Save changes if requested.
	// We acknowledge change status only
	// if we have a valid address.
	} else if ( ObjectExists() && changed ) 
	{
		// Open the files, writable.
		pDatabase->Open( true );
		
		// --- position the BaseDatabase file at the object's node
		PositionNode();

		// MDM Make sure we have generated the ID.
		GenerateID();

		// --- tell object to write its data members
		ObjectOut();
		// ---- update the object's indexes
		UpdateIndexes();
		RecordObject();

		// Close the files, writable.
		pDatabase->Close( true );
	
	// Delete if requested.
	// We acknowledge delete status only
	// if we have a valid address.
	} else if ( ObjectExists() && deleted ) 
	{
		// Open the files, writable.
		pDatabase->Open( true );
		
		// Get the first node.  Save the next node number
		// and then delete it.
		delete node;
		node = new Node( 
			&( pDatabase->datafile ), 
			objectaddress,
			NM_DELETE
		);
		NodeNbr nx = node->NextNode();
		delete node;
		node = 0;
		
		// Loop through the rest of object's nodes, deleting 
		// them.
		while (nx != 0)	
		{			
			// Get the next node, marking it for deletion.
			// It will be killed at the end of this loop
			// iteration, when it goes out of scope.
			Node nd( 
				&( pDatabase->datafile ), 
				nx,
				NM_DELETE
			);
			nx = nd.NextNode();

		}

		DeleteIndexes();
		objectaddress = 0;

		// Close the files, writable.
		pDatabase->Close( true );

	// MDM Added this case.
	} else 
	{
		bReturn = false;
	}

	newobject = false;
	deleted = false;
	changed = false;

	// Wait until after all the saves to close.
	if ( bOpen )
		pDatabase->Close( true );

	return bReturn;

}


//-------------------------------------------------------------------//
// ObjectOut()																			//
//-------------------------------------------------------------------//
// ------ write the object to the database
//-------------------------------------------------------------------//
void Persistent::ObjectOut()
{
	
	// Let the template functions have access to our pointer.
	pTargetObject = this;
	
	// --- tell object to write its data members
	WriteKeys();
	Write();

	// Turn off pointer hack.
	pTargetObject = 0;
	
	// Here, we end the node list.  We need to set the next
	// node to zero, encrypt the last buffer, and delete
	// it, which writes it out.
	NodeNbr nx = node->NextNode();
	node->SetNextNode(0);
	EncryptObjectData();
	delete node;
	node = 0;

	// If there were previously more nodes in the list, 
	// delete them; they're no longer needed.
	while (nx != 0)	{
		
		// Get the next node, marking it for deletion.
		// It will be killed at the end of this loop
		// iteration, when it goes out of scope.
		Node nd( 
			&( pDatabase->datafile ), 
			nx,
			NM_DELETE
		);
		nx = nd.NextNode();

	}

}


//-------------------------------------------------------------------//
// PutObjectHeader()																	//
//-------------------------------------------------------------------//
void Persistent::PutObjectHeader()
{
	node->PutToBuffer( &objhdr, sizeof ObjectHeader );
}


//-------------------------------------------------------------------//
// EncryptObjectData()																//
//-------------------------------------------------------------------//
// This function encrypts the object data in the node.
// We assume that the buffer is loaded and ready to go.
//-------------------------------------------------------------------//
void Persistent::EncryptObjectData()
{

	node->ClearRemainingBuffer();
	
	
	///////////////////////////////////////////////////
	/*
	// DEBUG
	// This section was used to emulate an old encryption mechanism.
	// It is not needed when updating from an old mechanism to a
	// new one; just override DecryptObjectData() and let the
	// encryption use the new method.  See DBMgrListControl::OnOtherTest()
	// for details.
	//
	
	static bool bUseOldEncryption = false;
	
	// This melarchy is so we can change the static.
	// There's no access to statics in the debugger!!
	bool bTemp = bUseOldEncryption;
	bUseOldEncryption = bTemp;
	
	if ( 
			pDatabase->GetDBID().User.UserNumber == 0xc5ae88e3			// Olly's orig. #
		&&	pDatabase->GetDBID().User.Group == MasterGroup
		&& bUseOldEncryption
	) {

		static BlowFish OldEncryptor;
		static bool bOldEncryptorInited = false;

		if ( !bOldEncryptorInited ) {
			bOldEncryptorInited = true;
			uHuge DBKey = 0xc23f92fef4f23d33;
			OldEncryptor.InitializeSubKeys( 
				(unsigned char*)&DBKey, 
				sizeof DBKey
			);
		}

		// Decrypt with the old.
		OldEncryptor.StreamEncrypt(
			node->pBuffer + node->NodeHeaderSize() + sizeof ObjectHeader,
			pDatabase->datafile.GetNodeDataLength() - sizeof ObjectHeader
		);

		node->MarkNodeChanged();

		return;
	
	}

	//
	*/
	///////////////////////////////////////////////////

	pDatabase->blow.StreamEncrypt(
		node->pBuffer + node->NodeHeaderSize() + sizeof ObjectHeader,
		pDatabase->datafile.GetNodeDataLength() - sizeof ObjectHeader
	);

	node->MarkNodeChanged();

}


//-------------------------------------------------------------------//
// WriteKeys()																			//
//-------------------------------------------------------------------//
// This function writes all key data.  See ReadKeys().
//-------------------------------------------------------------------//
void Persistent::WriteKeys() 
{

	/////////////////////////////////////////
	// All objects always start with the ID.
	WriteObject(ID.KeyValue());
	/////////////////////////////////////////

}


//-------------------------------------------------------------------//
// PdyWriteObject()																	//
//-------------------------------------------------------------------//
// --- write one data member of the object to the database
//-------------------------------------------------------------------//
void Persistent::PdyWriteObject( const void *buf, unsigned int length )
{
	while (node != 0 && length > 0)	{

		// If we hit the end of the node...
		if ( node->BufferPosition == pDatabase->datafile.GetNodeLength() ) {

			// Get the next node number, if available.
			NodeNbr nx = node->NextNode();
			
			// Get a new node, in create mode.  
			// This means that nothing will be read from the
			// node.  It also means that if the NodeNbr is
			// zero, the function will either appropriate a
			// previously deleted node or append a new one to the
			// database.  In this case, the node's nodenbr will 
			// be updated during construction.
			Node* pNextNode = new Node(
				&( pDatabase->datafile ),	//	NodeFile
				nx,								// nodenbr
				NM_CREATE
			);

			// Make sure the current node points to the next one.
			// Note that we should look up the next node number,
			// as nx may be zero.
			node->SetNextNode( pNextNode->GetNodeNbr() );

			// Encrypt the node's object data.
			EncryptObjectData();
			
			// Delete the full node, writing the data.
			delete node;

			// Now we are ready to move to the next node.
			node = pNextNode;

			objhdr.bFirstNode = false;
			PutObjectHeader();

		}

		// The length of the read is the shorter of the remaining length
		// of the data and the remaining length of the node.
		unsigned int len = min(
			length,
			(
					(unsigned int)	pDatabase->datafile.GetNodeLength() 
				-	(unsigned int) node->BufferPosition
			)
		);
		
		// Put the data block to the buffer.
		node->PutToBuffer( buf, len );

		buf = reinterpret_cast<const char*>(buf) + len;
		length -= len;

	}
}


//-------------------------------------------------------------------//
// FindIndex()																			//
//-------------------------------------------------------------------//
// ------ search the collected Btrees for this key's index
// MDM Gets the node that contains the header for the index.
// It uses the classname and the indexno to find it.
//-------------------------------------------------------------------//
PdyBtree *Persistent::FindIndex(PdyKey *key)
{
	PdyBtree *bt = 0;
	if (key == 0)
		key = keys.FirstEntry();
	if (key != 0)	{
		bt = pDatabase->btrees.FirstEntry();
		
		// Get the actual name of our class.
		const char *ty = typeid(*this).name();

		// MDM Don't bother checking the "class ..." that
		// preceeds each class name returned from typeid.
		ty += 6;

		while (bt != 0)	{

			if (strcmp(ty, bt->ClassIndexed()->classname) == 0)
				if (bt->Indexno() == key->indexno)
					break;
			bt = pDatabase->btrees.NextEntry();
		}
	}
	return bt;
}


//-------------------------------------------------------------------//
// SearchIndex()																		//
//-------------------------------------------------------------------//
// ------- search the index for a match on the key
// MDM Updated return type, for error checking.
// void Persistent::SearchIndex(PdyKey *key)
//-------------------------------------------------------------------//
bool Persistent::SearchIndex(PdyKey *key)
{
	
	// MDM Added return value.
	bool bReturn = false;
	
	objectaddress = 0;
	if (key != 0 && !key->isNullValue())	{
		
		// MDM This places us at the top of the index
		// for our key.  We then can search from there.
		PdyBtree *bt = FindIndex(key);
		
		// MDM This will scan the entire index, as needed,
		// to find our key.
		if (bt != 0 && bt->Find(key))	{
			
			// MDM All this code does is set us at the
			// FIRST occurance of our key, if there is
			// more than one.
			if (key->indexno != 0)	{
				PdyKey *bc;
				do
					bc = bt->Previous();
				while (bc != 0 && *bc == *key);
				key = bt->Next();
			}

			// MDM We set the address and we are done.
			objectaddress = key->fileaddr;

			// MDM Added.
			bReturn = true;
		
		}
	}

	// MDM Added.
	return bReturn;

}


//-------------------------------------------------------------------//
// AddIndexes()																		//
//-------------------------------------------------------------------//
// ---- add the index values to the object's index btrees
// MDM This function adds keys to each index btree for all
// keys defined for the current object.
//-------------------------------------------------------------------//
void Persistent::AddIndexes()
{
	PdyKey *key = keys.FirstEntry();
	while (key != 0)	{
		if (!key->isNullValue())	{
			PdyBtree *bt = FindIndex(key);
			key->fileaddr = objectaddress;
			bt->Insert(key);
		}
		key = keys.NextEntry();
	}
}


//-------------------------------------------------------------------//
// DeleteIndexes()																	//
//-------------------------------------------------------------------//
// -- delete the index values from the object's index btrees
//-------------------------------------------------------------------//
void Persistent::DeleteIndexes()
{
	PdyKey *key = orgkeys.FirstEntry();
	while (key != 0)	{
		if (!key->isNullValue())	{
			
			PdyBtree *bt = FindIndex(key);
			ASSERT( bt );

			key->fileaddr = objectaddress;
			bt->Delete(key);
		}
		key = orgkeys.NextEntry();
	}
}


//-------------------------------------------------------------------//
// UpdateIndexes()																	//
//-------------------------------------------------------------------//
// ---- update the index values in the object's index btrees
//-------------------------------------------------------------------//
void Persistent::UpdateIndexes()
{
	PdyKey *oky = orgkeys.FirstEntry();
	PdyKey *key = keys.FirstEntry();
	while (key != 0)	{
		if (!(*oky == *key))	{
			// --- key value has changed, update the index
			PdyBtree *bt = FindIndex(oky);
			// --- delete the old
			if (!oky->isNullValue())	{
				oky->fileaddr = objectaddress;
				bt->Delete(oky);
			}
			// --- insert the new
			if (!key->isNullValue())	{
				key->fileaddr = objectaddress;
				bt->Insert(key);
			}
		}
		oky = orgkeys.NextEntry();
		key = keys.NextEntry();
	}
}


//-------------------------------------------------------------------//
// PositionNode()																		//
//-------------------------------------------------------------------//
// ----- position the file to the specifed node number
//-------------------------------------------------------------------//
void Persistent::PositionNode()
		throw (BadObjAddr)
{
	
	// Is this needed?
	filepos = pDatabase->datafile.FilePosition();
	
	if (objectaddress)	{

		delete node;

		// Read in the node at the object address.
		node = new Node( 
			&( pDatabase->datafile ), 
			objectaddress
		);
		
		ObjectHeader oh;
		node->GetFromBuffer( &oh, sizeof ObjectHeader );
		if ( !oh.bFirstNode || oh.classid != objhdr.classid)
			throw BadObjAddr();

	}
}


// These functions are only for use with keyless objects, which we never
// use.  They were commented out, for use as example code.
/*
//-------------------------------------------------------------------//
// ScanForward()																		//
//-------------------------------------------------------------------//
// --- scan nodes forward to the first one of next object
//-------------------------------------------------------------------//
void Persistent::ScanForward(NodeNbr nd)
{
	ObjectHeader oh;
	while (nd++ < pDatabase->datafile.HighestNode())	{
		pDatabase->GetObjectHeader(nd, oh);
		if ( oh.classid == objhdr.classid && oh.bFirstNode )	{
			objectaddress = nd;
			break;
		}
	}
}
//-------------------------------------------------------------------//
// ScanBackward()																		//
//-------------------------------------------------------------------//
// --- scan nodes back to first one of the previous object
//-------------------------------------------------------------------//
void Persistent::ScanBackward(NodeNbr nd)
{
	ObjectHeader oh;
	while (--nd > 0)	{
		pDatabase->GetObjectHeader(nd, oh);
		if (oh.classid == objhdr.classid && oh.bFirstNode )	{
			objectaddress = nd;
			break;
		}
	}
}
// This was a BaseDatabase function needed by scan functions.  
// Note that it should use buffer, not direct read.
//-------------------------------------------------------------------//
// GetObjectHeader()																	//
//-------------------------------------------------------------------//
// ------- read an object header record
//-------------------------------------------------------------------//
void BaseDatabase::GetObjectHeader(ObjAddr nd,ObjectHeader &objhdr)
{
	// --- constructing this node seeks to the first data byte
	Node(&datafile, nd);
	datafile.ReadData(&objhdr, sizeof(ObjectHeader));
}
*/


//-------------------------------------------------------------------//
// FindObject()																		//
//-------------------------------------------------------------------//
// --------- find an object by a key value
//-------------------------------------------------------------------//
Persistent& Persistent::FindObject(PdyKey *key)
{

	// Open the files, read-only.
	pDatabase->Open( false );
		
	RemoveObject();
	SearchIndex(key);

	// We need to go through LoadObject() to process subobjects, hit 
	// versioning code, etc.
	if ( objectaddress )
		LoadObject( objectaddress );
	
	// Close the files, read-only.
	pDatabase->Close( false );
	
	return *this;
}


//-------------------------------------------------------------------//
// CurrentObject()																	//
//-------------------------------------------------------------------//
// --- retrieve the current object in a key sequence
//-------------------------------------------------------------------//
Persistent& Persistent::CurrentObject(PdyKey *key)
{
	
	// Open the files, read-only.
	pDatabase->Open( false );
		
	// MDM See note below.
	PdyKey* pOriginalKey = key;
	
	RemoveObject();
	PdyBtree *bt = FindIndex(key);
	if (bt != 0)	{
		if ((key = bt->Current()) != 0)
			objectaddress = key->fileaddr;
		ObjectIn();
	}
	
	// MDM BaseDatabase bug.
	// Unfortunately, key->filaddr is not updated in
	// the key that was passed to this function (we
	// changed the address of it).  This causes problems
	// with calls to FindObject after calls to NextObject.
	// Here, we update key->fileaddr manually.
	if ( bt && key && pOriginalKey )
		pOriginalKey->fileaddr = key->fileaddr;

	// Close the files, read-only.
	pDatabase->Close( false );
	
	return *this;

}


//-------------------------------------------------------------------//
// FirstObject()																		//
//-------------------------------------------------------------------//
// --- retrieve the first object in a key sequence
//-------------------------------------------------------------------//
Persistent& Persistent::FirstObject(PdyKey *key)
{
	
	// Open the files, read-only.
	pDatabase->Open( false );
		
	// MDM See note below.
	PdyKey* pOriginalKey = key;
	
	RemoveObject();
	objectaddress = 0;
	PdyBtree *bt = FindIndex(key);
	
	// We should always succeed with the find, as
	// all classes should be pre-registered.
	ASSERT( bt );

	if ((key = bt->First()) != 0)
		objectaddress = key->fileaddr;
	
	// We need to go through LoadObject() to process subobjects, hit 
	// versioning code, etc.
	if ( objectaddress )
		LoadObject( objectaddress );
	
	// MDM BaseDatabase bug.
	// Unfortunately, key->filaddr is not updated in
	// the key that was passed to this function (we
	// changed the address of it).  This causes problems
	// with calls to FindObject after calls to NextObject.
	// Here, we update key->fileaddr manually.
	if ( key && pOriginalKey )
		pOriginalKey->fileaddr = key->fileaddr;

	// Close the files, read-only.
	pDatabase->Close( false );
	
	return *this;
}


//-------------------------------------------------------------------//
// LastObject()																		//
//-------------------------------------------------------------------//
// --- retrieve the last object in a key sequence
//-------------------------------------------------------------------//
Persistent& Persistent::LastObject(PdyKey *key)
{
	
	// Open the files, read-only.
	pDatabase->Open( false );
		
	// MDM See note below.
	PdyKey* pOriginalKey = key;
	
	RemoveObject();
	objectaddress = 0;
	PdyBtree *bt = FindIndex(key);

	// We should always succeed with the find, as
	// all classes should be pre-registered.
	ASSERT( bt );
	
	if ((key = bt->Last()) != 0)
		objectaddress = key->fileaddr;
	
	// We need to go through LoadObject() to process subobjects, hit 
	// versioning code, etc.
	if ( objectaddress )
		LoadObject( objectaddress );
	
	// MDM BaseDatabase bug.
	// Unfortunately, key->filaddr is not updated in
	// the key that was passed to this function (we
	// changed the address of it).  This causes problems
	// with calls to FindObject after calls to NextObject.
	// Here, we update key->fileaddr manually.
	if ( key && pOriginalKey )
		pOriginalKey->fileaddr = key->fileaddr;

	// Close the files, read-only.
	pDatabase->Close( false );
	
	return *this;
}


//-------------------------------------------------------------------//
// NextObject()																		//
//-------------------------------------------------------------------//
// --- retrieve the next object in a key sequence
//-------------------------------------------------------------------//
Persistent& Persistent::NextObject(PdyKey *key)
{
	
	// Open the files, read-only.
	pDatabase->Open( false );
		
	// MDM See note below.
	PdyKey* pOriginalKey = key;
	
	RemoveObject();
	ObjAddr oa = objectaddress;
	objectaddress = 0;
	
	// MDM Get the starting node of the index.
	PdyBtree *bt = FindIndex(key);
	
	// We should always succeed with the find, as
	// all classes should be pre-registered.
	ASSERT( bt );
	
	// MDM Get the next key.
	// Note that trnode is used to determine the next object,
	// and we need trnode to be properly set before this point.
	// Originally, we were considering just re-finding trnode here
	// as needed, using the given key.  That doesn't work for the 
	// following reason: if we are dealing with a secondary key, and 
	// there are duplicate key values, we won't know which duplicate
	// is the right one - all we have to go on is the key itself.
	// We need to try to always preserve trnode, eg, in UpdateIndex.
	if ((key = bt->Next()) != 0)
		objectaddress = key->fileaddr;
	
	// We need to go through LoadObject() to process subobjects, hit 
	// versioning code, etc.
	if ( objectaddress )
		LoadObject( objectaddress );
	
	// MDM BaseDatabase bug.
	// Unfortunately, key->filaddr is not updated in
	// the key that was passed to this function (we
	// changed the address of it).  This causes problems
	// with calls to FindObject after calls to NextObject.
	// Here, we update key->fileaddr manually.
	if ( key && pOriginalKey )
		pOriginalKey->fileaddr = key->fileaddr;

	// Close the files, read-only.
	pDatabase->Close( false );
	
	return *this;
}


//-------------------------------------------------------------------//
// PreviousObject()																	//
//-------------------------------------------------------------------//
// --- retrieve the previous object in a key sequence
//-------------------------------------------------------------------//
Persistent& Persistent::PreviousObject(PdyKey *key)
{
	
	// Open the files, read-only.
	pDatabase->Open( false );
		
	// MDM See note below.
	PdyKey* pOriginalKey = key;
	
	RemoveObject();
	ObjAddr oa = objectaddress;
	objectaddress = 0;
	PdyBtree *bt = FindIndex(key);

	// We should always succeed with the find, as
	// all classes should be pre-registered.
	ASSERT( bt );
	
	if ((key = bt->Previous()) != 0)
		objectaddress = key->fileaddr;
	
	// We need to go through LoadObject() to process subobjects, hit 
	// versioning code, etc.
	if ( objectaddress )
		LoadObject( objectaddress );
	
	// MDM BaseDatabase bug.
	// Unfortunately, key->filaddr is not updated in
	// the key that was passed to this function (we
	// changed the address of it).  This causes problems
	// with calls to FindObject after calls to NextObject.
	// Here, we update key->fileaddr manually.
	if ( key && pOriginalKey )
		pOriginalKey->fileaddr = key->fileaddr;

	// Close the files, read-only.
	pDatabase->Close( false );
	
	return *this;
}


//-------------------------------------------------------------------//
// RebuildIndex()																		//
//-------------------------------------------------------------------//
// This is only used when rebuilding an entire index file.
// See BaseDatabase::RebuildIndexFile() for details.
// Note that we specify false for bReindexDataOnly when we are
// creating a log, so that we have the name information for the log.
//-------------------------------------------------------------------//
void Persistent::RebuildIndex( 
	NodeNbr	nd,
	bool		bReindexDataOnly
) {

	objectaddress = nd;
	ObjectIn( bReindexDataOnly );
	AddIndexes();

	// Since we are rebuilding the index file, we have lost the
	// highest-assigned ObjectID counter, which is stored in
	// the index file.  Now that we have the object loaded, we
	// should use the ObjectID value to make sure that the
	// highest-assigned ObjectID in the class header is at least 
	// as great as our current ObjectID.
	ObjectID CurrentID = GetID();

	// Get the highest known ID.
	ObjectID HighestID = GetHighestObjectID();

	// Write out a new one as needed.
	if ( CurrentID > HighestID )
		SetHighestObjectID( CurrentID );

}


//-------------------------------------------------------------------//
// LogBTree()																			//
//-------------------------------------------------------------------//
void Persistent::LogBTree( int nKeyNumber )
{

	// Get the requested key.
	PdyKey* key = keys.FirstEntry();
	int n = 0;
	while ( n < nKeyNumber ) {
		key = keys.NextEntry();
		n++;
	}

	// Get the associated tree.
	PdyBtree* bt = FindIndex( key );

	// Open the index file and log the tree.
	pDatabase->indexfile.Open( false );
	bt->Log();
	pDatabase->indexfile.Close( false );
	
}


//-------------------------------------------------------------------//
// ReadStrObject()																	//
//-------------------------------------------------------------------//
void Persistent::ReadStrObject(string& str)
{
	short int len;
	PdyReadObject(&len, sizeof(short int));
	char *s = new char[len+1];
	PdyReadObject(s, len);
	s[len] = '\0';
	str = s;
	delete s;
}


//-------------------------------------------------------------------//
// WriteStrObject()																	//
//-------------------------------------------------------------------//
void Persistent::WriteStrObject(const string& str)
{
	short int len = strlen(str.c_str());
	PdyWriteObject(&len, sizeof(short int));
	PdyWriteObject(str.c_str(), len);
}


//-------------------------------------------------------------------//
// ReadWStrObject()																	//
//-------------------------------------------------------------------//
// MDM Added two functions for Unicode wstring's.
//-------------------------------------------------------------------//
void Persistent::ReadWStrObject(wstring& str)
{
	short int len;
	PdyReadObject(&len, sizeof(short int));
	wchar_t* s = new wchar_t[len+1];
	
	// Read into s.  
	// Bytes = char len * bytes/char.
	PdyReadObject( s, len * sizeof(wchar_t) );
	
	s[len] = '\0';
	str = s;
	delete s;
}


//-------------------------------------------------------------------//
// WriteWStrObject()																	//
//-------------------------------------------------------------------//
void Persistent::WriteWStrObject(const wstring& str)
{
	short int len = wcslen( str.c_str() );
	
	// Write the string length header.
	PdyWriteObject(&len, sizeof(short int));
	
	// Write the data in the string.
	// Bytes = char len * bytes/char.
	PdyWriteObject( str.c_str(), len * sizeof(wchar_t) );

}


//-------------------------------------------------------------------//
// ReadObject()																		//
//-------------------------------------------------------------------//
// Overrode ReadObject (a templated function) when we are reading
// an ObjectReference.  We have some versioning to do.
//-------------------------------------------------------------------//
void ReadObject( ObjectReference& Ref )
{
	if ( bReadOldStyle )
	{
		// Set up a structure to hold the old-style ref.
		struct{
		
				// DBID
				unsigned int	bPublished:1;	
				unsigned int	DBNumber:31;	
			
					// UserID
					uLong			UserNumber;		
					uLong			Group;			
			
				uLong				DBVersion;				

			ObjectID				ObID;
		
		} OldRef;

		// Read the old-style ref.
		Persistent::pTargetObject->PdyReadObject( &OldRef, sizeof( OldRef ) );

		// Copy data to the new ref style.
		Ref.ObID = OldRef.ObID;
		Ref.DBID.DBNumber = OldRef.DBNumber;
		Ref.DBID.UserNumber = OldRef.UserNumber;
	
	} else 
	{
		Persistent::pTargetObject->PdyReadObject( &Ref, sizeof( ObjectReference ) );
	}
}

