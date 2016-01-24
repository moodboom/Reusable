// ---------------- btree.cpp

#include "stdafx.h"

#include <string>				// For string, wstring

#include "BaseDatabase.h"
#include "Class.h"


//-------------------------------------------------------------------//
// PdyBtree()																			//
//-------------------------------------------------------------------//
// ---------- constructor to open a btree
//-------------------------------------------------------------------//
PdyBtree::PdyBtree(
	NodeFile&	ndx,			// Index file
	Class*		cls, 
	PdyKey*		ky
)

	// Exceptions.
	throw (BadKeylength)

:

	// Init vars.
	index(ndx),
	keylength( 0 ),
	oldcurrnode( 0 ),			// Possible BaseDatabase oversight?
	trnode( 0 ),
	classindexed( cls ),
	currnode( 0 )

{
	

	nullkey = ky->MakeKey();
	nullkey->PdyKey::operator=(*ky);
	indexno = ky->indexno;

	// Read the key length.
	ReadKeyLength();
	
	// If we have a new key, keylength will be zero and
	// we should save the true key length from the key.
	if ( keylength == 0 ) {
		keylength = ky->keylength;
		WriteKeyLength();
	
	// Otherwise, they had better match.
	} else if ( 
			ky->keylength != 0 
		&& keylength != ky->keylength
	)
		throw BadKeylength();
}


//-------------------------------------------------------------------//
// ~PdyBtree()																			//
//-------------------------------------------------------------------//
// ---------- destructor for a btree
//-------------------------------------------------------------------//
PdyBtree::~PdyBtree()
{
	delete trnode;
	delete nullkey;
}


//-------------------------------------------------------------------//
// MakeKeyBuffer()																	//
//-------------------------------------------------------------------//
// ----- make a key buffer 
//-------------------------------------------------------------------//
PdyKey *PdyBtree::MakeKeyBuffer() const
{
	PdyKey *thiskey = nullkey->MakeKey();
	thiskey->indexno = indexno;
	return thiskey;
}


//-------------------------------------------------------------------//
// Insert()																				//
//-------------------------------------------------------------------//
// ---------------- insert a key into a btree
//-------------------------------------------------------------------//
void PdyBtree::Insert(PdyKey *keypointer)
{
	
	// MDM Added.  See comments below.
	bool NewCurrNode = false;
	
	// ---- don't insert duplicate keys
	if (!Find(keypointer))	{

		PdyKey *newkey = keypointer->MakeKey();
		*newkey = *keypointer;

		NodeNbr rootnode = 0, leftnode = 0, rightnode = 0;
		bool RootisLeaf = true;

		bool done = false;
		// -------- insert key into btree
		while (currnode)	{
			
			// Get target number of keys per node, max.
			int em = trnode->m();

			// first insertion is into leaf
			// if split, later insertions
			// are into parents (non-leaves)
			trnode->Insert(newkey);
			
			if (!trnode->header.isleaf)
				trnode->currkey->lowernode = rightnode;

			done = trnode->header.keycount <= em;
			
			if (!done) {
			
				// ---- node is full, 
				//      try to redistribute keys among siblings
				done = trnode->Redistribute(
							trnode->header.leftsibling);

				// MDM Can't count on currnode anymore.
				NewCurrNode = true;

			}

			if (!done) {
			
				done = trnode->Redistribute(
							trnode->header.rightsibling);

				// MDM Can't count on currnode anymore.
				NewCurrNode = true;

			}

			if (done)
				break;

			// ---- cannot redistribute filled node, split it
			RootisLeaf = false;

			// MDM Can't count on currnode anymore.
			NewCurrNode = true;

			leftnode = currnode;

			// Create a new right node.
			TNode right(
				this,
				0,					// We want a new number, use 0.
				true				// Create a new node.
			);
			
			// Get the new node's number.
			rightnode = right.GetNodeNbr();

			// --- establish sibling and parent relationships 
			//     between current node and new right sibling
			right.header.rightsibling =
						trnode->header.rightsibling;
			trnode->header.rightsibling = rightnode;
			right.header.leftsibling = currnode;
			right.header.parent = trnode->header.parent;

			// MDM Added.
			ASSERT( right.header.rightsibling >= 0 );
			ASSERT( right.header.leftsibling >= 0 );
			ASSERT( right.header.parent >= 0 );
			ASSERT( trnode->header.rightsibling > 0 );	
			
			// DEBUG
			ASSERT( right.header.rightsibling < 5000 );
			ASSERT( right.header.leftsibling < 5000 );
			ASSERT( right.header.parent < 5000 );
			ASSERT( trnode->header.rightsibling < 5000 );	
			
			// ----- if the current node is a leaf, 
			//        so is the new sibling
			right.header.isleaf = trnode->header.isleaf;

			// ----- compute new key counts for the two nodes
			trnode->header.keycount = (em + 1) / 2;
			right.header.keycount = em-trnode->header.keycount;

			// MDM Added.
			ASSERT( trnode->header.keycount > 0 );
			ASSERT( right.header.keycount > 0 );
			ASSERT( trnode->header.keycount < em );
			ASSERT( right.header.keycount < em );

			// ------ locate the middle key in the current node
			PdyKey *middlekey = 
				trnode->keys.FindEntry(trnode->header.keycount);

			// ---- set the pointer to keys less than 
			//      those in new node
			if (!right.header.isleaf)
				right.header.lowernode = middlekey->lowernode;

			// ----- point to the keys to move (1 past middle)
			PdyKey *movekey = trnode->keys.NextEntry(middlekey);

			// ----- middle key inserts into parent
			trnode->keys.RemoveEntry(middlekey);
			*newkey = *middlekey;
			delete middlekey;

			// ---- move keys from current to new right node
			for (int i = 0; i < right.header.keycount; i++)	{
				PdyKey *nkey = trnode->keys.NextEntry(movekey);
				trnode->keys.RemoveEntry(movekey);
				right.keys.AppendEntry(movekey);
				movekey = nkey;
			}

			// ---- prepare to insert key 
			//      into parent of split nodes
			currnode = trnode->header.parent;
			if (!currnode)	{

				// ---- no parent node, splitting the root node
				
				// Obtain the next node number.
				// This will be used when we create the root
				// node, after exiting the key insertion loop.
				rootnode = index.DeletedNode();
				if ( !rootnode )
					rootnode = index.HighestNode() + 1;

				// Set the right and left's parent to the new 
				// root number.
				right.header.parent = rootnode;
				trnode->header.parent = rootnode;

			}

			// --- the former right sibling of the current node
			//     is now the right sibling of the split node
			//     and must record the new node as left sibling

			if (right.header.rightsibling)	{
				
				// This node exists.
				TNode farright( this, right.header.rightsibling );
				
				farright.header.leftsibling = rightnode;
				farright.MarkNodeChanged();

			}

			// --- children of the new split node point to
			//     the current split node as parent. They must
			//	    be adopted by the new split node

			if (!right.header.isleaf)
				right.Adoption();

			// ----- if splitting other than root, read parent
			//       position currkey to key where split node
			//       key will be inserted

			if (currnode)	{

				delete trnode;	// writes the split node to disk

				// --- get the parent of the split nodes
				// This node exists.
				trnode = new TNode(this, currnode);

				// -- position currkey where new key will insert
				trnode->SearchNode(newkey);
			}
		}

		if (!done)	{

			// ------ new root node ------
			if ( trnode ) delete trnode;
			
			// This will add a new root node to the db.  It will 
			// also mark the node as changed.
			trnode = new TNode(
				this, 
				0,							// We want a new node, use 0.
				true						// bCreateAsNeeded
			);

			// If we predicted the root node number above, make sure
			// we got what we were supposed to.
			ASSERT( rootnode == 0 || rootnode == trnode->GetNodeNbr() );

			// Update rootnode, in case it was 0.
			rootnode = trnode->GetNodeNbr();
			
			trnode->header.isleaf = RootisLeaf;
			
			currnode = rootnode;

			// Here, we have a new btree root node.
			// Save the new rootnode in the header.
			// Note that this is the ONLY place where
			// root nodes are created.
			WriteRootNodeNbr( &rootnode );
			
			trnode->Insert(newkey);
			trnode->header.parent = 0;
			trnode->header.keycount = 1;
			if (!RootisLeaf) {
				trnode->header.lowernode = leftnode;
				trnode->currkey->lowernode = rightnode;
			}

		}
		delete newkey;
	
	} else {

		// MDM Can't count on currnode anymore.
		NewCurrNode = true;

	}

	
	// MDM
	// The original code in effect killed our ability to traverse the tree.
	// Here, we make sure that trnode and trnode->currkey point to our
	// newly-inserted key.
	// ( Another solution is to handle a null trnode in the traversal
	//	  functions, but there, we cannot reliably determine the current
	//   object for a secondary key with duplicate entries.  Here, since we
	//	  just added the key, it will be the first one found when using 
	//   SearchNode(). )
	////////////////////////////////////////
	// ORIGINAL CODE
	////////////////////////////////////////
	
	// delete trnode;			
	// trnode = 0;
	
	////////////////////////////////////////
	// NEW CODE
	////////////////////////////////////////
	
	// Reset trnode and trnode->currkey to point to our key.
	// This ensures that NextObject, etc., do not get derailed by a SaveObject.
	
	// Note that we may not be able to rely on currnode here.  The redistribution 
	// that just occurred may have set currnode to a node that does not contain 
	// our key.  If so, re-find the key within the btree.  Otherwise, find the
	// key in the current node.
	
	// Also note that we should delete the current tree node, to save it.  This
	// is done within Find(), but must be done directly if not calling Find.
	bool Result;
	if ( NewCurrNode ) {

		// We need to completely re-find the current node and key.
		Result = Find( keypointer );
		ASSERT( Result );

	} else {

		delete trnode;
		
		// We should have a good currnode, this node exists.
		trnode = new TNode(this, currnode);

		// Make sure trnode->currkey is set to our key.
		// This may fail if three keys can't fit in a node, tripping
		// below assert.  It may also fail on a bad database.
		Result = trnode->SearchNode(keypointer);
		ASSERT( Result );

	}
		
	// Make sure we initialize trnode properly.
	oldcurrnode = 0;
	oldcurrkey =  0;
	
	////////////////////////////////////////
	
}


//-------------------------------------------------------------------//
// SaveKeyPosition()																	//
//-------------------------------------------------------------------//
void PdyBtree::SaveKeyPosition()
{
	if (trnode->header.isleaf)	{
		oldcurrnode = 0;
		oldcurrkey =  0;
	}
	else	{
		oldcurrnode = currnode;
		oldcurrkey = trnode->keys.FindEntry(trnode->currkey);
	}
}


//-------------------------------------------------------------------//
// Find()																				//
//-------------------------------------------------------------------//
// ---------------- find a key in a btree
//-------------------------------------------------------------------//
bool PdyBtree::Find(PdyKey *keypointer)
{
	oldcurrnode = 0;
	oldcurrkey =  0;

	// Start at the root node.
	// currnode = header.rootnode;
	ReadRootNodeNbr( &currnode );

	while (currnode)	{

	  	delete trnode;

		// This node exists.
		trnode = new TNode( this, currnode );

		if (trnode->SearchNode(keypointer))	{
			// ---- search key is equal to a key in the node
			keypointer->fileaddr = trnode->currkey->fileaddr;
			oldcurrnode = 0;
			oldcurrkey =  0;
			return true;
		}

		// MDM We have been dumped out here after looking at
		// the keys in the current node.  Being dumped here
		// also means there was not a key match in the current
		// node.  We branch as follows: if we are at the first
		// key in the node, our key is < all the keys in the
		// current node, so get the lower child node;  if we
		// are at a specific key within the current node,
		// indicated by a non-NULL value in currkey, we want
		// to go to the child of that key, as our key is between
		// it and the next one in the current node; if the
		// currkey is NULL, SearchNode went through all its
		// nodes, and our key is > than all of them, so we want
		// to get the lower child node of the last key in
		// the current node.
		
		if (trnode->currkey == trnode->keys.FirstEntry())	{
		
			// ---- search key is < lowest key in node
			SaveKeyPosition();
			if (trnode->header.isleaf)
				break;
			currnode = trnode->header.lowernode;
		
		} else if (trnode->currkey)	{
		
			// --- search key is < current key in node
			SaveKeyPosition();
			if (trnode->header.isleaf)
				break;
			currnode = 
				trnode->keys.PrevEntry(trnode->currkey)->
													lowernode;
		} else {
			
			// --- search key > highest key in node
			if (trnode->header.isleaf)
				break;
			currnode = trnode->keys.LastEntry()->lowernode;
		
		}
	}
	return false;
}


//-------------------------------------------------------------------//
// Delete()																				//
//-------------------------------------------------------------------//
// ---------------- delete a key from a btree
//-------------------------------------------------------------------//
void PdyBtree::Delete(PdyKey *keypointer)
{

	// Read the root node number.
	// Also save a backup.  
	// If it has changed by the end of this function, we'll save it.
	NodeNbr HeaderRN;
	NodeNbr HeaderRNBackup;
	ReadRootNodeNbr( &HeaderRN );
	HeaderRNBackup = HeaderRN;

	if ( Find( keypointer ) ) {
		if ( !trnode->header.isleaf ) {

			// --- if not found in leaf node, go down to leaf
			// This should exist, the header is not a leaf.
			ASSERT( trnode->currkey->lowernode );
			TNode* leaf = new TNode( 
				this, 
				trnode->currkey->lowernode 
			);

			while (!leaf->header.isleaf)	{
				
				NodeNbr lf = leaf->header.lowernode;
				delete leaf;
	
				// This should exist, the parent was not a leaf.
				ASSERT( lf );
				leaf = new TNode( this, lf );

			}

			// ---- Move the left-most key from the leaf
			//		to where deleted key was in higher node
			PdyKey *movekey = leaf->keys.FirstEntry();
			leaf->keys.RemoveEntry(movekey);
			leaf->header.keycount--;
			
			// -------------------------------------------------------
			//                       WARNING
			// -------------------------------------------------------
			// Here, we are working on deleting a key that had
			// kids.  To do that, we have to get the left-most
			// bottom-most child and move it into the deleted
			// key's slot.  We are assuming that we can get
			// this child key without having to reorder anything.
			// For this assumption to be true, we can't have
			// nodes with one key.  In order for this to be true,
			// we have to ensure that key splits leave at least
			// 2 keys per node.  For this to be true, we have to
			// be able to fit at least 4 keys per node, even in
			// parent nodes that contain child node data.  And in
			// fact, we make sure that this is the case in TNode's
			// constructor.
			// -------------------------------------------------------
		
			// This makes sure we haven't pulled the last key from 
			// the child.
			ASSERT( leaf->header.keycount > 0 );
			
			leaf->MarkNodeChanged();

			trnode->keys.InsertEntry(movekey, trnode->currkey);

			movekey->lowernode = trnode->currkey->lowernode;

			trnode->keys.RemoveEntry(trnode->currkey);
			delete trnode->currkey;
			trnode->MarkNodeChanged();
			delete trnode;

			trnode = leaf;
			trnode->currkey = trnode->keys.FirstEntry();
			currnode = trnode->GetNodeNbr();

		} else {
		
			// ---- delete the key from the node
			trnode->keys.RemoveEntry(trnode->currkey);
			delete trnode->currkey;
			trnode->header.keycount--;

			// Keycount should never be negative.
			ASSERT( trnode->header.keycount >= 0 );

			trnode->MarkNodeChanged();
			
			// If we have a count of zero, we have just deleted the
			// last key in the tree, and we need to clear the root node.
			if ( trnode->header.keycount == 0 )
				HeaderRN = 0;

		}
		
		// ---- if the node shrinks to half capacity,
		//      try to combine it with a sibling node
		while (
				trnode->header.keycount > 0 
			&&	trnode->header.keycount <= trnode->m()/2
		) {

			if (trnode->header.rightsibling)	{

				// This should exist.
				TNode *right = new TNode(
					this,
					trnode->header.rightsibling
				);
				
				if (trnode->Implode(*right))	{
					delete right;
					NodeNbr parent = trnode->header.parent;
					if (parent == 0)	{
						HeaderRN = trnode->GetNodeNbr();
						break;
					}
					delete trnode;

					// This node should exist.
					trnode = new TNode(this, parent);

					continue;
				}
				delete right;
			}
			if (trnode->header.leftsibling)	{
	
				// This should exist.
				TNode *left = new TNode(
					this, 
					trnode->header.leftsibling
				);

				if (left->Implode(*trnode))	{
					delete trnode;
					NodeNbr parent = left->header.parent;
					if (parent == 0)	{
						HeaderRN = left->GetNodeNbr();
						trnode = left;
						break;
					}
					delete left;
					
					// This should exist.
					trnode = new TNode( this, parent );
					
					continue;
				}
				delete left;
			}

			// --- could not combine with either sibling, 
			//     try to redistribute
			if (!trnode->Redistribute(
						trnode->header.leftsibling))
				trnode->Redistribute(
						trnode->header.rightsibling);
			break;
		}
	}

	// This in effect kills our ability to continue traversing
	// the tree.
	delete trnode;			
	trnode = 0;

	// Save the root node if it changed.
	if ( HeaderRN != HeaderRNBackup )
		WriteRootNodeNbr( &HeaderRN );
	
}


//-------------------------------------------------------------------//
// UpdateLeafStatus()																//
//-------------------------------------------------------------------//
// MDM 
// Added this function to provide service for Delete() fix.
// We deleted the last key in a node in Delete(), and now
// we need to find out if the parent has become a leaf.
//-------------------------------------------------------------------//
bool PdyBtree::UpdateLeafStatus( TNode* pTreeNode )
{

	// First set isleaf based on whether the header has children.
	pTreeNode->header.isleaf = ( pTreeNode->header.lowernode == 0 );
	
	// Get the first key in node.
	PdyKey* pKey = pTreeNode->keys.FirstEntry();
	
	// Set isleaf to false if any keys have kids.
	// Drop out of loop as soon as we find a kid.
	while ( pKey && pTreeNode->header.isleaf == true ) {
		pTreeNode->header.isleaf = ( pKey->lowernode == 0 );
		pKey = pTreeNode->keys.NextEntry();
	}

	return pTreeNode->header.isleaf;

}


//-------------------------------------------------------------------//
// Current()																			//
//-------------------------------------------------------------------//
// ------ return the address of the current key
//-------------------------------------------------------------------//
PdyKey *PdyBtree::Current()
{
	if (trnode == 0)
		return 0;
	if (oldcurrnode != 0)	{
		currnode = oldcurrnode;
		delete trnode;
   	
		// This should exist.
		trnode = new TNode(this, currnode);

		trnode->currkey = trnode->keys.FindEntry(oldcurrkey);
		oldcurrnode = 0;
		oldcurrkey =  0;
	}
	return trnode->currkey;
}


//-------------------------------------------------------------------//
// First()																				//
//-------------------------------------------------------------------//
// ------ return the address of the first key
//-------------------------------------------------------------------//
PdyKey *PdyBtree::First()
{

	// Start at the root node.
	// currnode = header.rootnode;
	ReadRootNodeNbr( &currnode );

	if (currnode)	{
	  	delete trnode;

		// This should exist.
		trnode = new TNode( this, currnode );
		
		while (!trnode->header.isleaf)	{

			currnode = trnode->header.lowernode;
			delete trnode;

			// This should exist.
			trnode = new TNode( this, currnode );

		}
		trnode->currkey = trnode->keys.FirstEntry();
	
	} else if ( trnode ) {

		// We're adding an ASSERT here until we're sure this
		// is not being hit under other circumstances.
		ASSERT(false);
	
		// The only time we hit this case is when someone has
		// "pulled the rug out" from us.  An example:
		// The only time we currently hit this case is when 
		// we have done an append FROM this database and now
		// we are reading from it.  The btrees have not yet
		// been created.  However, the BaseDatabase database object
		// does not know this; it was done "behind its back".
		// We want to clear the trnode; it is no longer valid.
		delete trnode;
		trnode = 0;
		
	}

	return Current();
}


//-------------------------------------------------------------------//
// Last()																				//
//-------------------------------------------------------------------//
// ------ return the address of the last key
//-------------------------------------------------------------------//
PdyKey *PdyBtree::Last()
{

	// Start at the root node.
	// currnode = header.rootnode;
	ReadRootNodeNbr( &currnode );

	if (currnode)	{
	  	delete trnode;

		// This should exist.
		trnode = new TNode( this, currnode );
		
		while (!trnode->header.isleaf)	{
			currnode = trnode->keys.LastEntry()->lowernode;
			delete trnode;

			// This should exist.
			trnode = new TNode( this, currnode );

		}
		trnode->currkey = trnode->keys.LastEntry();
	}
	return Current();
}


//-------------------------------------------------------------------//
// Next()																				//
//-------------------------------------------------------------------//
// ------ return the address of the next key
//-------------------------------------------------------------------//
PdyKey *PdyBtree::Next()
{
	
	// MDM Note that we have hopefully handled this situation
	// upstream, so that the traversal node is always set here.
	if (trnode == 0 || trnode->currkey == 0)
		return First();
	
	if (!trnode->header.isleaf)	{																			// it still seems as if

		// --- current key is not in a leaf
		currnode = trnode->currkey->lowernode;
	  	delete trnode;

		// This should exist.
		trnode = new TNode(this, currnode);

		// ----- go down to the leaf
		while (!trnode->header.isleaf)	{
			currnode = trnode->header.lowernode;
		  	delete trnode;

			// This should exist.
			trnode = new TNode(this, currnode);
		}
		// ---- use the first key in the leaf as the next one
		trnode->currkey = trnode->keys.FirstEntry();
	}
	else	{
		
		// ------ current key is in a leaf
		PdyKey *thiskey = nullkey->MakeKey();
		*thiskey = *(trnode->currkey);

		// ----- point to the next key in the leaf
		trnode->currkey = trnode->keys.NextEntry( trnode->currkey );

		// If we ran out of keys in the leaf,
		// we need to get a key from the parent.
		if ( trnode->currkey == 0 ) {

			// Get the root so we know when to stop
			// getting parents.
			NodeNbr HeaderRN;
			ReadRootNodeNbr( &HeaderRN );
			
			// Loop up through parents, searching for the key
			// until we find it or we run out of parents.
			while (
					trnode->currkey == 0 
				&&	currnode != HeaderRN
			) {

				// --- current key was the last one in the leaf
				// We stop looping at root, so parent should 
				// exist here.
				
				// Get the parent node.
				TNode pnode(this, trnode->Parent());

				// Search it.  We will either find it here, or
				// go past the end of the node.  If we go past
				// the end, we'll get the next parent in the 
				// next loop and search that.
				pnode.SearchNode(thiskey);
				currnode = pnode.GetNodeNbr();
				
				// Copy the parent node to trnode.
				*trnode = pnode;
			
			}

		}

		delete thiskey;

	}

	return Current();

}


//-------------------------------------------------------------------//
// Previous()																			//
//-------------------------------------------------------------------//
// ------ return the address of the previous key
//-------------------------------------------------------------------//
PdyKey *PdyBtree::Previous()
{
	if (trnode == 0 || trnode->currkey == 0)
		return Last();

	if (!trnode->header.isleaf)	{

		// --- current key is not in a leaf
		PdyKey *ky = trnode->keys.PrevEntry(trnode->currkey);
		if (ky != 0)
			currnode = ky->lowernode;
		else
			currnode = trnode->header.lowernode;
	  	delete trnode;
		
		// This should exist.
		trnode = new TNode(this, currnode);

		// ----- go down to the leaf
		while (!trnode->header.isleaf)	{
		
			currnode = trnode->keys.LastEntry()->lowernode;
		  	delete trnode;
			
			// This should exist.
			trnode = new TNode(this, currnode);

		}
		// ---- use the last key in the leaf as the next one
		trnode->currkey = trnode->keys.LastEntry();
	}
	else	{
		// ------ current key is in a leaf
		PdyKey *thiskey = nullkey->MakeKey();
		*thiskey = *(trnode->currkey);

		// ----- point to the previous key in the leaf
		trnode->currkey = 
			trnode->keys.PrevEntry(trnode->currkey);

		if ( trnode->currkey == 0 ) {

			NodeNbr HeaderRN;
			ReadRootNodeNbr( &HeaderRN );

			while (
					trnode->currkey == 0 
				&&	currnode != HeaderRN
			) {

				// --- current key was the first one in the leaf
				// Parent should exist.
				TNode pnode(this, trnode->Parent());

				pnode.SearchNode(thiskey);

				if (pnode.currkey == 0)
					pnode.currkey =	pnode.keys.LastEntry();
				else 
					pnode.currkey =	
						pnode.keys.PrevEntry(pnode.currkey);
				currnode = pnode.GetNodeNbr();
				*trnode = pnode;
			
			}
		}

		delete thiskey;
	}
	return Current();
}


//-------------------------------------------------------------------//
// Log()																					//
//-------------------------------------------------------------------//
void PdyBtree::Log()
{

	fstream logfile;
	CString LogBuffer;
	CString strFilename;
	
	// Use our two strings to build the filename.
	LogBuffer = *index.GetFilename();
	LogBuffer = LogBuffer.SpanExcluding( _T(".") );
	strFilename.Format( 
		_T("%s_%i.BTreeLog"),
		LogBuffer,
		classindexed->classid
	);
	logfile.open(
		LPCSTR( strFilename ), 
		ios::out | ios_base::trunc 
	);

	// Start log.
	LogBuffer = "BTree log initialized.\n\n";
	logfile.write(
		LPCSTR( LogBuffer ), 
		LogBuffer.GetLength()
	);

	// Get first node.
	NodeNbr	LoopNodeNbr;
	ReadRootNodeNbr( &LoopNodeNbr );

	// Get the first leaf, so we can calculate the number 
	// of tabs to the center.
	NodeNbr Child;
	TNode* pNode = new TNode( this, LoopNodeNbr );
	while ( ( Child = pNode->header.lowernode ) != 0 )
	{
		delete pNode;
		pNode = new TNode( this, Child );
	}
	
	// Divide by two to get from the right to the center.
	int nTabsToCenter = GetWidth( pNode ) / 2;
	
	// Write header.
	LogBuffer = _T("");
	for ( int i = 0; i < nTabsToCenter; i++ ) LogBuffer += _T("\t");
	LogBuffer += _T("Root node\n");
	logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );

	if ( LoopNodeNbr )
	
		LogNodeAndChildren( 
			&logfile,
			LoopNodeNbr,
			nTabsToCenter
		);

	// Done.
	logfile.close();

}


//-------------------------------------------------------------------//
// GetWidth()																			//
//-------------------------------------------------------------------//
// This gets the width from the current node to the right.
// It is used during logging.
//-------------------------------------------------------------------//
int PdyBtree::GetWidth( TNode* pNode )
{
	// Go from left to right to calc the width.
	ASSERT( pNode->header.leftsibling == 0 );
	int nWidth = 1;
	NodeNbr Right;
	while ( ( Right = pNode->header.rightsibling ) != 0 )
	{
		delete pNode;
		pNode = new TNode( this, Right );
		nWidth++;
	}

	return nWidth;

}


//-------------------------------------------------------------------//
// LogNodeAndChildren()																//
//-------------------------------------------------------------------//
void PdyBtree::LogNodeAndChildren(
	fstream* pStream,
	NodeNbr	LoopNodeNbr,
	int		nTabsToCenter
) {

	TNode* pNode = new TNode( this, LoopNodeNbr );
		
	// Calc the starting tabs using the width.
	int nStartTabs = nTabsToCenter - GetWidth( pNode ) / 2;

	// Reset (GetWidth() trashed our node).  Obviously, we're not 
	// that worried about optimizing speed here.
	delete pNode;
	pNode = new TNode( this, LoopNodeNbr );

	// Log it.
	LogNode(
		pStream,
		pNode,
		nStartTabs
	);

	// Log its siblings.
	NodeNbr Right;
	while ( ( Right = pNode->header.rightsibling ) != 0 )
	{
		delete pNode;
		pNode = new TNode( this, Right );

		LogNode(
			pStream,
			pNode,
			++nStartTabs
		);

	}

	// Separate the next Btree level.
	pStream->write(
		LPCSTR( _T("\n") ), 
		sizeof TCHAR
	);
	
	NodeNbr Child = pNode->header.lowernode;
	delete pNode;

	// Recurse through child, if needed.
	if ( Child )
	{
		// Make sure we have the far-left child.
		pNode = new TNode( this, Child );
		while ( pNode->header.leftsibling )
		{
			Child = pNode->header.leftsibling;
			delete pNode;
			pNode = new TNode( this, Child );

		}

		LogNodeAndChildren( pStream, Child, nTabsToCenter );
	}

}


//-------------------------------------------------------------------//
// LogNode()																			//
//-------------------------------------------------------------------//
void PdyBtree::LogNode( 
	fstream* pStream,
	TNode*	pNode,
	int		nTabs
) {

	int i;
	int nValue;

	
	// Init the log string.  
	CString strKeys;
	for ( i = 0; i < nTabs; i++ ) strKeys += _T("\t");
	
	// Start it off as follows:
	// [ NodeNbr:Left,Child,Right ]
	CString strNodeNbr;
	strNodeNbr.Format( 
		_T("[%i:%i,%i,%i] "), 
		pNode->GetNodeNbr(),
		pNode->LeftSibling(),
		pNode->LowerNode(),
		pNode->RightSibling()
	);
	strKeys += strNodeNbr;

	// Write our keys.
	// Note: This currently only works for int keys.
	// ObjectID keys, which we usually are using, fill 
	// this requirement.
	CString strKey;
	Key<int>* pKey = (Key<int>*) pNode->keys.FirstEntry();

	for ( i = 0; i < pNode->KeyCount(); i++ ) {

		// Get it.
		nValue = pKey->KeyValue();
		strKey.Format( _T("%d"), nValue );
		
		// Follow the keys with a comma, space; follow the
		// last one with a return.
		if ( i < pNode->KeyCount() - 1 )
			strKey += _T( ", " );
		else
			strKey += _T( "\n" );
		
		// Add it.
		strKeys += strKey;

		// Get the next key.
		pKey = (Key<int>*) pNode->keys.NextEntry();

	}

	// Write it out.
	pStream->write(
		LPCSTR( strKeys ), 
		strKeys.GetLength()
	);

}
