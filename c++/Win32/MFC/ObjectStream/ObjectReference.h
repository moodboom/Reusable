//-------------------------------------------------------------------//
// ObjectReference.h
//
//	This class uniquely identifies objects in ObjectStream databases.
// It is used to track an object, the database it resides in, the
// user who created it, and some additional info.
//
//	NOTE: ObjectReference contains a couple of nested classes, UserID
// and DatabaseID.  These classes all have overloaded operators.
// Previously the ObjectReference operators would call the UserID
// and DatabaseID operators in turn.  However, inlining wasn't
// going as deep as we had hoped, so in the interest of speed we
// simply copied the code from one function to the next up the class
// hierarchy.  Hence, if you change one you must update all the
// others.
//
//-------------------------------------------------------------------//
#ifndef ID_DEFINITIONS_H
	#define ID_DEFINITIONS_H

#include <afxtempl.h>					// For CTypedPtrArray

#include "..\StandardTypes.h"

// ============================
// Object Identification
// ============================
typedef uLong ObjectID;


///////////////////////////////////////////////////////////////
// VERSION 1.14 UPDATE
///////////////////////////////////////////////////////////////
//
//	The following update took place in version 1.14:
//
//		Old structure						New structure
//		--------------------------------------------------------
//		ObjectReference					ObjectReference
//			ObjectID								ObjectID        
//			DatabaseID							DatabaseID      
//				bPublished:1						UserNumber
//				DBNumber:31							DBNumber
//				UserID								   
//					UserNumber							
//					Group
//				DBVersion					
//
//												bPublished:1 moved to header
//												DBVersion:31 moved to header
//												Group moved to header
//
///////////////////////////////////////////////////////////////


// ============================
// Database Identification
// ============================
// An object of this type is maintained within each
// database object to uniquely identify it.
class DatabaseID
{

public:
	
	// Constructor, zero values if not supplied.
	DatabaseID( 
		uLong				NewUser	= 0, 
		uLong				DBNo		= 0 
	):
	
		// Init var's based on constructor values.
		UserNumber	(	NewUser			),
		DBNumber		(	DBNo				)

	{
		// Make sure we didn't exceed the bit limit on DBNo.
		ASSERT( DBNo <= 0xFFFFFFF );
	}

	// Member variables, directly accessible.

	// Type			Name				  Size	Description
	//-----------------------------(bits)-------------------------------------------
	uLong				DBNumber;		//	 4		User DB number, unique for each user's db
	uLong				UserNumber;		//	 4		User number, unique for each user


	////////////////////////////////////////////////////////////////
	// Defining methods of comparing ObjectReferences numerically
	// so that they can be sorted.  Sorting allows us to use quick
	// search when looking for an ObjectReference within an array
	// of ObjectReferences.
	//
	// See NOTE at top of file before modifying.

	const bool operator==( const DatabaseID& DBIDCompare ) const
	{
		return
				DBIDCompare.DBNumber		==	DBNumber
			&& DBIDCompare.UserNumber	== UserNumber;
	}

	const bool operator!=( const DatabaseID& DBIDCompare ) const
	{
		return
				DBIDCompare.DBNumber		!=	DBNumber
			||	DBIDCompare.UserNumber	!= UserNumber;
	}

	const bool operator<( const DatabaseID& DBIDCompare ) const
	{
		return (
				UserNumber < DBIDCompare.UserNumber
			||	(
						UserNumber	== DBIDCompare.UserNumber
					&&	DBNumber		<  DBIDCompare.DBNumber
				)
		);
	}

	const bool operator>( const DatabaseID& DBIDCompare ) const
	{
		return (
				UserNumber > DBIDCompare.UserNumber
			|| (
						UserNumber	== DBIDCompare.UserNumber
					&&	DBNumber		>  DBIDCompare.DBNumber
				)
		);
	}

	//
	////////////////////////////////////////////////////////////////

};


// ============================
// Reference Identification
// ============================
class ObjectReference
{

public:

	// Constructor, zero values if not supplied.
	// Note that we are relying on DatabaseID's
	// default memberwise assignment operator.
	ObjectReference( 
		DatabaseID	NewDBID	= DatabaseID(),
		ObjectID		NewObID	= 0
	) :
	
		// Init var's based on constructor values.
		DBID( NewDBID ),
		ObID( NewObID )

	{
	}

	DatabaseID	DBID;
	ObjectID		ObID;

	// Clear the reference.
	// Note that we are relying on DatabaseID's
	// default memberwise assignment operator.
	void Clear()
		{ DBID = DatabaseID(); ObID = 0; }

	// We should begin using this function instead of
	// the "== ObjectReference()" method in case the
	// conditions of "blankness" change in the future.
	// NOTE: Used to check the DBID here too, but since
	// new objects have the DBID of the default DB then
	// DBID was never blank here.
	bool IsBlank() const
	{
		return ( ObID == 0 );
	}


	////////////////////////////////////////////////////////////////
	// Defining methods of comparing ObjectReferences numerically
	// so that they can be sorted.  Sorting allows us to use quick
	// search when looking for an ObjectReference within an array
	// of ObjectReferences.
	//
	// See NOTE at top of file before modifying.

	const bool operator==( const ObjectReference& ObRefCompare ) const
	{
		return (
				( ObRefCompare.ObID					== ObID						)
			&&	( ObRefCompare.DBID.DBNumber		==	DBID.DBNumber			)
			&& ( ObRefCompare.DBID.UserNumber	== DBID.UserNumber		)
		);
	}

	const bool operator!=( const ObjectReference& ObRefCompare ) const
	{
		return (
				( ObRefCompare.ObID					!= ObID						)
			||	( ObRefCompare.DBID.DBNumber		!=	DBID.DBNumber			)
			|| ( ObRefCompare.DBID.UserNumber	!= DBID.UserNumber		)
		);
	}

	const bool operator<( const ObjectReference& ObRefCompare ) const
	{
		return (
			( DBID.UserNumber < ObRefCompare.DBID.UserNumber	) ||
			(
				( DBID.UserNumber == ObRefCompare.DBID.UserNumber ) &&
				(
					( DBID.DBNumber < ObRefCompare.DBID.DBNumber ) ||
					(
						( DBID.DBNumber == ObRefCompare.DBID.DBNumber ) &&
						( ObID < ObRefCompare.ObID )
					)
				)
			)
		);
	}

	const bool operator>( const ObjectReference& ObRefCompare ) const
	{
		return (
			( DBID.UserNumber > ObRefCompare.DBID.UserNumber	) ||
			(
				( DBID.UserNumber == ObRefCompare.DBID.UserNumber ) &&
				(
					( DBID.DBNumber > ObRefCompare.DBID.DBNumber ) ||
					(
						( DBID.DBNumber == ObRefCompare.DBID.DBNumber ) &&
						( ObID > ObRefCompare.ObID )
					)
				)
			)
		);
	}

	//
	////////////////////////////////////////////////////////////////

};


//-------------------------------------------------------------------//
//																							//
// CArchive >> and << operators													//
//																							//
//-------------------------------------------------------------------//
inline void operator << ( CArchive& ar, const DatabaseID &DBID )
{
 	ar << DBID.DBNumber;
	ar << DBID.UserNumber;
}

inline void operator >> ( CArchive& ar, DatabaseID &DBID )
{
 	ar >> DBID.DBNumber;
	ar >> DBID.UserNumber;
}

inline void operator << ( CArchive& ar, const ObjectReference &ObRef )
{
	ar << ObRef.DBID;
 	ar << ObRef.ObID;
}

inline void operator >> ( CArchive& ar, ObjectReference &ObRef )
{
	ar >> ObRef.DBID;
 	ar >> ObRef.ObID;
}


//-------------------------------------------------------------------//
// RefArray																				//
//-------------------------------------------------------------------//
// Make Ref ptr arrays available.
//-------------------------------------------------------------------//
typedef CTypedPtrArray	<CPtrArray, ObjectReference*>	RefArray;


#endif	// ID_DEFINITIONS_H
