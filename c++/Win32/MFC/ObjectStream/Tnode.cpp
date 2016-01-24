// ------------- tnode.cpp

// ===============================
// B-tree Tnode class
// ===============================

#include "stdafx.h"

#include "BaseDatabase.h"


//-------------------------------------------------------------------//
// TNode()																				//
//-------------------------------------------------------------------//
TNode::TNode(
	PdyBtree*	bt, 
	NodeNbr		nd,
	bool			bCreateAsNeeded
) :
	
	// Call base class.
	Node(
		&(bt->GetIndexFile()), 
		nd,
		bCreateAsNeeded? NM_CREATE : NM_NORMAL
	)

{

	btree = bt;
	currkey = 0;
	NodeFile& nx = btree->GetIndexFile();

	// -------------------------------------------------------
	//                        WARNING
	// -------------------------------------------------------
	// We need at least 4 keys per node.  See TNode::Implode()
	// for details.
	// The calculation is similar to m(), except that we want
	// the result for a parent node, even if we are not a parent.
	// TO DO
	// We may want to do something more about this, such as throw
	// an exception that we catch upstream.
	ASSERT(
			( owner->GetNodeLength() - NodeHeaderSize() ) 
		/	( btree->GetKeyLength() + sizeof NodeNbr + sizeof NodeNbr ) 
		>= 4
	);
	// -------------------------------------------------------

	if ( bCreateAsNeeded ) {

		// Put the header into the buffer.
		PutToBuffer(
			&header, 
			sizeof(TNodeHeader)
		);

		return;

	}
	
	// Get the header from the buffer.
	GetFromBuffer(
		&header, 
		sizeof(TNodeHeader)
	);

	// MDM Added.
	ASSERT( header.keycount >= 0 );
	
	// DEBUG
	ASSERT( header.leftsibling < 50000 );
	ASSERT( header.rightsibling < 50000 );
	
	// ---- reading an existing node, read the keys
	for ( int i = 0; i < header.keycount; i++ ) {
	
		// ---- get memory for and read a key
		PdyKey *thiskey = btree->MakeKeyBuffer();
	
		// The key will extract itself using
		// this node, which contains a loaded
		// buffer.
		thiskey->GetFromNode( this );

		// Get the key item's node number.
		NodeNbr fa;
		GetFromBuffer( &fa, sizeof NodeNbr );
		
		thiskey->fileaddr = fa;

		if (!header.isleaf)	{
			
			NodeNbr lnode;
			GetFromBuffer( &lnode, sizeof(NodeNbr) );
			thiskey->lowernode = lnode;

		}
		keys.AppendEntry(thiskey);
	}
}


//-------------------------------------------------------------------//
// PutBTreeKeyToBuffer()															//
//-------------------------------------------------------------------//
// This function puts the key in the buffer.
//-------------------------------------------------------------------//
void TNode::PutBTreeKeyToBuffer( PdyKey* thiskey )
{

	// Put the key value.
	thiskey->PutToNode( this );

	// Put the key's file address.
	NodeNbr fa = thiskey->fileaddr;
	PutToBuffer( &fa, sizeof(NodeNbr) );
	
	// Put the lower node pointer for non-leaf keys.
	if ( !header.isleaf ) {
		NodeNbr lnode = thiskey->lowernode;
		PutToBuffer( &lnode, sizeof(NodeNbr) );
	}

}


//-------------------------------------------------------------------//
// ~TNode()																				//
//-------------------------------------------------------------------//
TNode::~TNode()
{

	// Just delete the node if keycount has 
	// dropped to zero.
	if (header.keycount == 0)
		deletenode = true;
	
	else {
		
		ASSERT( header.keycount > 0 );
		
		// DEBUG
		ASSERT( header.leftsibling < 50000 );
		ASSERT( header.rightsibling < 50000 );

		// If the node changed, we want to fill the node buffer 
		// with the new key contents.  Then the buffer will be 
		// saved in the base class.
		if ( nodechanged ) {

			// Position for putting right after the base
			// class header.
			BufferPosition = Node::NodeHeaderSize();

			// Put the TNode header.
			PutToBuffer( &header, sizeof(TNodeHeader) );

			// Put and delete the keys.
			PdyKey *thiskey = keys.FirstEntry();
			while ( thiskey != 0 ) {

				PutBTreeKeyToBuffer( thiskey );
				delete thiskey;
				thiskey = keys.NextEntry();

			}

		} else {
		
			// No changes, no need to fill the node buffer.
			// Just delete the keys.
			PdyKey *thiskey = keys.FirstEntry();
			while (thiskey != 0)	{
				delete thiskey;
				thiskey = keys.NextEntry();
			}
			
		}
	}
}


//-------------------------------------------------------------------//
// operator=()																			//
//-------------------------------------------------------------------//
// ------- assignment operator
//-------------------------------------------------------------------//
TNode& TNode::operator=(TNode& tnode)
{
	PdyKey *thiskey = keys.FirstEntry();
	// ---- if receiver has any keys, delete them
	while (header.keycount > 0)	{
		delete thiskey;
		--header.keycount;
		thiskey = keys.NextEntry();
	}
	keys.ClearList();
	Node::operator=(tnode);
	header = tnode.header;
	currkey = 0;
	// ------- copy the keys
	thiskey = tnode.keys.FirstEntry();
	while (thiskey != 0)	{
		PdyKey *newkey = btree->MakeKeyBuffer();
		*newkey = *thiskey;
		keys.AppendEntry(newkey);
		if (thiskey == tnode.currkey)
			currkey = newkey;
		thiskey = tnode.keys.NextEntry();
	}
	return *this;

}


//-------------------------------------------------------------------//
// m()																					//
//-------------------------------------------------------------------//
// -------- compute m value of node
//-------------------------------------------------------------------//
short int TNode::m()
{
	
	int keyspace = owner->GetNodeLength() - NodeHeaderSize();
	
	////////////////////////////////////////////////////
	// ERRATA, as per Al Stevens - thank you!
	// int keylen = btree->GetKeyLength();
	int keylen = btree->GetKeyLength() + sizeof( NodeNbr );
	////////////////////////////////////////////////////
	
	if (!header.isleaf)
		keylen += sizeof(NodeNbr);
	return keyspace / keylen;

}


//-------------------------------------------------------------------//
// SearchNode()																		//
//-------------------------------------------------------------------//
// ---------- search a node for a match on a key
// MDM This routine searches the current node for our key.
// It searches until our key value is found or exceeded.
// If a match is found, it returns true; otherwise, false.
// The currkey will point to the match or the first key
// that exceeded ours.
// See PdyBtree::Find() to see how to handle the result.
//-------------------------------------------------------------------//
bool TNode::SearchNode(PdyKey *keyvalue)
{
	
	// MDM Added.
	ASSERT( keyvalue != 0 );
	
	currkey = keys.FirstEntry();
	while (currkey != 0)	{
		
		// MDM Here, we are relying on an overridden > operator.
		// See key.h for > overrides and related problems 
		// with the MS template implementation.
		if (*currkey > *keyvalue)
			break;
		if (*currkey == *keyvalue)	{
			if (keyvalue->indexno == 0)
				return true;
			if (currkey->fileaddr == keyvalue->fileaddr)
				return true;
			if (keyvalue->fileaddr == 0)
				return true;
			if (currkey->fileaddr > keyvalue->fileaddr)
				break;
		}
		currkey = keys.NextEntry();
	}
	return false;
}


//-------------------------------------------------------------------//
// Insert()																				//
//-------------------------------------------------------------------//
void TNode::Insert(PdyKey *keyvalue)
{
	// -------- insert the new key
	PdyKey *ky = keyvalue->MakeKey();
	*ky = *keyvalue;
	if (currkey == 0)
		keys.AppendEntry(ky);
	else 
		keys.InsertEntry(ky, currkey);
	header.keycount++;
	nodechanged = true;
	currkey = ky;
}


//-------------------------------------------------------------------//
// Adoption()																			//
//-------------------------------------------------------------------//
// ---- a node "adopts" all its children by telling 
//      them to point to it as their parent
//-------------------------------------------------------------------//
void TNode::Adoption()
{
	Adopt(header.lowernode);
	PdyKey *thiskey = keys.FirstEntry();
	for (int i = 0; i < header.keycount; i++)	{
		Adopt(thiskey->lowernode);
		thiskey = keys.NextEntry();
	}
}


//-------------------------------------------------------------------//
// Adopt()																				//
//-------------------------------------------------------------------//
// --- adopt a child node
//-------------------------------------------------------------------//
void TNode::Adopt(NodeNbr node)
{
	if (node)	{

		// Child exists.
		TNode nd(btree, node);

		nd.header.parent = nodenbr;
		nd.nodechanged = true;
	}
}


//-------------------------------------------------------------------//
// Redistribute()																		//
//-------------------------------------------------------------------//
// ---- redistribute keys among two sibling nodes
//-------------------------------------------------------------------//
bool TNode::Redistribute(NodeNbr sib)
{
	
	if (sib == 0)
		return false;

	// Sibling exists.
	TNode sibling(btree, sib);
	
	// Should we ever hit this?
	if (sibling.header.parent != header.parent)
		return false;
	
	int totkeys = header.keycount + sibling.header.keycount;

	// See if we can fit all the keys in these two nodes.
	// Allow an exact fit!
	// if (totkeys >= m() * 2)
	if (totkeys > m() * 2)
		return false;
	
	// ---- assign left and right associations
	TNode *left, *right;
	if (sib == header.leftsibling)	{
		right = this;
		left = &sibling;
	}
	else	{
		right = &sibling;
		left = this;
	}
	// ------- compute number of keys to be in left node
	int leftct =
		(left->header.keycount + right->header.keycount) / 2;
	// ------- if no redistribution would occur
	if (leftct == left->header.keycount)
		return false;
	// ------- compute number of keys to be in right node
	int rightct =
		(left->header.keycount+right->header.keycount)-leftct;
	
	// MDM Added.
	ASSERT( rightct > 0 );
	ASSERT( leftct > 0 );
	
	// ------- get the parent
	// Parent should exist.
	TNode parent(btree, left->header.parent);
	
	// --- position parent's currkey 
	//     to one that points to siblings
	parent.SearchNode(left->keys.FirstEntry());
	
	// ----- will move keys from left to right or right to
	//		 left depending on which node has the greater
	//		 number of keys to start with.
	if (left->header.keycount < right->header.keycount)	{
		// ----- moving keys from right to left
		int mvkeys = right->header.keycount - rightct - 1;
		// ----- move key from parent to end of left node
		left->currkey = parent.currkey;
		parent.currkey = parent.keys.NextEntry(parent.currkey);
		// ---- remove parent key from its list
		parent.keys.RemoveEntry(left->currkey);
		// ---- put it in left node's list
		left->keys.AppendEntry(left->currkey);

		if (!left->header.isleaf)
			left->currkey->lowernode = right->header.lowernode;
		// --- point to the keys to move 
		//     (at front of right node)
		PdyKey *movekey = right->keys.FirstEntry();
		// ---- move keys from right to left node
		for (int i = 0; i < mvkeys; i++)	{
			PdyKey *nkey = right->keys.NextEntry(movekey);
			right->keys.RemoveEntry(movekey);
			left->keys.AppendEntry(movekey);
			movekey = nkey;
		}
		// --- move separating key from right node to parent
		right->keys.RemoveEntry(movekey);
		
		////////////////////////////////////////////////////
		// ERRATA, as per Al Stevens - thank you!
		// parent.keys.InsertEntry(movekey, parent.currkey);
		if ( parent.currkey == 0 )
			parent.keys.AppendEntry( movekey );
		else
			parent.keys.InsertEntry(movekey, parent.currkey);
		////////////////////////////////////////////////////
		
		if (!right->header.isleaf)
			right->header.lowernode = movekey->lowernode;
		movekey->lowernode = right->nodenbr;
		right->header.keycount = rightct;
		left->header.keycount  = leftct;

		// MDM Added.
		ASSERT( rightct > 0 );
		ASSERT( leftct > 0 );

		if (!left->header.isleaf)
			left->Adoption();
	}
	else 	{
		// -------- moving from left to right
		int mvkeys = left->header.keycount - leftct - 1;
		// ----- move key from parent to right node
		right->currkey = parent.currkey;
		parent.currkey = parent.keys.NextEntry(parent.currkey);
		// --- remove parent key from its list
		parent.keys.RemoveEntry(right->currkey);
		// ---- put it in right node's list
		right->keys.InsertEntry(right->currkey, 
							right->keys.FirstEntry());
		if (!right->header.isleaf)
			right->currkey->lowernode=right->header.lowernode;
		// ----- locate the first key to move in the left node
		PdyKey *movekey = left->keys.FindEntry(leftct);
		// ---- remember the key after the one being moved up
		PdyKey *nkey = left->keys.NextEntry(movekey);
		// ----- move key from left node up to parent
		left->keys.RemoveEntry(movekey);
		
		////////////////////////////////////////////////////
		// ERRATA, as per Al Stevens - thank you!
		// parent.keys.InsertEntry(movekey, parent.currkey);
		if ( parent.currkey == 0 )
			parent.keys.AppendEntry( movekey );
		else
			parent.keys.InsertEntry(movekey, parent.currkey);
		////////////////////////////////////////////////////
		
		right->header.lowernode = movekey->lowernode;
		movekey->lowernode = right->nodenbr;
		movekey = nkey;
		// --- move keys from the left node to the right node
		PdyKey *inskey = right->keys.FirstEntry();
		for (int i = 0; i < mvkeys; i++)	{
			PdyKey *nkey = left->keys.NextEntry(movekey);
			left->keys.RemoveEntry(movekey);
			right->keys.InsertEntry(movekey, inskey);
			movekey = nkey;
		}

		right->header.keycount = rightct;
		left->header.keycount  = leftct;
		
		// MDM Added.
		ASSERT( rightct > 0 );
		ASSERT( leftct > 0 );

		if (!right->header.isleaf)
			right->Adoption();
	}
	nodechanged =
	sibling.nodechanged =
	parent.nodechanged = true;
	return true;
}


//-------------------------------------------------------------------//
// Implode()																			//
//-------------------------------------------------------------------//
// ------ implode the keys of two sibling nodes
//-------------------------------------------------------------------//
bool TNode::Implode(TNode& right)
{
	int totkeys = right.header.keycount+header.keycount;
	if (totkeys >= m() ||
			right.header.parent != header.parent)
		return false;
	nodechanged = right.nodechanged = true;
	header.rightsibling = right.header.rightsibling;
	header.keycount += right.header.keycount+1;
	right.header.keycount = 0;

	// ---- get the parent of the imploding nodes
	// Parent should exist.
	TNode parent(btree, header.parent);

	parent.nodechanged = true;
	// ---- position parent's currkey to 
	//      key that points to siblings
	parent.SearchNode(keys.FirstEntry());
	// ---- move the parent's key to the left sibling
	parent.keys.RemoveEntry(parent.currkey);
	keys.AppendEntry(parent.currkey);
	parent.currkey->lowernode = right.header.lowernode;
	parent.header.keycount--;

	ASSERT( parent.header.keycount >= 0 );

	// We have pulled the last key from the parent.
	// It previously only had one key in it.  This
	// must be the root.  
	// -------------------------------------------------------
	//                        WARNING
	// -------------------------------------------------------
	// Note that for this assumption to be true, node splitting
	// must result in at least 2 nodes in each new node.  For
	// this to be true, nodes must always be able to hold four 
	// keys, even parent nodes that must contain child data.
	// The nodelength must be sufficiently long to accommodate
	// this.  See TNode constructor for the code that checks
	// for this condition.
	// -------------------------------------------------------
	if (parent.header.keycount == 0)
		// -- combined the last two leaf nodes into a new root
		header.parent = 0;
	
	// -- move the keys from the right sibling into the left
	PdyKey *movekey = right.keys.FirstEntry();
	while (movekey != 0)	{
		PdyKey *nkey = right.keys.NextEntry(movekey);
		right.keys.RemoveEntry(movekey);
		keys.AppendEntry(movekey);
		movekey = nkey;
	}
	if (header.rightsibling)	{
		// - point right sibling of old right to imploded node
		// Right sibling should exist.
		TNode farright(btree, header.rightsibling);
		
		farright.header.leftsibling = GetNodeNbr();
		farright.nodechanged = true;
	}
	Adoption();
	return true;

}



