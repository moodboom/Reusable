#include <RandomHelpers.h>        // For get_random_uByte()

#include "TestData.h"


TestData::TestData( int nSize )
{
    // Create the data.
    m_nTestBytes = nSize;
    m_pTestData = new uByte[nSize];
    
    // Initialize the byte data to random values.
	for ( int nA = 0; nA < m_nTestBytes; ++nA )
	{
	    m_pTestData[nA] = get_random_uByte();
	}
}


TestData::~TestData()
{
    // Destroy the data.
    delete[] m_pTestData;
}


bool TestData::operator==( const TestData& tdCompare )
{
	// Compare the entire extent of the data for an exact match.
    if ( m_nTestBytes != tdCompare.m_nTestBytes )
        return false;
        
	// Compare the entire extent of the data for an exact match.
	for ( int nA = 0; nA < m_nTestBytes; ++nA )
	{
	    if ( m_pTestData[nA] != tdCompare.m_pTestData[nA] )
	        return false;
	}
	
	return true;
}
