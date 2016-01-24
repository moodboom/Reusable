//-------------------------------------------------------------------//
// ColorHelpers
//-------------------------------------------------------------------//
//	Copyright © 2001, 20002, 2003 A better Software.
//-------------------------------------------------------------------//

#include <math.h>						// For abs()

#include "AssertHelpers.h"

#include "ColorHelpers.h"


//-------------------------------------------------------------------//
// bColorMatches()																	//
//-------------------------------------------------------------------//
// This function determines if the two given colors are a match,
// using the spread to determine how close they can be.  
// The match is done by individually comparing the R, G, and
// B values.  
//
// The spread should be a value between 0 and 255 that indicates
// the difference allowed between the individual RGB values of 
// each color while still considering the colors to be a match.
//-------------------------------------------------------------------//
bool bColorMatches( int nRGB1, const int nRGB2, const int nAllowableSpread )
{
	return 
			abs( GetRFromRGB( nRGB1 ) - GetRFromRGB( nRGB2 ) ) <= nAllowableSpread
		&&	abs( GetGFromRGB( nRGB1 ) - GetGFromRGB( nRGB2 ) ) <= nAllowableSpread
		&&	abs( GetBFromRGB( nRGB1 ) - GetBFromRGB( nRGB2 ) ) <= nAllowableSpread
	//	&&	abs( GetAFromRGB( nRGB1 ) - GetAFromRGB( nRGB2 ) ) <= nAllowableSpread
	;
}


//-------------------------------------------------------------------//
// GetColorAdjustment()																//
//-------------------------------------------------------------------//
// This is a local helper function for MoveColor.
// It adjusts an individual R/G/B/A component.
//-------------------------------------------------------------------//
int GetColorAdjustment( int nOldComponent, int nBaseComponent, int nAllowableSpread )
{
	int nReturn = nBaseComponent + nAllowableSpread * ( nBaseComponent > nOldComponent ? -1 : 1 );

	// ------------------------------------------------------//
	// Here, we check to see if we've gone "out of bounds".
	// We could just limit to the bounds, but that would
	// not guarantee that we have "moved" away from the base
	// color.  So if we have crossed a boundary, we basically
	// shift the new color the opposite direction away from the 
	// base.
	
	// First make sure we aren't going to go out of bounds
	// because of a ridiculous spread.
	ASSERT( nAllowableSpread >= 0 && nAllowableSpread < 128 );

	// Now we can be sure that adjusting the opposite direction
	// that we originally adjusted should leave us well within
	// bounds.
	if ( nReturn < 0 )
	{
		ASSERT( nBaseComponent > nOldComponent );
		nReturn = nBaseComponent + nAllowableSpread;
	}

	if ( nReturn > 255 )
	{
		ASSERT( nBaseComponent <= nOldComponent );
		nReturn = nBaseComponent - nAllowableSpread;
	}
	// ------------------------------------------------------//

	return nReturn;

}


//-------------------------------------------------------------------//
// MoveColor()																			//
//-------------------------------------------------------------------//
// This function will "move" the first color away from the second
// color so that they no longer match.  See bColorMatches()
// for the definition of "match".
//
// The resulting color is returned.
//-------------------------------------------------------------------//
void MoveColor( int& nRGB1, const int nRGB2, const int nAllowableSpread )
{
	if ( bColorMatches( nRGB1, nRGB2, nAllowableSpread ) )
	{
		int nRed		= GetRFromRGB( nRGB1 );
		int nGreen	= GetGFromRGB( nRGB1 );
		int nBlue	= GetBFromRGB( nRGB1 );
	//	int nAlpha	= GetAFromRGB( nRGB1 );

		int nBaseRed	= GetRFromRGB( nRGB2 );
		int nBaseGreen	= GetGFromRGB( nRGB2 );
		int nBaseBlue	= GetBFromRGB( nRGB2 );
	//	int nNewAlpha	= GetAFromRGB( nRGB2 );

		if ( abs( GetRFromRGB( nRGB1 ) - GetRFromRGB( nRGB2 ) ) <= nAllowableSpread ) { nRed	= GetColorAdjustment( nRed		, nBaseRed	, nAllowableSpread ); }
		if ( abs( GetGFromRGB( nRGB1 ) - GetGFromRGB( nRGB2 ) ) <= nAllowableSpread ) { nGreen	= GetColorAdjustment( nGreen	, nBaseGreen, nAllowableSpread ); }
		if ( abs( GetBFromRGB( nRGB1 ) - GetBFromRGB( nRGB2 ) ) <= nAllowableSpread ) { nBlue	= GetColorAdjustment( nBlue	, nBaseBlue	, nAllowableSpread ); }
	//	if ( abs( GetAFromRGB( nRGB1 ) - GetAFromRGB( nRGB2 ) ) <= nAllowableSpread ) { nAlpha	= GetColorAdjustment( nAlpha	, nBaseAlpha, nAllowableSpread ); }

		// Now rebuild.
		nRGB1 = CombineRGB( nRed, nGreen, nBlue );

	}

}


