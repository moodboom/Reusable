//-------------------------------------------------------------------//
//	Random Helpers
//
// MDM	3/1/2006
// Now cross-platform.
//
// MDM 6/9/2015
// Now using c++11 features.
//
//	Copyright � 2001-2015 A better Software.
//-------------------------------------------------------------------//

// C++11 and beyond
#include <random>
#include <functional>

#include <cassert>              // This is also included in "utilities.hpp", I THINK through #include <cerrno> maybe?  I didn't track it down.

#include "RandomHelpers.hpp"

using namespace std;


std::random_device rd;
std::mt19937 random_number_engine(rd());                                                        // pseudorandom number generator

uniform_int_distribution<uint_fast32_t> bool_distribution(0, 1);                                // 0,1
auto bool_roll = bind(bool_distribution, random_number_engine);

uniform_int_distribution<uint8_t>  uint8_distribution (0, numeric_limits<uint8_t >::max());    // random uint8
auto uint8_roll = bind(uint8_distribution, random_number_engine);

uniform_int_distribution<uint16_t> uint16_distribution(0, numeric_limits<uint16_t>::max());    // random uint16
auto uint16_roll = bind(uint16_distribution, random_number_engine);

uniform_int_distribution<uint32_t> uint32_distribution(0, numeric_limits<uint32_t>::max());    // random uint32
auto uint32_roll = bind(uint32_distribution, random_number_engine);

uniform_int_distribution<uint64_t> uint64_distribution(0, numeric_limits<uint64_t>::max());    // random uint64
auto uint64_roll = bind(uint64_distribution, random_number_engine);


uint8_t  get_random_uint8 ()  { return uint8_roll (); }
uint16_t get_random_uint16()  { return uint16_roll(); }
uint32_t get_random_uint32()  { return uint32_roll(); }
uint64_t get_random_uint64()  { return uint64_roll(); }

uint32_t get_random_uint32(uint32_t uHighValue, uint32_t uLowValue)
{
    // REMEMBER HIGH VALUE IS FIRST, so we can default low to zero.
    // Perhaps a bad idea... let's make sure we don't make mistakes...
    assert(uLowValue < uHighValue);

    uniform_int_distribution<uint32_t> custom_distribution(uLowValue, uHighValue);
    auto custom_roll = bind(custom_distribution, random_number_engine);
    return custom_roll();
}

bool     get_random_bool()    { return (bool_roll() > 0); }
double   get_random_percent() { return uint64_roll() / ((double)numeric_limits<uint64_t>::max()); }


// DEPRECATED
// #include <AssertHelpers.h>
// #include "time.h"
// #include "stdlib.h"
/*
//-------------------------------------------------------------------//
// SeedAsNeeded()																		//
//-------------------------------------------------------------------//
// We need to seed rand() before we starting pulling numbers from it.
// If we repeatedly seed with the time in a tight loop, 
// we'll get the exact same numbers out of every call to rand().
// Seed the random number generator if it's been at least three
// to five seconds since we last did.  Sure, this process can still
// all be reversed to predict the random results, but not without
// a lot of information about the current time, current processing 
// environment, and even then still not likely easily.
//
// The only real alternative to this approach is to obtain a
// truly random seed (tough) from another source.
//-------------------------------------------------------------------//
void SeedAsNeeded()
{
	static time_t LastSeedTime = 0;

	time_t CurrentTime = time( 0 );
	
	if ( 
			LastSeedTime == 0
			||	abs( (int)( CurrentTime - LastSeedTime )  ) > 3 + rand() / ( RAND_MAX / 3 )
	) {                                                                                                                                                              

		srand( (unsigned int) CurrentTime );
		LastSeedTime = CurrentTime;
	}
}


//-------------------------------------------------------------------//
// usRandom()																		//
//-------------------------------------------------------------------//
// This function gets a random uShort between the specified range.
// We can only handle a spread of 0x7fff with this function.
//-------------------------------------------------------------------//
uShort usRandom(
	uShort	usHighValue,
	uShort	usLowValue
) {
	SeedAsNeeded();

	// We can only handle a spread of 0x7fff with this function.
	ASSERT( RAND_MAX == 0x7fff );
	ASSERT( usHighValue - usLowValue < 0x7fff );

	return usLowValue + ( (uShort) ( ( (uLong) rand() ) * ( usHighValue - usLowValue ) / RAND_MAX ) );
}


//------------------------------------------------------------------//
// ulRandom()                                                       //
//------------------------------------------------------------------//
// This function gets a random uLong between the specified range.
//------------------------------------------------------------------//
uLong ulRandom(
	uLong	ulHighValue,
	uLong	ulLowValue
) {
	ASSERT( ulHighValue - ulLowValue > 0 );
	
	SeedAsNeeded();

	return 
			ulLowValue															// low +
		+	(uLong) (
					(double) get_random_uLong()							// random
				*	(double) ( ulHighValue - ulLowValue )				// * % of range
				/	(double) 0xFFFFFFFF
			);
}


//------------------------------------------------------------------//
// get_random_bool()                                                //
//------------------------------------------------------------------//
bool get_random_bool()
{
	SeedAsNeeded();

	ASSERT( RAND_MAX == 0x7fff );
	return ( ( rand() / 2 ) / RAND_MAX > 0 );
}


//------------------------------------------------------------------//
// get_random_uByte()                                               //
//------------------------------------------------------------------//
uByte get_random_uByte()
{
	SeedAsNeeded();

	return (uByte)usRandom( 255 );
}


//-------------------------------------------------------------------//
// get_random_uShort()																//
//-------------------------------------------------------------------//
uShort get_random_uShort()
{
	SeedAsNeeded();

	return
			get_random_bool() ? 0x8000 : 0		// Covers 0x8000, also calls srand()
		+	rand();										// Covers 0X7fff
}

//-------------------------------------------------------------------//
// get_random_uLong()																//
//-------------------------------------------------------------------//
uLong get_random_uLong()
{
	SeedAsNeeded();

	return
			( get_random_bool() ? 0x80000000 : 0 )		// Covers 0x80000000, also calls srand()
		+	( ( (uLong) rand() ) << 16 )					// Covers 0x7fff0000
		+	( get_random_bool() ? 0x00008000 : 0 )		// Covers 0x00008000
		+	rand();												// Covers 0X00007fff
}


//------------------------------------------------------------------//
// get_random_uHuge()                                               //
//------------------------------------------------------------------//
uHuge get_random_uHuge()
{
	SeedAsNeeded();

	return
		( (uHuge) get_random_uLong() << 32 ) + get_random_uLong();
}
*/
