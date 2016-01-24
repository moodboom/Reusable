#ifndef CLASS_H
	#define CLASS_H

#include <fstream>					// For streampos
using namespace std;


// ============================
// Class Identification
// ============================
typedef short int ClassID;

// =====================================
// Class Identification structure
// =====================================

// MDM referred to as a class elsewhere.
// struct Class {
class Class {

	// MDM Originally a struct.
	// Make everything accessible.
	public:

		char *classname;

		ClassID classid;
		
		streampos headeraddr;
		
		Class(char *cn = 0) :

			classname(cn), 
			classid(0), 
			headeraddr(0)
			
		{ /* ... */ }

};


#endif	// CLASS_H
