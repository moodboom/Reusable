#include "stdafx.h"

#include "BaseDatabase.h"


//-------------------------------------------------------------------//
// Node()																				//
//-------------------------------------------------------------------//
// ----------- construct a new node
// The following modes are available when constructing a node object:
//
//									Purpose
//									-------------------------------------------
//		1) NM_NORMAL			Read an existing node
//		2) NM_CREATE			Read an existing node (or create a new node if NewNodeNbr == 0)
//		3) NM_DELETE			Delete an existing node
//		4) NM_SCAN				Scan through the node, just getting the requested number of bytes
//
// When using NM_CREATE with a valid node, we want to make sure to preserve 
// the existing next node so we do not lose the old nodes in the object chain.
// But we don't want to waste time reading the remaining old node data.
//-------------------------------------------------------------------//
Node::Node(
	NodeFile*	hd, 
	NodeNbr		NewNodeNbr,
	NODEMODE		eMode,
	uShort		nBytes
) {
	
	nextnode = 0;
	nodechanged = ( eMode == NM_CREATE );	// Always save bCreateAsNeeded nodes.
	deletenode = ( eMode == NM_DELETE );	// Set delete status.
	nodenbr = NewNodeNbr;						// May be zero for temp or new nodes.
	owner = hd;
	pBuffer = 0;

	// We should either have a node or be ready to create one.
	if ( nodenbr || nodechanged ) {
	
		// For deleted nodes, we only want to read the next node number, and we are done.
		if ( deletenode ) {
		
			// Get the next node number.
			NewBuffer( sizeof nextnode );
			ReadBuffer( NodeAddress() );
			GetFromBuffer( &nextnode, sizeof nextnode );

		} else if ( eMode == NM_SCAN ) {

			// Get the requested number of bytes.
			// Note that we assume responsibility for the size of the next node;
			// the caller should not include it in nBytes.
			// Also, nBytes may be zero if only the next node is requested.
			ASSERT( nBytes < owner->GetNodeLength() - sizeof nextnode );
			NewBuffer( nBytes + sizeof nextnode );
			ReadBuffer( NodeAddress() );
			GetFromBuffer( &nextnode, sizeof nextnode );

		} else {
	
			// If we are creating...
			if ( nodechanged ) {

				// If we don't have a nodenbr, we need to get
				// one.  We first attempt to get a deleted 
				// node.  If not available, then we	append a 
				// new one to the database.  
				if ( !nodenbr ) {

					if ( owner->DeletedNode() ) {
					
						// We can appropriate a deleted node.
						nodenbr = owner->DeletedNode();
						
						// Now we need to clean up.  We'll need to
						// set the new deleted head to equal our
						// our next node.  Then we set our next node 
						// to zero, since we're new.
						
						// Get the previously deleted node's next node.
						NewBuffer( sizeof nextnode );
						ReadBuffer( NodeAddress() );
						GetFromBuffer( &nextnode, sizeof nextnode );
						
						// Update the node file header and our next node.
						owner->SetDeletedNode( nextnode );
						nextnode = 0;

					} else {

						nodenbr = owner->HighestNode() + 1;
						owner->SetHighestNode( nodenbr );

					}

					// Allocate the node buffer.
					NewBuffer( owner->GetNodeLength() );

					// Put nextnode header (nextnode = 0).
					PutToBuffer( &nextnode, sizeof nextnode );
				
				// If we HAVE a node number, we are rewriting a new object
				// over old nodes.  Make sure we preserve the next node, so it does
				// not get permanently lost.  However, do not waste time reading the 
				// old node contents.
				} else {
				
					NewBuffer( sizeof nextnode );
					ReadBuffer( NodeAddress() );
					GetFromBuffer( &nextnode, sizeof nextnode );

					// Allocate a full node buffer and set 
					// the next node.
					NewBuffer( owner->GetNodeLength() );
					PutToBuffer( &nextnode, sizeof nextnode );

				}

			// else we have a node to read...
			} else {

				// Allocate the node buffer.
				NewBuffer( owner->GetNodeLength() );

				// Read it.
				// TO DO
				// Handle exceptions from this (in parent).
				ReadBuffer( NodeAddress() );

				// Get nextnode header.
				GetFromBuffer( &nextnode, sizeof nextnode );

			}

		}

	} else {
	
		// A node number of zero was passed!
		// TO DO 
		// Identify and handle this exception.
		throw;

	}

}


//-------------------------------------------------------------------//
// NewBuffer																			//
//-------------------------------------------------------------------//
// This allocates a node buffer of the given size.  Usually, nodes
// get a full nodelength-sized buffer (unless we are just reading
// header info, for example).  The buffer may be reallocated with
// this function; the previous buffer will be deleted as needed.
//-------------------------------------------------------------------//
void Node::NewBuffer( uShort NewBufferLength )
{
	
	// Delete code uses buffer twice in one instance.
	// Clean up first.
	if ( pBuffer )
		delete pBuffer;

	// Save the buffer length and set position to 0.
	BufferLength = NewBufferLength;
	BufferPosition = 0;
	
	// Allocate it.
	pBuffer = new uByte[ BufferLength ];

}


//-------------------------------------------------------------------//
// NodeAddress()																		//
//-------------------------------------------------------------------//
// ---------- compute the disk address of a node
//-------------------------------------------------------------------//
long Node::NodeAddress()
{

	long adr = nodenbr-1;

	adr *= owner->GetNodeLength();

	adr += sizeof(FileHeader);

	adr += (long) owner->nExtendedHeaderSize;

	return adr;

}


//-------------------------------------------------------------------//
// operator=()																			//
//-------------------------------------------------------------------//
// ------- assignment operator
//-------------------------------------------------------------------//
Node& Node::operator=(Node& node)
{

	// This is only used to copy from a node that has already been
	// read and initialized.  If it is a TNode, the key data has
	// already been initialized, and will be copied in 
	// TNode::operator= using the key arrays.  Therefore, here,
	// we don't need to worry about copying the read buffer.  Just
	// close it.
	CloseNode();

	nextnode = node.nextnode;
	owner = node.owner;
	nodenbr = node.nodenbr;
	nodechanged = node.nodechanged;
	deletenode = node.deletenode;
	return *this;

}	


//-------------------------------------------------------------------//
// ClearRemainingBuffer()															//
//-------------------------------------------------------------------//
void Node::ClearRemainingBuffer()
{

	// If the buffer was not filled all the way,
	// stuff it with 0.
	if ( BufferPosition < BufferLength )
		memset( pBuffer + BufferPosition, 0, BufferLength - BufferPosition );

}


//-------------------------------------------------------------------//
// ~Node()																				//
//-------------------------------------------------------------------//
// ------- destroy the node
//-------------------------------------------------------------------//
Node::~Node()
{
	CloseNode();
}


//-------------------------------------------------------------------//
// CloseNode()																			//
//-------------------------------------------------------------------//
void Node::CloseNode()
{

	// If we are deleting...
	if ( deletenode ) {
	
		// Move node to deleted node list, and update
		// the node file header to reflect this.
		nextnode = owner->DeletedNode();
		owner->SetDeletedNode(nodenbr);

		// Now we need to write out the new node header
		// and the ClassID that marks the node as deleted.
		NewBuffer( sizeof nextnode + sizeof ClassID );
		PutToBuffer( &nextnode, sizeof nextnode );
		*( (ClassID*) ( pBuffer + BufferPosition ) ) = -1;
		WriteBuffer( NodeAddress() );

	// Write out changes.
	} else if ( owner && nodenbr && nodechanged ) {

		// Update nextnode, which is first in the buffer.
		// Note that the rest of the buffer has been updated 
		// before this point, either by a derived class or
		// externally.
		*( (NodeNbr*) pBuffer ) = nextnode;

		WriteBuffer( NodeAddress() );

	}

	// We can now delete the buffer.
	// ( We should always have one. )
	if ( pBuffer ) {
		delete pBuffer;
		pBuffer = 0;
	}

}
