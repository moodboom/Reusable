#ifndef INDEX_FILE_H
	#define INDEX_FILE_H


#include "Node.h"

// ============================
// IndexFile class
// ============================
class IndexFile : public NodeFile	{
public:
	
	IndexFile(
		const string&	NewFilename
	) :
		
		NodeFile( NewFilename )

	{
	}

};


#endif	// INDEX_FILE_H