// TNode class interface

#ifndef TNODE_H
	#define TNODE_H


// ------------- b-tree TNode class
class TNode : public Node	{

	friend class PdyBtree;
	struct TNodeHeader	{

										//													Bytes	Aligment
		bool isleaf;				// true if node is a leaf					1		1	
		short int keycount;		// number of keys in this node			2		2

		NodeNbr parent;			// parent to this node						4		4
		NodeNbr leftsibling;		// left sibling node							4		8
		NodeNbr rightsibling;	// right sibling node						4		12
		NodeNbr lowernode;		// lower node associated with				4		16
										//		keys < keys in this node

		TNodeHeader()
			{ isleaf = false; parent = leftsibling =
				rightsibling =	keycount = lowernode = 0; }
	} header;
	PdyKey *currkey;				// current key
	PdyBtree *btree;				// btree that owns this node
	LinkedList<PdyKey> keys;	// the keys in this node

	// constructor
	TNode(
		PdyBtree*	bt, 
		NodeNbr		node,
		bool			bCreateAsNeeded	= false
	);

	bool SearchNode(PdyKey *keyvalue);
	void Insert(PdyKey *keyvalue);
	short int m();
	
	void PutBTreeKeyToBuffer( PdyKey* thiskey );	

	void Adopt(NodeNbr node);
	void Adoption();
	bool isLeaf() const
		{ return header.isleaf; }
	NodeNbr Parent() const
		{ return header.parent; }
	NodeNbr LeftSibling() const
		{ return header.leftsibling; }
	NodeNbr RightSibling() const
		{ return header.rightsibling; }
	short int KeyCount() const
		{ return header.keycount; }
	NodeNbr LowerNode() const
		{ return header.lowernode; }
	bool Redistribute(NodeNbr sib);
	bool Implode(TNode& right);
	short int NodeHeaderSize() const
		{ return sizeof(TNodeHeader)+Node::NodeHeaderSize(); }
	TNode& operator=(TNode& tnode);
public:			// due to a bug in Borland C++ 4.0
	~TNode();
};


#endif	// TNODE_H
