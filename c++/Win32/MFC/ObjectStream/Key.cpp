// ------------- key.cpp

// MDM Must always be included.
// Note that the include file is actually "..\stdafx.h", but because
// we PRECOMPILE stdafx.h, MS does not allow the correct path to be
// used.   Without the path, it works just fine.
#include "stdafx.h"

#include "BaseDatabase.h"

// ============================================
// base PdyKey class member functions
// ============================================

PdyKey::PdyKey(NodeNbr fa)
{
	fileaddr = fa;
	lowernode = 0;
	indexno = 0;
	relatedclass = 0;

	// This weird hack is Al's way of indicating whether we
	// are creating a persistent class's base key or just
	// a key for use by a btree.
	if ( Persistent::pTargetObject != 0)	{
		// --- register the key with the object being built
		Persistent::pTargetObject->RegisterKey(this);
		// --- assign index number based on position in object
		indexno = Persistent::pTargetObject->indexcount++;
	}
}

// ------ overloaded =
PdyKey& PdyKey::operator=(const PdyKey& key)
{
	if (this != &key)	{
		fileaddr = key.fileaddr;
		lowernode = key.lowernode;
		indexno = key.indexno;
		keylength = key.keylength;
		relatedclass = key.relatedclass;
	}
	return *this;
}
