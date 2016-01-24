//-------------------------------------------------------------------//
// Standard variable types
//
// These types provide variables with known bit width.
// We will ensure that they will not fluctuate in size across compile 
// environments.  This is important for variables that will be stored
// on persistent media or that will be used across process boundaries.
//
//	Copyright � 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef STANDARD_TYPES_H
	#define STANDARD_TYPES_H


#include <stdint.h>             // Since C99, these are the correct standard types


// -------------------------------------------------------
// DEPRECATED, use newer C99 types like int_fast32_t
// See the above include file for details.
// -------------------------------------------------------

/*
// We will provide defines for a wide range of specifically-sized
// variable types here.  This is very important when dealing with
// portable binary formats (binary files, streamed data, etc.).

#ifdef WIN32

// Microsoft provides the following:
// 
//		__int8  nSmall;      // Declares  8-bit integer
//		__int16 nMedium;     // Declares 16-bit integer
//		__int32 nLarge;      // Declares 32-bit integer
//		__int64 nHuge;       // Declares 64-bit integer
// 
// Note that these are all signed integers.  

typedef __int8               Byte;		//  8-bit signed
typedef __int16              Short;		// 16-bit signed
typedef __int32              Long;		// 32-bit signed
typedef __int64              Huge;		// 64-bit signed

typedef unsigned char        uByte;             //  8-bit unsigned
typedef unsigned short int   uShort;	        // 16-bit unsigned
typedef unsigned int         uLong;		// 32-bit unsigned
typedef unsigned __int64     uHuge;		// 64-bit unsigned

typedef float                Float;		// 32-bit float
typedef double               Double;	        // 64-bit double

// #elif defined LINUX
#else

// MDM This include file already does exactly what we want, just map its types.
#include <inttypes.h>

typedef int8_t               Byte;		//  8-bit signed
typedef int16_t              Short;		// 16-bit signed
typedef int32_t              Long;		// 32-bit signed
typedef int64_t              Huge;		// 64-bit signed

typedef uint8_t              uByte;             //  8-bit unsigned
typedef uint16_t             uShort;	        // 16-bit unsigned
typedef uint32_t             uLong;		// 32-bit unsigned
typedef uint64_t             uHuge;		// 64-bit unsigned

typedef float                Float;		// 32-bit float
typedef double               Double;	        // 64-bit double

#elif defined OSX

TO DO
typedef __int8               Byte;		//  8-bit signed
typedef __int16              Short;		// 16-bit signed
typedef __int32              Long;		// 32-bit signed
typedef __int64              Huge;		// 64-bit signed

typedef unsigned char        uByte;             //  8-bit unsigned
typedef unsigned short int   uShort;	        // 16-bit unsigned
typedef unsigned int         uLong;		// 32-bit unsigned
typedef unsigned __int64     uHuge;		// 64-bit unsigned

typedef float                Float;		// 32-bit float
typedef double               Double;	        // 64-bit double

#endif
*/

#endif		// STANDARD_TYPES_H
