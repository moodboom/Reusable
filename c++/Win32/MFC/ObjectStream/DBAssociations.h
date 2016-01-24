#ifndef DB_ASSOCIATIONS_H
	#define DB_ASSOCIATIONS_H

#include "ObjectStream.h"						// Base class
#include "..\BaseProgressDlg.h"			// Extract() uses one.

class EMComponent;

typedef struct {
	DatabaseID		DBID;									//	 16	
	wchar_t			UserName[nUserNameLength];		//  64
	wchar_t			Company[nCompanyLength];		//  64
	wchar_t			Contact[nContactLength];		//  128
	unsigned int	nPercentage;
	wchar_t			DBName[43];					//  86
} DBAssociation;

typedef CTypedPtrArray< CPtrArray, DBAssociation* > ASSOCIATIONS;

// These are the return codes for Extract().
#define	DBA_SUCCESS					0x0000
#define	DBA_ASSOC_NOT_FOUND		0x0001
#define	DBA_COMPLETENESS_FAILED	0x0002

class DBAssociations : public Persistent
{

public:

	// Constructor.
	DBAssociations(
		ObjectStream*				pNewDB,
		bool					bNewTemp			= false
	);

	~DBAssociations();

	bool Extract(
		BaseProgressDlg*	pProgressDlg,
		int					nStartProgress			= 0,
		int					nEndProgress			= 100
	);

	// These functions break the process of compiling associations
	// into functions that can be called by an external loop through
	// all db objects.  This allows callers to loop through all 
	// objects, compiling associations, while also able to perform
	// other operations on each object ( see EMDatabase::Publish() for
	// the best example ).
	void Start();
	bool AddAssociations( 
		EMComponent*	pObject
	);
	bool Finish();

	// Our array of association data.
	// Once this is loaded, we want to have access to it.
	ASSOCIATIONS Associations;

protected:

	// These read/write AND fill/cache the tree data.
	void Read();
	void Write();

	void Clear();

	// This var indicates if this is a temp object.  If so, it is
	// just being used for class registration, and nothing is done.
	// If not, we set up for either a stand-alone associations update
	// with Extract(), or an object-loop associations update with
	// AddAssociations(), by calling Start() and Finish() in the 
	// con/destructor.
	bool bTemp;

	// These var's are required for us to keep temporary results
	// during scans for associations across objects.
	unsigned int nRefCount;
	CString strAssocNotFound;
	int nCurrentAssocCount;
	int nBadObjectCount;	
	bool bSuccess;

};


#endif		// DB_ASSOCIATIONS_H
