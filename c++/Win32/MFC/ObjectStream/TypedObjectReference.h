//-------------------------------------------------------------------//
// TypedObjectReference.h
//
//	An ObjectReference combined with an OBJECT_TYPE.
//
// This class declaration moved here from EMComponent.h because it
// was needed by DataGrid.h and DataGrid did not need to have all
// that other stuff from EMComponent included as well.
//
//-------------------------------------------------------------------//
#ifndef TYPED_OBJECT_REFERENCE
#define TYPED_OBJECT_REFERENCE

#include "ObjectReference.h"
// #include "..\ObjectTypes.h"

//-------------------------------------------------------------------//
// TypedRef																				//
//-------------------------------------------------------------------//
class TypedRef
{
// Constructors
public:

	TypedRef( OBJECT_TYPE eObType = OBJECT_LIST_LEN ) :
		eType( eObType )
	{}

	TypedRef( ObjectReference &ObRef, OBJECT_TYPE eObType ) :
		Ref( ObRef ), eType( eObType )
	{}

	TypedRef( ObjectReference *pRef, OBJECT_TYPE eObType ) :
		Ref( *pRef ), eType( eObType )
	{}

// Operators
public:

	// Comparison operator.
	bool operator==( const TypedRef &Compare ) const
	{
		return (
				( eType == Compare.eType )
			&& ( Ref == Compare.Ref )
		);
	}

	bool operator==( const OBJECT_TYPE Compare ) const	{ return ( eType == Compare ); }

	// These operators work well with "=".
	operator ObjectReference()		{ return Ref;		}
	operator OBJECT_TYPE()			{ return eType;	}

// Operations
public:

	// Tells us if the TypedRef is missing data.
	bool IsBlank()
	{
		return ( ( Ref.IsBlank() ) || ( eType == OBJECT_LIST_LEN ) );
	}

// Data
public:

	ObjectReference	Ref;
	OBJECT_TYPE			eType;
};


// Some files that include TypedObjectReference may not need
// TypedRefArray, so we make the include of <afxtempl.h>
// and "EMArray.h" optional as well.
#ifdef EM_ARRAY_H
#ifdef __AFXTEMPL_H__

typedef EMArray < TypedRef, TypedRef& > TypedRefArray;

#endif
#endif


//-------------------------------------------------------------------//
// class SwapRefSet																	//
//-------------------------------------------------------------------//
// Utility class used to form an array of paired ObjectReferences
// that need to be swapped.
//-------------------------------------------------------------------//
class SwapRefSet
{
public:

	SwapRefSet() {}

	SwapRefSet(
		ObjectReference	SeekRef,
		ObjectReference	SwapRef,
		OBJECT_TYPE			eType
	):
		m_SeekRef	( SeekRef	),
		m_SwapRef	( SwapRef	),
		m_eType		( eType  	)
	{}

	ObjectReference	m_SeekRef;
	ObjectReference	m_SwapRef;
	OBJECT_TYPE			m_eType;
};

typedef CArray< SwapRefSet, SwapRefSet& > SwapRefArray;


//-------------------------------------------------------------------//
#endif // TYPED_OBJECT_REFERENCE													//
//-------------------------------------------------------------------//

