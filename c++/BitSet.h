#ifndef BIT_SET_H
#define BIT_SET_H

//-------------------------------------------------------------------//
// BitSet.h
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


//-------------------------------------------------------------------//
// defines					//
//-------------------------------------------------------------------//

// Bit register base data type.
typedef unsigned int BitReg;


//-------------------------------------------------------------------//
// class BitSet																		//
//-------------------------------------------------------------------//
class BitSet
{
public:

// Construction

	// Default constructor.
	BitSet( int nSize = 0 );

	// Copy constructor.
	BitSet( BitSet &SrcSet );

	~BitSet();

// Bit Operations

	// Set the number of bit fields.
	// Note that you may call this function to dynamically
	// resize the bit set.  Data is preserved, except when
	// decreasing size, in which case data is truncated at
	// the new size.
	void SetSize( int nSize );

	// Sets a bit, as indexed by nIdx.  Not passing nIdx will Set
	// all bits.
	void Set( int nIdx = -1 );

	// Sets more than one bit, as indexed by any number of passed
	// integers.  Pass -1 to end the list.
	void SetMany( int nIdx ... );

	// Resets a bit, as indexed by nIdx.  Not passing nIdx will
	// Reset all bits.
	void Reset( int nIdx = -1 );

	// Tests a bit, as indexed by nIdx.  Not passing nIdx will
	// IsSet all bits.
	bool bIsSet( int nIdx = -1 ) const;

	// Finds the next Set bit, starting after bit nStartAfter.
	// Returns m_nSize if no next Set bit found.  Starts at
	// beginning if nStartAfter not passed.
	int nFindNext( int nStartAfter = -1 );

	// Returns the current size.
	int nGetSize();

	// This toggles and returns the new value of the given nIdx.
	bool bToggle( int nIdx );

	// This allows direct passing of a bool to Set or Reset.
	void Set( int nIdx, bool bOn );

	// These allow dynamic insertion and deletion.
	void Insert( int nIdx, bool bNewValue = true );
	void Delete( int nIdx );

	// These provide direct access to the data, use with caution.
	unsigned int* /*BitReg**/	GetBitData()		{ return m_pBits;		}
	int		GetBitDataDepth()	{ return m_nDepth;	}

// Overloaded Operators

	BitSet& operator=( BitSet &SrcSet );

	bool operator==( BitSet &CmpSet );

	BitSet& operator|=( BitSet &SrcSet );
	BitSet& operator&=( BitSet &SrcSet );

	BitSet operator|( BitSet &SrcSet );
	BitSet operator&( BitSet &SrcSet );

private:

	// Internal utility function for converting an index
	// to a depth and a bit mask.
	void ConvertIdx( int nIdx ) const;

private:

//	Attributes

	// The nSize passed in the constructor, which represents the
	// number of bits in the BitSet.
	int		m_nSize;

	// The number of BitReg registers needed to store m_nSize bits.
	int		m_nDepth;

	// The array of BitReg registers used to store m_nSize bits.
	unsigned int* /*BitReg**/	m_pBits;

   // Temp use members, as extracted from nIdx by ConvertIdx();
   // WOW I get to use "mutable", whoop!
   mutable unsigned int /*BitReg*/	m_IdxBitMask;	// The mask of the bit.
   mutable int		m_nIdxDepth;	// The depth of the bit in the m_pBits array.


};


#endif	// BIT_SET_H																//
