// ------------- node.h

#ifndef NODE_H
	#define NODE_H

#include "NodeFile.h"


// See constructor function in *.cpp for details.
typedef enum {
	NM_NORMAL,
	NM_CREATE,
	NM_DELETE,
	NM_SCAN
} NODEMODE;


class Node	{

	NodeNbr nextnode;
	inline void CloseNode();

public:
	
	uShort		BufferPosition;		// Used by Persistent during object load.
	uByte*		pBuffer;					// Used by BaseDatabase during en/decryption.

protected:

	uShort		BufferLength;

	NodeFile*	owner;
	NodeNbr		nodenbr;			// current node number
	bool			nodechanged;	// true if the node changed
	bool			deletenode;		// true if the node is being deleted

	void NewBuffer( uShort NewBufferLength );

	void ReadBuffer( long NodeAddress )
	{	owner->ReadData( pBuffer, BufferLength, NodeAddress );	}

	void WriteBuffer( long NodeAddress )
	{	owner->WriteData( pBuffer, BufferLength, NodeAddress );	}

public:
	
	// See constructor function in *.cpp for details.
	Node(
		NodeFile*	hd			= 0, 
		NodeNbr		node		= 0,
		NODEMODE		eMode		= NM_NORMAL,
		uShort		nBytes	= 0
	);

	virtual ~Node();
	Node& operator=(Node& node);
	void SetNextNode(NodeNbr node)
		{ nextnode = node; MarkNodeChanged(); }
	NodeNbr NextNode() const
		{ return nextnode; }
	void SetNodeNbr(NodeNbr node)
		{ nodenbr = node; }
	NodeNbr GetNodeNbr() const
		{ return nodenbr; }
	void MarkNodeDeleted()
		{ deletenode = true; }
	void MarkNodeChanged()
		{ nodechanged = true; }
	bool NodeChanged() const
		{ return nodechanged; }
	long NodeAddress();
	virtual short int NodeHeaderSize() const
		{ return sizeof(NodeNbr); }

	void GetFromBuffer( void* pOut, unsigned int OutLength )
	{
		ASSERT( BufferPosition + OutLength <= BufferLength );
		memcpy( pOut, pBuffer + BufferPosition, OutLength );
		BufferPosition += OutLength;
	}	

	void PutToBuffer( const void* pIn, unsigned int InLength )
	{
		ASSERT( BufferPosition + InLength <= BufferLength );
		memcpy( pBuffer + BufferPosition, pIn, InLength );
		BufferPosition += InLength;
	}

	void SetBufferPosition( uShort NewPosition )
	{	BufferPosition = NewPosition;		}

	void ClearRemainingBuffer();

};

#endif // NODE_H

