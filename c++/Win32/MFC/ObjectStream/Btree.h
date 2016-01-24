// ------------- btree.h

#ifndef BTREE_H
#define BTREE_H

#include <fstream>
#include <string>

#include "linklist.h"
#include "node.h"
#include "NodeFile.h"		// For index file access.

class PdyKey;
class TNode;
class Class;

const int classnamesize = 16;

// --------- b-tree index
class PdyBtree	{

// MDM Made this member public, Persistent::GenerateID() accesses it.
public:
	Class* classindexed;  // -> class structure of indexed class

private:
	TNode *trnode;				// -> current node value
	PdyKey *nullkey;			// for building empty derived key
	NodeFile& index;			// index file this tree lives in
	IndexNo indexno;			// 0=primary key, > 0=secondary key
	NodeNbr currnode;			// current node number
	NodeNbr oldcurrnode;		// for repositioning
	short oldcurrkey;			//  "        "

	
	// ------------------------------------------------------------------
	// HEADER ACCESS
	//
	// Btree headers are composed of the following:
	//
	//		NodeNbr		rootnode
	//		KeyLength	keylength
	//
	// The keylength is stored in a member variable; its
	// value is set in the constructor and will not change.
	// The rootnode may change during write operations; it
	// is loaded into a local var at the beginning of functions
	// that need it, and then saved at the end, when it's 
	// final value is known.  This minimizes writes.
	
	KeyLength keylength;		// We store the key length here (it's static)
	
	void ReadRootNodeNbr( NodeNbr* pRoot )
	{
		index.ReadData(
			pRoot,
			sizeof NodeNbr,
			HdrPos()												// root is first
		);
	}
	
	void WriteRootNodeNbr( NodeNbr* pRoot )
	{
		index.WriteData(
			pRoot,
			sizeof NodeNbr,
			HdrPos()												// root is first
		);
	}

	void ReadKeyLength()
	{	
		index.ReadData(
			&keylength,
			sizeof keylength,
			HdrPos() + (streampos) sizeof ( NodeNbr )	// follows root
		);
	}

	void WriteKeyLength()
	{	
		index.WriteData(
			&keylength,
			sizeof keylength,
			HdrPos() + (streampos) sizeof ( NodeNbr )	// follows root
		);
	}

	streampos HdrPos()
	{ 
		
		// See Persistent::GetHighestObjectID() for class header details.
		return (
			
			classindexed->headeraddr +

			// This accounts for the highest ObjectID, which
			// is located at the very beginning of the header.
			(streampos) ( sizeof ObjectID ) +

			// This bool was originally for cache status, but is now
			// reserved for future needs.  It follows HighestObjectID.
			(streampos) ( sizeof (bool) ) +
			
			// This accounts for all the previous key headers.  
			(streampos) ( 
				indexno * ( 
						sizeof NodeNbr				// root
					+	sizeof KeyLength			// keylength
				)
			)
		); 
	}

	//
	// ------------------------------------------------------------------
	
	
	void SaveKeyPosition();

	// MDM Delete() helper.
	bool UpdateLeafStatus( TNode* pTreeNode );

public:
	
	PdyBtree(
		NodeFile&	ndx,			// Index file
		Class*		cls,			// Class
		PdyKey*		ky				// Key
	)
		throw (BadKeylength);;

	~PdyBtree();
	void Insert(PdyKey *keypointer);
	void Delete(PdyKey *keypointer);
	bool Find(PdyKey *keypointer);
	PdyKey *Current();
	PdyKey *First();
	PdyKey *Last();
	PdyKey *Next();
	PdyKey *Previous();
	NodeFile& GetIndexFile() const
		{ return index; }
	PdyKey *NullKey() const
		{ return nullkey; }
	PdyKey *MakeKeyBuffer() const;
	NodeNbr GetKeyLength() const
		{ return keylength; }
	IndexNo Indexno() const
		{ return indexno; }
	const Class *ClassIndexed() const
		{ return classindexed; }
	void SetClassIndexed(Class *cid)
		{ classindexed = cid; }


	//////////////////////////////////////////
	// LOGGING FUNCTIONS
	//

	void Log();

protected:

	void LogNodeAndChildren(
		fstream* pStream,
		NodeNbr	LoopNodeNbr,
		int		nTabs
	);

	void LogNode( 
		fstream* pStream,
		TNode*	pNode,
		int		nTabs
	);

	int GetWidth( TNode* pNode );

	//
	//////////////////////////////////////////

};

#endif


