//-------------------------------------------------------------------//
//	Random Helpers
//
//	Copyright � 2015 A better Software.
//-------------------------------------------------------------------//

#ifndef RANDOM_HELPERS_H
	#define RANDOM_HELPERS_H


#include <stdint.h>             // Since C99, these are the correct standard types

uint8_t  get_random_uint8();
uint16_t get_random_uint16();
uint32_t get_random_uint32();
uint64_t get_random_uint64();

uint32_t get_random_uint32(uint32_t uHighValue, uint32_t uLowValue = 0);

bool     get_random_bool();
double   get_random_percent();


// NOTE: DEPRECATED, if you used to use them, use the above types instead.
/*
#include "StandardTypes.h"      

// This will be called for you by the following
// functions.  You may wish to call it before doing
// any direct calls to rand() on your own (or
// before you call any functions that CALL rand(),
// such as "random_shuffle()", an STL algorithm ).
void SeedAsNeeded();

uShort usRandom(
	uShort	usHighValue,
	uShort	usLowValue		= 0
);

uLong ulRandom(
	uLong		ulHighValue,
	uLong		ulLowValue		= 0
);

bool    get_random_bool();
uByte   get_random_uByte();
uShort  get_random_uShort();
uLong   get_random_uLong();
uHuge   get_random_uHuge();
*/

#endif	// RANDOM_HELPERS_H
