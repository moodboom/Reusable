
#include "stdafx.h"						// Required for precompiled header

#include "EMDatabase.h"					// For GetClassID() - Note that DBAssociations is a Persistent,
												// not EMComponent, object.

#include "..\UnicodeConversions.h"	// For CString->wstring.

#include "..\DisplayMessage.h"		// We report errors here.

#include "..\EMComponentInit.h"		// For access to subobject size.
#include "..\EMComponent.h"			// Forward referenced in DBAssociations.h.

#include "EMDBArray.h"					// Needed for db sort category indexes.

#include "..\RefScanData.h"			// We scan refs for associations, of course.

#include "DBAssociations.h"


//-------------------------------------------------------------------//
// constructor																			//
// Note that there should only be one of this object per database.
// Instead of specifying an ObjectID, we want to know if this is
// a temp object or not.  If it is a temp object, we are just here
// to register the class; use ObjectID = 0.  If this is not a temp
// object, we always use an ObjectID of 1.
//-------------------------------------------------------------------//
DBAssociations::DBAssociations(
	ObjectStream*				pNewDB,
	bool					bTemp
) : 

	// Call base class.
	Persistent( 
		&ObjectReference(
			DatabaseID(),
			bTemp? 0 : 1
		),
		pNewDB
	)

{

	LoadObject();

}


//-------------------------------------------------------------------//
// DBAssociations() destructor														//
//-------------------------------------------------------------------//
DBAssociations::~DBAssociations() {

	SaveObject();

	// Clean up the array allocations.
	Clear();

}

//-------------------------------------------------------------------//
// Read()																				//
//-------------------------------------------------------------------//
void DBAssociations::Read()
{

	// The base class read in the ObjectID.  Verify.
	ASSERT( ID.KeyValue() == 1 );

	// Read the number of associations.
	int nAssociationsCount;
	ReadObject( nAssociationsCount );

	// Read the size of the current structure.
	int nStructureSize;
	ReadObject( nStructureSize );

	// Read each of the associations and add to the list.
	for ( int i = 0; i < nAssociationsCount; i++ ) {
	
		// Get a new ass.
		DBAssociation* pLoop = new DBAssociation;

		// Since we saved the structure size, we can use it here
		// to handle different versions.
		// For now, no need, only 1 version.
		// if ( nStructureSize != sizeof DBAssociation ) {
		//		// Handle older versions.
		// } else
		ReadObject( pLoop->DBID );
		ReadObject( pLoop->UserName );
		ReadObject( pLoop->Company );
		ReadObject( pLoop->Contact );
		ReadObject( pLoop->nPercentage );
		ReadObject( pLoop->DBName );

		// Save the association in our list.
		Associations.Add( pLoop );

	}

	bReadOK = true;

}


//-------------------------------------------------------------------//
// Write()																				//
//-------------------------------------------------------------------//
void DBAssociations::Write()
{

	// The base class wrote the ObjectID.  Verify.
	ASSERT( ID.KeyValue() == 1 );

	// Write the number of associations.
	int nAssociationsCount = Associations.GetSize();
	WriteObject( nAssociationsCount );

	// Write the size of the current structure.
	WriteObject( (int) sizeof DBAssociation );

	// Write the associations.
	for ( int i = 0; i < nAssociationsCount; i++ ) {
		WriteObject( ( Associations[i] )->DBID );
		WriteObject( ( Associations[i] )->UserName );
		WriteObject( ( Associations[i] )->Company );
		WriteObject( ( Associations[i] )->Contact );
		WriteObject( ( Associations[i] )->nPercentage );
		WriteObject( ( Associations[i] )->DBName );
	}

}


//-------------------------------------------------------------------//
// Extract()																			//
//-------------------------------------------------------------------//
// This rips through the entire database, scanning for all references.
// It compiles a list of all databases referenced by this one, and
// places the info in our Associations array.
//
// It returns true if we were able to extract data about all 
// referenced databases.  If we are not currently connected to a 
// referenced database, this function returns false.  Regardless,
// all objects are scanned, so the error message is complete.
//-------------------------------------------------------------------//
bool DBAssociations::Extract(
	BaseProgressDlg*	pProgressDlg,
	int					nStartProgress,
	int					nEndProgress
) {

	// Init progress dlg.
	if ( pProgressDlg )
	{
		pProgressDlg->UpdateProgress( nStartProgress );
		pDatabase->SetProgressTitle(
			pProgressDlg,
			IDS_DB_EXTRACTING_ASSOCIATIONS
		);
	}

	Start();

	// Loop through all component object types.
	// Note that this does not include internal component objects.  This is OK
	// because internal component objects cannot (currently) have subobjects in other
	// databases.
	EMComponent* pObject;
	for ( int i = 0; i <= TYPE_TREE_LAST; i++ ) 
	{
		// Get the first object of this type in our database.  We need to specify
		// our Ref in order to make sure we are getting objects from the same 
		// database that this association exists in.
		pObject = pEMDBArray->NewComponent( 
			(OBJECT_TYPE) i,
			&GetRef()
		);
		pObject->FirstObject();

		// Loop through all the objects of this type.
		while ( pObject->ObjectExists() ) 
		{
			AddAssociations( pObject );
			pObject->NextObject();
		}

		delete pObject;

		// Update the progress.
		if ( pProgressDlg )
			pProgressDlg->UpdateProgress(
				nStartProgress + ( nEndProgress - nStartProgress ) * ( i + 1 ) / ( TYPE_TREE_LAST + 1 )
			);
		
	}

	return Finish();

}


//-------------------------------------------------------------------//
// Start()																				//
//-------------------------------------------------------------------//
// This function initializes variables required for compilation of
// associations.
//-------------------------------------------------------------------//
void DBAssociations::Start()
{

	// We may have had an associations object that we are now updating.
	// Clear the previous contents.
	Clear();
	
	// Load up the initial error string.  It will be appended to for
	// each object that has errors.
	strAssocNotFound.LoadString( IDS_DB_PUBLISH_ASSOC_NOT_FOUND );

	// Clear temp variables.
	nRefCount = 0;
	nCurrentAssocCount = 0;
	nBadObjectCount = 0;
	bSuccess = true;
	
}


//-------------------------------------------------------------------//
// AddAssociations()																	//
//-------------------------------------------------------------------//
// This function has TWO purposes.  First, it adds association data 
// from the given object to our current association list.  Secondly,
// it updates any subobject references of the given object with the
// newly specified data.
//
// This function allows users to rip through objects
// and call this function, in addition to whatever else they need to
// do to each object.  See EMDatabase::Publish() for a good example.
//
// This function must be used in conjunction with Start() and Finish().
// TO DO
// Remove that requirement by using the con/destructor, along with 
// the bool that determines if this is a temp object.
//
// This function returns true unless an unknown association was found.
//-------------------------------------------------------------------//
bool DBAssociations::AddAssociations(
	EMComponent*		pObject
) {

	CString strTemp;
	int k;

	DatabaseID ThisDBID = pObject->GetRef().DBID;

	// Used to add object desc to associations warning message only once.
	bool bObjectAssocOK = true;

	// We'll be looping through all Refs of the object.  We want ALL refs,
	// especially unavailables, but we don't care about blanks.
	RefScanData Data(
		false,				//	bSubobjectsOnly
		false,				//	bIncludeBlankRefs
		true					//	bIncludeUnavailableRefs
	);
	
	// Loop through all the Refs.
	while ( pObject->GetNextRef( Data ) )
	{

		/*
		#ifdef _DEBUG
		{

			// DEBUG
			// Show me the Refs.
			strTemp.Format(
				_T("NextRef: %-28s [%4i-%4i-%9i-%9i-%4i]\n"),
				EMComponentInit::ByType( Data.Type )->strDescSingular(),
				Data.pRef->ObID,
				Data.pRef->DBID.User.UserNumber,
				Data.pRef->DBID.User.Group,
				Data.pRef->DBID.DBNumber,
				Data.pRef->DBID.DBVersion
			);
			TRACE0( strTemp );
		
		}
		#endif
		*/

		// This is used to make the code read better.
		DatabaseID& CurDBID = Data.pRef->DBID;

		// If this ref is located in another database and is not blank...
		if ( 
				CurDBID != ThisDBID
			&&	CurDBID != DatabaseID() 
		) {
			
			// Note: the following routines were more efficient when they looked
			// up whether we had processed the current Ref first, skipping processing
			// if so.  But we reorganized it to split into two sections based on
			// whether the database was available first.  That way, if it was not,
			// we had a place to process every object in the unfound database, so
			// it could be placed in the error message.
			
			// Search for the DBID in our list.
			// k will contain the result of the search, which
			// we will use below.
			for ( k = 0; k < nCurrentAssocCount; k++ )
				if ( ( Associations[k] )->DBID == CurDBID ) break;

			// If we can successfully look up the database...
			ObjectStream* pCurDB;
			if ( pDBArray->LookUpDatabase( &CurDBID, &pCurDB ) ) {

				// If DBID was already in our list... 
				if ( k < nCurrentAssocCount ) {

					// Increment percentage counter.
					// This will be converted to percentage later.
					Associations[k]->nPercentage++;

				} else {

					// Fill a new association with db data and add it.
					DBAssociation* pNewAssoc = new DBAssociation;
					pNewAssoc->nPercentage = 1;
					pNewAssoc->DBID = CurDBID;
				
					wcsncpy(
						pNewAssoc->UserName,
						( (EMDatabase*) pCurDB )->UserName,
						sizeof pNewAssoc->UserName / sizeof wchar_t
					);
					wcsncpy(
						pNewAssoc->Company,
						( (EMDatabase*) pCurDB )->Company,
						sizeof pNewAssoc->Company / sizeof wchar_t
					);
					wcsncpy(
						pNewAssoc->Contact,
						( (EMDatabase*) pCurDB )->Contact,
						sizeof pNewAssoc->Contact / sizeof wchar_t
					);
					wcsncpy(
						pNewAssoc->DBName,
						pCurDB->DBHeader.DBName,
						sizeof pNewAssoc->DBName / sizeof wchar_t
					);

					Associations.Add( pNewAssoc );
					nCurrentAssocCount++;

				}

			} else {

				// Is subobject ref in our list yet?
				if ( k < nCurrentAssocCount ) {

					// Increment percentage counter.
					// This will be converted to percentage later.
					Associations[k]->nPercentage++;

				} else {

					// Add a new "unknown" association.
					DBAssociation* pNewAssoc = new DBAssociation;
					pNewAssoc->nPercentage = 1;
					pNewAssoc->DBID = CurDBID;
					pNewAssoc->UserName[0] = 0;
					pNewAssoc->Company[0] = 0;
					pNewAssoc->Contact[0] = 0;

					//////////////////////////////////////////////////////////
					// All this to ram a string from the resource file into
					// a wchar_t array.
					strTemp.LoadString( IDS_DB_ASSOCIATION_UNKNOWN );
					wstring wsBadAssoc;
					CopyCStringToWstring(
						&strTemp,
						&wsBadAssoc
					);
					wcsncpy(
						pNewAssoc->DBName,
						wsBadAssoc.c_str(),
						sizeof pNewAssoc->DBName / sizeof wchar_t
					);
					//////////////////////////////////////////////////////////

					Associations.Add( pNewAssoc );
					nCurrentAssocCount++;

					bSuccess = false;

				}
				
				// Add this object to the message string, if it hasn't
				// been added already.  We stop after a maximum is hit.
				const int nMaxBadObjects = 15;
				if ( bObjectAssocOK && nBadObjectCount < nMaxBadObjects ) {
					
					bObjectAssocOK = false;

					nBadObjectCount++;
					
					if ( nBadObjectCount == nMaxBadObjects )
						strTemp = _T("etc.");
					else {
						pObject->GetDisplayName( &strTemp );
						strTemp = 
								pObject->pEMInit->strDescSingular()
							+	_T(" ") 
							+	strTemp;
					}

					strAssocNotFound += _T("  ");
					strAssocNotFound += strTemp;
					strAssocNotFound += _T("\n");

				}
				
			}

		}

		// Update the count of the total number of references.
		nRefCount++;

	}

	return bSuccess;

}


//-------------------------------------------------------------------//
// Finish()																				//
//-------------------------------------------------------------------//
// This function is called after all objects have been added to the
// associations list.
//
// Two tasks are required to complete the compilation of all associated db's:
//
//		1) The percentages are converted from a count
//			to a percentage, based on the total ref count compiled during
//			addition of objects.
//		2) Any errors are reported.
//
//-------------------------------------------------------------------//
bool DBAssociations::Finish()
{

	int nAssocCount = Associations.GetSize();
	for ( int i = 0; i < nAssocCount; i++ )
	{
		if ( nRefCount > 0 )
		{
			Associations[i]->nPercentage = (int) ( ( 0.5 + Associations[i]->nPercentage ) / nRefCount );
			
			// Always make it at least one.
			Associations[i]->nPercentage = max( Associations[i]->nPercentage, 1 );
		}
		else 
			Associations[i]->nPercentage = -1;
	}

	if ( !bSuccess ) {
		DisplayMessage( strAssocNotFound );
	}
	
	// Always force a save of the object.
	if ( ObjectExists() )	
		ChangeObject();
	else							
		AddObject();

	return bSuccess;

}


//-------------------------------------------------------------------//
// Clear()																				//
//-------------------------------------------------------------------//
// This function clears the current associations and the memory
// associated with them.  It is called within Start() and in the
// destructor.
//-------------------------------------------------------------------//
void DBAssociations::Clear() 
{

	int nAssocCount = Associations.GetSize();
	for ( int i = 0; i < nAssocCount; i ++ )
		delete Associations[i];

	Associations.RemoveAll();

}
