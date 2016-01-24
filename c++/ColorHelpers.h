//-------------------------------------------------------------------//
// ColorHelpers
//-------------------------------------------------------------------//
//	Copyright © 2001, 20002, 2003 A better Software.
//-------------------------------------------------------------------//

#ifndef COLOR_HELPERS_H
	#define COLOR_HELPERS_H

#include "StandardTypes.h"


inline int GetRFromRGB( const int nRGB ) { return   nRGB & 0x000000FF        ; }
inline int GetGFromRGB( const int nRGB ) { return ( nRGB & 0x0000FF00 ) >>  8; }
inline int GetBFromRGB( const int nRGB ) { return ( nRGB & 0x00FF0000 ) >> 16; }
inline int GetAFromRGB( const int nRGB ) { return ( nRGB & 0xFF000000 ) >> 24; }		// Alpha (transparency)

inline int CombineRGB( const int nRed, const int nGreen, const int nBlue, const int nAlpha = 0 ) 
{ 
	return 
			  nRed 
		+	( nGreen	<< 8	)
		+	( nBlue	<< 16	)
		+	( nAlpha	<< 24	)
	;
}


//-------------------------------------------------------------------//
// bColorMatches()																	//
//-------------------------------------------------------------------//
// This function determines if the two given colors are a match,
// using the spread to determine how close they can be.  
//-------------------------------------------------------------------//
bool bColorMatches( const int nRGB1, const int nRGB2, const int nAllowableSpread = 10 );


//-------------------------------------------------------------------//
// MoveColor()																			//
//-------------------------------------------------------------------//
// This function will "move" the first color away from the second
// color so that they no longer match.  See bColorMatches()
// for the definition of "match".
//
// The resulting color is returned.
//-------------------------------------------------------------------//
void MoveColor( int& nRGB1, const int nRGB2, const int nAllowableSpread = 10 );


#endif		// COLOR_HELPERS_H
