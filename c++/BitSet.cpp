//-------------------------------------------------------------------//
// BitSet.cpp
//
// This class is used to create bit registers of variable length.
// The standard C++ libraries contain a similar <bitset> class,
// but it is a templated class that determines the # of bit fields
// at compile time.  This class differs in that only one set of 
// classes is created instead of one for each template.
//
//	Now, you can dynamically resize.  Updated to preserve data
// on calls to SetSize().  Of course, you will lose some data
// if you DOWNsize the number of bits.  Also added Toggle() and
// Set( nIndex, bOn ).  They sure ain't fancy!
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include <memory.h>					// for memcpy
#include <min_max_fix.h>			// for min/max

#include <stdio.h>					// for va_arg etc.
#include <stdarg.h>					//   "
// #include <varargs.h>					//   "

#include "AssertHelpers.h"

#include "BitSet.h"


//-------------------------------------------------------------------//
// constants																			//
//-------------------------------------------------------------------//

// Width of that type in bits.
const int RegWidth = ( sizeof( int /*BitReg*/ ) * 8 );

// Bit depth shift value.  This is the amount that a bit index
// will have to be right-shifted to get the register depth.
const int RegShift = 5;


//-------------------------------------------------------------------//
// BitSet()																				//
//-------------------------------------------------------------------//
// Construction
//-------------------------------------------------------------------//
BitSet::BitSet( int nSize )
:
	m_nSize	( -1		),
	m_pBits	( 0	),
	m_nDepth	( 0		)
{
	ASSERT( RegWidth == ( 1 << RegShift ) );

	SetSize( nSize );
}

//-------------------------------------------------------------------//
// BitSet()																				//
//-------------------------------------------------------------------//
// Copy constructor
//-------------------------------------------------------------------//
BitSet::BitSet( BitSet &SrcSet )
:
	m_nSize	( -1 ),
	m_pBits	( 0 )
{
	*this = SrcSet;
}

//-------------------------------------------------------------------//
// SetSize()																			//
//-------------------------------------------------------------------//
// Initialize the number of bit fields.
// This now preserves any previous data.
//-------------------------------------------------------------------//
void BitSet::SetSize( int nSize )
{
	if ( m_nSize != nSize )
	{
		// Store the old data temporarily.
		int nPrevDepth = m_nDepth;
		unsigned int* /*BitReg**/ pPrevBits = m_pBits;
		m_pBits = NULL;

		if ( nSize > 0 )
		{
			// Determine how many bit registers will be
			// needed to contain nSize bits.
			m_nDepth = 1 + ( ( nSize - 1 ) >> RegShift );

			// Allocate mem for those registers.
			m_pBits = new BitReg[ m_nDepth ];

			// Reset all the bits.
			Reset();
		}

		// Copy over any previous bit register memory
		// that fits in our new register, then delete
		// the old memory.
		if ( pPrevBits )
		{
			if ( m_pBits )
				memcpy( m_pBits, pPrevBits, min( m_nDepth, nPrevDepth ) * sizeof( BitReg ) );

			delete[] pPrevBits;
		}

		// Now update the final size.
		m_nSize = nSize;

	}
}


//-------------------------------------------------------------------//
// ~BitSet()																			//
//-------------------------------------------------------------------//
// Destruction
//-------------------------------------------------------------------//
BitSet::~BitSet()
{
	// Delete the bit register mem.
	if ( m_pBits )
		delete[] m_pBits;

	// For debug purposes...
	m_pBits = NULL;
}

//-------------------------------------------------------------------//
// Set()																					//
//-------------------------------------------------------------------//
// Sets a bit, as indexed by nIdx.  Not passing nIdx will Set
// all bits.
//-------------------------------------------------------------------//
void BitSet::Set( int nIdx )
{
    ASSERT( nIdx < m_nSize );
    if ( nIdx >= m_nSize )
        return;
        
	if ( nIdx > -1 )
	{
		ConvertIdx( nIdx );
		m_pBits[ m_nIdxDepth ] |= m_IdxBitMask;
	}
	else
	{
		for ( int nA = 0; nA < m_nDepth; nA++ )
			m_pBits[ nA ] = ( ~ ( (BitReg)0 ) );
	}
}

//-------------------------------------------------------------------//
// SetMany()																			//
//-------------------------------------------------------------------//
// Sets more than one bit, as indexed by any number of passed
// integers.  Pass -1 to end the list.
//-------------------------------------------------------------------//
void BitSet::SetMany( int nIdx ... )
{
	// Initalize the variable argument list.
	va_list VarList;
	va_start( VarList, nIdx );

	while ( nIdx != -1 )
	{
		Set( nIdx );
		nIdx = va_arg( VarList, int );
	}

	// Clean up the stack from the above operations.
	va_end( VarList );
}


//-------------------------------------------------------------------//
// Reset()																				//
//-------------------------------------------------------------------//
// Resets a bit, as indexed by nIdx.  Not passing nIdx will
// Reset all bits.
//-------------------------------------------------------------------//
void BitSet::Reset( int nIdx )
{
	if ( nIdx > -1 )
	{
		ConvertIdx( nIdx );
		m_pBits[ m_nIdxDepth ] &= ~m_IdxBitMask;
	}
	else
	{
		for ( int nA = 0; nA < m_nDepth; nA++ )
			m_pBits[ nA ] = 0;
	}
}


//-------------------------------------------------------------------//
// bToggle()																			//
//-------------------------------------------------------------------//
// This toggles and returns the new value of the given nIdx.
//-------------------------------------------------------------------//
bool BitSet::bToggle( int nIdx )
{
	Set( nIdx, !bIsSet( nIdx ) );
	return bIsSet( nIdx );
}


//-------------------------------------------------------------------//
// Set()																					//
//-------------------------------------------------------------------//
// This allows direct passing of a bool to Set or Reset.
//-------------------------------------------------------------------//
void BitSet::Set( int nIdx, bool bOn )
{
	if ( bOn )
		Set( nIdx );
	else
		Reset( nIdx );
}


//-------------------------------------------------------------------//
// bIsSet()																				//
//-------------------------------------------------------------------//
// Tests a bit, as indexed by nIdx.  Not passing nIdx will
// IsSet all bits.
//-------------------------------------------------------------------//
bool BitSet::bIsSet( int nIdx ) const
{
	if ( nIdx > -1 )
	{
		ConvertIdx( nIdx );
		return ( 0 != ( m_pBits[ m_nIdxDepth ] & m_IdxBitMask ) );
	}
	else
	{
		// Test each individual slot.
		// Do not just check bytes, as Set() and Reset() set
		// bytes to 0xFF and 0X00 without regard to actual size.
		// Here, we only want to test to the TRUE end of bitset.
		bool bTrue = false;
		for ( int nA = 0; nA < m_nSize; nA++ )
			bTrue |= bIsSet( nA );
		return bTrue;

		/*
		BitReg Test = 0;
		for ( int nA = 0; nA < m_nDepth; nA++ )
			Test |= m_pBits[ nA ];
		return ( 0 != Test );
		*/
	}
}

//-------------------------------------------------------------------//
// ConvertIdx()																		//
//-------------------------------------------------------------------//
// Internal utility function for converting an index
// to a depth and a bit mask.
//-------------------------------------------------------------------//
void BitSet::ConvertIdx( int nIdx ) const
{
	ASSERT( nIdx < m_nSize );

	m_nIdxDepth = nIdx >> RegShift;

	ASSERT( m_nIdxDepth < m_nDepth );

	int nBit = nIdx % RegWidth;
	m_IdxBitMask = 1 << nBit;

	ASSERT( m_IdxBitMask );
}

//-------------------------------------------------------------------//
// nFindNext()																			//
//-------------------------------------------------------------------//
// Finds the next Set bit, starting after bit nStartAfter.
// Returns m_nSize if no next Set bit found.  Starts at
// beginning if nStartAfter not passed.
//-------------------------------------------------------------------//
int BitSet::nFindNext( int nStartAfter )
{
  int nA;
	for ( nA = ( nStartAfter + 1 ); nA < m_nSize; nA++ )
		if ( bIsSet( nA ) )
			break;

	return nA;
}

//-------------------------------------------------------------------//
// nGetSize()																			//
//-------------------------------------------------------------------//
// Returns the current size.
//-------------------------------------------------------------------//
int BitSet::nGetSize()
{
	return m_nSize;
}


void BitSet::Insert( int nIdx, bool bNewValue )
{
	// If insertion was requested past the end of the array, ASSERT()!
	ASSERT( nIdx <= nGetSize() );

	// Expand.
	SetSize( nGetSize() + 1 );

	// Now move data to make room for new bit.
	for ( int nA = nGetSize() - 1; nA > nIdx; nA-- )
		Set( nA, bIsSet( nA - 1 ) );

	// Now set new bit.
	Set( nIdx, bNewValue );
}


void BitSet::Delete( int nIdx )
{
	// If deletion was requested past the end of the array, ASSERT()!
	ASSERT( nIdx < nGetSize() );

	// Now move data to prepare to get rid of the old bit.
	for ( int nA = nIdx; nA < nGetSize() - 1; nA++ )
		Set( nA, bIsSet( nA + 1 ) );

	// Now contract.
	SetSize( nGetSize() - 1 );
}


//-------------------------------------------------------------------//
// operator=()																			//
//-------------------------------------------------------------------//
// Assignment operator.  For now, only allow assignment of BitSets
// of the same size.  We may decide in the future to allow assigning
// a different size,  which would mean deleting and then
// reallocating the m_pBits array.
//-------------------------------------------------------------------//
BitSet& BitSet::operator=( BitSet &SrcSet )
{
	SetSize( SrcSet.nGetSize() );

	for ( int nA = 0; nA < m_nDepth; nA++ )
		m_pBits[ nA ] = SrcSet.m_pBits[ nA ];

	return *this;
}

//-------------------------------------------------------------------//
// operator==()																		//
//-------------------------------------------------------------------//
// Comparison operator.
//-------------------------------------------------------------------//
bool BitSet::operator==( BitSet &CmpSet )
{
	bool bReturn = false;

	if ( m_nSize == CmpSet.m_nSize )
	{
		bReturn = true;
		for ( int nA = 0; nA < m_nDepth; nA++ )
			bReturn &= ( m_pBits[ nA ] == CmpSet.m_pBits[ nA ] );
	}

	return bReturn;
}

//-------------------------------------------------------------------//
// operator|=()																		//
//-------------------------------------------------------------------//
BitSet& BitSet::operator|=( BitSet &SrcSet )
{
	SetSize( SrcSet.nGetSize() );

	for ( int nA = 0; nA < m_nDepth; nA++ )
		m_pBits[ nA ] |= SrcSet.m_pBits[ nA ];

	return *this;
}

//-------------------------------------------------------------------//
// operator&=()																		//
//-------------------------------------------------------------------//
BitSet& BitSet::operator&=( BitSet &SrcSet )
{
	SetSize( SrcSet.nGetSize() );

	for ( int nA = 0; nA < m_nDepth; nA++ )
		m_pBits[ nA ] &= SrcSet.m_pBits[ nA ];

	return *this;
}

//-------------------------------------------------------------------//
// operator|()																			//
//-------------------------------------------------------------------//
BitSet BitSet::operator|( BitSet &SrcSet )
{
	BitSet Set( *this );
	Set.SetSize( SrcSet.nGetSize() );

	for ( int nA = 0; nA < Set.m_nDepth; nA++ )
		Set.m_pBits[ nA ] |= SrcSet.m_pBits[ nA ];

	return Set;
}

//-------------------------------------------------------------------//
// operator&()																			//
//-------------------------------------------------------------------//
BitSet BitSet::operator&( BitSet &SrcSet )
{
	BitSet Set( *this );
	Set.SetSize( SrcSet.nGetSize() );

	for ( int nA = 0; nA < Set.m_nDepth; nA++ )
		Set.m_pBits[ nA ] &= SrcSet.m_pBits[ nA ];

	return Set;
}
