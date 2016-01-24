//-------------------------------------------------------------------//
// EMComponent.cpp																	//
//																							//
//	See header file for notes.														//
//																							//
// EarthMover Tires: Accelerator Inc.											//
//-------------------------------------------------------------------//
#include "stdafx.h"

#include "EMComponent.h"
#include "EMComponentInit.h"
#include "EMComponent_Search.h"
#include "EMComponent_SearchDlg.h"
#include "EMSubObjectVector.h"

#include "DataEnumTypes.h"
#include "DisplayMessage.h"
#include "Source\Source.h"
#include "StringHelpers.h"
#include "2DChart.h"
#include "SpecFrame.h"
#include "LogFrame.h"							// Object debug logging
#include "ObjectStream\EMDBArray.h"					// For NewComponent()

#include "DBTreeControl.h"
#include "ConfiguratorTreeControl.h"		// We build configurators here.
#include "Option\Option.h"						// We build configurators here with Options.
#include "Option\OptionUtility.h"			// We build configurators here with Options.
#include "RefScanData.h"						// Helper for GetNextRef().
#include "ReportSelectionDlg.h"

#include "DBTreeCache.h"						// We invalidate caches on Save().

#include "Media\Media.h"						// For default multimedia image handling.

//-------------------------------------------------------------------------------//
// Defines																								//
//-------------------------------------------------------------------------------//

typedef CWrapArray< DataItem	, DataItem&		>	CDIWrapArray;
typedef CWrapArray< DataEnum	, DataEnum&		>	CDEWrapArray;
typedef CWrapArray< DataString, DataString&	>	CDSWrapArray;


//-------------------------------------------------------------------------------//
//																											//
// class EMComponent																					//
//																											//
//-------------------------------------------------------------------------------//

// This boolean is used to speed up certain Earthmover database
// operations by loading only the SubObject information of objects
// and skipping the BaseData and derived class proprietary data.
// As such, this boolean should be used sparingly and with much care.
bool bLoadSubObjOnly = false;


//-------------------------------------------------------------------//
// EMComponent() construction														//
//-------------------------------------------------------------------//
EMComponent::EMComponent(
	const ObjectReference	*pRef,
	ObjectStream						*pNewDatabase,
	EMComponentInit			*pInit
) :

	Persistent(
		pRef,
		pNewDatabase
	),

	pEMInit				  	( pInit	),

	di							( NULL	),
	de							( NULL	),
	dstr						( NULL	)

{
	ASSERT( pEMInit );


	////////////////////////////////////////////////////
	// Set up the initial defaults for the bit flags.
	////////////////////////////////////////////////////
	//

	// First, set 'em all false, then set true as needed.
	PersistentFlags.reset();
	NonPersistentFlags.reset();

	SetRefresh();

	//
	////////////////////////////////////////////////////


	GetNewDataArrays();
	Clear();
}

//-------------------------------------------------------------------//
// ~EMComponent() destruction														//
//-------------------------------------------------------------------//
EMComponent::~EMComponent()
{

	// Make sure you matched up your InitState() and EndInitState()
	// calls.
	// ASSERT( !bStateInitialized );

	// Set this to false to avoid tripping ASSERTs within
	// DeleteDataArrays() call to SubObj_Release().
	SetIsPropSheet( false );

	DeleteDataArrays();
}

//-------------------------------------------------------------------//
// GetNewDataArrays()																//
//-------------------------------------------------------------------//
// Allocates all member arrays that are dependant upon pEMInit.
//-------------------------------------------------------------------//
void EMComponent::GetNewDataArrays()
{
	int nSize;

	// Was this called twice in a row without a call to
	// DeleteDataArrays() in between?
	ASSERT( !di && !de && !dstr );

	// Initialize BaseData.
	if ( nSize = pEMInit->GetSizeDI() )
		di = new DataItem[ nSize ];

	if ( nSize = pEMInit->GetSizeDE() )
		de = new DataEnum[ nSize ];

	if ( nSize = pEMInit->GetSizeDS() )
		dstr = new DataString[ nSize ];

	// Initialize SubObjects.
	SubObj.SetSize( pEMInit->GetSubObjSize(), 0 );

	for ( int nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
		SubObj[ nA ].pInit() = pEMInit->GetSubObjInit( nA );

	// Initialize SubObject Lists.
	if ( nSize = pEMInit->GetNumLists() )
	{
		SubObjList.RemoveAll();		// Force array item destruction.
		SubObjList.SetSize( nSize );
	}
}

//-------------------------------------------------------------------//
// DeleteDataArrays()																//
//-------------------------------------------------------------------//
// Deallocates all member arrays that are dependant upon pEMInit.
//-------------------------------------------------------------------//
void EMComponent::DeleteDataArrays()
{
	if ( di	 ) delete[] di;
	if ( de	 ) delete[] de;
	if ( dstr ) delete[] dstr;

	DeleteSubObjBackup();

	SubObj_Release();

	SubObj.RemoveAll();
	SubObjList.RemoveAll();

	di = NULL;
	de = NULL;
	dstr = NULL;
}

//----------------------------------------------------------------------------------//
//																												//
// DATABASE ACCESS: used in derived object(s) database functions.							//
//																												//
//----------------------------------------------------------------------------------//


// The traversal functions were overridden so we could clean up the subobjects
// before traversing.  This allows the subobjects to be saved, if needed.
Persistent& EMComponent::FindObject		( PdyKey *key )	{ SubObj_Release(); return Persistent::FindObject		( key );	}
Persistent& EMComponent::CurrentObject	( PdyKey *key )	{ SubObj_Release(); return Persistent::CurrentObject	( key );	}
Persistent& EMComponent::FirstObject	( PdyKey *key )	{ SubObj_Release(); return Persistent::FirstObject		( key );	}
Persistent& EMComponent::LastObject		( PdyKey *key )	{ SubObj_Release(); return Persistent::LastObject		( key );	}
Persistent& EMComponent::NextObject		( PdyKey *key )	{ SubObj_Release(); return Persistent::NextObject		( key );	}
Persistent& EMComponent::PreviousObject( PdyKey *key )	{ SubObj_Release(); return Persistent::PreviousObject	( key );	}


//-------------------------------------------------------------------//
// Read()																				//
//-------------------------------------------------------------------//
// This handles all reading of DataItems, DataEnums, DataStrings,
// registered DataSpecials, and SubObject references.
// Derived classes should override, call the base, and then handle
// any proprietary data.
//-------------------------------------------------------------------//
void EMComponent::Read()
{

	// Read in the SubObjects and SubObject Lists.
	bReadOK &= SubObj_Read();

	// This boolean is used to speed up certain Earthmover database
	// operations by loading only the SubObject information of objects
	// and skipping the BaseData and derived class proprietary data.
	if ( bLoadSubObjOnly ) {
		bReadOK = false;
		return;
	}

	if ( !bReadOK )
		return;

	// Read in the bit flags.
	ReadObject( PersistentFlags );

	if ( BeforeVersion( 1, 18 ) )
	{
		// Set the rev eng completeness flags to true.
		// Any incomplete objects will update this flag
		// to false as needed the next time changes are
		// made.
		SetRevEngComplete();

	}

	// Read in the default source if there is one.
	if ( pEMInit->UsesDefaultSource() )
		dsDefaultDataSource.ReadObject();

	// Read in the BaseData, i.e. DataItems, DataEnums, DataStrings.
	if ( bReadOK )
		bReadOK = BaseData_Read();

	// Read DataSpecials array.
	if ( bReadOK )
		ReadDataSpecials();

	// If bReadOK is true here, we can be fairly sure that we
	// have a valid object.  If it is false, we can display a
	// message at this point to tell the user that the object
	// is no good.  We could have let derived classes handle
	// bad results, as they may have additional validity checks,
	// but handling it here should be more than sufficient,
	// and it removes the responsibility from the derived class.
	if ( !bReadOK )
	{
		static bool sbShowReadErrorWarning = true;
		if ( sbShowReadErrorWarning )
		{
			CString strTemp = GetDisplayNameFromTree( GetObjectType(), &GetRef() );
			CString strLabel = _T("Database Read Error");
			CString strMsg;

			strMsg.LoadString( IDM_BASE_DATA_READ_ERROR );

			strMsg += _T("\n\nObject Type: ");
			strMsg += pEMInit->strDescSingular() + _T("\n");
			strMsg += _T("Object Name: ") + strTemp;

		 	KillableWarning( strMsg, strLabel, sbShowReadErrorWarning );
		}
	}

}

//-------------------------------------------------------------------//
// Write()																				//
//-------------------------------------------------------------------//
// This handles all writing of DataItems, DataEnums, DataStrings,
// registered DataSpecials, and SubObject references.
// Derived classes should override, call the base, and then handle
// any proprietary data.
//-------------------------------------------------------------------//
void EMComponent::Write()
{

	///////////////////////////////////////////////////////////////////
	// This boolean is used to speed up certain Earthmover database
	// operations by loading only the SubObject information of objects
	// and skipping the BaseData and derived class proprietary data.
	//
	// It should NEVER be set during a write operation.  The database
	// is probably just about to get trashed, my friend.....
	//
	// Try to minimize the damage caused by this and get outta
	// here.  Any changes made to this object will be lost, but at
	// least no garbage BaseData got written out.
	//
	if ( bLoadSubObjOnly )
	{
		#ifdef ACCELERATOR_INHOUSE
		AfxMessageBox(
			"UhOh - bLoadSubObjOnly is true during an EMComponent::Write().\n" \
			"This implies something ralphed all through static mem recently -" \
			"Everything is now hozed.\nThis is an Accelerator in-house message" \
			"only.  Enjoy!"
		);
		#endif
		return;
	}
	//
	///////////////////////////////////////////////////////////////////


	// Write out the SubObjects and SubObject Lists.
	SubObj_Write();

	// Write out the persistent bit flags.
	WriteObject( PersistentFlags );

	// Write out the default source if there is one.
	if ( pEMInit->UsesDefaultSource() )
		dsDefaultDataSource.WriteObject();

	// Write out the BaseData, i.e. DataItems, DataEnums, DataStrings.
	BaseData_Write();

	// Write DataSpecials array.
	WriteDataSpecials();

}


//-------------------------------------------------------------------//
// Copy()																				//
//-------------------------------------------------------------------//
// We found that in Earthmover there are various situations that
// require different kinds of "Copy" functions.  Originally, it was
// all handled through an operator= function, which quickly got
// hacked up with ambiguous flags that directed the program flow.
// Now we try to clean that up using regular functions that take
// arguments to direct the action, instead of static or member
// flags that control the action behind the scenes.
//-------------------------------------------------------------------//
void EMComponent::Copy(
	EMComponent		*pSrcEMC,					// Source EMComponent pointer.
	bool				bDupePriv,					// Duplicate private objects in the DB?
	bool				bDupeBackups,				// Copy SubObj backup arrays too?
	SwapRefArray	*parSwap		 				// Used internally by EMComponent to update linked refs.
){

	// Quick-check that the source is from the same class.
	ASSERT( pSrcEMC->GetObjectType() == GetObjectType() );

	// Copy the default data source, if needed.
	if ( pEMInit->UsesDefaultSource() )
		dsDefaultDataSource =
			pSrcEMC->dsDefaultDataSource;

	// Copy flags.
	PersistentFlags = pSrcEMC->PersistentFlags;

	// Call the derived class to copy its own proprietary data.
	//
	// We do this before the BaseData_Copy() so that Option
	// objects can set up any customized EMComponentInit
	// information that affects the BaseData arrays.  At the
	// time that I made this change I checked all derived
	// versions of DerivedCopy() and made sure that this will
	// not have any adverse effects.  JWC 12/10/1998
	//
	// We do this before the SubObject call so that
	// if SwapRef() is called from within SubObj_Copy() to
	// update a private object's ref then it will catch any
	// of the refs in the derived class's data.
	DerivedCopy( pSrcEMC );

	// Call BaseData copy function to handle
	// all DataItems, DataEnums and DataStrings.
	BaseData_Copy( pSrcEMC );

	// Copy DataSpecials array.  See the comments above as
	// to why we now do this before the SubObject copy.
	for ( int nA = 0; nA < arpDataSpecials.GetSize(); nA++ )
		arpDataSpecials[ nA ]->Copy( pSrcEMC->arpDataSpecials[ nA ] );

	// Call Sub-Object copy function to handle subobject,
	// subobject lists and the backup lists if necessary.

	SubObj_Copy(
		pSrcEMC,				// Source EMComponent pointer.
		bDupePriv,			// Duplicate private objects in the DB?
		bDupeBackups,		// Copy SubObj backup arrays too?
		parSwap				// Used internally by EMComponent to update linked refs.
	);

}

//-------------------------------------------------------------------//
// ReportCopy()																		//
//-------------------------------------------------------------------//
// This virtual function, where overridden, copies any data in the
// EMComponent derived classes that is not handled by EMComponent.
// It is specific to copies of objects created for reports, which
// we commonly refer to as "snapshot" objects.
// The EMComponent version copies over all subobject pointers.
//-------------------------------------------------------------------//
void EMComponent::ReportCopy( EMComponent	*pSrcEMC	)
{
	int nA, nB, nSubSize, nSize, nLists;
	EMSubObject	*pSrcSub, *pDestSub;

	// If you hit this assert then something is majorly wrong,
	// two different types of objects are being copied to each other.
	ASSERT( GetObjectType() == pSrcEMC->GetObjectType() );

	// Currently loaded SubObj's no longer valid.
	SubObj_Clear();

	nSize 	= pEMInit->GetSubObjSize();
	nLists	= pEMInit->GetNumLists();

	// Using a local EMSubObject pointer to save array lookups.
	pSrcSub	= pSrcEMC->SubObj.GetData();
	pDestSub	= SubObj.GetData();

	// Loop through all the SubObjects:
	for ( nA = 0; nA < nSize; nA++ )
	{
		// Copies the ref, flags, and initilizer ptrs.
		pDestSub->Copy( pSrcSub, true );
		pDestSub->m_pEMC = pSrcSub->m_pEMC;
		if ( pDestSub->m_pEMC )
			pSrcSub->Detach();

		pDestSub++;
		pSrcSub++;
	}

	// Loop through all the SubObject lists:
	for ( nA = 0; nA < nLists; nA++ )
	{
		// Set the CArray to be the same size as the pSrcEMC->
		nSubSize = pSrcEMC->GetListSize( nA );
		SubObjList_RemoveAll( nA );
		SubObjList[ nA ].SetSize( nSubSize );

		pSrcSub	= pSrcEMC->SubObjList[ nA ].GetData();
		pDestSub	= SubObjList[ nA ].GetData();

		SubObj_Init *pInit = pEMInit->GetSubObjListsInit( nA );

		// Loop through all the SubObjects:
		for ( nB = 0; nB < nSubSize; nB++ )
		{
			ASSERT( pSrcSub->pInit() == pInit );

			// Copies the ref, flags, and initilizer ptrs.
			pDestSub->Copy( pSrcSub, true );
			pDestSub->m_pEMC = pSrcSub->m_pEMC;
			if ( pDestSub->m_pEMC )
				pSrcSub->Detach();

			pDestSub++;
			pSrcSub++;
		}
	}
}


//-------------------------------------------------------------------//
// Clear()																				//
//-------------------------------------------------------------------//
void EMComponent::Clear() {

	// EMComponent specific data.
	dsDefaultDataSource.SetCalculated();

	BaseData_SetSource();

	SetCompleteness( false );	// Completeness flag

	SubObj_Clear();	// Call Sub-Object version
	BaseData_Clear();	// Call Base Data version

	// Clear DataSpecials array.
	for ( int nA = 0; nA < arpDataSpecials.GetSize(); nA++ )
		arpDataSpecials[ nA ]->Clear();
}


//-------------------------------------------------------------------//
// InitState()																			//
//-------------------------------------------------------------------//
// This function is called to reset the state for the object.
// Derived classes should override to reset their specific "state"
// variables ( non-persistent member variables) after calling the
// base-class version.
//
// Making this virtual function in the base class allows us to
// automatically set the initial state for all objects before,
// for example, beginning simulation calculations.
//
// Normally, the function should only be called once for any object.
// If a reset is needed, set bResetAsNeeded to true, or this function
// will just return.
//-------------------------------------------------------------------//
void EMComponent::InitState(
	bool bResetAsNeeded
) {

	// Objects should only be initialized once, unless the caller
	// specifically intends to reinitialize by setting bResetAsNeeded
	// to true.
	if ( bIsStateInitialized() && !bResetAsNeeded )
		return;

	SetStateInitialized();

	//	We assume that the user will be interested
	// in initializing and then preserving the states of any
	// accessed subobjects, so we set up some flags to
	// indicate this.  These flags will be handled in
	// GetObjPtr().

	// First, we release all existing subobjects.
	// Originally, we did not want to release subobjects if the
	// user was dealing with a "snapshot" situation like spec sheets.
	// If we do not release them, we need to reset the bStateInitialized
	// flag for all currently existing subobjects, and then GetObjPtr()
	// needs to always call InitState for subobjects whose
	// bStateInitialized flag is not true.
	// The other way to handle things is to ALWAYS release all sub-
	// objects here, and in GetObjPtr(), just call InitState for
	// newly loaded SO's.
	// The first method means that the subobject gets loaded less
	// frequently, which is prolly good for specs and bad for
	// simulation.  We'll go with the second method for now.
	// if ( !bDoNotRefresh )
		SubObj_Release();

	// Turn off refreshing.  We want the state to stick
	// through multiple calls to GetObjPtr().  We will reset
	// to the previous state in EndInitState().
	SetPreviousRefresh( bRefresh() );
	SetRefresh( false );

}


//-------------------------------------------------------------------//
// EndInitState()																		//
//-------------------------------------------------------------------//
// This function marks the end of usage of the state variables of
// the object.
//
// Typically, this will be called just before the object is destroyed.
// But this function also allows the user to continue using the object
// in some non-state capacity before it is destroyed.  This will likely
// never be used, but there is another reason for the function.  If the
// user does not mark the end before the object is destroyed, then we
// ASSERT().  This helps to keep the user aware of the state status.
//-------------------------------------------------------------------//
void EMComponent::EndInitState()
{

	ASSERT( bIsStateInitialized() );

	// Revert to uninitialized and previous refresh status.
	SetStateInitialized( false );
	SetRefresh( bPreviousRefresh() );

	// Call EndInitState() for all existing subobjects.
	// This will recurse through all subobjects' subobjects.
	int nA, nB;
	EMSubObject* pSub = SubObj.GetData();
	for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
	{
		if ( pSub->pEMC() )

			// Note that we have defined state initialization to only
			// be available for objects existing in the database.  GetObjPtr()
			// does not call InitState() if the subobject is not found.
			// Therefore, we don't want to call EndInitState() either, for
			// objects that were not successfully loaded.
			if ( pSub->pEMC()->ObjectExistsAndReadOK() )
				pSub->pEMC()->EndInitState();

		pSub++;
	}

	// Do lists, too.
	for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ )
	{
		pSub = SubObjList[ nA ].GetData();
		for ( nB = 0; nB < GetListSize( nA ); nB++ )
		{

			// Skip blank objects - see note above.
			if (
				pSub->pEMC()									&&
				pSub->pEMC()->ObjectExistsAndReadOK()
			)
				pSub->pEMC()->EndInitState();

			pSub++;
		}
	}

	if ( bRefresh() )
		SubObj_Release();

}


//-------------------------------------------------------------------//
// GetNextRef()																		//
//-------------------------------------------------------------------//
// This function handles all cases where we need to loop through
// the Refs contained within the object.
//
// There are variables defining the scan type, there are
// variables defining where we are in the scan, and there are
// variables specifying the result of the scan.  Instead of making
// static variables, and instead of passing a large number of
// pointers to parameters, we lump all the variables into a class,
// and then provide a pointer to an object of the class.
//
// The parameter class is called RefScanData.  Here are descriptions
// of its members that must be set before calling (they are set
// via constructor params):
//
//
//		RefScanData::bSubObjectsOnly
//		----------------------------
//		Sometimes, we want every Ref of the object, which includes:
//
//			1) Refs of single subobjects;
//			2) Refs of subobjects in subobject lists;
//			3) Ref of the default source;
//			4) Refs of sources of DataItems/Enums/Strings; and
//			5) Refs of DataSpecials.
//
//		Other times,  we just want the Refs for subobjects of this object.
//		This param defines which.
//
//
//		RefScanData::bIncludeBlankRefs
//		------------------------------------
//		This param defines whether to return blank Refs or not.  You
//		might want blank Refs if you are updating them all manually, for
//		example.
//
//
//		RefScanData::bIncludeUnavailableRefs
//		------------------------------------
//		This param defines whether to test for availability of the Ref.
//		If the Ref refers to a db that is not available, the Ref will
//		be skipped (and bFoundUnavailableRefs will be set to true - see
//		below).
//
//
// Here are descriptions of public member variables providing results
// of the Ref scan:
//
//
//		RefScanData::pRef
//		-----------------
//		This param is the resulting next Ref as found by the scan.  It
//		is a pointer to the actual Ref data, and so it may be directly
//		updated if needed - be careful!  :>
//
//
//		RefScanData::Type
//		-----------------
//		This param contains the type of object for the returned Ref.
//
//
//		RefScanData::bFoundUnavailableRefs
//		----------------------------------
//		If it is determined that a Ref is unavailable, this flag will be
//		set to false.  Note that the release version will only set this
//		flag properly if bIncludeUnavailableRefs was originally set to
//		false.  Otherwise, it just bypasses the check.
//
//
//	The remaining member variables of RefScanData are protected and
// inaccessible.  They are used internally to determine where we are
// in the scan.   However, the following functions are available to
// allow you to peek at the values:
//
//
//		RefScanData::	GetSO(), GetSOList, GetSOListIndex(),
//							GetSpecial(), GetDI(), GetDE(), GetDS()
//		------------------------------------------------------
//		These provide read-only access to the indexes.
//		Note that the caller is left responsible to check
//		the meaning of the result.  It will be in one of three
//		ranges of values:
//
//			-1									Not an index for the current Ref.
//			0 to highest valid index	The index for the current Ref
//			> highest valid index		Not an index for the current Ref.
//
//-------------------------------------------------------------------//
bool EMComponent::GetNextRef( RefScanData& Data )
{

	bool bBlank			= false;
	bool bUnavailable = false;

	do
	{

		// Reset the ref result.
		Data.pRef = 0;

		if ( !Data.bSubobjectsOnly )
		{

			Data.Type = OBJECT_SOURCE;

			// Default source.
			if ( !Data.bDefaultSource )
			{
				Data.pRef = dsDefaultDataSource.GetSourceRefPtr();
				Data.bDefaultSource = true;
			}

			// DataItem sources.
			else if ( ++Data.nDI < pEMInit->GetSizeDI() )
			{
	 			Data.pRef = di[ Data.nDI ].GetSourceRefPtr();
			}

			// DataEnum sources.
			else if ( ++Data.nDE < pEMInit->GetSizeDE() )
			{
	 			Data.pRef = de[ Data.nDE ].GetSourceRefPtr();
			}

			// DataString sources.
			else if ( ++Data.nDS < pEMInit->GetSizeDS() )
			{
	 			Data.pRef = dstr[ Data.nDS ].GetSourceRefPtr();
			}

			// DataSpecial array.
			// The DataSpecial class contains a virtual function GetNextRef( RefScanData )
			// that derived classes can override to provide access to their Refs.
			// RefScanData contains two counters and a bool to allow the DataSpecial-derived
			// class to keep track of doing its thing.  Don't forget to set the type, too!
			if ( !Data.pRef )
			{
				while (
					Data.nSpecial == -1															||
					(
						Data.nSpecial < arpDataSpecials.GetSize()						&&
						!arpDataSpecials[ Data.nSpecial ]->GetNextRef( Data )
					)
				) {

					// Get the next DataSpecial.
					Data.nSpecial++;

					// Reset for the next Special run.
					Data.nGridColumn		= -1;
					Data.nGridRow			= -1;
					Data.bSpecialSource	= false;

				}

				// Make sure we got one if appropriate.
				ASSERT( Data.nSpecial == arpDataSpecials.GetSize() || Data.pRef != 0 );
			}

		}

		// If we don't have one yet...
		if ( !Data.pRef )
		{

			// Get the next single subobject.
			if ( ++Data.nSO < pEMInit->GetSubObjSize() )
			{
				Data.pRef = GetSubObjRef( Data.nSO );
				Data.Type = pEMInit->GetSubObjType( Data.nSO );
			}

			// Get the next SO from the lists.
			else
			{

				// If we haven't found anything, and while we have a valid list...
				while (
					( Data.pRef == 0									)	&&
					( Data.nSOList < pEMInit->GetNumLists()	)
				) {

					// If another item is available (note we check for first pass)...
					if (
						( Data.nSOList != -1												)	&&
						( ++Data.nSOListIndex <  GetListSize( Data.nSOList )	)
					) {

						// Get it
						Data.pRef = GetSubObjRef( Data.nSOList, Data.nSOListIndex );
						Data.Type = pEMInit->GetSubObjType( Data.nSOList, Data.nSOListIndex );

					}
					else
					{
						// Increment the list.
						Data.nSOList++;
						Data.nSOListIndex = -1;

					}

				}

			}

		}

		// If we don't have one yet...
		if ( !Data.pRef )
		{
			// See if we have any "other" Refs, as defined by derived classes in
			// GetOtherRef().  If pRef is still 0 after this call, we're finished!
			GetOtherRef( Data );

		}

		// If we STILL didn't find one, we're done.
		if ( !Data.pRef )
			return false;

		// We found a Ref, but we want to test for blanks and unavailables.

		// Is it blank?
		bBlank = ( *Data.pRef == ObjectReference() );

		// Is it unavailable?
		bUnavailable = false;
		if ( !bBlank )
		{
			// Note that we only test if the user requested.
			if ( !Data.bIncludeUnavailableRefs )
			{
				ObjectStream* pUnused;
				bUnavailable = !pEMDBArray->LookUpDatabase( &( Data.pRef->DBID ), &pUnused );
				Data.bFoundUnavailableRefs |= bUnavailable;

				// Just a warning that we found some unavailable Refs while scanning.
				// But you prolly already knew that.  :>
				ASSERT( !bUnavailable );

			}
		}


	} while(

		// Loop if skipping blank Refs.
		( bBlank && !Data.bIncludeBlankRefs					)		||

		// Loop if skipping unavailable Refs.
		( bUnavailable && !Data.bIncludeUnavailableRefs	)

	);

	return true;

}


//-------------------------------------------------------------------//
// IsDataComplete()																	//
//-------------------------------------------------------------------//
// Checks the completeness of an object.  Objects that have some		//
// non-standard (proprietary?) data in them should override this		//
// function to include their own checks, but should call this base	//
// class version to start off with.												//
//																							//
// RETURNS:																				//
//   bool			: overall completeness true / false.					//
//   p_ar_strErr	: if passed, will add error strings to the array.	//
//-------------------------------------------------------------------//
bool EMComponent::IsDataComplete(
	CStringArray*	p_ar_strErr			,		// Array for completeness error strings.
	bool*				pbFoundRevEngItems,	// true if we found items that may be rev-eng'ed
	int				nIndent						// # of tabs to add before error string.
) {
	bool bReturn;

	// Put up a wait cursor.  It'll die in destructor.  Nice, MS!
	CWaitCursor wait;

	// Check base data for completeness.
	bReturn = BaseData_IsDataComplete( p_ar_strErr, pbFoundRevEngItems, nIndent );

	// Check sub objects for completeness.
	bReturn &= SubObj_IsDataComplete( p_ar_strErr, pbFoundRevEngItems, nIndent );

	// Check DataSpecials array for completeness.
	for ( int nA = 0; nA < arpDataSpecials.GetSize(); nA++ )
		bReturn &= arpDataSpecials[ nA ]->IsDataComplete( p_ar_strErr, nIndent );

	// MDM	7/12/99 2:14:48 PM
	// Check the new DoRevEng status flag.
	// Note that the only way to get that flag to reset is to go to
	// the prop sheet and force a resave.
	if ( !bRevEngComplete() )
	{
		bReturn = false;

		// If we are at the top level object, we indicate to the user that
		// they will have to press Apply() to get the details about
		// RevEng errors.
		if ( ( nIndent > 0 ) && p_ar_strErr )
			p_ar_strErr->Add( strIndentString( nIndent ) + ResStr( IDS_OBJECT_FAILED_DATA_VALIDATION ) );
	}

	return bReturn;
}


//-------------------------------------------------------------------//
// GetDataCompleteErrors()															//
//-------------------------------------------------------------------//
// This function does the little things that are a pain to
// recode when you want output of the data completion errors.
// You'll have to provide your error string arrays.
//
// The result is formatted for display based on bUseExtraCR.
// (Typically, in edit boxes, an extra carriage return is required
//  per line.)
//-------------------------------------------------------------------//
CString EMComponent::GetDataCompleteErrors(
	CStringArray*	p_ar_strErr			,		// Array for completeness error strings.
	bool				bFoundRevEngItems			// true if we found items that may be rev-eng'ed
) {
	int nA;
	CString strErrors;

	int nStrCount = p_ar_strErr->GetSize();
	for ( nA = 0; nA < nStrCount; nA++ )
	{
		strErrors += p_ar_strErr->GetAt( nA );
		if ( nA < nStrCount - 1 )
			strErrors += strCR;
	}

	// Add a note about reverse-engineered candidates, if any.
	if ( bFoundRevEngItems )
	{
		strErrors +=
				strCR
			+	strCR
			+	strRevEngCandidateMarker
			+	CString( (LPSTR) IDS_DB_COMPLETE_ERRORS_REV_ENG );
	}

	// MDM	7/11/2000 9:38:10 AM
	// The following is not true.  For new objects, IsRevEngComplete()
	// returns false, so we are incomplete, but there are no strings
	// to display.  This situation is now properly handled in 
	// ComponentStatusPage::GetCompletnessInfo().
	/*
	// Make sure we had some errors to show if we are incomplete!
	ASSERT(
			IsDataComplete() 
		|| nStrCount > 0
		||	m_ar_strDoRevEngMsgs.GetSize() > 0
	);
	*/

	return strErrors;
}


//-------------------------------------------------------------------//
// GetDoRevEngErrors()																//
//-------------------------------------------------------------------//
// This function returns the error string containing all errors
// that occurred in DoRevEng().
//
// The result is formatted for display based on bUseExtraCR.
// (Typically, in edit boxes, an extra carriage return is required
//  per line.)
//-------------------------------------------------------------------//
CString EMComponent::GetDoRevEngErrors()
{
	int nA;
	CString strErrors;

	// Only call this function after rev eng routines are called.
	// Note that we now MAY call this even if rev eng succeeded,
	// to get warnings and other non-failure messages.
	ASSERT( bCalledDoRevEng() );

	int nDREMsgCount = m_ar_strDoRevEngMsgs.GetSize();

	if ( nDREMsgCount > 0 )
	{
		for ( nA = 0; nA < nDREMsgCount; nA++ )
		{
			strErrors += m_ar_strDoRevEngMsgs[nA];
			if ( nA < nDREMsgCount - 1 )
				strErrors += strCR;
		}

	} else
	{
		// We did not get all the way through DoRevEng, but there
		// were no suggestions made.  This is most likely because
		// DoRevEng did not have basic data that it needed.  Just
		// tell the user to see the data entry list.
		strErrors.LoadString( IDM_REV_ENG_SEE_DATA_ENTRY_RESULTS );
	}

	return strErrors;
}


//-------------------------------------------------------------------//
// SetSource()																			//
//-------------------------------------------------------------------//
// This sets all data items to use the default source.
// It also sets the data items of all DataSpecial subobjects.
//-------------------------------------------------------------------//
void EMComponent::SetSource( bool bUserEnteredOnly )
{
	int nA;

	BaseData_SetSource( bUserEnteredOnly );

	// Set DataSpecials array sources.
	for ( nA = 0; nA < arpDataSpecials.GetSize(); nA++ )
		arpDataSpecials[ nA ]->SetSource( dsDefaultDataSource, bUserEnteredOnly );

}

//-------------------------------------------------------------------//
// DoCalcOtherData()																	//
//-------------------------------------------------------------------//
// Fills in any Data that needs to be calculated.  Is called from the
// propsheet when OK or Apply is hit, to update the calculated data
// with any changes that were made.
//
// Only enters this routine when IsDataComplete is true.
// Derived versions need to call the base class version.
//-------------------------------------------------------------------//
void EMComponent::DoCalcOtherData()
 {
	int nA;

	// Call for DataSpecials array.
	for ( nA = 0; nA < arpDataSpecials.GetSize(); nA++ )
		arpDataSpecials[ nA ]->DoCalcOtherData();

}



//----------------------------------------------------------------------------------//
//																												//
// BASIC DATA TYPE HANDLING:																			//
//																												//
//----------------------------------------------------------------------------------//

//-------------------------------------------------------------------//
// BaseData_Read()																	//
//-------------------------------------------------------------------//
// Reads in "BaseData" data, such as DataItems, DataEnums, and
// DataStrings.
//-------------------------------------------------------------------//
bool EMComponent::BaseData_Read()
{
	short sDI = 0, sDE = 0, sDSTR = 0, sCnt;
	bool bReturn = false;
	bool bCheckComplete = false;
	static bool bShowErrorMessage = true;

	// If this class is derived from another EMComponent derived class,
	// then we need to loop through each class in turn to keep
	// versioning intact.
	int nClassLoop;
	int nClassCnt = pEMInit->GetNumBaseObjectTypes();

	ASSERT( nClassCnt >= 1 );

	for ( nClassLoop = 0; nClassLoop < nClassCnt; nClassLoop++ )
	{
		// Options objects have their own custom EMComponentInit
		// pointers.  Hence for non-derived classes we just take
		// the current pointer instead of indexing to the default
		// as we do below.
		EMComponentInit *pCurInit = pEMInit;

		if ( nClassCnt > 1 )
			pCurInit = EMComponentInit::ByType(
				pEMInit->GetBaseObjectType( nClassLoop )
			);

		///////////////////////////////////////
		// Read DataItems.
		///////////////////////////////////////

		ReadObject( sCnt );
		sCnt += sDI;

		if ( ( sCnt < sDI ) || ( sCnt > pCurInit->GetSizeDI() ) )
		{
			#ifdef ACCELERATOR_INHOUSE
			{
				if ( bShowErrorMessage )
				{
					CString strTemp = GetDisplayNameFromTree( GetObjectType(), &GetRef() );
					CString strLabel = _T("DataItem count error");
					CString strMsg = _T("Object Type: ");

					strMsg += pEMInit->strDescSingular() + _T("\n");
					strMsg += _T("Object Name: ") + strTemp + _T("\n\n");

					strTemp.Format( _T("sCnt = %d\n"), sCnt );
					strMsg += strTemp;
					strTemp.Format( _T("sDI = %d\n"), sDI );
					strMsg += strTemp;
					strTemp.Format( _T("pCurInit->GetSizeDI() = %d\n\n"), pCurInit->GetSizeDI() );
					strMsg += strTemp;

					strMsg += __FILE__;
					strTemp.Format( _T("\nLine %d\n\n"), __LINE__ );
					strMsg += strTemp;
					strMsg += _T("This message will not be shown outside of Accelerator.");

					KillableWarning( strMsg, strLabel, bShowErrorMessage );
				}
			}
			#endif

			return false;
		}

		// New items added to this object?
		if ( sCnt < pCurInit->GetSizeDI() )
			bCheckComplete = true;					// Flag a completeness check at end.

		// Use database count, and fill in any extras with defaults.
		for ( ; sDI < sCnt; sDI++ )
			di[ sDI ].ReadObject();
		for ( ; sDI < pCurInit->GetSizeDI(); sDI++ )
			di[ sDI ] = pCurInit->GetDI( sDI );

		///////////////////////////////////////
		// Read DataEnums.
		///////////////////////////////////////

		ReadObject( sCnt );
		sCnt += sDE;

		if ( ( sCnt < sDE ) || ( sCnt > pCurInit->GetSizeDE() ) )
		{
			#ifdef ACCELERATOR_INHOUSE
			{
				if ( bShowErrorMessage )
				{
					CString strTemp = GetDisplayNameFromTree( GetObjectType(), &GetRef() );
					CString strLabel = _T("DataEnum count error");
					CString strMsg = _T("Object Type: ");

					strMsg += pEMInit->strDescSingular() + _T("\n");
					strMsg += _T("Object Name: ") + strTemp + _T("\n\n");

					strTemp.Format( _T("sCnt = %d\n"), sCnt );
					strMsg += strTemp;
					strTemp.Format( _T("sDE = %d\n"), sDE );
					strMsg += strTemp;
					strTemp.Format( _T("pCurInit->GetSizeDE() = %d\n\n"), pCurInit->GetSizeDE() );
					strMsg += strTemp;

					strMsg += __FILE__;
					strTemp.Format( _T("\nLine %d\n\n"), __LINE__ );
					strMsg += strTemp;
					strMsg += _T("This message will not be shown outside of Accelerator.");

					KillableWarning( strMsg, strLabel, bShowErrorMessage );
				}
			}
			#endif

			return false;
		}

		// New items added to this object?
		if ( sCnt < pCurInit->GetSizeDE() )
			bCheckComplete = true;					// Flag a completeness check at end.

		// Use database count, and fill in any extras with defaults.
		for ( ; sDE < sCnt; sDE++ )
			de[ sDE ].ReadObject();
		for ( ; sDE < pCurInit->GetSizeDE(); sDE++ )
			de[ sDE ] = pCurInit->GetDE( sDE );

		///////////////////////////////////////
		// Read DataStrings.
		///////////////////////////////////////

		ReadObject( sCnt );
		sCnt += sDSTR;

		if ( ( sCnt < sDSTR ) || ( sCnt > pCurInit->GetSizeDS() ) )
		{
			#ifdef ACCELERATOR_INHOUSE
			{
				if ( bShowErrorMessage )
				{
					CString strTemp = GetDisplayNameFromTree( GetObjectType(), &GetRef() );
					CString strLabel = _T("DataString count error");
					CString strMsg = _T("Object Type: ");

					strMsg += pEMInit->strDescSingular() + _T("\n");
					strMsg += _T("Object Name: ") + strTemp + _T("\n\n");

					strTemp.Format( _T("sCnt = %d\n"), sCnt );
					strMsg += strTemp;
					strTemp.Format( _T("sDSTR = %d\n"), sDSTR );
					strMsg += strTemp;
					strTemp.Format( _T("pCurInit->GetSizeDS() = %d\n\n"), pCurInit->GetSizeDS() );
					strMsg += strTemp;

					strMsg += __FILE__;
					strTemp.Format( _T("\nLine %d\n\n"), __LINE__ );
					strMsg += strTemp;
					strMsg += _T("This message will not be shown outside of Accelerator.");

					KillableWarning( strMsg, strLabel, bShowErrorMessage );
				}
			}
			#endif

			return false;
		}

		// New items added to this object?
		if ( sCnt < pCurInit->GetSizeDS() )
			bCheckComplete = true;					// Flag a completeness check at end.

		// Use database count, and fill in any extras with defaults.
		for ( ; sDSTR < sCnt; sDSTR++ )
			dstr[ sDSTR ].ReadObject();
		for ( ; sDSTR < pCurInit->GetSizeDS(); sDSTR++ )
			dstr[ sDSTR ].Clear();

	}

	///////////////////////////////////////
	// Completeness Double-check.
	///////////////////////////////////////

	// Do we need to check the completeness of this object?
	if ( bCheckComplete )
	{
		// Check the base data completeness.
		bool bIsBaseComplete = BaseData_IsDataComplete();

		// Set the completeness flag of the object, if necessary.
		if ( bIsComplete() && !bIsBaseComplete )
		{
			SetCompleteness( false );

			// We do NOT want to call ChangeObject anywhere within
			// Read().  Read() is called during lower-level operations
			// like Compact().  The db is not able to save objects
			// during these operations.
			// The object's status has been updated, but the status
			// will have to be re-updated every time the object is
			// read, until it is saved from a propsheet, or other
			// user-driven mechanism such as Verify().
			// ChangeObject();

		}
	}

	// Check for and fix any bogus values.
	// Do NOT call ChangeObject - see above note.
	// if ( !BaseData_IsValid() )
	// 	ChangeObject();	// Save recovery changes made in IsValid().
	BaseData_IsValid();

	// Successful exit.
	return true;
}


//-------------------------------------------------------------------//
// BaseData_Write()																	//
//-------------------------------------------------------------------//
void EMComponent::BaseData_Write()
{
	short sDI = 0, sDE = 0, sDSTR = 0, sCnt, sTemp;

	// Check (and default) any values with bogus
	// units before saving them out.  Must continue
	// with the save because we don't want to lose
	// any other potentially good data.
	BaseData_IsValid();

	// If this class is derived from another EMComponent derived class,
	// then we need to loop through each class in turn to keep
	// versioning intact.
	int nClassLoop;
	int nClassCnt = pEMInit->GetNumBaseObjectTypes();

	ASSERT( nClassCnt >= 1 );

	for ( nClassLoop = 0; nClassLoop < nClassCnt; nClassLoop++ )
	{
		// Options objects have their own custom EMComponentInit
		// pointers.  Hence for non-derived classes we just take
		// the current pointer instead of indexing to the default
		// as we do below.
		EMComponentInit *pCurInit = pEMInit;

		if ( nClassCnt > 1 )
			pCurInit = EMComponentInit::ByType(
				pEMInit->GetBaseObjectType( nClassLoop )
			);

		///////////////////////////////////////
		// Write DataItem count and data.
		///////////////////////////////////////

		sCnt = pCurInit->GetSizeDI();
		ASSERT( sCnt >= sDI );			// Count should be non-negative

		WriteObject( ( sTemp = ( sCnt - sDI ) ) );
		for ( ; sDI < sCnt; sDI++ )
			di[ sDI ].WriteObject();

		///////////////////////////////////////
		// Write DataEnum count and data.
		///////////////////////////////////////

		sCnt = pCurInit->GetSizeDE();
		ASSERT( sCnt >= sDE );			// Count should be non-negative

		WriteObject( ( sTemp = ( sCnt - sDE ) ) );
		for ( ; sDE < sCnt; sDE++ )
			de[ sDE ].WriteObject();

		///////////////////////////////////////
		// Write DataString count and data.
		///////////////////////////////////////

		sCnt = pCurInit->GetSizeDS();
		ASSERT( sCnt >= sDSTR );		// Count should be non-negative

		WriteObject( ( sTemp = ( sCnt - sDSTR ) ) );
		for ( ; sDSTR < sCnt; sDSTR++ )
			dstr[ sDSTR ].WriteObject();

	}
}


//-------------------------------------------------------------------//
// BaseData_Clear()																	//
//-------------------------------------------------------------------//
void EMComponent::BaseData_Clear()
{
	short sA;

	// DataItem
	for ( sA = 0; sA < pEMInit->GetSizeDI(); sA++ )
	{
		// Set DataItem to the intial value and units.
		di[sA] = pEMInit->GetDI( sA );
		// Convert straight off to the user units.
		di[sA].SetUnits( pUnitSet->eUserUnits( pEMInit->GetUnitsDI( sA ) ) );
		// Init base DataSource
		di[sA].DataSource::Clear();
	}

	// DataEnum
	for ( sA = 0; sA < pEMInit->GetSizeDE(); sA++ )
	{
		// Set DataEnum to the intial value and units.
		de[sA] = pEMInit->GetDE( sA );
		// Init base DataSource
		de[sA].DataSource::Clear();
	}

	// DataString
	for ( sA = 0; sA < pEMInit->GetSizeDS(); sA++ )
	{
		// Clear DataString
		dstr[sA].Clear();
		// Init base DataSource
		dstr[sA].DataSource::Clear();
	}

}


//-------------------------------------------------------------------//
// BaseData_Copy()																	//
//-------------------------------------------------------------------//
void EMComponent::BaseData_Copy( EMComponent *pSrcEMC )
{
	int nA;

	// Check (and default) any values with bogus
	// units before saving them out.  Must continue
	// with the copy because we don't want to lose
	// any other potentially good data.
	pSrcEMC->BaseData_IsValid();

	for ( nA = 0; nA < pEMInit->GetSizeDI(); nA++ )
		di[nA] = pSrcEMC->di[nA];

	for ( nA = 0; nA < pEMInit->GetSizeDE(); nA++ )
		de[nA] = pSrcEMC->de[nA];

	for ( nA = 0; nA < pEMInit->GetSizeDS(); nA++ )
		dstr[nA] = pSrcEMC->dstr[nA];

}


//-------------------------------------------------------------------//
// BaseData_IsValid()																//
//-------------------------------------------------------------------//
// This function is NOT a completeness check - it only ensures that	//
// the units are still at what they were initilized at.  Basically	//
// checks to see if anything has been galloping through memory and	//
// trashing things, since this should NEVER return false.				//
//																							//
// NOTE: This function has been modified to fix trashed values by		//
// initializing them to their defaults, since we may get hits here	//
// due to changes within versions that we want to recover from.		//
//-------------------------------------------------------------------//
bool EMComponent::BaseData_IsValid()
{
	static bool bShowErrorMessage = true;

	bool	bRetVal = true;
	short sA;

	// DataItem
	for ( sA = 0; sA < pEMInit->GetSizeDI(); sA++ )
	{
		// Check to make sure that the base units are correct.
		if (
				( pUnitSet->eBaseClass( di[sA].eUnits() ) )
			!= ( pUnitSet->eBaseClass( pEMInit->GetUnitsDI( sA ) ) )
		){
			// If there is a mismatch, try to resolve it by attempting to
			// convert the value across units classes that are compatible.
			// This uses a function in pUnitSet that is programmed to handle
			// specific problems that have arisen across specific versions
			// of Earthmover, and is not made to handle every possible case.
			SI_UNIT eNewUnits =
				pUnitSet->ResolveBaseClassError(
					di[sA].eUnits(),
					pEMInit->GetUnitsDI( sA )
				);

			// Did return value specify a match had been found?
			// Convert the value to the new units.
			if ( eNewUnits != SI_NULL )
				di[sA].SetUnits( eNewUnits );

			// No match was found - we have an error.  Either a new case needs
			// to be added to ResolveBaseClassError() function to handle legitimate
			// conflicts, or the DataItem read from the DB was indeed total
			// garbage and the true error resides either in a bogus DB read
			// or in the process writing out the DataItem to the DB.
			// Convert the DataItem to its default value and flag the error.
			else
			{
				#ifdef ACCELERATOR_INHOUSE
				{
					if ( bShowErrorMessage )
					{
						CString strTemp = GetDisplayNameFromTree( GetObjectType(), &GetRef() );
						CString strLabel = _T("DataItem Validation Error");
						CString strMsg = _T("Object Type: ");

						strMsg += pEMInit->strDescSingular() + _T("\n");
						strMsg += _T("Object Name: ") + strTemp + _T("\n\n");

						if ( UINT uiRes = pEMInit->uiLabelResDI( sA ) )
							strTemp.LoadString( uiRes );
						else
							strTemp.Format( _T("DataItem %d"), sA );

						strMsg += _T("DataItem: ") + strTemp  + _T("\n");
						strMsg += _T("Database units: ") + pUnitSet->strUnitString( di[sA].eUnits(), SI_NULL ) + _T("\n");
						strMsg += _T("Default units: ") + pUnitSet->strUnitString( pEMInit->GetUnitsDI(sA), SI_NULL ) + _T("\n\n");

						strMsg += _T("This message will not be shown outside of Accelerator.");

						KillableWarning( strMsg, strLabel, bShowErrorMessage );
					}
				}
				#endif

				di[sA] = pEMInit->GetDI( sA );
				bRetVal = false;
			}
		}
	}

	// DataEnum
	for ( sA = 0; sA < pEMInit->GetSizeDE(); sA++ )
	{
		// Check to make sure that the units are correct and
		// the value falls within the range for their units.
		// Convert the DataEnum to its default value and flag the error.
		if ( ( de[sA].eUnits() != pEMInit->GetUnitsDE( sA ) ) || !de[sA].FindSubTable() )
		{
			#ifdef ACCELERATOR_INHOUSE
			{
				if ( bShowErrorMessage )
				{
					CString strTemp = GetDisplayNameFromTree( GetObjectType(), &GetRef() );
					CString strLabel = _T("DataEnum Validation Error");
					CString strMsg = _T("Object Type: ");

					strMsg += pEMInit->strDescSingular() + _T("\n");
					strMsg += _T("Object Name: ") + strTemp + _T("\n\n");

					if ( UINT uiRes = pEMInit->uiLabelResDE( sA ) )
						strTemp.LoadString( uiRes );
					else
						strTemp.Format( _T("DataEnum %d"), sA );

					strMsg += _T("DataEnum: ") + strTemp +
						_T("\n\nThis message will not be shown outside of Accelerator.");

					KillableWarning( strMsg, strLabel, bShowErrorMessage );
				}
			}
			#endif

			de[sA] = pEMInit->GetDE( sA );
			bRetVal = false;
		}
	}

	// DataString

	// ( No validity checking done for strings )

	return bRetVal;
}


//-------------------------------------------------------------------//
// BaseData_IsComplete()															//
//-------------------------------------------------------------------//
// For those base data items that are flagged for completeness			//
// checks, check to make sure that they have values.						//
// RETURNS:																				//
//   bool			: overall completeness true / false.					//
//   p_ar_strErr	: if passed, will add error strings to the array.	//
//-------------------------------------------------------------------//
bool EMComponent::BaseData_IsDataComplete(
	CStringArray*	p_ar_strErr			,		// Array to pass error strings back through.
	bool*				pbFoundRevEngItems,		// true if we found items that may be rev-eng'ed
	int				nIndent						// # of tabs to add before error string.
) {
	short sA;
	bool bRetVal = true;
	UINT uiRes;
	CString strErr, strIndent = strIndentString( nIndent );

	// DataItem
	for ( sA = 0; sA < pEMInit->GetSizeDI(); sA++ )
	{
		// Check complete flag.
		if ( pEMInit->bNeedCompleteDI( sA ) )
		{
			DataItem &rdi = di[ sA ];
			enum { diZero, diOutOfRange, diOK };
			int nState = diOK;

			if ( rdi.IsZero() )
				nState = diZero;
			else
			{
				DI_Init &Init = pEMInit->GetInitDI( sA );

				// If diMax and diMin are the same then this DataItem does not
				// need to go through min/max checking by design.
				if ( Init.dMin() != Init.dMax() )
				{
					// Check against the initializer min and max
					if (
							( rdi < DataItem( Init.dMin(), Init.eUnits() ) )
						|| ( rdi > DataItem( Init.dMax(), Init.eUnits() ) )
					){
						nState = diOutOfRange;
					}
				}
			}

			if ( nState != diOK )
			{
				bRetVal = false;
				// If error string array passed, do string build.
				if ( p_ar_strErr )
				{
					if ( uiRes = pEMInit->uiLabelResDI( sA ) )	// Try labeled version first
						strErr.LoadString( uiRes );
					else
						strErr.Format( _T("DataItem %d"), sA );

					if ( nState == diZero )
						strErr += ResStr( IDM_DATAITEM_NO_VALUE );
					else
						strErr += ResStr( IDM_DATAITEM_OUT_OF_RANGE );

					// Get the details about the requirements for this DI.
					DI_REQ_STATUS eReqStatus = pEMInit->GetInitDI( sA ).eReqStatus();
					ASSERT( eReqStatus != DIR_NEVER );

					// DIR_HELPFUL is not yet implemented.  Implement as needed!
					// We will need the name of the data item, plus information about
					// what advantages there are in providing the number.
					ASSERT( eReqStatus != DIR_HELPFUL );

					// Is this value a rev eng candidate?
					// Note that we cannot rely on the parent to fill public objects' DataItems,
					// so we treat DIR_REV_ENG like DIR_ALWAYS for public objects.
					if ( eReqStatus == DIR_REV_ENG && bIsPrivate() )
					{
						strErr += strRevEngCandidateMarker;
						*pbFoundRevEngItems = true;
					}

					// Add this DI to the list.
					p_ar_strErr->Add( strIndent + strErr );

				}
				// If no array passed, can bag out now.
				else
					return false;
			}
		}
	}

	// DataEnum
	for ( sA = 0; sA < pEMInit->GetSizeDE(); sA++ )
	{
		if (
				( pEMInit->bNeedCompleteDE( sA ) )		// Check complete flag.
			&&	( de[sA].IsZero() )
		){
			bRetVal = false;
			if ( p_ar_strErr )							// If no array passed, can skip string build.
			{
				if ( uiRes = pEMInit->uiLabelResDE( sA ) )	// Try labeled version first
					strErr.LoadString( uiRes );
				else
					strErr.Format( _T("DataEnum %d"), sA );

				p_ar_strErr->Add( strIndent + strErr + ResStr( IDM_DATAENUM_HAS_NO_SELECTION ) );
			}
			else
				return false;									// If no array passed, can bag out now.
		}
	}

	// DataString
	for ( sA = 0; sA < pEMInit->GetSizeDS(); sA++ )
	{
		if (
				( pEMInit->bNeedCompleteDS( sA ) )		// Check complete flag.
			&&	( dstr[sA].IsZero() )
		){
			bRetVal = false;
			if ( p_ar_strErr )								// If no array passed, can skip string build.
			{
				if ( uiRes = pEMInit->uiLabelResDS( sA ) )	// Try labeled version first
					strErr.LoadString( uiRes );
				else
					strErr.Format( _T("DataString %d"), sA );

				p_ar_strErr->Add( strIndent + strErr + ResStr( IDM_DATASTRING_IS_BLANK ) );
			}
			else
				return false;									// If no array passed, can bag out now.
		}
	}

	return bRetVal;
}




//-------------------------------------------------------------------//
// BaseData_SetSource()																//
//-------------------------------------------------------------------//
void EMComponent::BaseData_SetSource( bool bUserEnteredOnly )
{
	short sA;

	DataItem	*pDI = di;	// Use ptr to save CArray lookup time.
	for ( sA = 0; sA < pEMInit->GetSizeDI(); sA++ )
	{
		pDI->SetSource( dsDefaultDataSource, bUserEnteredOnly );
		pDI++;
	}

	DataEnum *pDE = de;
	for ( sA = 0; sA < pEMInit->GetSizeDE(); sA++ )
	{
		pDE->SetSource( dsDefaultDataSource, bUserEnteredOnly );
		pDE++;
	}

	DataString *pDS = dstr;
	for ( sA = 0; sA < pEMInit->GetSizeDS(); sA++ )
	{
		pDS->SetSource( dsDefaultDataSource, bUserEnteredOnly );
		pDS++;
	}
}


//----------------------------------------------------------------------------------//
//																												//
// SUB-OBJECT HANDLING:																					//
//																												//
//----------------------------------------------------------------------------------//

//-------------------------------------------------------------------//
// SubObj_Release()																	//
//-------------------------------------------------------------------//
// Releases all memory associated with the SubObj array.					//
// Also handles SubObj lists.														//
//-------------------------------------------------------------------//
void EMComponent::SubObj_Release()
{
	int nA, nB;

	// Using a local EMSubObject pointer to save array lookups.
	EMSubObject *pSub;

	// Handle single subobjects in SubObj array.
	pSub = SubObj.GetData();
	for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
	{
		ASSERT( ! ( bIsPropSheet() && pSub->IsInMemoryOnly() ) );
		pSub->ClearPtr();

		pSub++;
	}

	// Handle subobject lists.
	for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ )
	{
		pSub = SubObjList[ nA ].GetData();
		for ( nB = 0; nB < GetListSize( nA ); nB++ )
		{
			pSub->ClearPtr();
			pSub++;
		}
	}

}

//-------------------------------------------------------------------//
// SubObj_Clear()																		//
//-------------------------------------------------------------------//
// Releases all memory associated with the SubObj array and blanks	//
// all object references.  Also handles SubObj lists.						//
//-------------------------------------------------------------------//
void EMComponent::SubObj_Clear()
{
	int nA, nSize;

	// Using a local EMSubObject pointer to save array lookups.
	EMSubObject *pSub;

	pSub = SubObj.GetData();
	nSize = pEMInit->GetSubObjSize();

	// Scan through sub objects.
	for ( nA = 0; nA < nSize; nA++ )
	{
		pSub->Clear();
		pSub++;
	}

	// Handle subobject lists.
	for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ )
		SubObjList_RemoveAll( nA );

}

//-------------------------------------------------------------------//
// DeletePrivateSubObjects()														//
//-------------------------------------------------------------------//
// Delete any private SubObjects of this object.
//-------------------------------------------------------------------//
void EMComponent::DeletePrivateSubObjects()
{
	int nA, nB;

	// Scan through sub-objects one by one.
	for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
	{
		EMSubObject &Sub = GetSubObj( nA );
		if ( Sub.IsPrivate() )
			Sub.RemovePrivateObject();
	}

	// Scan through sub-object lists one by one.
	for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ )
	{
		for ( nB = 0; nB < GetListSize( nA ); nB++ )
		{
			EMSubObject &Sub = GetSubObj( nA, nB );
			if ( Sub.IsPrivate() )
			{
				Sub.RemovePrivateObject();
				SubObjList[ nA ].RemoveAt( nB );	// Remove EMSubObject struct from CArray.
				nB--;											// Back up loop counter.
			}
		}
	}
}


//-------------------------------------------------------------------//
// SubObj_Read()																		//
//-------------------------------------------------------------------//
bool EMComponent::SubObj_Read()
{
	Short sSize, sSubSize;
	Short sA, sB;

	// Using a local EMSubObject pointer to save array lookups.
	EMSubObject *pSub;

	///////////////////////////////////////
	// SubObject read
	///////////////////////////////////////

	bool bUsesFlags = false;

	// Currently loaded SubObj's no longer valid.
	SubObj_Clear();


	// If this class is derived from another EMComponent derived class,
	// then we need to loop through each class in turn to keep
	// versioning intact.
	int nClassLoop;
	int nClassCnt = pEMInit->GetNumBaseObjectTypes();

	ASSERT( nClassCnt >= 1 );

	pSub = SubObj.GetData();
	sA = 0;

	for ( nClassLoop = 0; nClassLoop < nClassCnt; nClassLoop++ )
	{
		EMComponentInit *pCurInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nClassLoop ) );

		// Read the # of SubObj's for version checking - any
		// new SubObjects were cleared above.
		ReadObject( sSize );

		// EMSubObject Flag fields.
		// To save from writing a lot of blank flag fields
		// out to the Earthmover DB I use a Size of -1 to
		// signify that flag fields need to be read at all.
		// The real Size is the next Short immediately following.
		if ( sSize == -1 )
		{
			ReadObject( sSize );
			bUsesFlags = true;
		}
		sSize += sA;

		// If you don't pass this test then the database is
		// trashed, or a component change has reorganized
		// the order at which items are read from the db.
		// Either way the ObjectReferences being read will
		// be complete garbage - this code just avoids
		// the first exception fault.
		if (
				( sSize < sA )
			|| ( sSize > pCurInit->GetSubObjSize() )
		){

			#ifdef ACCELERATOR_INHOUSE
			{
				static bool bShowErrorMessage = true;
				if ( bShowErrorMessage )
				{
					CString strTemp = GetDisplayNameFromTree( GetObjectType(), &GetRef() );
					CString strLabel = _T("SubObject size error");
					CString strMsg = _T("Object Type: ");

					strMsg += pEMInit->strDescSingular() + _T("\n");
					strMsg += _T("Object Name: ") + strTemp + _T("\n\n");

					strTemp.Format( _T("sA = %d\n"), sA );
					strMsg += strTemp;
					strTemp.Format( _T("sSize = %d\n"), sSize );
					strMsg += strTemp;
					strTemp.Format( _T("pCurInit->GetSubObjSize() = %d\n\n"), pCurInit->GetSubObjSize() );
					strMsg += strTemp;

					strMsg += __FILE__;
					strTemp.Format( _T("\nLine %d\n\n"), __LINE__ );
					strMsg += strTemp;
					strMsg += _T("This message will not be shown outside of Accelerator.");

					KillableWarning( strMsg, strLabel, bShowErrorMessage );
				}
			}
			#endif

			return false;
		}

		// Loop through all the SubObjects, and read the
		// ones in from the database that we can.
		for ( ; sA < sSize; sA++ )
		{
			// Read in the ObjectReference for this SubObject.
			ReadObject( pSub->ObRef() );

			// Read in EMSubObject flag field(s) if necessary.
			if ( bUsesFlags )
				ReadObject( pSub->ubDBFlags() );
			else
				pSub->ubDBFlags() = 0x00;

			pSub++;
		}

		// Skip past any that might have been added in a
		// newer version of Earthmover - they were cleared
		// in the call to SubObj_Clear() above.
		for ( ; sA < pCurInit->GetSubObjSize(); sA++ )
			pSub++;
	}

	///////////////////////////////////////
	// SubObject lists read
	///////////////////////////////////////

	sA = 0;

	for ( nClassLoop = 0; nClassLoop < nClassCnt; nClassLoop++ )
	{
		EMComponentInit *pCurInit =
			EMComponentInit::ByType( pEMInit->GetBaseObjectType( nClassLoop ) );

		// Read the # of SubObj arrays for version checking - any
		// new SubObjects were cleared above.
		ReadObject( sSize );
		sSize += sA;

		// If you don't pass this test then the database is
		// trashed, or a component change has reorganized
		// the order at which items are read from the db.
		// Either way the ObjectReferences being read will
		// be complete garbage - this code just avoids
		// the first exception fault.
		if (
				( sSize < sA )
			|| ( sSize > pCurInit->GetNumLists() )
		){

			#ifdef ACCELERATOR_INHOUSE
			{
				static bool bShowErrorMessage = true;
				if ( bShowErrorMessage )
				{
					CString strTemp = GetDisplayNameFromTree( GetObjectType(), &GetRef() );
					CString strLabel = _T("SubObject list size error");
					CString strMsg = _T("Object Type: ");

					strMsg += pEMInit->strDescSingular() + _T("\n");
					strMsg += _T("Object Name: ") + strTemp + _T("\n\n");

					strTemp.Format( _T("sA = %d\n"), sA );
					strMsg += strTemp;
					strTemp.Format( _T("sSize = %d\n"), sSize );
					strMsg += strTemp;
					strTemp.Format( _T("pCurInit->GetNumLists() = %d\n\n"), pCurInit->GetNumLists() );
					strMsg += strTemp;

					strMsg += __FILE__;
					strTemp.Format( _T("\nLine %d\n\n"), __LINE__ );
					strMsg += strTemp;
					strMsg += _T("This message will not be shown outside of Accelerator.");

					KillableWarning( strMsg, strLabel, bShowErrorMessage );
				}
			}
			#endif

			return false;
		}

		// Loop through all the SubObjects, and read the
		// ones in from the database that we can.
		for ( ; sA < sSize; sA++ )
		{
			// Read in the size of this list.
			ReadObject( sSubSize );

			if ( ( sSubSize < 0 ) || ( sSubSize >= 500 ) )
			{
				// Garbage list size!!!
				ASSERT( false );

				// Avoid the exception.  Perhaps the poor user can
				// recover by just deleting the object and creating
				// a new one from scratch.
				sSubSize = 0;
			}

			// Expand the CArray to the correct size.
			SubObjList[ sA ].SetSize( sSubSize );

			pSub = SubObjList[ sA ].GetData();

			SubObj_Init *pInit = pEMInit->GetSubObjListsInit( sA );

			// Loop through all the SubObject in this list.
			for ( sB = 0; sB < sSubSize; sB++ )
			{
				pSub->pInit() = pInit;

				// Read in the ObjectReference for this SubObject.
				ReadObject( pSub->ObRef() );

				// Read in EMSubObject flag field(s) if necessary.
				if ( bUsesFlags )
					ReadObject( pSub->ubDBFlags() );
				else
					pSub->ubDBFlags() = 0x00;

				pSub++;
			}
		}

		// It's possible that, due to versioning, sSize is not
		// equal to pCurInit->GetNumLists().  We correct sA for
		// this now, so that multi-level EMComponent derived
		// classes will keep correct list alignment.
		sA = pCurInit->GetNumLists();

	}

	return true;
}

//-------------------------------------------------------------------//
// SubObj_Write()																		//
//-------------------------------------------------------------------//
void EMComponent::SubObj_Write()
{
	Short sA, sB;
	Short sSize, sSubLists, sSubSize, sTemp;

	// Using a local EMSubObject pointer to save array lookups.
	EMSubObject *pSub;

	// EMSubObject Flag fields.
	// To save from writing a lot of blank flag fields
	// out to the Earthmover DB I use a Size of -1 to
	// signify that flag fields need to be read at all.
	// The real Size is the next Short immediately following.

	// Determine if any flags are set at all in the EMSubObject arrays.
	uByte ubFlagsSet = 0x00;

	sSize			= pEMInit->GetSubObjSize();
	sSubLists	= pEMInit->GetNumLists();

	pSub = SubObj.GetData();
	for ( sA = 0; sA < sSize; sA++ )
	{
		ubFlagsSet |= pSub->ubDBFlags();
		pSub++;
	}

	for ( sA = 0; sA < sSubLists; sA++ )
	{
		sSubSize = GetListSize( sA );
		pSub = SubObjList[ sA ].GetData();
		for ( sB = 0; sB < sSubSize; sB++ )
		{
			ubFlagsSet |= pSub->ubDBFlags();
			pSub++;
		}
	}

	// Flags were set?  Write out a -1 for the size so that the
	// EMComponent::SubObj_Read() function knows that flags
	// will need to be read in.
	if ( ubFlagsSet )
	{
		sA = -1;
		WriteObject( sA );
	}

	///////////////////////////////////////
	// SubObject Write
	///////////////////////////////////////

	// If this class is derived from another EMComponent derived class,
	// then we need to loop through each class in turn to keep
	// versioning intact.
	int nClassLoop;
	int nClassCnt = pEMInit->GetNumBaseObjectTypes();

	ASSERT( nClassCnt >= 1 );

	pSub = SubObj.GetData();
	sA = 0;

	for ( nClassLoop = 0; nClassLoop < nClassCnt; nClassLoop++ )
	{
		EMComponentInit *pCurInit =
			EMComponentInit::ByType( pEMInit->GetBaseObjectType( nClassLoop ) );

		sSize	= pCurInit->GetSubObjSize();

		// Write out the # of SubObj's for automatic versioning.
		WriteObject( ( sTemp = ( sSize - sA ) ) );

		// Write out the ObjectReferences.
		for ( ; sA < sSize; sA++ )
		{
			WriteObject( pSub->ObRef() );

			if ( ubFlagsSet )
				WriteObject( pSub->ubDBFlags() );

			pSub++;
		}

	}

	///////////////////////////////////////
	// SubObject lists write.
	///////////////////////////////////////

	sA = 0;

	for ( nClassLoop = 0; nClassLoop < nClassCnt; nClassLoop++ )
	{
		EMComponentInit *pCurInit =
			EMComponentInit::ByType( pEMInit->GetBaseObjectType( nClassLoop ) );

		sSubLists = pCurInit->GetNumLists();

		// Write out the # of SubObj's for automatic versioning.
		WriteObject( ( sTemp = ( sSubLists - sA ) ) );

		// Write out the arrays.
		for ( ; sA < sSubLists; sA++ )
		{
			sSubSize = GetListSize( sA );
			WriteObject( sSubSize );
			pSub = SubObjList[ sA ].GetData();

			for ( sB = 0; sB < sSubSize; sB++ )
			{
				WriteObject( pSub->ObRef() );

				if ( ubFlagsSet )
					WriteObject( pSub->ubDBFlags() );

				pSub++;
			}
		}
	}
}

//-------------------------------------------------------------------//
// LoadObject()																		//
//-------------------------------------------------------------------//
void EMComponent::LoadObject( ObjAddr nd )
{

	Persistent::LoadObject( nd );

	// This function called at the end of EMComponent::LoadObject()
	// to allow for any versioning that needs to take place *after*
	// the object has been completely loaded and all derived classes
	// have finished their nested Read() calls.  This is the correct
	// place to put any EMComponent::***DatabaseUpdate() calls.
	if ( bReadOK )
		PostLoadVersionUpdate();
}


//-------------------------------------------------------------------//
// SaveObject()																		//
//-------------------------------------------------------------------//
// Here, we verify the save request, invalidate the associated
// cache and clean up private objects before saving through the
// base class.
//-------------------------------------------------------------------//
bool EMComponent::SaveObject()
{

	// This ensures that the user has not requested incompatible
	// save actions.  It will ASSERT() if so.
	VerifySaveRequest();

	// Note that we need to open the file for writing
	// in order to handle the subobjects.  Therefore, we
	// wrap everything in our own open/close group.  That
	// way, we nest the open/closes, and only one occurs.
	// Note that we only open if needed.  We need to go
	// through SaveObject even if we don't save, to reset
	// variables, but we should avoid opening the file.
	// This bypass will cascade through the subobjects.
	bool bOpen = ( changed || newobject || deleted );
	if ( bOpen ) {

		pDatabase->Open( true );

		// Since we know we will be changing the database contents
		// for this object, the previous tree cache is no longer valid.
		// Get the cache corresponding to our object type and set it
		// to invalid.
		// TO DO
		// Consider keeping track of cache status in memory, and bypass
		// invalidation if already invalid.
		DBTreeCache BadCache(
			GetObjectType(),
			pDatabase,
			false,			// bTestValidity
			true				// bInvalidate
		);
	}

	//////////////////////////////////////////////////////////////////
	//
	//	PRIVATE OBJECT HANDLING
	//
	//////////////////////////////////////////////////////////////////

	// Delete any private SubObjects of this object.
	if ( deleted )
	{
		DeletePrivateSubObjects();
	}

	// If this is a Property Sheet object, then we need to
	// do some special handling for private objects.
	else if (
			( bIsPropSheet()			)
		&&	( newobject || changed	)
	){
		SaveSubObjConfigChanges();
	}

	//////////////////////////////////////////////////////////////////
	//	END: PRIVATE OBJECT HANDLING
	//////////////////////////////////////////////////////////////////

	// Save the object.
	bool bReturn = Persistent::SaveObject();

	// Wait until after all the saves to close.
	if ( bOpen )
		pDatabase->Close( true );

	return bReturn;

}

//-------------------------------------------------------------------//
// ValidateAndSave()																	//
//-------------------------------------------------------------------//
// Silly function created to remove some duplicate code.  Changes
// to an object are usually followed by various calls to
// SubObj_Validate(), IsDataComplete(), DoCalcOtherData(),
// BuildDisplayName(), AddObject(), ChangeObject(), SaveObject(),
// etc.  This function wraps all those, and then updates the
// tree with the correct function calls.
//
// Notice the various options available through the arguments.
// The default args will do the standard save, calc and update tree.
//-------------------------------------------------------------------//
ObjectReference EMComponent::ValidateAndSave(
	bool					bAdd,				  		// Add it as a new object?
	bool					bDiffRef,		  		// Copy over an existing object?
	ObjectReference	*pSaveAs,		  		// The object to copy over.
	bool					bUpdateTree,	  		// Update tree sort & name?
	bool					bErrorUpdateTree,		// Update tree if errors found?
	CStringArray*		p_ar_strErr,			// Array for completeness error strings.
	bool*					pbFoundRevEngItems,	// true if we found items that may be rev-eng'ed
	int					nIndent					// # of tabs to add before error string.
) {

	EMComponent *pNewEMC;
	ObjectReference ObRef;

	bool bError = false;

	bool bPreSaveRefresh = bRefresh();

	bool bNewComplete = false;

	// Do a check here for bReadOK.  If this object is being
	// added then bReadOK will never have been set correctly,
	// hence the OR condition below.  Otherwise, assume that
	// if bReadOK is false then this object is garbage and
	// we definitely do not want to fall into the calculation
	// functions below.  Because bNewComplete will be false
	// the DoCalcOtherData() call will be skipped as well.
	if ( bReadOK || bAdd )
	{
		// Have to save out any in-memory sub-objects before
		// we can validate them.
		if ( bIsPropSheet() )
			SaveSubObjConfigChanges();

		// Make sure all sub-objects still match up.
		// Don't turn off refreshing yet.
		SubObj_Validate();

		// Sort the sub-object lists as required.
		SortSubObjectLists();

		// We are about to do major calculations that may involve
		// multiple accesses to subobjects.  It is now safe to
		// turn off refreshing until the save is done.
		SetRefresh( false );

		// Here we have MINIMAL exception handling for floating point errors.
		// If you update this, also update CSpecDoc::RefreshSpecTable()
		// exception handling.  And the other try/catch below.

		// If we are inhouse then let the exception puke where it lay so
		// that we can catch it in the debugger or with mapfile unravelling.
		#ifndef ACCELERATOR_INHOUSE
		try
		#endif
		{
			// "Reverse-engineer" to fill in configuration-dependant values.
			// Also set the resulting DoRevEng() status flags.
			m_ar_strDoRevEngMsgs.RemoveAll();					// Clear the previous msgs.
			SetOptimizeStandard();
			SetRevEngComplete( DoReverseEngineering() );
			SetCalledDoRevEng();
			SetOptimizeStandard( false );

			// Before checking completeness, we want to update the status
			// of all subobjects that were modified by DoReverseEngineering().
			UpdateRevEngSubObjects();

			// Get completeness status.
			// Note that this will also check the RevEngComplete flag that
			// we just set above.
			bNewComplete = IsDataComplete(
				p_ar_strErr,
				pbFoundRevEngItems,
				nIndent
			);
		}

		// If we are inhouse then let the exception puke where it lay so
		// that we can catch it in the debugger or with mapfile unravelling.
		#ifndef ACCELERATOR_INHOUSE

		// Catch overflows here.
		catch( int nMSError )
		{
			// Display a msg that includes the exception code.
			// Search for STATUS_FLOAT_INVALID_OPERATION for codes.
			CString strMsg;
			strMsg.Format(
				_T("%s\nException code %d"),
				CString( LPTSTR( IDM_SPEC_UNKNOWN_EXCEPTION_ERROR ) ),
				nMSError
			);
			DisplayMessage( strMsg );
		}

		// Catch all exceptions here.
		catch( ... )
		{
			DisplayMessage( IDM_SPEC_UNKNOWN_EXCEPTION_ERROR );
		}

		#endif
	}

	// This object was not read correctly.
	else
	{
		// Add an error string to any passed error string array.
		if ( p_ar_strErr )
			p_ar_strErr->Add( strIndentString( nIndent ) + ResStr( IDM_DB_READ_FAILURE ) );

	}

	bError = ( bNewComplete != bIsComplete() );

	// Set the completeness flag of the object.
	SetCompleteness( bNewComplete );

	// It is now safe to turn off refreshing now until we are done with the save.
	SetRefresh( false );

	// Fill in calculated data fields from the changes.
	if ( bIsComplete() )
	{
		// Here is where we do all our calcs.
		// We'll trap for errors if not in-house.

		#ifdef ACCELERATOR_INHOUSE
		{
			DoCalcOtherData();
		}
		#else
		{
			// Here we have MINIMAL exception handling for floating point errors.
			// If you update this, also update CSpecDoc::RefreshSpecTable()
			// exception handling.  And the other try/catch above.
			try
			{
				DoCalcOtherData();
			}

			// Catch overflows here.
			catch( int nMSError )
			{
				// Display a msg that includes the exception code.
				// Search for STATUS_FLOAT_INVALID_OPERATION for codes.
				CString strMsg;
				strMsg.Format(
					_T("%s\nException code %d"),
					CString( LPTSTR( IDM_CALC_UNKNOWN_EXCEPTION_ERROR ) ),
					nMSError
				);
				DisplayMessage( strMsg );
			}

			// Catch all exceptions here.
			catch( ... )
			{
				DisplayMessage( IDM_CALC_UNKNOWN_EXCEPTION_ERROR );
			}

			// Try to continue.
			// TO DO
			// Seems to continue OK on NT, but not Win9x.  Anything
			// we can do about it?

		}
		#endif

		// Rebuild the object's display name.
		BuildDisplayName();

	}

	if ( bAdd )
	{
		ASSERT( !pSaveAs );	// Should always be NULL here.

		// Create a new object that is a copy of this one.
		if ( bDiffRef )
		{
			pNewEMC = pEMDBArray->NewComponent( GetObjectType() );

			pNewEMC->Copy(
				this,		// Source EMComponent pointer.
				true,		// Duplicate private objects in the DB?
				false		// Copy SubObj backup arrays too?
			);

			ObRef	= pNewEMC->ValidateAndSave( true );

			delete pNewEMC;
			SetRefresh( bPreSaveRefresh );
			return ObRef;
		}

		// Or just saving our current object.
		AddObject();
	}

	else
	{
		// Open the selected object from the DB and stuff it with
		// the contents of this one.  Save it.
		if ( pSaveAs )
		{
			ASSERT( bDiffRef );	// Should always be true here.

			pNewEMC = pEMDBArray->NewComponent( GetObjectType(), pSaveAs );

			pNewEMC->Copy(
				this,		// Source EMComponent pointer.
				true,		// Duplicate private objects in the DB?
				false		// Copy SubObj backup arrays too?
			);

			ObRef = pNewEMC->ValidateAndSave( false );

			delete pNewEMC;
			SetRefresh( bPreSaveRefresh );
			return ObRef;
		}

		// Or just saving our current object.
		ChangeObject();
	}

	// Save the object to the DB.
	VERIFY( SaveObject() );

	if ( bUpdateTree || ( bError && bErrorUpdateTree ) )
	{
		DBTreeControl *pTree = DBTreeControl::GetTreePtr( GetObjectType() );

		if ( bAdd )
		{
			// Update the tree with this new object.
			pTree->AddObjectToTree( this );
		}

		// Update the tree with the object changes.
		else
		{
			// Get its spot in the linked list.
			POSITION Pos = pTree->FindListItem( &GetRef() );

			// Private objects need updating to the list only.
			if ( bIsPrivate() )
			{
				// Delete the old item (if found).
				pTree->DeleteItemData( Pos );

				// And add it anew.
				pTree->AddObjectToTree( this );
			}

			// Public objects can just go through UpdateObject.
			else
			{
				pTree->UpdateObject( this, NULL, true, true, Pos );
			}
		}
	}

	// Reset refresh status.
	SetRefresh( bPreSaveRefresh );

	return GetRef();
}



//-------------------------------------------------------------------//
// SubObj_Copy()																		//
//-------------------------------------------------------------------//
// This function handles the SubObject portion of copying one
// EMComponent derived object to another.
//-------------------------------------------------------------------//
void EMComponent::SubObj_Copy(
	EMComponent		*pSrcEMC,			// Source EMComponent pointer.
	bool				bDupePriv,			// Duplicate private objects in the DB?
	bool				bDupeBackups,		// Copy SubObj backup arrays too?
	SwapRefArray	*parSwap				// Used internally by EMComponent to update linked refs.
){

	int nA, nB, nSubSize, nSize, nLists;
	EMSubObject	*pSrcSub, *pDestSub;
	SwapRefArray arSwap;


	// Before getting into the nitty gritty, we need to
	// know if the database this object will be saved to
	// has been connected to.  This will be false if the
	// object is currently being published, since when
	// you publish a DB you are only connected to the
	// working version, not the published version.  This
	// bool gets passed along to DuplicateDBObject, so
	// that it in turn will or will not update the
	// trees.

	// Finds the index of the passed pointer within the
	// Databases array.  Returns -1 if not found.
	bool bDestIsInConnectedDB =
		( -1 != pEMDBArray->GetDBIndex( (EMDatabase*) pDatabase ) );

	// If you hit this assert then something is majorly wrong,
	// two different types of objects are being copied to each other.
	ASSERT( GetObjectType() == pSrcEMC->GetObjectType() );

	// Currently loaded SubObj's no longer valid.
	SubObj_Clear();

	nSize 	= pEMInit->GetSubObjSize();
	nLists	= pEMInit->GetNumLists();

	//
	// COPY SUB-OBJECT REFERENCES
	//

	// Using a local EMSubObject pointer to save array lookups.
	pSrcSub	= pSrcEMC->SubObj.GetData();
	pDestSub	= SubObj.GetData();

	// Loop through all the SubObjects:
	for ( nA = 0; nA < nSize; nA++ ) {

		// Copies the ref, flags, and initializer ptrs.
		pDestSub->Copy( pSrcSub, true );

		//ASSERT( pSrcSub->IsPrivate() ==
		//	GetCompletenessFromTree( pSrcSub->pInit->eType(), pSrcSub->ObRef() )
		//);

		// If the source SubObj is private then we need
		// to make a new object in the DB that is a copy
		// of that object and stuff the new ObRef into the
		// destination SubObject array.
		if (
				( bDupePriv						)
			&&	( pSrcSub->IsPrivate()		)
			&&	( pSrcSub->IsManagedHere()	)
		){
			pDestSub->ObRef() =
				((EMDatabase*) pDatabase)->
					DuplicateDBObject(
						TypedRef(
							pSrcSub->ObRef(),
							pSrcSub->pInit()->eType()
						),
						bDestIsInConnectedDB,
						&arSwap
					);

			arSwap.Add(
				SwapRefSet(
					pSrcSub->ObRef(),
					pDestSub->ObRef(),
					pSrcSub->pInit()->eType()
				)
			);
		}

		pDestSub++;
		pSrcSub++;

	}

	//
	// COPY SUB-OBJECT LIST REFERENCES
	//

	// Loop through all the SubObject lists:
	for ( nA = 0; nA < nLists; nA++ ) {

		// Set the CArray to be the same size as the pSrcEMC->
		nSubSize = pSrcEMC->GetListSize( nA );
		SubObjList_RemoveAll( nA );
		SubObjList[ nA ].SetSize( nSubSize );

		pSrcSub	= pSrcEMC->SubObjList[ nA ].GetData();
		pDestSub	= SubObjList[ nA ].GetData();

		SubObj_Init *pInit = pEMInit->GetSubObjListsInit( nA );

		// Loop through all the SubObjects:
		for ( nB = 0; nB < nSubSize; nB++ ) {

			ASSERT( pSrcSub->pInit() == pInit );

			// Copies the ref, flags, and initilizer ptrs.
			pDestSub->Copy( pSrcSub, true );

			// If the source SubObj is private then we need
			// to make a new object in the DB that is a copy
			// of that object and stuff the new ObRef into the
			// destination SubObject array.
			if (
					( bDupePriv					)
				&&	( pSrcSub->IsPrivate()	)
			){
				pDestSub->ObRef() =
					((EMDatabase*) pDatabase)->
						DuplicateDBObject(
							TypedRef(
								pSrcSub->ObRef(),
								pEMInit->GetSubObjType( nA, nB )
							),
							bDestIsInConnectedDB,
							&arSwap
						);

				arSwap.Add(
					SwapRefSet(
						pSrcSub->ObRef(),
						pDestSub->ObRef(),
						pSrcSub->pInit()->eType()
					)
				);
			}

			pDestSub++;
			pSrcSub++;

		}

	}

	//
	// COPY SUB-OBJECT BACKUP ARRAYS
	//

	if ( bDupeBackups )
	{
		if ( pSrcEMC->bIsPropSheet() )
		{
			// Transfer SubObject Backup Lists.
			SetIsPropSheet( true );
			DeleteSubObjBackup();
			SubObjBackup.SetSize( nLists + 1 );

			for ( nA = 0; nA < ( nLists + 1 ); nA++ )
			{
				nSubSize = pSrcEMC->SubObjBackup[ nA ].GetSize();
				SubObjBackup[ nA ].SetSize( nSubSize );

				pSrcSub	= pSrcEMC->SubObjBackup[ nA ].GetData();
				pDestSub	= SubObjBackup[ nA ].GetData();

				for ( nB = 0; nB < nSubSize; nB++ )
					*(pDestSub++) = *(pSrcSub++);
			}

		}
		else if ( bIsPropSheet() )
		{
			SetPropSheetObject();
		}
	}


	SwapRefs( arSwap );

	if ( parSwap )
		parSwap->Append( arSwap );
}

//-------------------------------------------------------------------//
// SwapRefs()																			//
//-------------------------------------------------------------------//
// This function scans through the non-list SubObjects of type
// eSwapType and swaps any instances of SeekRef with SwapRef.
//-------------------------------------------------------------------//
void EMComponent::SwapRefs( SwapRefArray &arSwap )
{
	RefScanData Data;	// Use the default params to scan ALL refs.

	while ( GetNextRef( Data ) )
	{
		for ( int nA = 0; nA < arSwap.GetSize(); nA++ )
		{
			SwapRefSet &Set = arSwap[ nA ];

			if (
					( Data.Type == Set.m_eType )
				&&	( *( Data.pRef ) == Set.m_SeekRef )
			){
				*( Data.pRef ) = Set.m_SwapRef;
			}
		}
	}
}


//-------------------------------------------------------------------//
// Gather()																				//
//-------------------------------------------------------------------//
// Note: An object is ONE of the following:
//
//		Private
//		Public
//
//	Also, it belongs to ONE of the following types of databases:
//
//		Published
//		Working
//
// This function gathers public objects.  It gathers them from
// working databases only, unless inclusion of published objects
// is specifically requested.
//-------------------------------------------------------------------//
bool EMComponent::Gather(
	bool					bIncludePublished,
	EMDatabase*			pGatheringDB,
	EMDatabase*			pGatheringFromDB,
	BaseProgressDlg*	pProgressDlg,
	fstream*				pLogFile,
	int					nLogIndents
) {

	bool bObjectUpdated = false;
	bool bSubObjUnavailable = false;

	CString strBuffer;
	CString strTemp;
	CString strIndents;
	bool bParentObjectLogged = false;

	if ( pLogFile && pLogFile->is_open() )
	{
		// Set up the indents string.
		strIndents.Empty();
		for ( int i = 0; i < nLogIndents; i++ )
			strIndents += _T("\t");

		// Add the name of the top-level object.
		if ( !pGatheringDB )
		{
			CString strDBHeader;
			pDatabase->GetDatabaseHeader( &strDBHeader );
			bParentObjectLogged = true;
			GetDisplayName( &strTemp );
			strBuffer.Format(
				IDS_GATHER_HEADER_LOG,
				strTemp,
				pEMInit->ByType( GetObjectType() )->strDescSingular(),
				GetID(),
				strDBHeader
			);
			strBuffer = strIndents + strBuffer;
			pLogFile->write( LPCSTR( strBuffer ), strBuffer.GetLength() );
		}

	}

	// Vars for tracking progress through the top-level object only.
	int nRefCount = 0;
	int nCurrentRef = 0;

	// Gather() is called with the default db param of 0 for the top level
	// object being gathered.
	if ( !pGatheringDB )
	{
		// We will set the db pointer to the current db.  When we call Gather()
		// for subsequent subobjects, we will specify OUR database as the gathering
		// db target, so all the duplication ends up in the top-level object's db.
		pGatheringDB = (EMDatabase*) pDatabase;

		// If a progress dlg was requested, only show it at the top level, and
		// only if there are enough objects being gathered to warrant it.
		// Otherwise, delete the progress dlg now.
		if ( pProgressDlg )
		{
			// Since we know we are at the top-level object, we can count up the immediate
			// Refs and prepare a progress bar if there are quite a few.
			// We dynamically allocate it, since we'll be recursing, to avoid massive
			// copies of RefScanData on the stack.
			RefScanData* pData = new RefScanData(
				false,					// bSubobjectsOnly
				false,					// bIncludeBlankRefs
				false						// bIncludeUnavailableRefs
			);

			while( GetNextRef( *pData ) )
				nRefCount++;

			if ( nRefCount > 3 )
			{
				pProgressDlg->UpdateProgress( 0 );
				pProgressDlg->SetText( IDS_GATHER_SEARCHING );
			} else
			{
				pProgressDlg->DestroyWindow();
				delete pProgressDlg;
				pProgressDlg = 0;
			}

			// Clean up.
			delete pData;
		}
	}

	// Loop through all Refs within this object.  We want all valid Refs.  We
	// skip blanks and unavailables, as we couldn't do anything with them
	// anyway.
	RefScanData Data(
		false,					// bSubobjectsOnly
		false,					// bIncludeBlankRefs
		false						// bIncludeUnavailableRefs
	);

	while( GetNextRef( Data ) )
	{

		// Make sure RefScanData is behaving.
		ASSERT( !Data.pRef->IsBlank() );

		// Output each subobject ref.
		if ( pLogFile && pLogFile->is_open() && ( Data.GetSO() != -1 ) )
		{
			strTemp.Format(
				IDS_GATHER_CHECKING_1_LOG,
				strIndents,
				pEMInit->ByType( Data.Type )->strDescSingular()
			);

			strBuffer.Format(
				IDS_GATHER_CHECKING_2_LOG,
				strTemp,
				Data.pRef->ObID,
				Data.pRef->DBID.UserNumber,
				Data.pRef->DBID.DBNumber
			);

			pLogFile->write( LPCSTR( strBuffer ), strBuffer.GetLength() );

		}

		// If we have a progress bar, update it.
		if ( pProgressDlg )
		{
			strBuffer.Format(
				IDS_GATHER_EXAMINING_OBJECT_PROGRESS,
				pEMInit->ByType( Data.Type )->strDescSingular(),
				Data.pRef->ObID
			);
			pProgressDlg->SetText( strBuffer );
			if ( nRefCount )
				pProgressDlg->UpdateProgress( nCurrentRef * 100 / nRefCount );
		}

		// Next, we want to make sure we don't process backpointers, unless
		// told to do so.  Backpointers are identified by the fact that their
		// OBJECT_TYPE is greater than their parent's OBJECT_TYPE.
		// Back pointers may cause circular reference loops,
		// which cause the originally gathered object to be duplicated in
		// the gather database.  So clear backpointers, unless specifically
		// told to gather them.
		if (
				Data.Type > GetObjectType()								// BACKPOINTER
			&& !pEMInit->ByType( Data.Type )->bGatherBackPtrs()	// Gather BP?
		) {

			Data.pRef->Clear();

		} else
		{

			// Get the subobject's database pointer.
			// We will use it to determine if the subobject is from a
			// published db, without having to actually LOAD the subobject.
			EMDatabase* pSubDB;
			ObjectStream* pSubObjectStreamDB;
			pEMDBArray->LookUpDatabase( &( Data.pRef->DBID ), &pSubObjectStreamDB );
			pSubDB = dynamic_cast< EMDatabase* >( pSubObjectStreamDB );

			// We asked GetNextRef() to skip unavailable refs, so we should always find the db.
			ASSERT( pSubDB != 0 );


			// Do we want to gather this subobject?
			//
			// Unless requested, we skip published objects.
			// This is because published db's gather themselves during publishing,
			// so we know that published objects only reference published subobjects.
			// Most Gather() requests just want to make sure that all references
			// are to published objects.
			//
			// MDM	3/28/2000 2:10:47 PM
			// We may only be gathering from a specific source database.  If that is
			// the case, we check pGatheringFromDB pointer directly.  We don't care about
			// its published state.
			if (
					(
							pGatheringFromDB != 0
						&& pGatheringFromDB == pSubDB
					)
				||	(
							pGatheringFromDB == 0
						&&	( bIncludePublished || !pSubDB->IsPublished() )
					)
			) {

				// The tree data will tell us if the object is private, and whether
				// we have already gather-checked this object.  We will look it up
				// later if and when it is needed, since it requires a search
				// through the tree's data list.
				DBTreeItemData* pTreeData = 0;

				// What was this for?
				/*
				// We are only interested in public objects.
				// We need to determine the private status.
				bool bSingleSO =
					Data.GetSO() >= 0										&&
					Data.GetSO() < pEMInit->GetSubObjSize();
				*/

				// We usually recurse.
				bool bRecurse = true;

				// We don't need to gather an object already in the gathering db.
				// ( We WILL recurse, however. )
				if ( !( pGatheringDB->GetDBID() == Data.pRef->DBID ) )
				{

					// We found a Ref requiring gathering.
					bObjectUpdated = true;

					// Look up the tree data now.
					pTreeData = DBTreeControl::GetTreePtr( Data.Type )->GetItemDBData( Data.pRef );

					if ( !pTreeData )
					{

						// We SHOULD have found the data, as we are only requesting valid Refs.
						// This must be a Ref to an object that was deleted.  We'll clear it.
						Data.pRef->Clear();

					// We don't gather private objects.
					// ( We WILL recurse, however. )
					} else if ( !pTreeData->bIsPrivate() )
					{

						// Look for the ref in the previously gathered array.
						if (
							!pGatheringDB->FindAndReplaceGatheredRef(
								*Data.pRef,			// We need DIRECT access to the actual ref, as we may be updating it.
								Data.Type
							)
						) {
							// We don't have this object yet, so copy it over!
							ObjectReference NewRef = pGatheringDB->DuplicateDBObject(
								TypedRef(
									*Data.pRef,
									Data.Type
								),
								true					// bDestIsInConnectedDB
							);

							// Add a new array entry.
							pGatheringDB->AddGatheredObject(
								TypedRef(
									*Data.pRef,
									Data.Type
								),
								NewRef.ObID
							);

							// If we have a progress bar, update it.
							if ( pProgressDlg )
							{
								strBuffer.Format(
									IDS_GATHER_GETTING_OBJECT_PROGRESS,
									pEMInit->ByType( Data.Type )->strDescSingular(),
									Data.pRef->ObID
								);
								pProgressDlg->SetText( strBuffer );
							}

							// If a log was requested, output the gathered object.
							if ( pLogFile && pLogFile->is_open() )
							{

								strBuffer.Format(
									IDS_GATHER_SUBOBJECT_LOG,
									strIndents,
									pEMInit->ByType( Data.Type )->strDescSingular(),
									NewRef.ObID
								);
								pLogFile->write( LPCSTR( strBuffer ), strBuffer.GetLength() );

							}

							// Update the subobject Ref.
							*Data.pRef = NewRef;

							// We'll need to re-look up the tree data, too, since we have a new Ref now.
							pTreeData = 0;

						} else {

							// This was previously gathered, no need to dig down further.
							bRecurse = false;

							if ( pLogFile && pLogFile->is_open() )
							{
								strBuffer.Format(
									IDS_GATHER_UPDATE_LOG,
									strIndents,
									pEMInit->ByType( Data.Type )->strDescSingular(),
									Data.pRef->ObID
								);
								pLogFile->write( LPCSTR( strBuffer ), strBuffer.GetLength() );

							}

						}

					}

				}

				if ( bRecurse )
				{

					// Look up the tree data now, if we don't already have it.
					if ( !pTreeData )
						pTreeData = DBTreeControl::GetTreePtr( Data.Type )->GetItemDBData( Data.pRef );

					// Note that GetNextRef() will not return any Ref that does
					// not have a DBID of a currently connected db.  This
					// test here adds to that check by determining if we can find
					// the object's tree data.
					if ( !pTreeData )
					{
						bSubObjUnavailable = true;

						// These items should have been cleared in the above code.
						ASSERT( Data.pRef->IsBlank() );

						// If a log was requested, output the skipped object.
						if ( pLogFile && pLogFile->is_open() )
						{

							strBuffer.Format(
								_T("%sSkipping  %s [REF NOT FOUND IN TREE!]\n"),
								strIndents,
								pEMInit->ByType( Data.Type )->strDescSingular()
							);
							pLogFile->write( LPCSTR( strBuffer ), strBuffer.GetLength() );

						}

					// If the object has not yet been checked, it is time to do so.
					} else if ( !pTreeData->GatherIsChecked() )
					{

						// Before we gather the subobject, mark it as checked.  This prevents
						// any circular references (e.g., a tire with a tiretest subobject with
						// the same tire as its subobject) from causing infinite loops.
						pTreeData->GatherSetChecked();

						// Gather the objects of the object specified by the (possibly new) Ref.
						// To do so, we need to load the corresponding object.
						EMComponent* pRefObj = pEMDBArray->NewComponent( Data.Type, Data.pRef );

						// Note that GetNextRef() will not return any Ref that does
						// not have a DBID of a currently connected db.  This
						// test here adds to that check by determining if we can get
						// the actual object specified by the Ref.
						ASSERT( pRefObj->ObjectExistsAndReadOK() );
						if ( pRefObj->ObjectExistsAndReadOK() )
						{

							// Check the subobject's subobjects.
							bSubObjUnavailable |= !pRefObj->Gather(
								bIncludePublished,
								pGatheringDB,
								pGatheringFromDB,
								// pProgressDlg,
								// Can't pass this along, or it might get killed
								// "down there" and then used/killed again "up here".
								NULL,
								pLogFile,
								nLogIndents + 1
							);

						}
						else
							bSubObjUnavailable = true;

						// Clean up the object now that we're done with it.
						delete pRefObj;

					} else
					{

						// If a log was requested, output the skipped object.
						if ( pLogFile && pLogFile->is_open() )
						{

							strBuffer.Format(
								_T("%sSkipping  %s\n"),
								strIndents,
								pEMInit->ByType( Data.Type )->strDescSingular()
							);
							pLogFile->write( LPCSTR( strBuffer ), strBuffer.GetLength() );

						}

					}

				}

			}

		}

		// Increment the Ref count.
		nCurrentRef++;

	}

	// We need to save any changes.
	if ( bObjectUpdated )
		ValidateAndSave(
			false,
			false,
			NULL,
			false				// bUpdateTree
		);

	// Clean up the progress dlg.
	if ( pProgressDlg )
	{
		pProgressDlg->DestroyWindow();
		delete pProgressDlg;
		pProgressDlg = 0;
	}

	// Return true only if we didn't find any unavailable refs ANYWHERE in
	// the object or its subobjects.
	return (
		!Data.bFoundUnavailableRefs		&&
		!bSubObjUnavailable
	);

}


//-------------------------------------------------------------------//
// SetPropSheetObject()																//
//-------------------------------------------------------------------//
// This function switches this object from normal operation to
// property sheet mode.  This mode is needed so that the Cancel
// button can back out of private SubObject changes on the prop
// sheet.  All private SubObject changes are kept in memory until
// Apply or OK or Cancel is pressed.
//-------------------------------------------------------------------//
void EMComponent::SetPropSheetObject()
{
	int nA, nB, nSize, nLists, nSubSize;

	SetIsPropSheet( true );
	DeleteSubObjBackup();

	nSize		= pEMInit->GetSubObjSize();
	nLists	= pEMInit->GetNumLists();

	if ( ( nSize + nLists ) == 0 )
		return;

	SubObjBackup.SetSize( pEMInit->GetNumLists() + 1 );

	// Set the CArray to be the same size as the source.
	SubObjBackup[ nLists ].SetSize( nSize );

	// Using a local EMSubObject pointer to save array lookups.
	EMSubObject *pSrcSub		= SubObj.GetData();
	EMSubObject *pDestSub	= SubObjBackup[ nLists ].GetData();

	// Loop through all the SubObjects:
	for ( nA = 0; nA < nSize; nA++ )
	{
		pDestSub->Copy( pSrcSub, true );
		pDestSub++;
		pSrcSub++;
	}

	// Loop through all the SubObject lists:
	for ( nA = 0; nA < nLists; nA++ )
	{
		// Set the CArray to be the same size as the source.
		nSubSize = GetListSize( nA );
		SubObjBackup[ nA ].SetSize( nSubSize );

		pSrcSub	= SubObjList[ nA ].GetData();
		pDestSub	= SubObjBackup[ nA ].GetData();

		// Loop through all the SubObjects:
		for ( nB = 0; nB < nSubSize; nB++ )
		{
			pDestSub->Copy( pSrcSub, true );
			pDestSub++;
			pSrcSub++;
		}
	}
}

//-------------------------------------------------------------------//
// DeleteSubObjBackup()																//
//-------------------------------------------------------------------//
// Cleans up the SubObject backup information allocated by
// EMComponent::SetPropSheetObject().
//-------------------------------------------------------------------//
void EMComponent::DeleteSubObjBackup()
{
	SubObjBackup.RemoveAll();
}

//-------------------------------------------------------------------//
// GetListSize()																		//
//-------------------------------------------------------------------//
// Returns the length of the requested SubObject List.
// Would have moved this simple function to the header file, but
// then would have had to include EMComponentInit.h in the header
// as well.  A compile time trade off for exe speed :-).
//-------------------------------------------------------------------//
int EMComponent::GetListSize( int nListIdx )
{
	return ( ( pEMInit->GetNumLists() > nListIdx ) ? SubObjList[ nListIdx ].GetSize() : 0 );
}


//-------------------------------------------------------------------//
// SetSubObjRef()																		//
//-------------------------------------------------------------------//
// Set a SubObj reference
// Returns true if the passed ref is not the same as the backup ref.
//-------------------------------------------------------------------//
bool EMComponent::SetSubObjRef(
	int					nArrayIdx,		 // Which sub object to set.
	ObjectReference	*pRef,		 	// The ref of the new sub object.
	bool					bIsPrivate,		// Is this a private object?
	bool					bIsConfigured	// Is this a temp mem object?
) {

	ASSERT( nArrayIdx < pEMInit->GetSubObjSize() );

	bool bSameAsBackup = false;

	// Use local pointer to avoid multiple CArray lookups.
	EMSubObject *pSub = &SubObj[ nArrayIdx ];

	// Copy the ref over.
	if ( !pRef )
	{
		pSub->Clear();

		// Allows derived classes to do any special handling
		// that results from the setting of a subobject ref.
		OnSetSubObjRef( nArrayIdx, true );

		return false;

	}
	else
	{
		pSub->ObRef() = *pRef;

		pSub->SetPrivate( bIsPrivate );
		pSub->SetManagedHere( bIsConfigured );
		pSub->SetModified( true );

		if ( bIsPropSheet() )
		{
			pSub->SetInMemoryOnly( bIsConfigured );
			bSameAsBackup =
				( pSub->ObRef() == SubObjBackup[ pEMInit->GetNumLists() ][ nArrayIdx ].ObRef() );
		}

		// Allows derived classes to do any special handling
		// that results from the setting of a subobject ref.
		OnSetSubObjRef( nArrayIdx, pRef->IsBlank() );
	}

	return bSameAsBackup;
}


//-------------------------------------------------------------------//
// SubObjList_AddRef()																//
//-------------------------------------------------------------------//
// Add a reference to a sub object list.
//-------------------------------------------------------------------//
int EMComponent::SubObjList_AddRef(
	int					nListIdx,	 	// Which sub object list to add to.
	ObjectReference	*pRef,		 	// The ref of the new sub object.
	bool					bIsPrivate,		// Is this a private object?
	bool					bIsConfigured	// Is this a temp mem object?
) {

	ASSERT( pRef );
	ASSERT( nListIdx < pEMInit->GetNumLists() );

	int nPos = SubObjList[ nListIdx ].Add( EMSubObject() );
	EMSubObject &Sub = SubObjList[ nListIdx ][ nPos ];

	Sub.pEMC() = NULL;
	Sub.ObRef() = *pRef;
	Sub.SetPrivate( bIsPrivate );
	Sub.SetManagedHere( bIsConfigured );
	Sub.SetModified( true );
	Sub.pInit() = pEMInit->GetSubObjListsInit( nListIdx );

	if ( bIsPropSheet() )
		Sub.SetInMemoryOnly( bIsConfigured );

	return nPos;
}

//-------------------------------------------------------------------//
// SubObjList_AddRef()																//
//-------------------------------------------------------------------//
// Add a SubObject List reference to the backup list held in
// memory for property sheet objects.  Used to ensure that
// duplicate private objects created from drag and drop
// operations across prop sheets can clean up after delete
// and cancel actions.
//-------------------------------------------------------------------//
int EMComponent::SubObjList_AddBackupRef(
	int					nListIdx,	 	// Which sub object list to add to.
	ObjectReference	*pRef,		 	// The ref of the new sub object.
	bool					bIsPrivate,		// Is this a private object?
	bool					bIsConfigured	// Is this a temp mem object?
) {

	ASSERT( pRef );
	ASSERT( nListIdx < ( pEMInit->GetNumLists() + 1 ) );
	ASSERT( bIsPropSheet() );

	int nPos = SubObjBackup[ nListIdx ].Add( EMSubObject() );
	EMSubObject &Sub = SubObjBackup[ nListIdx ][ nPos ];

	Sub.pEMC() = NULL;
	Sub.ObRef() = *pRef;
	Sub.SetPrivate( bIsPrivate );
	Sub.SetManagedHere( bIsConfigured	);
	Sub.pInit() = pEMInit->GetSubObjListsInit( nListIdx );

	// Uses this flag to determine that this object was added
	// to the backup as the result of creation of a private object
	// during a propsheet session.
	Sub.SetModified( true );

	return nPos;
}


//-------------------------------------------------------------------//
// SubObjList_DeleteRef()															//
//-------------------------------------------------------------------//
// Delete a reference from a sub object list.
//-------------------------------------------------------------------//
void EMComponent::SubObjList_DeleteRef( int nListIdx, const ObjectReference &Ref )
{
	int nIdx = SubObjList_FindRef( nListIdx, Ref );

	if ( nIdx >= 0 )
	{
		GetSubObj( nListIdx, nIdx ).ClearPtr();
		SubObjList[ nListIdx ].RemoveAt( nIdx );
	}
}

//-------------------------------------------------------------------//
// SubObjList_FindRef()																//
//-------------------------------------------------------------------//
// Finds a ref within the specified list.
// Returns the list index if found, or -1 if not found.
//-------------------------------------------------------------------//
int EMComponent::SubObjList_FindRef( int nListIdx, const ObjectReference &Ref )
{
	for ( int nA = 0; nA < GetListSize( nListIdx ); nA++ )
	{
		if ( Ref == SubObjList[ nListIdx ][ nA ].ObRef() )
		{
			return nA;
		}
	}

	return -1;
}


//-------------------------------------------------------------------//
// UpdateRevEngSubObjects()														//
//-------------------------------------------------------------------//
// This function checks for changes to subobjects.  If changes
// are found, it saves the subobject and updates the completeness
// status.  It is called after DoReverseEngineering(), in which
// subobjects are often updated.
// By updating the status of all previously loaded subobjects, we catch
// the majority of all subobject changes.  Note that external changes
// to public subobjects will not be reflected in the parent until
// publishing takes place.
//
// Warning: there's a whole lotta recursion potentially goin' on
// here.  We call pSO->m_pEMC->ValidateAndSave() on each subobject,
// which in turn calls UpdateRevEngSubObjects() again.  We minimize
// this unnecessary recursion by clearing subobject pointers once
// we are finished saving them.
//
// WARNING
// There is a chance that DoRevEng() has changed two different
// instances of one object.  For example, the std axle and current
// axle may have both been loaded up and modified, yet they may
// be the same object.  One will be saved after the other, overwriting
// any changes made to the first.  We need to avoid this in code
// in DoRevEng().  For example, if we know that the std axle is
// already loaded and potentially modified, and we need to load the
// current axle for modification, we MUST use the std in place of
// the current if they are the same.  This same rule applies to
// subobject lists.  If an object from a list is modified, we must
// not blindly load up either a std or current, as they may be the
// same object as the list subobject that was changed.
// We check for this problem below by keeping track of all saved
// subobject Ref's.  If we get a duplicate, we display a
// killable warning.  You should take this warning seriously, and
// update the code so that you no longer see it.
//-------------------------------------------------------------------//
void EMComponent::UpdateRevEngSubObjects()
{

	// Scan through sub-objects.
	EMSubObject* pSO;
	ResetSubObjectLoop();
	while ( GetNextSubObject( pSO ) )
	{
		// This is a candidate if it has been loaded up, and is
		// not from a published db.
		if (
				pSO->m_pEMC != 0
			&& pSO->m_pEMC->ObjectExistsAndReadOK()
			&&	!pSO->m_pEMC->pDatabase->IsPublished()
		) {

			// First, recurse down to the lowest subobject.
			pSO->m_pEMC->UpdateRevEngSubObjects();

			// Now, save as needed.
			if ( pSO->m_pEMC->changed || pSO->m_pEMC->newobject )
			{

				// TO DO
				// Check for more than one changed instance of an object.
				// This is lower priority.  It will help prevent coding
				// errors.  In the meantime, we must prevent them ourselves
				// without help here.
				/*
				#ifdef ACCEL_INHOUSE
					static bool bOverwriteWarn = true;

					// Iterate through copied object array looking for
					// a repeat.
					bool bRepeat = false;
					Iterator
					while( !bRepeat )
						bRepeat = ( *Iterator == *pSO->m_pEMC->GetRef() )


					if ( bRepeat )
					{

						CString strMsg;
						strMsg.Format(
							_T("There were two modified instances of the following object.\nThe changes for one of the modifications will be lost.\nYou should update the code to avoid loading two instances.\n\n  Type %d\n  %s"),
							pSO->m_pEMC->GetObjectType(),
							pSO->m_pEMC->GetDisplayName()
						);

						// WARNING!
						// If you hit this message, read the warning in the function comment block.
						KillableWarning(
							strMsg,
							CString( _T("CODING ERROR!  YOU MUST UPDATE THE CODE TO FIX IT!") ),
							bOverwriteWarn
						);

					} else
						// Now add the Ref to the list.
						RefVector

				#endif
				*/

				// Update the subobject.
				pSO->m_pEMC->ValidateAndSave();

				// Now delete the subobject, so it is not checked again.
				// We call ValidateAndSave() as we step up the parent
				// chain.  ValidateAndSave() re-calls UpdateRevEngSubObjects().
				// So this avoids gobs more recursion that we don't need.
				// If we determine that we would like to keep the subobjects
				// around, e.g., for use by DoCalcOtherData(), at the expense
				// of more recursion here, just comment out this line.
				pSO->ClearPtr();

				// Prepare the parent for saving, so changes recurse upwards.
				// If we are at the top-level parent, this won't change anything,
				// as it is already in the process of being saved.
				// We don't need to do anything for new objects, they are
				// also already slated for being saved.
				if ( ObjectExists() )
					ChangeObject();

			}

		}

	}

}


//-------------------------------------------------------------------//
// GetNextSubObject()																//
//-------------------------------------------------------------------//
// This function allows the caller to loop through all subobjects
// (both single and in subobject lists).  It fills the provided
// pointer with the next available SO and returns true, or returns
// false when no more SO's are available.
// Do not add or delete from lists while using this function, as
// the results would be unpredictable (you'll prolly go out of
// bounds).
//-------------------------------------------------------------------//
bool EMComponent::GetNextSubObject( EMSubObject*& pSO )
{

	if ( nSO < pEMInit->GetSubObjSize() )
	{
		pSO = &( GetSubObj( nSO ) );
		nSO++;
		return true;

	} else
	{

		// Look for next SO in lists.

		// While we haven't tried all the lists...
		while ( nSOList < pEMInit->GetNumLists() )
		{
			// See if the next index is available.
			if ( nSOIndex < GetListSize( nSOList ) )
			{
				// If so, get it and return.
				pSO = &( GetSubObj( nSOList, nSOIndex ) );
				nSOIndex++;
				return true;

			} else
			{
				// Check the next list.
				nSOList++;
				nSOIndex = 0;
			}

		}

	}

	// Fresh outta piglet shirts.
	return false;

}


//-------------------------------------------------------------------//
// SubObj_IsDataComplete()															//
//-------------------------------------------------------------------//
// Checks the completeness of an object's SubObjects.
//
// This function calls EMComponent::CheckSubObjCompleteness() to
// determine which SubObjects to check.  For now, lists are only
// checked for whether they contain references.  The objects
// that those references refer to are not themselves checked.  This
// is only being left out because it is not necessary yet.
//
// RETURNS:
//   bool			: overall completeness true / false.
//   p_ar_strErr	: if passed, will add error strings to the array.
//-------------------------------------------------------------------//
bool EMComponent::SubObj_IsDataComplete(
	CStringArray*	p_ar_strErr,			// Array for completeness error strings.
	bool*				pbFoundRevEngItems,	// true if we found items that may be rev-eng'ed
	int				nIndent					// # of tabs to add before error string.
){
	bool bRetVal = true;
	CString strErr, strIndent;
	int nA, nB;

	// If the error CString array is going to be used, create a
	// TypedRefArray to track what objects have been added to the
	// error array so that we do not end up duplicating the same
	// error strings because of subobjects that have standard
	// selections referencing the same object.
	TypedRefArray*	parFoundRefs = NULL;
	if ( p_ar_strErr )
	{
		strIndent = strIndentString( nIndent );
		parFoundRefs = new TypedRefArray;
	}

	////////////////////////////////////////////////////////////////////
	//
	//	SUB-OBJECTS
	//
	////////////////////////////////////////////////////////////////////

	// Scan through sub-objects and look for incompleteness.
	for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
	{
		EMSubObject &Sub = GetSubObj( nA );

		// We must consider whether an object is in memory or not.
		// If it is in memory, we may or may not have a valid Ref,
		// but we should always have a pointer to an object.
		if ( Sub.IsInMemoryOnly() )
		{
			bRetVal &= GetObjPtr( nA )->IsDataComplete( p_ar_strErr, pbFoundRevEngItems, nIndent+1 );

		// If it is persistent, we need to check to see if the Ref
		// is filled in.
		} else
		{
			// Get a local copy of info about this subobject.
			ObjectReference *pRef = &Sub.ObRef();
			OBJECT_TYPE	eSubType = pEMInit->GetSubObjType( nA );

			// Check if we have an unselected external item.
			if ( pRef->IsBlank() )
			{
				// Use CheckSubObjCompleteness() to determine if we need to
				// check this SubObject or not based upon any custom
				// code decision rules.
				if ( CheckSubObjCompleteness( NotList, nA ) )
				{
					// If array passed, do error string build.
					if ( p_ar_strErr )
					{
						bRetVal = false;

						// Returns NoStd if nA is not a Standard SubObject.
						if ( NoStd == pEMInit->IsStdSubObj( nA ) )
							strErr.Empty();
						else
							strErr = ResStr( IDS_STANDARD ) + _T(" ");

						// Build error string.
						strErr
							+=	EMComponentInit::ByType( eSubType )->strDescSingular()
							+	ResStr( IDM_SUB_OBJ_NOT_SELECTED );

						p_ar_strErr->Add( strIndent + strErr );
					}

					// If no array passed, can bag out now.  No need
					// to check all the other subobjects because the
					// return value is all that matters.
					else
					{
						return false;
					}
				}

			}

			// External subobject is present, so check its completeness.
			// This command also checks completeness of all
			// sub-objects, down through the hierarchy.
			else if ( !GetCompletenessFromTree( eSubType, pRef ) )
			{
				// If array passed, do error string build.
				if ( p_ar_strErr )
				{
					bRetVal = false;

					// Have we checked this object already?
					if ( NotFound == parFoundRefs->Search( TypedRef( *pRef, eSubType ) ) )
					{
						//	If not, add it to the array for future checks.
						parFoundRefs->Add( TypedRef( *pRef, eSubType ) );

						// Build error string.
						strErr
							=	EMComponentInit::ByType( eSubType )->strDescSingular()
							+	ResStr( IDM_SUB_OBJ_NOT_COMPLETE );

						p_ar_strErr->Add( strIndent + strErr );

						// Recursively check sub objects for completeness,
						// Indenting one more tab for each step we dig down
						// into the object hierarchy.
						EMComponent *pSubEMC = GetObjPtr( nA );

						if ( pSubEMC->ObjectExistsAndReadOK() )
							pSubEMC->IsDataComplete( p_ar_strErr, pbFoundRevEngItems, nIndent + 1 );
						else
							p_ar_strErr->Add(
									strIndentString( nIndent + 1 )
								+	ResStr( IDM_DB_READ_FAILURE )
							);
					}

				}

				// If no array passed, can bag out now.  No need
				// to check all the other subobjects because the
				// return value is all that matters.
				else
				{
					return false;
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////
	//
	//	SUB-OBJECT LISTS
	//
	////////////////////////////////////////////////////////////////////

	// Scan through sub-object lists.  If we need to check them
	// for completeness (which we usually don't) then do so.
	for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ )
	{
		// Use CheckSubObjCompleteness() to determine if we need to
		// check this SubObject list or not based upon any custom
		// code decision rules.
		if ( CheckSubObjCompleteness( nA, 0 ) )
		{
			int nListSize = GetListSize( nA );

			// Ensure that list has some objects in it.  Note that this particular
			// check could be bypassed in the derived class by returning false
			// through CheckSubObjCompleteness() if GetListSize() returns zero.
			if ( nListSize == 0 )
			{
				// If array passed, do error string build.
				if ( p_ar_strErr )
				{
					bRetVal = false;

					// Build error string.
					strErr
						=	EMComponentInit::ByType( pEMInit->GetSubObjType( nA, 0 ) )->strDescPlural()
						+	_T(" ") + ResStr( IDM_MISSING_LIST_SELECTION );

					p_ar_strErr->Add( strIndent + strErr );
				}

				// If no array passed, can bag out now.  No need
				// to check all the other subobjects because the
				// return value is all that matters.
				else
				{
					return false;
				}
			}

			// We have objects in the list....
			else
			{
				OBJECT_TYPE	eSubType = pEMInit->GetSubObjType( nA, 0 );

				// Scan through list sub-objects and look for incompleteness.
				for ( nB = 0; nB < nListSize; nB++ )
				{
					// Get a local copy of info about this subobject.
					EMSubObject &Sub = GetSubObj( nA, nB );
					ObjectReference *pRef = &Sub.ObRef();

					// If this is a memory only object then call the object
					// to check itself.
					if ( Sub.IsInMemoryOnly() )
					{
						bRetVal &= Sub.pEMC()->IsDataComplete( p_ar_strErr, pbFoundRevEngItems, nIndent+1 );
						if ( !p_ar_strErr && !bRetVal )
							return false;
					}

					// External subobject is present, so check its completeness.
					// This command alse checks completeness of all
					// sub-objects, down through the hierarchy.
					else if ( ! GetCompletenessFromTree( eSubType, pRef ) )
					{
						// If array passed, do error string build.
						if ( p_ar_strErr )
						{
							bRetVal = false;

							// Have we checked this object already?
							if ( NotFound == parFoundRefs->Search( TypedRef( *pRef, eSubType ) ) )
							{
								//	If not, add it to the array for future checks.
								parFoundRefs->Add( TypedRef( *pRef, eSubType ) );

								// Build error string.
								strErr
									=	EMComponentInit::ByType( eSubType )->strDescPlural()
									+	ResStr( IDM_SUB_OBJ_NOT_COMPLETE );

								p_ar_strErr->Add( strIndent + strErr );

								// Recursively check sub objects for completeness,
								// Indenting one more tab for each step we dig down
								// into the object hierarchy.
								EMComponent *pSubEMC = GetObjPtr( nA, nB );

								if ( pSubEMC->ObjectExistsAndReadOK() )
									pSubEMC->IsDataComplete( p_ar_strErr, pbFoundRevEngItems, nIndent + 1 );
								else
									p_ar_strErr->Add(
											strIndentString( nIndent + 1 )
										+	ResStr( IDM_DB_READ_FAILURE )
									);
							}
						}

						// If no array passed, can bag out now.  No need
						// to check all the other subobjects because the
						// return value is all that matters.
						else
						{
							return false;
						}
					}
				}
			}
		}
	}

	if ( parFoundRefs )
		delete parFoundRefs;

	////////////////////////////////////////////////////////////////////
	//
	//	MISSING OPTIONS
	//
	////////////////////////////////////////////////////////////////////

	// Only need to check for missing options if this object HAS options.
	if ( -1 != pEMInit->nOptionListIdx() )
	{
		// Get a subset of the options this object can possibly have.
		OptSet OptionSet = pEMInit->GetOptionSet();

		// Loop though all possible options type.
		for ( int nOptionType = OT_Root; nOptionType < OptTypeLen; nOptionType++ )
		{
			OptType eOptionType = (OptType) nOptionType;

			// Screen for those types that are leaves and are in the subset.
			if (
					( OptionSet.bIsSet( eOptionType ) )
				&&	( OptionManager.bIsLeaf( eOptionType )	)
			){
				// Now screen for those that require at least one option to
				// be selected.  At least one must EXIST in order for at
				// least one to be selected.  These are the ones that we
				// are checking for here.
				OptCfg eOptionCfg = OptionManager.eGetOptCfg( eOptionType );

				if ( eOptionCfg == OC_1 || eOptionCfg == OC_1_N )
				{
					// Get the parent branch.  We only need to find one of
					// the types under that branch to be "complete" for all
					// the attached leaf types.
					OptType eParent = OptionManager.eParentType( eOptionType );
					int nParent = (int) eParent;

					// Search for the first option under the parent type.
					if ( NotAvail == nFindOption( &nParent ) )
					{
						// If array passed, do error string build.
						if ( p_ar_strErr )
						{
							bRetVal = false;

							// Build error string.
							CString strComma, strErr;
							while ( eParent > OT_Root )
							{
								strErr = ResStr( OptionManager.uiNameString( eParent ) ) + strComma + strErr;
								eParent = OptionManager.eParentType( eParent );
								strComma = _T(", ");
							}
							strErr = ResStr( IDM_MISSING_OPTION ) + strErr;

							p_ar_strErr->Add( strIndent + strErr );
						}

						// If no array passed, can bag out now.  No need
						// to check all the other subobjects because the
						// return value is all that matters.
						else
						{
							return false;
						}
					}

					// Now skip past the rest of the leaves under this branch.
					// They were all covered under the check done above.
					while (
							( nOptionType < OptTypeLen )
						&& ( OptionManager.bIsLeaf( (OptType) nOptionType ) )
					){
						nOptionType++;
					}
				}
			}
		}
	}


	return bRetVal;
}


//-------------------------------------------------------------------//
// SubObj_Search()																	//
//-------------------------------------------------------------------//
// Scans through this object's sub-objects for a match with class		//
// type Type.  If ID is non-zero, then the scan will be for the		//
// specific object Type( ID ).  If bRecursive is set then this f'n	//
// will search all sub-objects of this object's sub-objects, too.		//
//																							//
// NOTE: If search type is anything but Source, then any memory		//
// alloc'd in this routine will automatically be cleaned up by			//
// SubObj_Release() in the destructor(s).  However, if you called		//
// this function looking for Sources, and passed a ResultList,			//
// then you will have to delete the sources yourself or there will	//
// be MASSIVE memory leaks.														//
//																							//
// RETURNS:																				//
//		bool		If a match for Type is found.									//
//		CArray	A list of those matches found.								//
//-------------------------------------------------------------------//
bool EMComponent::SubObj_Search(
	OBJECT_TYPE			eType,
	EMCArray				*pResultList,
	ObjectReference	*pRef,
	bool					bRecursive
) {
	EMComponent	*pSub;
	bool			bFound = false;
	int			nA, nB;
	OBJECT_TYPE	eSub;

	bool bCheckSource(
		DataSource			ds,
		EMCArray 			*pResultList,
		ObjectReference	*pRef,
		bool		 			*bFound
	);

	// Assume that OBJECT_TYPE hierarchy is valid,
	// and we don't need to check for sub objects of
	// an OBJECT_TYPE higher than our own.
	if ( eType > GetObjectType() ) return false;

	//
	// Separate search functions for SubObjects and Sources:
	// This is the SubObject (non-list) version.
	//
	if ( eType != OBJECT_SOURCE ) {

		// Scan through sub-objects one by one.
		for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ ) {

			// Found a class type match?
			eSub = pEMInit->GetSubObjType( nA );
			if ( eSub == eType ) {

				// Need to match search Reference as well?
				if ( pRef ) {
					// Does this ID match the search ID?
					if ( *GetSubObjRef( nA ) == *pRef ) {
						// Flag the return value.
						bFound = true;
						// Add to the result list.
						if ( pResultList )
							pResultList->Add( GetObjPtr( nA ) );
					}
				}

				// If ID match not required then we have a match
				// if the ID is valid.
				else {

					// Get local pointer to save CArray access and
					// allow for debugging of EMComponent* struct.
					pSub = GetObjPtr( nA );

					// Only looking for valid choices
					if ( ! GetSubObjRef( nA )->IsBlank() ) {
						// Flag the return value.
						bFound = true;
						// Add to the result list.
						if ( pResultList )
							pResultList->Add( GetObjPtr( nA ) );
					}
				}
			}

			// If class type doesn't match then check
			// to see if we need recursive scanning.
			else if ( bRecursive && ( eType < eSub ) ) {

				// Recursively scan sub-objects.
				bFound |= GetObjPtr( nA )->
					SubObj_Search( eType, pResultList, pRef, true );

			}

			// If we aren't looking for a full return list
			// then we can just return true now and save some effort.
			if ( bFound && !pResultList ) return true;

		}

		//
		// Separate search functions for SubObjects and Sources:
		// This is the SubObject (list) version.
		//

		// Scan through sub-objects one by one.
		for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ ) {

			for ( nB = 0; nB < GetListSize( nA ); nB++ ) {

				// Found a class type match?
				eSub = pEMInit->GetSubObjType( nA, nB );
				if ( eSub == eType ) {

					// Need to match search Reference as well?
					if ( pRef ) {
						// Does this ID match the search ID?
						if ( *GetSubObjRef( nA, nB ) == *pRef ) {
							// Flag the return value.
							bFound = true;
							// Add to the result list.
							if ( pResultList )
								pResultList->Add( GetObjPtr( nA, nB ) );
						}
					}

					// If ID match not required then we have a match
					// if the ID is valid.
					else {
						// Only looking for valid choices
						if ( ! GetSubObjRef( nA, nB )->IsBlank() ) {
							// Flag the return value.
							bFound = true;
							// Add to the result list.
							if ( pResultList )
								pResultList->Add( GetObjPtr( nA, nB ) );
						}
					}
				}

				// If class type doesn't match then check
				// to see if we need recursive scanning.
				else if ( bRecursive && ( eType < eSub ) ) {

					// Recursively scan sub-objects.
					bFound |= GetObjPtr( nA, nB )->
						SubObj_Search( eType, pResultList, pRef, true );

				}

				// If we aren't looking for a full return list
				// then we can just return true now and save some effort.
				if ( bFound && !pResultList ) return true;

			}
		}
	}


	//
	// Separate search functions for SubObjects and Sources:
	// This is the Source version.
	//
	else {

		// Check all the sources in this object - the default
		// and all the DataItem/DataEnum/DataString sources.
		if ( bCheckSource( dsDefaultDataSource,
			pResultList, pRef, &bFound ) ) return true;

		for ( short sA = 0; sA < pEMInit->GetSizeDI(); sA++ )	// DataItem
			if ( bCheckSource( di[sA].GetSource(),
				pResultList, pRef, &bFound ) ) return true;

		for ( sA = 0; sA < pEMInit->GetSizeDE(); sA++ )			// DataEnum
			if ( bCheckSource( de[sA].GetSource(),
				pResultList, pRef, &bFound ) ) return true;

		for ( sA = 0; sA < pEMInit->GetSizeDS(); sA++ )			// DataString
			if ( bCheckSource( dstr[sA].GetSource(),
				pResultList, pRef, &bFound ) ) return true;

	}


	return bFound;

}

//-------------------------------------------------------------------//
// CheckSource()																		//
//-------------------------------------------------------------------//
// Utility function for SubObj_Search() to try and get rid of some	//
// gobs of duplicate code.  The return value is just to tell the		//
// caller that it can bag out and quit, if possible.						//
//-------------------------------------------------------------------//
bool bCheckSource(
	DataSource			ds,
	EMCArray				*pResultList,
	ObjectReference	*pRef,
	bool					*pbFound
) {

	bool	bResult = false;

	// Check to see if we have a match.
	if ( ( pRef && ( *pRef == ds.GetSourceRef() ) ) ||
		( !pRef && ( ! ds.GetSourceRef().IsBlank() ) ) )
		bResult = true;

	// Found a match:
	if ( bResult ) {

		*pbFound = true;

		// If we have a match and we aren't looking for
		// a return list then return true to quit and
		// tell the caller that it can quit too.
		if ( !pResultList ) return true;

		if ( pRef ) {
			// Search through the ResultList.  If the current
			// Source is in the list then we can leave.
			for ( int nA = 0; nA < pResultList->GetSize(); nA++ ) {
				if ( (*pResultList)[nA]->GetRef() == *pRef ) return false;
			}
		}

		Source *pSrc = new Source( pRef );
		pResultList->Add( pSrc );
	}

	return false;
}

//-------------------------------------------------------------------//
// bIsSubObject()																		//
//-------------------------------------------------------------------//
// Searches through DB for objects that contain this one.  If			//
// bShowDlg is true, then a dialog is put up with a list of all		//
// such DB objects.  Returns true if any found.								//
//-------------------------------------------------------------------//
bool EMComponent::bIsSubObject(
	OBJECT_TYPE				eType,			// OBJECT_TYPE of search object.
	ObjectReference		&ObRef,			// ObjectReference of search object.
	bool						bShowDlg,		// Show a results dialog?
	CString					*pstrDlgTitle,	// Results dialog title.
	EMComponent_Search	*pSearch			// Search results object.
) {

	bool				bSearchThisOne;
	int				nA;

	CWaitCursor Zzzzzzzzzzzz;						// Put up the wait cursor.

	// This boolean is used to speed up certain Earthmover database
	// operations by loading only the SubObject information of objects
	// and skipping the BaseData and derived class proprietary data.
	bLoadSubObjOnly = true;

	// If no search object passed then use our local one.
	EMComponent_Search Search;						// Search dialog class.
	if ( !pSearch )
		pSearch = &Search;

	// Initialize the search dialog.
	// Note that we do not show progress dlgs unless the user
	// has requested a results summary dialog.  On program startup,
	// we call this to clean up if program crashed.  We don't want
	// all the progress bar chatter.
	pSearch->SetParameters(
		&ObRef,
		eType,
		false,
		bShowDlg					// bShowProgressDlg
	);

	// Loop through all OBJECT_TYPE objects that have their own
	// tree.  Doing this loop here eliminates the need for the
	// SubObject_SetSearchParam() virtual function that used to
	// be needed in every EMComponent derived class.

	// NOTE - used eType as the loop start instead of TYPE_TREE_LAST
	// because we assume that through the OBJECT_TYPE hierarchy that
	// we do not need to check above eType for objects that have
	// eType SubObjects.  However, these "back-pointers" do exist,
	// as in Tires point back to their TireTests, but we assume that
	// these relationships are not critical and any errors caused
	// by a lost SubObj link of this type are handled appropriately.

	for ( int eLoop = ( eType + 1 ); eLoop <= TYPE_TREE_LAST; eLoop++ ) {

		// Force the search if the search type is Source.
		bSearchThisOne = ( eType == OBJECT_SOURCE ) ? true : false ;

		EMComponentInit *pEMInit = EMComponentInit::ByType( eLoop );

		// Scan through SubObj initializers.
		for ( nA = 0; ( ( nA < pEMInit->GetSubObjSize() ) && ( !bSearchThisOne ) ); nA++ )
			if ( eType == pEMInit->GetSubObjType( nA ) )
				bSearchThisOne = true;

		// Scan through SubObjList initializers.
		for ( nA = 0; ( ( nA < pEMInit->GetNumLists() ) && ( !bSearchThisOne ) ); nA++ )
			if ( eType == pEMInit->GetSubObjType( nA, 0 ) )
				bSearchThisOne = true;

		// If a type match was found between this object's type and
		// the eLoop object's SubObjects, then search all eLoop type
		// objects for occurances of refs to this object.
		if ( bSearchThisOne )
			pSearch->SearchTree( (OBJECT_TYPE) eLoop );

	}

	// Clean up the progress dialog.
	pSearch->KillProgressDlg();

	bool bIsSubObj = false;

	// This boolean is used to speed up certain Earthmover database
	// operations by loading only the SubObject information of objects
	// and skipping the BaseData and derived class proprietary data.
	bLoadSubObjOnly = false;

	if ( pSearch->GetSize() ) {

		bIsSubObj = true;

		// Show the results dialog if required.
		if ( bShowDlg ) {
			EMComponent_SearchDlg dlg( NULL, pSearch, *pstrDlgTitle );
			dlg.DoModal();
		}

	}

	return bIsSubObj;	// This is a SubObject?
}

//-------------------------------------------------------------------//
// bAcceptSubObj()																	//
//-------------------------------------------------------------------//
// This function determines whether an object can accept another
// object as a SubObject.  Note that there are several comments
// pertaining to this function in the "SubObj struct types"
// section at the top of EMComponentInit.h.  If pnWhereToPut
// points to an integer with a value of cnSearchAllSubObjs, then
// all SubObjs and SubObjLists are searched for an appropriate
// place to put the SubObj.  If not, then the position specified
// by pnWhereToPut and pbListOnly will be the only one checked.
//
// The base class version only checks the types of SubObjs held
// within the Target for a match.  More specific checking can be done
// in the derived classes, following a call to the base version.
//
// Addendum:  This base class version now also checks for links of
// SubObjs to a SubObjList.  If one is found in the initializers,
// then the link is checked for a reference match before coninuing
// on and signifying acceptance of the SubObj.
//-------------------------------------------------------------------//
bool EMComponent::bAcceptSubObj(
	OBJECT_TYPE			eSubObjType,		// Drop OBJECT_TYPE.
	ObjectReference	*pSubObjRef,		// Drop ObRef.
	bool					*pbAcceptInList,	// Returns whether drop accepted in SubObj or SubObjList.
	int					*pnWhereToPut,		// Where we would drop the SubObj if we could.
	CString				*pstrWarning,		// Warning message to pass back to caller.
	bool					*pbListOnly,		// Passed if we know whether to check list or non-list only.
	bool					bVerifying			// Indicates whether we are verifying an existing item or adding a new
) {
	bool	bFoundMatch			= false;
	bool	bIsListObj			= false;

	int	nWhere = 0;

	// These conditions specify to search the SubObj and
	// SubObjLists to try and find an appropriate place for
	// the object.
	if ( !pnWhereToPut || !pbListOnly || ( *pnWhereToPut == cnSearchAllSubObjs ) )
	{
		// Do we know whether to check list or non-list only?
		if ( ( pbListOnly == NULL ) || ( *pbListOnly == false ) )
		{
			// Cyle through the SubObjs, search for an OBJECT_TYPE match.
			for ( nWhere = 0; nWhere < pEMInit->GetSubObjSize(); nWhere++ )
			{
				if ( eSubObjType == pEMInit->GetSubObjType( nWhere ) )
				{
					bFoundMatch = true;
					break;
				}
			}
		}

		// Do we know whether to check list or non-list only?
		if ( ( pbListOnly == NULL ) || ( *pbListOnly == true ) )
		{
			// Skip the SubObj lists if we found a hit in the SubObjs.
			if ( !bFoundMatch )
			{
				// Cyle through the SubObj Lists, search for an OBJECT_TYPE match.
				for ( nWhere = 0; nWhere < pEMInit->GetNumLists(); nWhere++ )
				{
					if ( eSubObjType == pEMInit->GetSubObjType( nWhere, 0 ) )
					{
						bIsListObj	= true;
						bFoundMatch	= true;
						break;
					}
				}

			}
		}

		if ( pnWhereToPut )
			*pnWhereToPut = nWhere;
	}

	// Calling function specified a single place to look to try and
	// fit the object within.
	else
	{
		nWhere		= *pnWhereToPut;
		bIsListObj	= *pbListOnly;

		// Check non-list only.
		if ( *pbListOnly == false )
		{
			if (
					( nWhere < pEMInit->GetSubObjSize() )
				&&	( eSubObjType == pEMInit->GetSubObjType( nWhere ) )
			)
				bFoundMatch = true;
		}

		// Check list only.
		else
		{
			if (
					( nWhere < pEMInit->GetNumLists() )
				&&	( eSubObjType == pEMInit->GetSubObjType( nWhere, 0 ) )
			)
				bFoundMatch	= true;
		}
	}


	// Special case - Manufacturers.  Check mfr type against the
	// DBTreeControl::m_eMfrScreenWhereode passed in the constructor.
	// This special case handled here to cut out %90 of the
	// derived versions of this function.  Lazy, lazy, lazy....
	if ( bFoundMatch && ( eSubObjType == OBJECT_MFR ) )
	{
		// Use DE_UNDEFINED as a signifier to accept ALL mfr catagories.
		if ( pEMInit->GetMfrScreenCode() != DE_UNDEFINED )
		{
			bFoundMatch =
				(	pEMInit->GetMfrScreenCode() ==
					DBTreeControl::GetTreePtr( OBJECT_MFR )->
						nGetSortCode( 0, pSubObjRef )
				);
		}
	}

	// Special case - Multimedia.  We ASSUME that any multimedia
	// subobjects contained by this object are of the default
	// report cover bitmap/jpeg nature.  With this assumption we
	// can handle this case here in the base class instead of
	// within every derived class that has a default cover image.
	else if ( bFoundMatch && ( eSubObjType == OBJECT_MEDIA ) )
	{
		bFoundMatch = false;

		DBTreeControl *pTree = DBTreeControl::GetTreePtr( OBJECT_MEDIA );

		HTREEITEM hItem = pTree->FindItem( pSubObjRef );
		if ( hItem )
		{
			// Make sure that this media object is equipment specific.
			if (
					( pEMInit->GetMfrScreenCode() == DE_UNDEFINED )
				|| ( pTree->nGetSortCode( MEDIA_SORT_CAT_MFR, hItem ) == pEMInit->GetMfrScreenCode() )
			){
				// Drop object's file extension string.
				CString *pstrSubObj =
					pTree->pstrGetSortString( MEDIA_SORT_CAT_FILE_EXTENSION, hItem );

				// Make sure that this multimedia object is a bitmap.
				// If it is a bitmap, then open the object up and make
				// sure that it refers to this Equipment object.
				if (
						( 0 == pstrSubObj->CompareNoCase( _T("bmp") ) )
					||	( 0 == pstrSubObj->CompareNoCase( _T("jpg") ) )
				){
					EMComponent *pMedia =
						pEMDBArray->NewComponent( OBJECT_MEDIA, pSubObjRef );

					bFoundMatch = pMedia->SubObj_Search(
						GetObjectType(),	// eType
						NULL,					// *pResultList
						&GetRef(),			// *pRef
						false					// bRecursive
					);

					if ( !bFoundMatch && pstrWarning )
						pstrWarning->LoadString( IDM_COVER_IMAGE_NOT_LINKED );

					delete pMedia;
				}

				// File type didn't match?  Stuff warning message to pass back to caller.
				else if ( pstrWarning )
					pstrWarning->LoadString( IDM_COVER_IMAGE_WRONG_FILE_TYPE );
			}
			// Manfacturer type didn't match?  Stuff warning message to pass back to caller.
			else if ( pstrWarning )
				pstrWarning->LoadString( IDM_COVER_IMAGE_WRONG_MFR_TYPE );
		}
	}


	// Special case - check for screen links in the initializer
	// arrays that we need to chase down.  The end result of this
	// mess is that the current sub obj is limited to values that
	// reside in a sub obj list.  This sub obj list can be from
	// the object itself, or in one of the object's sub objs.  The
	// array of shorts accessed through GetSubObjLink sets a path
	// that we dig down through to reach the correct list.  You
	// can dig down as far as you want in the object hierarchy,
	// but please note that the farther you dig, the slower
	// Earthmover will operate.

	else if ( bFoundMatch && !bIsListObj )
	{
		int nLink = pEMInit->GetSubObjLink( nWhere, 0 );

		// Check to see if a link path exists.
		if ( ( nLink != nNoLink ) && ( nLink != nCodeRule ) )
		{
			// Get pass-back variables ready.
			int nLinkIdx, nDepth = 0;
			EMComponent *pSubObj;
			bool bFoundInList = false;

			// Let the object check it's own SubObj Links.  Loop to allow
			// multiple links.
			while (
				( !bFoundInList ) &&
				( ResolveSubObjLink( &pSubObj, &nLinkIdx, &nDepth, nWhere ) )
			){

				// Now that we have the linked SubObjList, scan through the list and
				// look for a match to the SubObj Ref we are trying to drop.
				for ( int nA = 0; nA < pSubObj->GetListSize( nLinkIdx ); nA++ )
				{
					if ( *pSubObj->GetSubObjRef( nLinkIdx, nA ) == *pSubObjRef )
					{
						// Found a match!  The drop can be completed.
						bFoundInList = true;
						break;
					}
				}
			}

			// Our "found" criteria now depends upon whether a match was found
			// for this object in one of the linked subobj lists.
			bFoundMatch = bFoundInList;

			// Warning message to pass back to caller.
			if ( pstrWarning && !bFoundMatch )
			{
				CString strError;
				strError.LoadString( IDS_SUBOBJ_NOT_FOUND_IN_SUBOBJ_LIST );

				OBJECT_TYPE eMatchType = pEMInit->GetSubObjType( nWhere );

				pstrWarning->Format(
					strError,
					EMComponentInit::ByType( eMatchType )->strDescSingular(),
					EMComponentInit::ByType( eMatchType )->strDescPlural(),
					pEMInit->strDescSingular()
				);
			}
		}
	}

	// Special case - accept source as the default source.
	if ( eSubObjType == OBJECT_SOURCE )
		bFoundMatch = true;

	if ( pbAcceptInList )
		*pbAcceptInList = bIsListObj;

	return bFoundMatch;

}

//-------------------------------------------------------------------//
// bProcessDragArray()																//
//-------------------------------------------------------------------//
// Calls bAcceptSubObj, and goes ahead and stuffs the SubObj into
// the target object if it is accepted.  This EMComponent version
// just calls the DBTreeControl version, but we can lose a couple
// of the arguments in this version because we have the object.
//-------------------------------------------------------------------//
bool EMComponent::bProcessDragArray(
	OleDragData	*pDragData,		// Items to add to SubObj.
	CString		*pstrWarning,	// Warning message to pass back to caller.
	bool			bSaveToDB,		// Save to DB if changed?
	bool			bDeleteIfBad,	// Delete items from pDragData that were unacceptable.
	SubObjSlot	SlotToPut,		// Where we would drop the SubObj if we could.
	BYTE			eFindFlags		// Flag fields to determine search behavior.
) {
	bool bResult, bChanged = false;
	DBTreeControl *pTree = GetTreePtr();

	ASSERT( sizeof( eFindFlags ) == sizeof( teFindFlags ) );

	for ( int nA = 0; nA < pDragData->Items.GetSize(); nA++ )
	{
		TypedRef SubRef = pDragData->Items[ nA ].GetTypedRef();

		// Special default source handling.
		if (
				( OBJECT_SOURCE == SubRef )
			&& ( pTree->pEMInit->UsesDefaultSource() )
		) {
			dsDefaultDataSource.SetSourceRef( SubRef.Ref );
		}

		// Normal SubObject handling.
		else if ( SubRef < GetObjectType() )
		{
			bResult = false;

			// Find the dropped object in the appropriate DBTreeControl.
			// If it is not found, then it is a memory object and we are
			// not handling those yet.  Or maybe there's something wrong
			// with the database, in which case we still don't follow
			// through with the drop.
			DBTreeControl *pSubTree = DBTreeControl::GetTreePtr( SubRef );
			POSITION Pos = pSubTree->FindListItem( &SubRef.Ref );

			if ( Pos ) {

				// Get the private status of the dropped object.
				DBTreeItemData *pData = pSubTree->GetItemDBData( Pos );
				bool bIsPrivate = pData->bIsPrivate();

				// Get the SubObject Vector
				EMSubObjectVectorArray Vector;

				// Do we have any clues as to how to resolve the vector?
				if ( SlotToPut.bIsValid() )
					Vector.SetDirect( GetObjectType(), SlotToPut );
				else
					Vector.FindVectors( GetObjectType(), SubRef, eFindFlags );

				bResult =
					Vector.ApplyAllVectors(
						*this,			// Topmost object we are dropping to.
						SubRef,			// SubObject ObjectReference.
						pstrWarning,	// Warning string to pass back to caller.
						
						// MDM	8/1/2000 11:08:08 AM
						// Rolled in private status.
						af_StuffRef | ( bIsPrivate ? af_Private : 0 )	// flags
					);
			}

			// Delete items from pDragData that were unacceptable.
			if ( bDeleteIfBad && !bResult )
			{
				pDragData->Items.RemoveAt( nA );
				nA--;
			}

			bChanged |= bResult;
		}
	}

	// Save to DB if changed?
	if ( bChanged && bSaveToDB )
		ValidateAndSave( false );

	return bResult;
}



//-------------------------------------------------------------------//
// GetTreePtr()																		//
//-------------------------------------------------------------------//
// Gets the tree for this object.  Just a shortcut.
//-------------------------------------------------------------------//
DBTreeControl *EMComponent::GetTreePtr() {

	OBJECT_TYPE eTreeType = GetObjectType();

	if ( eTreeType > TYPE_TREE_LAST )
		return NULL;

	return DBTreeControl::GetTreePtr( eTreeType );
}


//-------------------------------------------------------------------//
// eCreateChart()																		//
//-------------------------------------------------------------------//
// See comments in ReportSource.cpp
//-------------------------------------------------------------------//
ChartState EMComponent::eCreateChart(
	int			nChartIdx,		// Chart # to create
	int			nNumObjs,			// # of EMComponent derived objects in chart.
	C2DChart		*pChart,			// Chart to modify for creation.
	int			*pnTitleRes,	// Title of chart.
	int			*pnBranchRes,	// Chart type string.
	EMIntArray	*parNesting,	// Used to track "nested" subobject charts.  Used by 2DChart_Tree only.
	OBJECT_TYPE	eType				// The type of the topmost object, may be used to determine which charts to show.
){
	ASSERT( nNumObjs > 0 );

	// Initialize the resource label string indexes to 0.  They may
	// be set later if a valid chart index was found, but we want them
	// to be zero unless they were specifically set.
	*pnTitleRes = 0;
	//*pnBranchRes = 0;

	//////////////////////////////////////////////////////////////////////////////
	// DataItem Bar Charts
	//////////////////////////////////////////////////////////////////////////////
	// If the passed chart index is less than the number of DataItems in this
	// object, then this chart is a simple bar chart of EMComponent based DataItems.
	// Handle it here and pass back an appropriate return value.
	if ( nChartIdx < pEMInit->GetSizeDI() )
	{
		// Only allow bar charts for comparisons, not specs.
		if ( nNumObjs == 1 )
			return CG_UNAVAILABLE;

		// If a branch title is requested then return one
		// for the first DataItem bar chart to create the branch.
		if ( pnBranchRes && !nChartIdx )
			*pnBranchRes = IDS_COMPARISON_BAR_CHARTS;

		// Only allow bar charts for those DataItems we
		// flagged in the initializer data as being chartable.
		//if ( !pDI_Init[ nChartIdx ].bComplete )
		//	return CG_UNAVAILABLE;

		UINT uiLabelRes = pEMInit->uiLabelResDI( nChartIdx );

		// Only allow bar charts for those DataItems we have a label resource string for.
		if ( !uiLabelRes )
			return CG_UNAVAILABLE;

		// If chart pointer passed then add chart data for bar chart.
		if ( pChart )
		{
			pChart->Settings(	CHART_2D_BAR_GRAPH, CHART_2D_COLORS_AST_GREYS, CHART_2D_LEGEND_NONE );
			pChart->m_Data.Add_Bar( di[ nChartIdx ], strReportTitle( 1, 2 ) );
		}

		// If title resource requested then use the initializer
		// resource string label as the title string.
		if ( pnTitleRes )
			*pnTitleRes = uiLabelRes;

		// Valid chart created for this nChartIdx.
		return CG_AVAILABLE;

	}
	//////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////
	// Derived Class Charts
	//////////////////////////////////////////////////////////////////////////////
	// If the passed chart index is less than the EMComponentInit set number
	// of charts for this object, but was not a DataItem chart, then it is
	// a derived-class specific chart and needs to be handled in the derived
	// class.  The CG_SPECIFIC return value tells the derived class to do so.
	else if ( nChartIdx < pEMInit->nCharts() + pEMInit->GetSizeDI() )
	{
		return CG_SPECIFIC;
	}
	//////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////
	// SubObject Nested Charts
	//////////////////////////////////////////////////////////////////////////////
	// If the passed chart index is greater than the EMComponentInit set
	// number of charts for this object, then perhaps the chart index
	// represents a chart in a subobject of this object.  Try to find it here.
	else if ( nChartIdx < pEMInit->nNestedCharts() )
	{
		// Adjust the chart index.
		nChartIdx -= pEMInit->nCharts();
		nChartIdx -= pEMInit->GetSizeDI();

		// Loop through the subobjects.
		for ( int nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
		{
			// Avoid standard objects, they are just placeholders.
			if ( NoStd == pEMInit->IsStdSubObj( nA ) )
			{
				EMComponentInit *pSubInit =
					pEMInit->ByType( pEMInit->GetSubObjType( nA ) );

				// Get the count of nested charts for this subobject.
				int nCharts = pSubInit->nNestedCharts();

				// If the chart is definitely in this subobject, then
				// get a pointer to that object and call it to handle the
				// chart itself.
				if ( nChartIdx < nCharts )
				{
					if ( parNesting )
						parNesting->Add( pSubInit->uiDescSingular() );

					EMComponent *pSubEMC = GetObjPtr( nA );

					if ( pSubEMC->ObjectExistsAndReadOK() && pSubEMC->bIsComplete() )
					{
						return
							pSubEMC->eCreateChart(
								nChartIdx, nNumObjs, pChart, pnTitleRes, pnBranchRes, parNesting, eType
							);
					}
					else
					{
						return CG_UNAVAILABLE;
					}
				}
				// Otherwise adjust the chart index again and try the next subobject.
				else
				{
					nChartIdx -= nCharts;
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////

	// No valid chart created for this nChartIdx.
	return CG_OUT_OF_RANGE;
}

//-------------------------------------------------------------------//
// nDrawGraphic()																		//
//-------------------------------------------------------------------//
// See ReportSource.cpp for comments.
// This overrid version handles the default graphics (bmp/jpg)
// automatically for those objects that have them.
//-------------------------------------------------------------------//
int EMComponent::nDrawGraphic(
	int		nGraphicIdx,	// Graphic # to create.
	int		nNumObjs,		// # of objects in graphic.
	int		*pnTitleRes,	// Title of graphic.
	CDC		*pDC,				// Device context to draw within.  If NULL, then dont draw.
	CRect		rect,				// Rectangle to draw within.
	int		nAnimStep,		// Animation step to draw (mostly unused).
	GraphicContext eContext	// Where this graphic is being used.
) {
	ASSERT( nNumObjs > 0 );

	if (
			( pEMInit->nDefaultImageIdx() != NoImage		)
		&&	( pEMInit->nDefaultMultimedia() != NoMMedia	)
		&&	( pEMInit->nDefaultImageIdx() == nGraphicIdx )
	){

		if ( pnTitleRes ) *pnTitleRes = IDS_DEFAULT_BITMAP_IMAGE;

		if ( pDC )
		{
			Media *pMedia = (Media*) GetObjPtr( pEMInit->nDefaultMultimedia() );
			pMedia->bDrawImage( pDC, rect, eContext );
		}

		return 1;	// All is well, give the OK sign of one animation step....
	}

	return CG_OUT_OF_RANGE;
}

//-------------------------------------------------------------------//
// FillWithTestData()																//
//-------------------------------------------------------------------//
// This function initializes the object using the nTestNumber
// count: the values of all the data items are set to the test
// number; the enums are selected based on the number % enum max; the
// strings contain a representation of the number; the subobjects
// are chosen based on the number, looping through the subobjects until
// the "numbereth" item is found; and subobject lists are stuffed with
// one item based on the "numbereth" item.
//-------------------------------------------------------------------//
void EMComponent::FillWithTestData(
	int nTestNumber
) {

	// Fill base data items with test number.
	int i;

	for ( i = 0; i < pEMInit->GetSizeDI(); i++ )
		di[i] = nTestNumber;

	for ( i = 0; i < pEMInit->GetSizeDE(); i++ )
		de[i].SetVal( nTestNumber % DataEnum::GetMaxOffset( pEMInit->GetUnitsDE( i ) ) );

	CString strData;
	strData.Format( _T("Test Object %i"), nTestNumber );
	for ( i = 0; i < pEMInit->GetSizeDS(); i++ )
		dstr[i] = strData;

	// Note:
	// We're making an educated guess at the subobject Refs; if there
	// were objects already in the database before the stuff, these
	// will be off.  Worst case, an old ID has been deleted and we
	// will end up with a bogus Ref.  But as long as we are aware of
	// this, it is better than scanning for the nth object, because
	// that is toooo slow.

   // Select single subobjects based on test number.
	int nSubObjCount = pEMInit->GetSubObjSize();
	for ( i = 0; i < nSubObjCount; i++ ) {
		SubObj[i].ObRef() = ObjectReference(
			pDatabase->GetDBID(),					// Stuff the current DBID, so we have a valid Ref
			nTestNumber
		);
	}

	// Add a subobject to each subobject list based on test number.
	for ( i = 0; i < pEMInit->GetNumLists(); i++ ) {
		SubObjList_RemoveAll( i );
		SubObjList_AddRef(
			i,
			&ObjectReference(
				pDatabase->GetDBID(),					// Stuff the current DBID, so we have a valid Ref
				nTestNumber
			)
		);
	}

}


//-------------------------------------------------------------------//
// operator==()																		//
//-------------------------------------------------------------------//
//	This function compares all EMComponent data.
// This is currently only used during testing, to verify that a
// test object was written properly.
//-------------------------------------------------------------------//
bool EMComponent::operator==(
	EMComponent& SrcEMC
){

	bool bReturn = true;

	int i;
	for ( i = 0; i < pEMInit->GetSizeDI() && bReturn == true; i++ ) {
		bReturn = ( di[i] == SrcEMC.di[i] );
		ASSERT( bReturn );
	}

	for ( i = 0; i < pEMInit->GetSizeDE() && bReturn == true; i++ ) {
		bReturn = ( de[i] == SrcEMC.de[i] );
		ASSERT( bReturn );
	}

	for ( i = 0; i < pEMInit->GetSizeDS() && bReturn == true; i++ ) {
		bReturn = ( dstr[i].strOutput() == SrcEMC.dstr[i].strOutput() );
		ASSERT( bReturn );
	}

	int nSubObjCount = pEMInit->GetSubObjSize();
	for ( i = 0; i < nSubObjCount && bReturn == true; i++ ) {

		bReturn = ( SubObj[i].ObRef() == SrcEMC.SubObj[i].ObRef() );
		ASSERT( bReturn );

	}

	for ( i = 0; i < pEMInit->GetNumLists() && bReturn == true; i++ ) {
		bReturn = ( *( GetSubObjRef( i, 0 ) ) == *( SrcEMC.GetSubObjRef( i, 0 ) ) );
		ASSERT( bReturn );
	}

	return bReturn;

}


//-------------------------------------------------------------------//
// GetObjectType()																	//
//-------------------------------------------------------------------//
// This function provides on the fly type identification.
// You may use the base class's GetClassID to get the ClassID, or
// this function to get the OBJECT_TYPE.  This function calls
// GetClassID and then converts the result.
//-------------------------------------------------------------------//
OBJECT_TYPE EMComponent::GetObjectType()
{
	ASSERT(
			( pEMInit->GetObjectType() )
		==	( (EMDatabase*)pDatabase )->GetObjectType( GetClassID() )
	);

	return pEMInit->GetObjectType();
}


//-------------------------------------------------------------------//
// FillComponentTree()																//
//-------------------------------------------------------------------//
// This function adds this object and its sub-objects to the
// Component Tree passed through the pointer.  We allow each
// object to add itself so that any special cases can be
// handled by the object itself.  The default EMComponent version
// adds the object and all of its sub objects according to the
// booleans in the sub object initializer arrays, so overriding
// this function will be the exception and not the rule.
//-------------------------------------------------------------------//
HTREEITEM EMComponent::FillComponentTree(
	ConfiguratorTreeControl	*pTree,				// Tree we are filling.
	HTREEITEM					hParent,				// HTREEITEM of the parent node.
	bool							bAddSelfFirst,		// Add 'this' object first?
	OBJECT_TYPE					eRootType,			// Root object type.
	OBJECT_TYPE					eParentType			// Immediate parent object type.
) {
	int					nA, nB, nPos;
   CString				strName;
	OBJECT_TYPE			eSubType;
	UINT					uiBmpID;
	EMComponent			*pEMC;
	ObjectReference	*pRef;
	EMComponentInit	*pSubInit;
	bool					bPrivate;

	// Have this object add itself to the tree,
	// if so requested by the bool arg above.
	if ( bAddSelfFirst )
	{
		eRootType = GetObjectType();
		eParentType = GetObjectType();

		// Get the display name.
		GetDisplayName( &strName );

		if ( strName.IsEmpty() )
			strName.LoadString( IDS_OBJECT_BLANK_NAME );

		if ( pTree->m_bAllowTypesWithNames )
			strName = pEMInit->strDescSingular() + _T(" : ") + strName;

		hParent = pTree->AddObject(
			hParent,						// Parent tree item.
			GetObjectType(),			// Object type.
			&GetRef(),					// Object reference.
			&strName,					// Object name string.
			pEMInit->uiTreeIconID()	// Tree bitmap resource id.
		);

	}

	///////////////////////////////////////////////////////////////
	//
	// SubObjects
	//
	///////////////////////////////////////////////////////////////

	// Loop through (possible) position vectors.
	for ( nPos = 0; nPos < pEMInit->GetSubObjSize(); nPos++ )
	{
		// Now loop through all of this object's subobjs.
		// Call the subobjs in turn to add their subobjs.
		for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
		{
			// Match position vector.
			if ( pEMInit->GetShowPos( nA ) == nPos )
			{
				eSubType = pEMInit->GetSubObjType( nA );
				pSubInit	= EMComponentInit::ByType( eSubType );
				pRef		= GetSubObjRef( nA );
				uiBmpID	= pSubInit->uiTreeIconID();
				bPrivate	= GetSubObj( nA ).IsPrivate();

				if (
					EnableComponentItem(
						eRootType,			// Root object type.
						eParentType,		// Immediate parent object type.
						NotList,				// The SubObj List (or NotList for SubObj).
						nA						// The SubObj (List) Idx
					)
				){

					// Call the subobj to add its subobjs.
					if ( pSubInit->bHasConfigurables() )
						pEMC = GetObjPtr( nA );
					else
						pEMC = NULL;

					// If there is no sub object selected yet...
					if ( pRef->IsBlank() )
					{
						// If the SubObject is optional, use "[none]".
						if ( IsSubObjOptional( NotList, nA ) )
							strName.LoadString( IDS_SUB_OBJECT_NONE_SELECTED );

						// Otherwise, use "[unspecified]".
						else
							strName.LoadString( IDS_SUB_OBJECT_UNSPECIFIED );
					}

					// Or if sub object name is blank then use "[unnamed]".
					else
					{
						if ( pEMC )
							pEMC->GetDisplayName( &strName );
						else
							GetDisplayNameFromTree( &strName, eSubType, pRef );

						if ( strName.IsEmpty() )
							strName.LoadString( IDS_OBJECT_BLANK_NAME );

						if ( pTree->m_bAllowTypesWithNames )
							strName = EMComponentInit::ByType( eSubType )->strDescSingular() + _T(" : ") + strName;
					}

					HTREEITEM hSubObj = pTree->AddObject(
						hParent,		// Parent tree item.
						eSubType,	// Object type.
						pRef,			// Object reference.
						&strName,	// Object name string.
						uiBmpID,		// Tree bitmap resource id.
						nA,			// Position of this object in the subobj array.
						NotList,		// Is this a subobj or subobj list?
						this,			// Parent EMComponent pointer.
						bPrivate		// Is this object private?
					);

					// Call the subobj to add its subobjs.
					if ( pEMC )
						pEMC->FillComponentTree( pTree, hSubObj, false, eRootType, eParentType );
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////
	//
	// List SubObjects
	//
	///////////////////////////////////////////////////////////////

	// Loop through (possible) position vectors.
	for ( nPos = 0; nPos < pEMInit->GetNumLists(); nPos++ )
	{
		// Now loop through all of this object's list subobjs.
		// Call the subobjs in turn to add their subobjs.
		for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ )
		{
			// Match position vector.
			if ( pEMInit->GetShowPos( nA, 0 ) == nPos )
			{
				for ( nB = 0; nB < GetListSize( nA ); nB++ )
				{
					if (
						EnableComponentItem(
							eRootType,			// Root object type.
							eParentType,		// Immediate parent object type.
							nA,					// The SubObj List (or NotList for SubObj).
							nB						// The SubObj (List) Idx
						)
					){

						eSubType = pEMInit->GetSubObjType( nA, nB );
						pSubInit	= EMComponentInit::ByType( eSubType );
						pRef		= GetSubObjRef( nA, nB );
						uiBmpID	= pSubInit->uiTreeIconID();
						bPrivate	= GetSubObj( nA, nB ).IsPrivate();

						// If there is no sub object selected yet...
						if ( pRef->IsBlank() )
						{
							// If the SubObject is optional, use "[none]".
							if ( IsSubObjOptional( nA, nB ) )
								strName.LoadString( IDS_SUB_OBJECT_NONE_SELECTED );

							// Otherwise, use "[unspecified]".
							else
								strName.LoadString( IDS_SUB_OBJECT_UNSPECIFIED );
						}

						// Or if sub object name is blank then use "[unnamed]".
						else
						{
							GetDisplayNameFromTree( &strName, eSubType, pRef );

							if ( strName.IsEmpty() )
								strName.LoadString( IDS_OBJECT_BLANK_NAME );

							if ( pTree->m_bAllowTypesWithNames )
								strName = EMComponentInit::ByType( eSubType )->strDescSingular() + _T(" : ") + strName;
						}

						///////////////////////////////////////////////////////////////
						//
						// Options
						//
						///////////////////////////////////////////////////////////////

						// Special handling for Options - add additional
						// tree branches to break Options up into special
						// groups.

						HTREEITEM hCurParent = hParent;
						OptType eType = OT_Null;
						OptType eChildType = OT_Null;

						if ( eSubType == OBJECT_OPTION )
						{
							Option *pOption = (Option*) GetObjPtr( nA, nB );
							EMSubObject	&EMSub = GetSubObj( nA, nB );

							eType	= pOption->eGetOptType();

							OptTreeNode	&Node		= OptionManager.GetNode( eType );
							OptVector	Vector	= Node.GetVector();

							HTREEITEM hChild, hCur = hParent;

							// Loop through all the branches in the vector and make
							// sure that they exist in the tree.
							for ( int nC = 0; nC < Vector.GetSize(); nC++ )
							{
								OptType eChildType = Vector[ nC ];

								// If the allowable types can be defined as a subset
								// of a single type (say OT_Engine) then clip that
								// particular branch, since an OT_Engine branch under
								// the Option branch under the Engine object would be
								// boringly redundant.
								if (
										( pEMInit->GetOptionTypes().GetSize() != 1 )
									|| ( eChildType != pEMInit->GetOptionTypes().GetAt( 0 ) )
								){

									// Try to find an existing branch of this OptType.
									// We search the first level of children by OptType.
									hChild = pTree->GetChildItem( hCur );
									while ( hChild )
									{
										if ( pTree->GetItemOptType( hChild )  == eChildType )
											break;
										hChild = pTree->GetNextSiblingItem( hChild );
									}

									// No matching branch found?  Must add it as a new branch.
									if ( hChild == NULL )
									{
										UINT uiNameString = OptionManager.uiNameString( eChildType );
										UINT uiBitmapID = OptionManager.uiBitmapID( eChildType );

										hChild = pTree->AddObject(
											hCur,								// Parent tree item.
											OBJECT_LIST_LEN,				// Object type.
											NULL,								// Object reference.
											&ResStr( uiNameString ),	// tree item text
											uiBitmapID,						// Tree bitmap resource id.
											nB,								// Position of this object in the subobj array.
											nA,								// NotList or subobj list position?
											NULL,								// Parent EMComponent object pointer.
											false,							// Is this object private?
											eChildType						// Option object type specifier.
										);
									}

									hCur = hChild;
								}
							}

							hCurParent = hCur;

							// Now add our Option object leaf to the tree.

							// Only add installed objects to the tree.
							if ( !EMSub.IsInstalled() )
								continue;

							pOption->GetDisplayName( &strName );
						}


						HTREEITEM hSubObj = pTree->AddObject(
							hCurParent,	// Parent tree item.
							eSubType,	// Object type.
							pRef,			// Object reference.
							&strName,	// Object name string.
							uiBmpID,		// Tree bitmap resource id.
							nB,			// Position of this object in the subobj array.
							nA,			// Is this a subobj or subobj list?
							this,			// Parent EMComponent object pointer.
							bPrivate,	// Is this object private?
							eType			// OptType for Option objects.
						);

						// Call the subobj to add its subobjs.
						if ( pSubInit->bHasConfigurables() )
						{
							pEMC = GetObjPtr( nA, nB );
							pEMC->FillComponentTree( pTree, hSubObj, false, eRootType, eParentType );
						}
					}
				}
			}
		}
	}

	return hParent;
}



//-------------------------------------------------------------------//
// ResolveSubObjLink()																//
//-------------------------------------------------------------------//
// This utility function handles a sub obj link to a sub obj list.
// Note that this list can reside in any of the sub objects
// recusively, as specified by the SUB_OBJ_INIT arrays.
//
// Digs down to find the list arrays.  The end result of this
// mess is that the current sub obj is limited to values that
// reside in a sub obj list.  This sub obj list can be from
// the object itself, or in one of the object's sub objs.  The
// array of shorts accessed through GetSubObjLink sets a path
// that we dig down through to reach the correct list.  You
// can dig down as far as you want in the object hierarchy,
// but please note that the farther you dig, the slower
// Earthmover will operate in general.  This is a very powerful
// yet very confusing concept.
//-------------------------------------------------------------------//
bool EMComponent::ResolveSubObjLink(
	EMComponent		**ppLinkObj,		// Ptr to return found object through.
	int				*pnLinkIdx,			// Ptr to return list index through.
	int				*pnDepth,			// Ptr to return new list array depth.
	int				nWhere				// Which sub object we are trying to match.
){

	bool	bReturn		= false;		// Assume a valid link will be found.
	bool	bLinkEnd		= false;		// Assume we are not at the end yet.
	int	nIdx;

	OBJECT_TYPE eMatchType = pEMInit->GetSubObjType( nWhere );

	int nLink = pEMInit->GetSubObjLink( nWhere, *pnDepth );

	EMComponent *pCurObj = this;
	(*pnDepth)++;

	// Loop on the NoLink specifier.
	while ( ( nLink != nNoLink ) && pCurObj && !bLinkEnd )
	{
		nIdx = nLink;
		nLink = pEMInit->GetSubObjLink( nWhere, *pnDepth );

		// If the next link was the last, then the
		// current nIdx is the subobj list index in
		// the current object.  Otherwise, we need to
		// keep digging.
		if ( nLink != nLastLink )
		{
			// Signifies that this is the end of a link, but there
			// are still more to follow.
			if ( nLink == nEndLink )
			{
				ASSERT( nIdx < pCurObj->pEMInit->GetNumLists() );

				// Double check that the OBJECT_TYPEs of the search
				// type and the found list type match up.
				if ( pCurObj->pEMInit->GetSubObjType( nIdx, 0 ) == eMatchType )
				{
					// Break loop and return everything is happy.
					bReturn = bLinkEnd = true;
				}
				else
				{
					// If you tripped this AfxDebugBreak(), then something is
					// probably wrong with the SubObj initializer SubObjLink list.
					AfxDebugBreak();
					bReturn = false;
					bLinkEnd = true;
				}
			}

			// Dig down to next level, assuming everything appears OK.
			else
			{
				ASSERT( nIdx < pCurObj->pEMInit->GetSubObjSize() );

				// Make sure that the SubObj we are about to dig down
				// through is a real, valid DB object.  Otherwise, we
				// will end up digging down through a temporary object.
				if ( pCurObj->bHasSubObj( nIdx ) )
				{
					pCurObj = pCurObj->GetObjPtr( nIdx );
					(*pnDepth)++;
				}

				// The object doesn't currently have a SubObj here, so
				// we can quit digging here and go to the next link.  Or
				// quit if this was the last link.
				else
				{
					while ( ( nLink != nNoLink ) && ( nLink != nEndLink ) )
					{
						(*pnDepth)++;
						nLink = pEMInit->GetSubObjLink( nWhere, *pnDepth );
					}

					(*pnDepth)++;

					if ( nLink == nLastLink )
					{
						bReturn = false;
						bLinkEnd = true;
					}
				}
			}
		}
		else
		{
			// Double check that the OBJECT_TYPEs of the search
			// type and the found list type match up.
			if ( pCurObj->pEMInit->GetSubObjType( nIdx, 0 ) == eMatchType )
			{
				// Break loop and return everything is happy.
				bLinkEnd = bReturn = true;
			}
			else
			{
				// If you tripped this AfxDebugBreak(), then something is
				// probably wrong with the SubObj initializer SubObjLink list.
				bReturn = false;
				bLinkEnd = true;
				AfxDebugBreak();
			}
		}
	}

	// Set up pass back pointers.
	if ( bReturn )
	{
		*pnLinkIdx = nIdx;
		*ppLinkObj = pCurObj;
	}

	return bReturn;
}


//-------------------------------------------------------------------//
// SubObj_Validate()																	//
//-------------------------------------------------------------------//
// This function takes every SubObject in this object and checks it
// to make sure that it is still valid through the bAcceptSubObj()
// function.  This is needed because a lot of SubObjects are
// dependant upon the contents of other SubObjects.  For example, if
// the manufacturer of a tire is changed, then we need to check the
// compound and tread of that tire to make sure that they have the
// same manufacturer.
//
// For SubObjects that are found to be invalid, this function blanks
// the ObjectReference of that SubObject.
//
// RETURNS: 'False' if a change was made to the object due to one of
// its SubObjects being invalid.  This allows the calling function
// to dirty and save the object.  'True' if all were 'Validated'.
//-------------------------------------------------------------------//
bool EMComponent::SubObj_Validate()
{
	int nA, nB;

	bool bModified = false;
	bool bListOnly = false;
	bool bGoodRef	= false;

	ObjectReference BlankRef;
	ObjectReference *pRef;

	OBJECT_TYPE	eSubType;

	// Scan through sub-objects and look for invalid objects.
	for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
	{
		pRef = GetSubObjRef( nA );

		// Check if we have an unselected external item. If so, we can skip the check.
		if ( pRef->IsBlank() )
			continue;

		// External subobject is present, so check it.
		else
		{
			eSubType = pEMInit->GetSubObjType( nA );

			EMSubObject &EMSubObj = GetSubObj( nA );

			// Get the private status as saved in the sub object info.
			bool bPrivateInSubObj = EMSubObj.IsPrivate();
			bool bMemoryOnlyInSubObj = EMSubObj.IsInMemoryOnly();

			// Can only validate objects that have been saved out.
			// If the user has objects lying around in memory then
			// he will just have to wait until he hits Apply before
			// the checks are done.
			if ( !bMemoryOnlyInSubObj )
			{
				DBTreeControl *pSubTree = DBTreeControl::GetTreePtr( eSubType );
				POSITION Pos = pSubTree->FindListItem( pRef );

				if ( Pos )
				{
					// Get the private status as saved in the tree cache.
					bool bPrivateInTree = pSubTree->GetItemDBData( Pos )->bIsPrivate();

					// Should always be the same.  SubObj List objects are the
					// only ones that should be able to be different, as the
					// result of a drag and drop from a DBListControl to a
					// DBTreeControl.  And that is only for a brief moment,
					// as this function is called to clean that up.
					if (
							bPrivateInTree != bPrivateInSubObj

							// Note: bNoListRemoval tells us that the subobjects are typically
							// private, but marked as public, so they are not manipulated
							// by the base class.  Leave these types alone.
						&& !EMSubObj.m_pInit->bNoListRemoval()

					) {
						EMSubObj.SetPrivate( bPrivateInTree );
						bModified = true;
					}
				}

				// Object not found in tree - it must have been deleted
				// from the database.  Normally I would ASSERT this, but
				// there is a valid case where we might come through
				// here in this state: a private object has been deleted
				// from a list control on a prop sheet and then this
				// function is called to clean up any configurable sub
				// objects that might have had that object selected.
				// Do that clean up now:
				if ( Pos == NULL )
				{
					bGoodRef = false;
				}
				else
				{
					// If the object is a configured private object, then
					// automatically accept it as good.  If we don't do this,
					// then bAcceptSubObj might reject it because a subobj
					// that is screened by a subobj list can have a configured
					// subobj that does NOT appear in the list.
					bGoodRef = EMSubObj.IsManagedHere();

					if ( !bGoodRef )
					{
						bGoodRef = bAcceptSubObj(
							eSubType,		// Drop OBJECT_TYPE.
							pRef,				// Drop ObRef.
							NULL,				// Returns whether drop accepted in SubObj or SubObjList.
							&nA,				// Where we would drop the SubObj if we could.
							NULL,				// Warning message to pass back to caller.
							&bListOnly		// Passed if we know whether to check list or non-list only.
						);
					}
				}

				if ( !bGoodRef )
				{
					SetSubObjRef( nA, &BlankRef );
					bModified = true;
				}
			}	// !bMemoryOnlyInSubObj
		}
	}

	// We will be checking lists only in this next section.
	bListOnly = true;

	// Scan through sub-object lists and look for invalid objects.
	for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ )
	{
		// Get object type and bNoListRemoval here to save inner loop effort.
		eSubType = pEMInit->GetSubObjType( nA, 0 );
		bool bNoListRemoval = pEMInit->GetSubObjListsInit( nA )->bNoListRemoval();

		// If SubObj is private we can skip the bAcceptSubObj check below.
		DBTreeControl *pSubTree = DBTreeControl::GetTreePtr( eSubType );

		// Scan through all the SubObjs in this list.
		for ( nB = 0; nB < GetListSize( nA ); nB++ )
		{
			EMSubObject &EMSubObj = GetSubObj( nA, nB );
			pRef = &EMSubObj.ObRef();

			// If the Ref is blank and no list removal is requested,
			// we don't do anything.
			if ( !( bNoListRemoval && pRef->IsBlank() ) )
			{

				// Get the private status as saved in the sub object info.
				bool bPrivateInSubObj = EMSubObj.IsPrivate();
				bool bMemoryOnlyInSubObj = EMSubObj.IsInMemoryOnly();

				// Can only validate objects that have been saved out.
				// If the user have objects lying around in memory then
				// he will just have to wait until he hits Apply before
				// the checks are done.
				if ( !bMemoryOnlyInSubObj )
				{

					POSITION Pos = pSubTree->FindListItem( pRef );

					if ( Pos )
					{
						// Get the private status as saved in the tree cache.
						bool bPrivateInTree = pSubTree->GetItemDBData( Pos )->bIsPrivate();

						// Clean up any discrepancies in private status.
						if (
								bPrivateInTree != bPrivateInSubObj

								// Note: bNoListRemoval tells us that the subobjects are typically
								// private, but marked as public, so they are not manipulated
								// by the base class.  Leave these types alone.
							&& !EMSubObj.m_pInit->bNoListRemoval()

						) {
							EMSubObj.SetPrivate( bPrivateInTree );
							bModified = true;
						}
					}

					// Object not found in tree - it must have been deleted
					// from the database.  Normally I would ASSERT this, but
					// there is a valid case where we might come through
					// here in this state: a private object has been deleted
					// from a list control on a prop sheet and then this
					// function is called to clean up any configurable sub
					// objects that might have had that object selected.
					// Do that clean up now:
					if ( Pos == NULL )
					{
						bGoodRef = false;
					}
					else
					{
						bGoodRef = bAcceptSubObj(
							eSubType,		// Drop OBJECT_TYPE.
							pRef,				// Drop ObRef.
							NULL,				// Returns whether drop accepted in SubObj or SubObjList.
							&nA,				// Where we would drop the SubObj if we could.
							NULL,				// Warning message to pass back to caller.
							&bListOnly		// Passed if we know whether to check list or non-list only.
						);
					}

					if ( !bGoodRef )
					{
						// We have a bad ref!

						// Just blank the ref if bNoListRemoval requested.
						if ( bNoListRemoval )
						{
							pRef->Clear();

						// Otherwise, kill it from the list!
						} else
						{
							SubObjList_DeleteRef( nA, *pRef );
							nB--;
						}

						bModified = true;
					}

				}	// !bMemoryOnlyInSubObj

			}	// !( bNoListRemoval && pRef->IsBlank() )

		}
	}

	return !bModified;
}


//-------------------------------------------------------------------//
// bHasModifiedSubObj()																//
//-------------------------------------------------------------------//
// Checks to see if the Modified flag is set on any of
// the SubObjects of this object.
//-------------------------------------------------------------------//
bool EMComponent::bHasModifiedSubObj() {
	int nA, nB, nSize;

	// Using a local EMSubObject pointer to save array lookups.
	EMSubObject *pSub;

	pSub = SubObj.GetData();
	nSize = pEMInit->GetSubObjSize();

	// Scan through sub objects.
	for ( nA = 0; nA < nSize; nA++ ) {
		if ( pSub->IsModified() )
			return true;
		pSub++;
	}

	// Handle subobject lists.
	for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ ) {

		pSub = SubObjList[ nA ].GetData();
		nSize = GetListSize( nA );

		for ( nB = 0; nB < nSize; nB++ ) {
			if ( pSub->IsModified() )
				return true;
			pSub++;
		}

	}

	return false;
}


//-------------------------------------------------------------------//
// FindAcceptableSubObjRefs()														//
//-------------------------------------------------------------------//
// Finds all the objects in the databases that could be possible
// candidates for the SubObject specified by nSubObjIdx.  Returns
// the results in arRefs.
//-------------------------------------------------------------------//
void EMComponent::FindAcceptableSubObjRefs(
	int					nSubObjIdx,						// SubObj index we are searching for.
	ObRefArray			&arRefs							// CArray of refs to return results through.
) {

	ObjectReference	*pCurrentRef	= GetSubObjRef( nSubObjIdx );
	OBJECT_TYPE			eMatchType		= pEMInit->GetSubObjType( nSubObjIdx );
	int					nLink				= pEMInit->GetSubObjLink( nSubObjIdx, 0 );

	ObjectReference *pRef;

	EMComponent *pLinkObj;
	int nA, nIdx, nDepth = 0;

	arRefs.RemoveAll();

	// If this is not a CodeRule link, then it is a real list of
	// links.  Go ahead and chase it down and add all the items to
	// the menu.
	if ( ( nLink != nCodeRule ) && ( nLink != nNoLink ) ) {

		// Let the sheet parent object check it's own sub obj links.
		while ( ResolveSubObjLink( &pLinkObj, &nIdx, &nDepth, nSubObjIdx ) ) {

			// Loop through the found links.
			for ( nA = 0; nA < pLinkObj->GetListSize( nIdx ); nA++ ) {

				// Add them to the ref array.
				arRefs.Add( *pLinkObj->GetSubObjRef( nIdx, nA ) );

			}
		}

	}

	// CodeRule: We need to scan every object in the database
	// to find the ones that match.  Is this worth it?  We shall see.
	else {

		DBTreeControl *pTree = DBTreeControl::GetTreePtr( eMatchType );
		HTREEITEM LoopItem = pTree->GetFirstLeaf();
		bool bListOnly = false;

		while ( LoopItem ) {

			pRef = pTree->GetRef( LoopItem );

			if (
				bAcceptSubObj(
					eMatchType,			// Drop OBJECT_TYPE.
					pRef,					// Drop ObRef.
					NULL,					// Returns whether drop accepted in SubObj or SubObjList.
					&nSubObjIdx,		// Where we would drop the SubObj if we could.
					NULL,					// Warning message to pass back to caller.
					&bListOnly			// Passed if we know whether to check list or non-list only.
				)
			) {

				// Add them to the ref array.
				arRefs.Add( *pRef );

			}

			LoopItem = pTree->GetNextLeafItemAcrossBranches( LoopItem );
		}

	}

	// If the currently selected sub object is private and
	// configured, then add it to the list as well.

	if (
		( GetSubObj( nSubObjIdx ).IsPrivate() ) &&
		( GetSubObj( nSubObjIdx ).IsManagedHere() )
	) {

		if ( !pCurrentRef->IsBlank() ) {

			// Add it to the ref array.
			arRefs.Add( *pCurrentRef );

		}

	}

}


//-------------------------------------------------------------------//
// SaveSubObjConfigChanges()														//
//-------------------------------------------------------------------//
// Called from SaveObject() to handle configured private
// sub objects.
//-------------------------------------------------------------------//
void EMComponent::SaveSubObjConfigChanges()
{
	int nA, nB, nC;

	int nSize = pEMInit->GetSubObjSize();
	int nLists = pEMInit->GetNumLists();

	if ( ( nSize + nLists ) == 0 )
		return;

	// Using a local EMSubObject pointer to save array lookups.
	EMSubObject *pBackSub	= SubObjBackup[ nLists ].GetData();
	EMSubObject *pNewSub		= SubObj.GetData();

	// Check all the SubObjects for changes that need to
	// be updated.
	for ( nA = 0; nA < nSize; nA++ )
	{
		pNewSub->OnSaveObject( pBackSub );

		pBackSub++;
		pNewSub++;
	}


	// Loop through all the SubObject lists, checking for any
	// private object deletions that need to be carried through.

	for ( nA = 0; nA < nLists; nA++ )
	{
		// Get the CArray lengths for the new and backup.
		int nNewSize	= SubObjList[ nA ].GetSize();
		int nBackSize	= SubObjBackup[ nA ].GetSize();

		EMSubObject *pBackSub = SubObjBackup[ nA ].GetData();

		// Loop through all the "backup" SubObjects:
		for ( nB = 0; nB < nBackSize; nB++ )
		{
			if ( pBackSub->IsPrivate()	)
			{

				EMSubObject *pNewSub	= SubObjList[ nA ].GetData();

				// Loop through all the "new" SubObjects:
				for ( nC = 0; nC < nNewSize; nC++ )
				{
					// Only need to check private, modified
					// Look for an ObjectReference match.
					if ( pBackSub->ObRef() == pNewSub->ObRef() )
					{
						ASSERT(
							( !pNewSub->IsManagedHere()	) &&
							( !pNewSub->IsInMemoryOnly()	)
						);

						break;
					}

					pNewSub++;
				}

				// Did NOT find a match above?  Then the sub object won't exist in
				// the object anymore as it was when it was last saved to the
				// database.  So whack the sub object entirely.
				if ( nC == nNewSize )
					pBackSub->RemovePrivateObject();

			}

			pBackSub++;
		}

	}



	// Update backup information now to match what was saved
	// to the database.  This is needed in case the user hit
	// Apply instead of OK, and the property sheet is still active.
	SetIsPropSheet( false );

	if ( bRefresh() )
		SubObj_Release();

	SetPropSheetObject();

}

//-------------------------------------------------------------------//
// CancelSubObjListChanges()														//
//-------------------------------------------------------------------//
// Called from DBPropFrame::Cancel() to remove any SubObjList
// objects that were added during that property sheet session.
// (Since the last Apply, of course)
//
// We do this by checking the backup array for objects that have
// the "modified" flag set.  These are objects that were created
// during this property sheet "session".
//-------------------------------------------------------------------//
void EMComponent::CancelSubObjListChanges()
{
	int nLists = pEMInit->GetNumLists();

	// Loop through all the SubObject lists:
	for ( int nA = 0; nA < nLists; nA++ )
	{
		int nBackSize = SubObjBackup[ nA ].GetSize();
		EMSubObject *pBackSub = SubObjBackup[ nA ].GetData();

		// Loop through all the "backup" SubObjects:
		for ( int nB = 0; nB < nBackSize; nB++ )
		{
			pBackSub->OnPropertiesCancel();
			pBackSub++;
		}
	}
}

//-------------------------------------------------------------------//
// SubObjList_DeleteAll()															//
//-------------------------------------------------------------------//
// Delete all SubObjects in the requested list.
//-------------------------------------------------------------------//
void EMComponent::SubObjList_RemoveAll( int nListIdx )
{
	ASSERT( nListIdx < pEMInit->GetNumLists() );

	// Free up EMSubObject::pEMC pointers first.
	EMSubObject *pSub = SubObjList[ nListIdx ].GetData();
	int nListSize = GetListSize( nListIdx );
	for ( int nB = 0; nB < nListSize; nB++ ) {
		pSub->ClearPtr();
		pSub++;
	}

	// Now can allow CArray to whack itself.
	SubObjList[ nListIdx ].RemoveAll();
}

//-------------------------------------------------------------------//
// GetDisplayNameFromTree()														//
//-------------------------------------------------------------------//
// Display name access using ID.  This version returns
// the string instead of using the pointer, for those
// cases where the return bool is unimportant and no
// speed savings is required.
//-------------------------------------------------------------------//
CString EMComponent::GetDisplayNameFromTree(
   OBJECT_TYPE			eWhich,
	ObjectReference*	pRef
) {
	CString strName;
	GetDisplayNameFromTree( &strName, eWhich, pRef );
	return strName;
}

//-------------------------------------------------------------------//
// GetDisplayNameFromTree()														//
//-------------------------------------------------------------------//
// Get the label string of an object from the tree cache using
// its ObjectReference to find it.
//
// This function returns true if the object was found in the tree
// cache.  It returns false if there was an error.
//-------------------------------------------------------------------//
bool EMComponent::GetDisplayNameFromTree(
	CString*				pName,	// CString to stuff name into.
   OBJECT_TYPE			eWhich,	//	the object's type.
	ObjectReference*	pRef		//	ObjectReference to search for.
) {

	if (
		( pRef == NULL		) ||
		( pRef->IsBlank()	)
	){
		pName->Empty();
	}
	else
	{
		if ( DBTreeControl* pDBTree = DBTreeControl::GetTreePtr( eWhich ) )
			return pDBTree->GetItemListText( pRef, pName );
		else
			pName->Format( _T("Unavailable - ID %i"), pRef->ObID );
	}

	return false;
}

//-------------------------------------------------------------------//
// GetCompletenessFromTree()														//
//-------------------------------------------------------------------//
// Determines whether an object is complete or not based
// upon its sort order information.  Use of this function
// assumes that the object's sort information has been
// updated, which in most cases will be true.
//-------------------------------------------------------------------//
bool EMComponent::GetCompletenessFromTree(
   OBJECT_TYPE			eWhich,
	ObjectReference*	pRef
){
	bool bReturn = false;

	if ( pRef && !pRef->IsBlank()	)
	{
		return (

			DBTreeControl::GetTreePtr( eWhich )->
				nGetSortCode(
					EMComponentInit::ByType( eWhich )->nNumSortCats(),
					pRef
				)

			== DE_COMPLETE
		);
	}

	return false;
}

//-------------------------------------------------------------------//
// SetSubObjPtr()																		//
//-------------------------------------------------------------------//
// Sets the SubObj.pEMC pointer.  This function should only be used
// in the rarest of circumstances, such as in Tread\Tread3dPage where
// we are making a bogus tire and need to bypass the SubObj handling.
//-------------------------------------------------------------------//
void EMComponent::SetSubObjPtr( int nArrayIdx, EMComponent *pEMC )
{
	SubObj[ nArrayIdx ].pEMC() = pEMC;
}

//-------------------------------------------------------------------//
// AddDataSpecial()																	//
//-------------------------------------------------------------------//
// The major disadvantage of using DataSpecial is that you have to
// remember to pass Add the DataSpecial ptr to arpDataSpecials in
// the component's constructor.  Do so through this function.  The
// bUseAutoReadWrite boolean is for backwards compatability in EM
// to before the DataSpecial automation was added to EMComponent.
//-------------------------------------------------------------------//
void EMComponent::AddDataSpecial( DataSpecial *pDataSpecial, bool bUseAutoReadWrite )
{
	ASSERT( pDataSpecial );
	arpDataSpecials.Add( pDataSpecial );
	pDataSpecial->m_bUseAutoReadWrite = bUseAutoReadWrite;
}

//-------------------------------------------------------------------//
// nGetDataSpecialSize()															//
//-------------------------------------------------------------------//
// Gets the number of DataSpecial ptrs in the CArray.
//-------------------------------------------------------------------//
int EMComponent::nGetDataSpecialSize()
{
	return arpDataSpecials.GetSize();
}

//-------------------------------------------------------------------//
// pGetDataSpecialPtr()																//
//-------------------------------------------------------------------//
// Returns a given DataSpecial ptr from the CArray.
//-------------------------------------------------------------------//
DataSpecial *EMComponent::pGetDataSpecialPtr( int nIdx )
{
	return arpDataSpecials[ nIdx ];
}

//-------------------------------------------------------------------//
// GetSubObjTypedRef()																//
//-------------------------------------------------------------------//
// Get the TypedRef of a SubObj.
// SubObj version
//-------------------------------------------------------------------//
TypedRef EMComponent::GetSubObjTypedRef( int nArrayIdx )
{
	return TypedRef(
		SubObj[ nArrayIdx ].ObRef(),
		pEMInit->GetSubObjType( nArrayIdx )
	);
}

//-------------------------------------------------------------------//
// GetSubObjTypedRef()																//
//-------------------------------------------------------------------//
// Get the TypedRef of a SubObj.
// SubObj lists version
//-------------------------------------------------------------------//
TypedRef EMComponent::GetSubObjTypedRef( int nListIdx, int nArrayIdx )
{
	return TypedRef(
		SubObjList[ nListIdx ][ nArrayIdx ].ObRef(),
		pEMInit->GetSubObjType( nListIdx, nArrayIdx )
	);
}

//-------------------------------------------------------------------//
// GetSubObjTypedRefArray()														//
//-------------------------------------------------------------------//
// Returns all the refs in this subobject list.
//-------------------------------------------------------------------//
TypedRefArray EMComponent::GetSubObjTypedRefArray( int nListIdx )
{
	TypedRefArray arRefs;
	for ( int nA = 0; nA < GetListSize( nListIdx ); nA++ )
		arRefs.Add( GetSubObjTypedRef( nListIdx, nA ) );
	return arRefs;
}


//-------------------------------------------------------------------//
// ReadDataSpecials()																//
//-------------------------------------------------------------------//
// ObjectStream read function for arpDataSpecials.  All other DataSpecial
// calls have been included within their EMComponent equivalents.
// Note that this function does not test for a successful read at
// this time.
//-------------------------------------------------------------------//
void EMComponent::ReadDataSpecials()
{
	// Get the array size for version checking.
	short sSize;
	ReadObject( sSize );

	// ASSERT if we have a size mismatch.  All mismatches should be
	// handled in versioning code using Insert/DeleteDataSpecial(),
	// with the exception of DataSpecials newly added to the end
	// of the array.
	ASSERT( sSize <= arpDataSpecials.GetSize() );

	// Loop through the array and read in the individual items.
	for ( int nA = 0; nA < (int)sSize; nA++ )
	{
		DataSpecial	*pDataSpecial = arpDataSpecials[ nA ];
		ASSERT( pDataSpecial );

		if ( pDataSpecial->m_bUseAutoReadWrite )
		{
			pDataSpecial->ReadObject( GetDatabasePtr()->GetDBVersion() );
		}
	}
}

//-------------------------------------------------------------------//
// WriteDataSpecials()																//
//-------------------------------------------------------------------//
// ObjectStream write function for arpDataSpecials.  All other DataSpecial
// calls have been included within their EMComponent equivalents.
//-------------------------------------------------------------------//
void EMComponent::WriteDataSpecials()
{
	// Get the array size and write it out for version checking.
	short sSize = (short) arpDataSpecials.GetSize();
	WriteObject( sSize );

	// Loop through the array and write out the individual items.
	for ( int nA = 0; nA < (int)sSize; nA++ )
	{
		DataSpecial	*pDataSpecial = arpDataSpecials[ nA ];
		ASSERT( pDataSpecial );

		if ( pDataSpecial->m_bUseAutoReadWrite )
		{
			pDataSpecial->WriteObject();
		}
	}
}

//-------------------------------------------------------------------//
// GetDisplayName()																	//
//-------------------------------------------------------------------//
// Gets a standard display name for this object.  This "default"
// version just gets the tag DataString index from the
// EMComponentInit pointer.
//-------------------------------------------------------------------//
void EMComponent::GetDisplayName( CString* pstrName )
{
	// Where the tag DataString is in the list.  NoTag if none.
	int nIdx = pEMInit->nTagDataStringIdx();

	// This function should be overrid in the derived class
	// if there is no tag DataString.
	ASSERT( nIdx != NoTag );

	*pstrName = dstr[ nIdx ].strOutput();

	// Call clean-up helper.
	// It will pack our string and assign the default "unnamed"
	// string if blank.
	CleanUpDisplayName( pstrName );
}

//-------------------------------------------------------------------//
// SubObjDatabaseUpdate()															//
//-------------------------------------------------------------------//
// This utility function is useful for backwards compatability
// database maintenance.  Use it to move a sub object ref from
// one place to another, either within the same object or
// possibly across a subobject relationship.  It checks first to
// see if this operation has been completed previously.  Ask
// JWC about this before using it.  The return value specifies
// whether the change was made.
//-------------------------------------------------------------------//
bool EMComponent::SubObjDatabaseUpdate(
	EMComponent *pFromObj,
	int			nFromIdx,
	EMComponent *pToObj,
	int			nToIdx
){
	if (
			( !pFromObj->GetSubObjRef( nFromIdx )->IsBlank()	)
		&& ( pToObj->GetSubObjRef( nToIdx )->IsBlank()		)
	){
		// Move the reference over to the new position.
		pToObj->GetSubObj( nToIdx ) = pToObj->GetSubObj( nFromIdx );

		// Clear it from the old position.
		pFromObj->SetSubObjRef( nFromIdx, &ObjectReference() );

		// Mark both objects as changed and return true to
		// let the caller know they are changed.
		pToObj->ChangeObject();
		pFromObj->ChangeObject();
		return true;
	}

	return false;
}

//-------------------------------------------------------------------//
// SubObjListDatabaseUpdate()														//
//-------------------------------------------------------------------//
// The subobject LIST version of the above function.
//-------------------------------------------------------------------//
bool EMComponent::SubObjListDatabaseUpdate(
	EMComponent *pFromObj,
	int			nFromIdx,
	EMComponent *pToObj,
	int			nToIdx
){
	// Check Engine subobject list.
	if (
			( pFromObj->GetListSize( nFromIdx ) )
		&& ( 0 == pToObj->GetListSize( nToIdx ) )
	){
		// Get and set the array size.
		int nSize = pFromObj->GetListSize( nFromIdx );
		pToObj->SubObjList[ nToIdx ].SetSize( nSize );

		// Move all references over to the new positions.
		for ( int nA = 0; nA < nSize; nA++ )
			pToObj->GetSubObj( nToIdx, nA ) = pFromObj->GetSubObj( nFromIdx, nA );

		// Clear them from the old positions.
		pFromObj->SubObjList[ nFromIdx ].SetSize( 0 );

		// Mark both objects as changed and return true to
		// let the caller know they are changed.
		pToObj->ChangeObject();
		pFromObj->ChangeObject();
		return true;
	}

	return false;
}


//-------------------------------------------------------------------//
// EnableComponentItem()															//
//-------------------------------------------------------------------//
// Called by FillComponentTree to allow any special enabling
// on the component tree.
//
// Optional subobjects are always shown by default.  We will provide
// "none" as a selection for these subobjects.
//
// Note that the return value of this function
// is ignored if the SubObj_Init ShowPos value for the
// requested SubObject is set to NoShow.
//-------------------------------------------------------------------//
bool EMComponent::EnableComponentItem(
	OBJECT_TYPE		eRootType,		// Root object type.
	OBJECT_TYPE		eParentType,	// Immediate parent object type.
	int				nWhichList,		// The SubObj List (or NotList for SubObj).
	int				nIdx				// The SubObj (List) Idx
){
	if ( IsSubObjOptional( nWhichList, nIdx ) )
		return true;
	else
		return CheckSubObjCompleteness( nWhichList, nIdx );
}


//-------------------------------------------------------------------//
// CheckSubObjCompleteness()														//
//-------------------------------------------------------------------//
// Used to allow special cases when checking the completeness
// of SubObjects.  The default behavior is as follows:
//		1) Do not check SubObject list objects.
//		2) If the SubObject is optional, check for completeness
//			only if the ref is not blank.
//		3) If the SubObject is not optional, check it.
// Override this function in your derived class to change this
// behavior from the default.
//-------------------------------------------------------------------//
bool EMComponent::CheckSubObjCompleteness(
	int nWhichList,	// The SubObj List (or NotList for SubObj).
	int nIdx				// The SubObj (List) Idx
){
	if ( nWhichList != NotList )
		return false;
	else if ( IsSubObjOptional( nWhichList, nIdx ) )
		return ( !GetSubObjRef( nIdx )->IsBlank() );
	else
		return true;
}


//-------------------------------------------------------------------//
// IsSubObjOptional()																//
//-------------------------------------------------------------------//
// By default, subobjects are not optional and lists are.
// Multimedia objects are the exception to this rule; they are
// optional by default.
//-------------------------------------------------------------------//
bool EMComponent::IsSubObjOptional(
	int nWhichList,	// The SubObj List (or NotList for SubObj).
	int nIdx				// The SubObj (List) Idx
){
	if ( nWhichList == NotList )
		return ( pEMInit->GetSubObjType( nIdx ) == OBJECT_MEDIA );
	else
		return true;
}

//-------------------------------------------------------------------//
// GetLowestSafeVersion()															//
//-------------------------------------------------------------------//
// This function returns the lowest version number for which this
// object does not need to be updated.  Persistent derived classes
// override this and provide the version number during which the
// last change to the database structure of the object occurred.
// EMComponent does this through EMComponentInit, therefore this
// function is not virtual here.
//-------------------------------------------------------------------//
uHuge EMComponent::GetLowestSafeVersion()
{
	return max( Version( 1, 26 ), pEMInit->GetLowestSafeVersion() );
}

//-------------------------------------------------------------------//
// InsertSubObj()																		//
//-------------------------------------------------------------------//
// InsertSubObj() and RemoveSubObj() allow EMComponent derived
// classes to insert and remove SubObjects prior to and after
// reading from the database, for versioning purposes.
//
// They MUST be used in conjunction (i.e. Remove after Insert)
// to repair any changes made to the static EMComponentInit classes.
//
// Pass the class's pInit for pBaseInit and NOT the pEMInit pointer!!!!
//
// See Widget.cpp for an example of their use.
//-------------------------------------------------------------------//
void EMComponent::InsertSubObj( int nInsertAt, SubObj_Init &Initializer, EMComponentInit *pBaseInit  )
{
	pEMInit->m_arSubObjInit.InsertAt( nInsertAt, Initializer );
	SubObj.InsertAt( nInsertAt, EMSubObject() );

	// Because of the nested class hierarchy thang, we need to go
	// through all the classes equal to or above this one and adjust
	// the sub object count.
	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSubObjSize++;
	}

	for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
		SubObj[ nA ].pInit() = pEMInit->GetSubObjInit( nA );
}

//-------------------------------------------------------------------//
// RemoveSubObj()																		//
//-------------------------------------------------------------------//
void EMComponent::RemoveSubObj( int nRemoveAt, EMComponentInit *pBaseInit  )
{
	pEMInit->m_arSubObjInit.RemoveAt( nRemoveAt );
	SubObj.RemoveAt( nRemoveAt );

	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSubObjSize--;
	}

	for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
		SubObj[ nA ].pInit() = pEMInit->GetSubObjInit( nA );
}


//-------------------------------------------------------------------//
// InsertSubObjList()																//
//-------------------------------------------------------------------//
void EMComponent::InsertSubObjList( int nInsertAt, SubObj_Init &Initializer, EMComponentInit *pBaseInit  )
{
	pEMInit->m_arSubObjListInit.InsertAt( nInsertAt, Initializer );
	SubObjList.InsertAt( nInsertAt, EMSubObjArray() );

	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSubObjListSize++;
	}
}

//-------------------------------------------------------------------//
// RemoveSubObjList()																//
//-------------------------------------------------------------------//
void EMComponent::RemoveSubObjList( int nRemoveAt, EMComponentInit *pBaseInit  )
{
	pEMInit->m_arSubObjListInit.RemoveAt( nRemoveAt );
	SubObjList.RemoveAt( nRemoveAt );

	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSubObjListSize--;
	}
}

//-------------------------------------------------------------------//
// InsertDataItem()																	//
//-------------------------------------------------------------------//
void EMComponent::InsertDataItem( int nInsertAt, DI_Init &Initializer, EMComponentInit *pBaseInit )
{
	CDIWrapArray arDI( &di, pEMInit->GetSizeDI() );
	pEMInit->m_arDI_Init.InsertAt( nInsertAt, Initializer );

	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSizeDI++;
	}

	arDI.InsertAt( nInsertAt, pEMInit->GetDI( nInsertAt ) );
}

//-------------------------------------------------------------------//
// RemoveDataItem()																	//
//-------------------------------------------------------------------//
void EMComponent::RemoveDataItem( int nRemoveAt, EMComponentInit *pBaseInit  )
{
	CDIWrapArray arDI( &di, pEMInit->GetSizeDI() );
	pEMInit->m_arDI_Init.RemoveAt( nRemoveAt );
	arDI.RemoveAt( nRemoveAt );

	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSizeDI--;
	}
}

//-------------------------------------------------------------------//
// InsertDataEnum()																	//
//-------------------------------------------------------------------//
void EMComponent::InsertDataEnum( int nInsertAt, DE_Init &Initializer, EMComponentInit *pBaseInit )
{
	CDEWrapArray arDE( &de, pEMInit->GetSizeDE() );
	pEMInit->m_arDE_Init.InsertAt( nInsertAt, Initializer );

	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSizeDE++;
	}

	arDE.InsertAt( nInsertAt, pEMInit->GetDE( nInsertAt ) );
}

//-------------------------------------------------------------------//
// RemoveDataEnum()																	//
//-------------------------------------------------------------------//
void EMComponent::RemoveDataEnum( int nRemoveAt, EMComponentInit *pBaseInit  )
{
	CDEWrapArray arDE( &de, pEMInit->GetSizeDE() );
	pEMInit->m_arDE_Init.RemoveAt( nRemoveAt );
	arDE.RemoveAt( nRemoveAt );

	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSizeDE--;
	}
}

//-------------------------------------------------------------------//
// InsertDataString()																//
//-------------------------------------------------------------------//
void EMComponent::InsertDataString( int nInsertAt, DS_Init &Initializer, EMComponentInit *pBaseInit )
{
	CDSWrapArray arDS( &dstr, pEMInit->GetSizeDS() );
	pEMInit->m_arDS_Init.InsertAt( nInsertAt, Initializer );
	arDS.InsertAt( nInsertAt, DataString() );

	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSizeDS++;
	}
}

//-------------------------------------------------------------------//
// RemoveDataString()																//
//-------------------------------------------------------------------//
void EMComponent::RemoveDataString( int nRemoveAt, EMComponentInit *pBaseInit  )
{
	CDSWrapArray arDS( &dstr, pEMInit->GetSizeDS() );
	pEMInit->m_arDS_Init.RemoveAt( nRemoveAt );
	arDS.RemoveAt( nRemoveAt );

	bool bFoundBase = false;
	for ( int nA = 0; nA < pEMInit->GetNumBaseObjectTypes(); nA++ )
	{
		EMComponentInit *pInit =
			pEMInit->ByType( pEMInit->GetBaseObjectType( nA ) );

		if ( bFoundBase |= ( pInit == pBaseInit ) )
			pInit->m_nSizeDS--;
	}
}

//-------------------------------------------------------------------//
// InsertDataSpecial()																	//
//-------------------------------------------------------------------//
void EMComponent::InsertDataSpecial( int nInsertAt, DataSpecial *pDataSpecial, bool bUseAutoReadWrite )
{
	ASSERT( pDataSpecial );
	arpDataSpecials.InsertAt( nInsertAt, pDataSpecial );
	pDataSpecial->m_bUseAutoReadWrite = bUseAutoReadWrite;
}

//-------------------------------------------------------------------//
// RemoveDataSpecial()																	//
//-------------------------------------------------------------------//
void EMComponent::RemoveDataSpecial( int nRemoveAt )
{
	arpDataSpecials.RemoveAt( nRemoveAt );
}

//-------------------------------------------------------------------//
// nFindOption()																		//
//-------------------------------------------------------------------//
// Returns the index of the Option that matches requested option
// type and attributes.  Returns NotAvail if no matching option found.
//
// If a branch OptType is passed instead of a leaf OptType, this
// function will return the first eligible Option that falls under
// that branch.  Returns the found leaf OptType through pnOptionType.
//
// Note that SearchFlags comes from EMSubObject::DB_Flags, to allow
// screening by "Installed" or "Standard".  The default is to just
// search for availability.
//
// Note also that an int is used instead of an OptType for nOptionType
// so that OptionTypes.h did not have to be included here.
//-------------------------------------------------------------------//
int EMComponent::nFindOption(
	int *				pnOptionType,	// OptType to search for.
	uByte				SearchFlags,	// Attributes to match.
	int				nStartFrom,		// List index to start from.
	EMComponent **	ppOption			// Ptr to pass Option back through.
){
	OptType eOptionType = (OptType) *pnOptionType;
	int nOptList = pEMInit->nOptionListIdx();

	// Returns NotAvail if no matching option found.
	int nRetVal = NotAvail;

	if ( ( nOptList != -1 ) && ( bIsValidOptType( eOptionType ) ) )
	{
		// Get and OptSet to screen for valid OptTypes with.
		OptSet ValidSet = OptionManager.GetNode( eOptionType ).GetSet();

		// Loop through all the Options subobjects within this object.
		int nListLen = GetListSize( nOptList );
		for ( int nA = nStartFrom; nA < nListLen; nA++ )
		{
			// Check to see if the attributes of this Option match
			// up with the criteria passed in SearchFlags.  We do this
			// first because we have this information readily available
			// without having to read from the DB yet.
			if ( SearchFlags == ( GetSubObj( nOptList, nA ).ubDBFlags() & SearchFlags ) )
			{
				// Now get this Option object so that we can read
				// its OptType for comparison vs ValidSet.
				Option *pOpt = (Option*) GetObjPtr( nOptList, nA );

				if ( ValidSet.bIsSet( pOpt->eGetOptType() ) )
				{
					// Return the found leaf OptType through pnOptionType.
					*pnOptionType = (int) pOpt->eGetOptType();

					// Return the index of the Option that matches.
					nRetVal = nA;

					// Return a pointer to the found Option through ppOption.
					if ( ppOption ) *ppOption = pOpt;

					break;	// Can stop the loop now!
				}
			}
		}
	}

	return nRetVal;
}


//-------------------------------------------------------------------//
// diWeight()																			//
//-------------------------------------------------------------------//
// Many EMC-derived classes will have weights associated with them.
// We provide a base-class virtual function to faciliate gathering
// the total weight of an object and its components.  See
// Equipment::diNonStdWeightAdjustment() for an example.
//
// This function looks up the weight of this object based on
// nWeightDataItemIdx, specified in EMCInit.
//
// Derived classes may override this function to directly provide
// the weight; see Options for an exanple.
//
// If you do not provide nWeightDataItemIdx and you do not overload
// this function in your derived class, the component's weight will
// be returned as zero.
//-------------------------------------------------------------------//
DataItem EMComponent::diWeight()
{
	if ( pEMInit->nWeightDataItemIdx() != NoWeight )
		return di[ pEMInit->nWeightDataItemIdx() ];
	else
		return DataItem( 0.0, SI_KG );
}


//-------------------------------------------------------------------//
// GetObjPtr()																			//
//-------------------------------------------------------------------//
// MDM	7/7/99 1:39:28 PM
// If a subobject is requested, and there is a corresponding std
// subobject that matches, this returns the std.  This reduces the
// instances of an object, thus reducing chances of lost changes.
// This safeguard was added for DoReverseEngineering(), where many
// nested levels request many different subobjects.
//-------------------------------------------------------------------//
EMComponent* EMComponent::GetObjPtr( int nArrayIdx )
{
	if ( bOptimizeStandard() )
	{
		int nStd = pEMInit->GetStdIdx( nArrayIdx );
		if ( ( nStd != NoStd ) && ( *GetSubObjRef( nStd ) == *GetSubObjRef( nArrayIdx ) ) )
				nArrayIdx = nStd;
	}
	return GetSubObj( nArrayIdx ).GetObjPtr( NonPersistentFlags );
}

//-------------------------------------------------------------------//
// Detach()																				//
//-------------------------------------------------------------------//
// See above comments for GetObjPtr()
//-------------------------------------------------------------------//
void EMComponent::Detach( int nArrayIdx )
{
	if ( bOptimizeStandard() )
	{
		int nStd = pEMInit->GetStdIdx( nArrayIdx );
		if ( ( nStd != NoStd ) && ( *GetSubObjRef( nStd ) == *GetSubObjRef( nArrayIdx ) ) )
				nArrayIdx = nStd;
	}
	GetSubObj( nArrayIdx ).Detach();
}


//-------------------------------------------------------------------//
// nDefaultGraphicIdx()																//
//-------------------------------------------------------------------//
// Returns an index to the default graphic.
//-------------------------------------------------------------------//
int EMComponent::nDefaultGraphicIdx()
{
	return pEMInit->nDefaultImageIdx();
}

//-------------------------------------------------------------------//
// strSubjectTypeName()																//
//-------------------------------------------------------------------//
// Get an appropriate title string for the top left side "object type".
//-------------------------------------------------------------------//
CString EMComponent::strSubjectTypeName( int nCnt )
{
	return pEMInit->strDesc( nCnt );
}

//-------------------------------------------------------------------//
// nChartCount()																		//
//-------------------------------------------------------------------//
// Returns the total number of charts available.
//-------------------------------------------------------------------//
int EMComponent::nChartCount()
{
	return pEMInit->nNestedCharts();
}

//-------------------------------------------------------------------//
// strSubObjectLog()																	//
//-------------------------------------------------------------------//
// Used to generate a subobject log file for this object.
// Made this virtual so that derived classes can add their
// own proprietary line items.
//-------------------------------------------------------------------//
CString EMComponent::strSubObjectLog(
	bool		bRecursive,	// Recur down through subobjects as well?
	CString	strIndent	// Indent level for line items - keep this default.
){
	int nA, nB;
	CString strReturn, strObj;
	TypedRef TRef;

	// Add a display name string for the topmost object.
	if ( strIndent.GetLength() == 0 )
	{
		strObj.Format(
			_T("%.4d  %s : %s"),
			GetRef().ObID,
			pEMInit->strDescSingular(),
			strDisplayName()
		);
		strReturn = strObj + strCR;
	}

	strIndent += strTAB;

	// Get display strings for subobjects.
	for ( nA = 0; nA < pEMInit->GetSubObjSize(); nA++ )
	{
		// Exclude "back-pointer" subobjects or else recursive will hang.
		if ( pEMInit->GetSubObjType( nA ) < GetObjectType() )
		{
			if ( bRecursive )
			{
			TRef = GetSubObjTypedRef( nA );
			strObj.Format(
				_T("%.2d        %2d:%.3d  %s : %s"),
				nA,
				TRef.Ref.DBID.DBNumber,
				TRef.Ref.ObID,
				EMComponentInit::ByType( TRef )->strDescSingular(),
				GetDisplayNameFromTree( TRef )
			);
			strReturn += strIndent + strObj + strCR;
			}
			//if ( bRecursive )
				strReturn += GetObjPtr( nA )->strSubObjectLog( bRecursive, strIndent );
		}
	}

	// Get display strings for subobjects lists.
	for ( nA = 0; nA < pEMInit->GetNumLists(); nA++ )
	{
		// Exclude "back-pointer" subobjects or else recursive will hang.
		if ( pEMInit->GetSubObjType( nA, 0 ) < GetObjectType() )
		{
			for ( nB = 0; nB < GetListSize( nA ); nB++ )
			{
			if ( bRecursive )
			{
				TRef = GetSubObjTypedRef( nA, nB );
				strObj.Format(
					_T("%.2d  %.2d  %2d:%.3d  %s : %s"),
					nA,
					nB,
					TRef.Ref.DBID.DBNumber,
					TRef.Ref.ObID,
					EMComponentInit::ByType( TRef )->strDescSingular(),
					GetDisplayNameFromTree( TRef )
				);
				strReturn += strIndent + strObj + strCR;
			}
			//	if ( bRecursive )
					strReturn += GetObjPtr( nA, nB )->strSubObjectLog( bRecursive, strIndent );
			}
		}
	}

	if ( !bRecursive )
	{

	// Skip past basedata
	RefScanData Data;
	while ( GetNextRef( Data ) )
	{
		if (
				Data.nSpecial != -1
			&&	Data.nSpecial < arpDataSpecials.GetSize()
			&& Data.nGridRow != -1
			&& Data.nGridColumn != -1
		){
			strObj.Format(
				_T("%.2d %.2d %.2d %2d:%.3d  %s::%s"),
				Data.nSpecial,
				Data.nGridRow,
				Data.nGridColumn,
				Data.pRef->DBID.DBNumber,
				Data.pRef->ObID,
				EMComponentInit::ByType( Data.Type )->strDescSingular(),
				GetDisplayNameFromTree( Data.Type, Data.pRef )
			);
			strReturn += strObj + strCR;
		}
	}
	}

	return strReturn;
}

//-------------------------------------------------------------------//
// SortSubObjectLists()																//
//-------------------------------------------------------------------//
// This function gets called from ValidateAndSave() after the
// SubObjects have been validated.  Like its title sez, its only
// purpose is to sort the SubObject lists.  This base class version
// sorts Options into their tree hierarchy order.  This function
// is virtual and can be overrid to implement any special sorting
// for derived classes.  For example, you may decide to sort objects
// for a cleaner visual representation in propsheet list controls.
//-------------------------------------------------------------------//
void EMComponent::SortSubObjectLists()
{
	int nList, nIdx, nA, nB;

	for ( nList = 0; nList < pEMInit->GetNumLists(); nList++ )
	{
		if ( pEMInit->GetSubObjType( nList, 0 ) == OBJECT_OPTION )
		{
			int nSize = GetListSize( nList );

			CIntArray arnListIdx, arnTypeIdx;
			arnListIdx.SetSize( nSize );
			arnTypeIdx.SetSize( nSize );

			// First go through all the list objects and fill the index
			// arrays.  We do this here so that we do not need to call
			// GetObjPtr() throughout the doubly nested sort loops below.
			for ( nIdx = 0; nIdx < nSize; nIdx++ )
			{
				Option *pOpt = (Option*) GetObjPtr( nList, nIdx );

				arnListIdx[ nIdx ] = nIdx;
				arnTypeIdx[ nIdx ] = OptionManager.nGetOptTypeIdx( pOpt->eGetOptType() );
			}

			// Bubble sort the type indexes, moving the list indexes along with
			// them so that we can have a quick lookup of where things go later.
			for ( nA = 0; nA < nSize - 1; nA++ )
			{
				for ( nB = nA + 1; nB < nSize; nB++ )
				{
					if ( arnTypeIdx[ nA ] > arnTypeIdx[ nB ] )
					{
						int nTemp = arnTypeIdx[ nA ];
						arnTypeIdx[ nA ] = arnTypeIdx[ nB ];
						arnTypeIdx[ nB ] = nTemp;

						nTemp = arnListIdx[ nA ];
						arnListIdx[ nA ] = arnListIdx[ nB ];
						arnListIdx[ nB ] = nTemp;
					}
				}
			}

			// Now reorder the subobjects based upon the sort order
			// calculated in the above loops.
			EMSubObjArray arCurrent;
			arCurrent.Copy( SubObjList[ nList ] );

			for ( nIdx = 0; nIdx < nSize; nIdx++ )
				SubObjList[ nList ][ nIdx ] = arCurrent[ arnListIdx[ nIdx ] ];
		}
	}
}

//-------------------------------------------------------------------//
// GetNextSlot()																		//
//-------------------------------------------------------------------//
// Very useful for looping through all the SubObjects within an
// EMComponent object.  Returns true if a valid slot was found.
//-------------------------------------------------------------------//
bool EMComponent::GetNextSlot( SubObjSlot &Slot )
{
	if ( Slot.m_nListIdx == -1 )
	{
		Slot.m_nArrayIdx++;
		if ( Slot.m_nArrayIdx < pEMInit->GetSubObjSize() )
			return true;
		else
		{
			Slot.m_nArrayIdx = -1;
			Slot.m_nListIdx = 0;
		}
	}

	int nLists = pEMInit->GetNumLists();

	if ( Slot.m_nListIdx >= nLists )
		return false;

	Slot.m_nArrayIdx++;

	while ( Slot.m_nArrayIdx >= GetListSize( Slot.m_nListIdx ) )
	{
		Slot.m_nArrayIdx = 0;
		Slot.m_nListIdx++;

		if ( Slot.m_nListIdx >= nLists )
			return false;
	}

	return true;
}


//-------------------------------------------------------------------//
// FillReportSelectionTree()														//
//-------------------------------------------------------------------//
// This method is used to fill the passed report selection tree
// with available reports for this EMComponent.
// Returns true if report items were added.
// This version adds the basic "Overview" reports as well as
// any nested subobject reports (recursively).
//-------------------------------------------------------------------//
bool EMComponent::FillReportSelectionTree(
	MultiSelectTreeControl &rTree,
	TypedRefArray &rarAdded,
	HTREEITEM hParent
){
	// Add a "general report" item first
	RSD_TreeItemData Data;
	Data.Attrib.m_nID = (int) GetObjectType();

	OBJECT_TYPE eReportTreeExpand;

	// This is not the root item.
	// Expand the Data vector to include the vector of the parent item.
	if ( hParent )
	{
		eReportTreeExpand = EMComponentInit::ByType( rarAdded[ 0 ] )->eReportTreeExpand();
		RSD_TreeItemData &ParentData = * ( (RSD_TreeItemData*) rTree.GetItemData( hParent ) );
		Data.Vector = ParentData.Vector;
	}
	// This is the root item.
	// Add a tree item for this object.
	else
	{
		eReportTreeExpand = pEMInit->eReportTreeExpand();
		UINT uiState = ( eReportTreeExpand <= GetObjectType() ) ? TVIS_EXPANDED : 0;
		CString strName = pEMInit->strDescSingular() + _T(" : ") + strDisplayName();

		hParent = rTree.InsertItem(
			&strName,					// Text to display
			(LPARAM) Data.pDup(),	// Any 4 bytes, use memory handle if needed
			uiState,						// Expanded and bold status
			rTree.GetRootItem()		// Parent node
		);
		rarAdded.AddOnce( GetTypedRef() );
	}

	// This does the actual work of adding the reports.
	// Had to create a second function for this.
	bool bAdded = FillReportSelectionTree_AddReports( rTree, Data, hParent, eReportTreeExpand );

	SubObjSlot Slot;
	while ( GetNextSlot( Slot ) )
	{
		TypedRef SubRef = GetSubObjTypedRef( Slot );

		// Add objects to the report tree only once.
		// Eliminate "back pointer" infinite loops.
		if (
				( SubRef.eType < GetObjectType() )
			&&	( !SubRef.IsBlank() )
			&&	( EMComponentInit::ByType( SubRef.eType )->HasSpecSheets() )
			&& ( NotFound == rarAdded.Search( SubRef ) )
		){
			rarAdded.Add( SubRef );

			RSD_TreeItemData SubData;
			SubData.Vector = Data.Vector;
			SubData.Vector.Add( Slot );

			EMComponent *pEMC = GetObjPtr( Slot );
			CString strName = pEMC->pEMInit->strDescSingular() + _T(" : ") + pEMC->strDisplayName();
			UINT uiState = ( eReportTreeExpand <= SubRef.eType ) ? TVIS_EXPANDED : 0;

			HTREEITEM hSubParent = rTree.InsertItem(
				&strName,						// Text to display
				(LPARAM) SubData.pDup(),	// Any 4 bytes, use memory handle if needed
				uiState,							// Expanded and bold status
				hParent							// Parent node
			);

			bAdded |= pEMC->FillReportSelectionTree( rTree, rarAdded, hSubParent );
		}
	}

	return bAdded;
}

//-------------------------------------------------------------------//
// FillReportSelectionTree_AddReports()										//
//-------------------------------------------------------------------//
// This method is used to fill the passed report selection tree
// with available reports for this EMComponent.
// Returns true if report items were added.
//-------------------------------------------------------------------//
bool EMComponent::FillReportSelectionTree_AddReports(
	MultiSelectTreeControl &rTree,	// The tree we are building up.
	RSD_TreeItemData &rData,			// The TreeItemData for the current object.
	HTREEITEM hParent,					// The parent HTREEITEM to add from.
	OBJECT_TYPE eReportTreeExpand		// The lowest object type for expand report selection tree branches.
){
	UINT uiState = ( eReportTreeExpand <= GetObjectType() ) ? TVIS_EXPANDED : 0;

	rTree.InsertItem(
		&ResStrStatic( IDS_GENERAL_REPORT ),	// Text to display
		(LPARAM) rData.pDup(),	// Any 4 bytes, use memory handle if needed
		uiState,						// Expanded and bold status
		hParent						// Parent node
	);

	return true;
}

//-------------------------------------------------------------------//
// GetTypedRef()																		//
//-------------------------------------------------------------------//
// Gets the typed object reference for this object.
//-------------------------------------------------------------------//
TypedRef EMComponent::GetTypedRef()
{
	return TypedRef( GetRef(), GetObjectType() );
}
