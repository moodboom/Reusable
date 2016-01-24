#ifndef TEST_DATA_H
#define TEST_DATA_H


#include <boost_helpers/serialization/serialization_headers.h>

#include <STLExtensions.h>                                     // For Byte, etc.


// This class represents test data.
// More specifically, a user-specified lump of random data.
// You can use this class as a simple starting point for your own serialized objects.
class TestData
{
    // =============
    // SERIALIZATION
    // =============
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int /* file_version */);
    // =============

public:

	// NOTE: boost::serialize requires the default constructor.
	// If we have a constructor at all, we have to make sure we
	// include a no-param (default) constructor.
	// If we have no constructors of our own, no worries, the compiler
	// provides the default.

    TestData( int nSize = 1024 );
    virtual ~TestData();
    
	bool operator==( const TestData& tdCompare );
	
	int Size() const { return m_nTestBytes; }
	int Checksum() const 
	{
	    int nCS = 0;
	    for ( int nA = 0; nA < m_nTestBytes; nA++ )
	        nCS += m_pTestData[nA];
	    return nCS;
	}

protected:

    int m_nTestBytes;
    uByte* m_pTestData;
};


// Template implementation
#include "TestData.hpp"


typedef boost::shared_ptr<TestData> TestDataSharedPtr;


#endif  // TEST_DATA_H
