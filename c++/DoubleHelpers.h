//-------------------------------------------------------------------//
// DoubleHelpers
//-------------------------------------------------------------------//
// These helpers mainly deal with double comparison, including 
// comparison to zero.  The basic concept is to elicit the use
// of a "minimum valid value" to do the comparison.  There is
// a default minimum value, or the caller can provide one.
//
//	Copyright © 2001, 2002, 2003 A better Software.
//-------------------------------------------------------------------//

#ifndef DOUBLE_HELPERS_H
	#define DOUBLE_HELPERS_H

#include <math.h>							// For fabs()
#include "AssertHelpers.h"				// For ASSERT(), duh.

///////////////////////////////////////////////////////////////////
// Double Helpers
///////////////////////////////////////////////////////////////////
// This module contains functions to help with double calculations.
//
// One of the biggest concerns with doubles is determining an
// exact value.  Because the precision is so high, exact values
// are often hard to determine.  This concern is handled by
// defining the DBL_EPSILON value.
//
// DBL_EPSILON is the smallest positive number x, such that 
//	x + 1.0 is not equal to 1.0.  
//
// However, according to MSDN docs:
//
//		Because this is a very small number it is advisable that you 
//		employ user-defined tolerance for calculations involving very 
//		large numbers. 
//
// DBL_EPSILON = 2.2204460492503131e-016 for VC++ 6.0.
// We use a constant of slightly greater value to make it a bit 
// more certain:
//
const double cdAbsoluteMinValue = 1.0e-014;
// 
// This is still a very small number.  You should be able to
// use the functions here with great confidence in the majority
// of situations.
//
// Note that most of these functions allow you to specify your 
// own value in place of our choice for DBL_EPSILON.  Because 
// our value is still very small, you should supply your own 
// value using the highest number possible whenever practical,
// so that your convergance loops are optimized, etc.
//
// Note that there are typically two versions of each function,
// one that uses the const min value, and another that lets
// the caller specify a value.  The functions could have been 
// combined into one with a default parameter, but they were 
// optimized to be as "inline" as possible, eliminating passed 
// parameter data wherever possible.
//
///////////////////////////////////////////////////////////////////


// Note: Goofy MS claims that you may use the inline specifier for a 
// function, and then place the body of the code in the implementation
// file.  We have not found this to be the case.  So all inline code is
// contained within this header file.


//-------------------------------------------------------------------//
// IsNonZero()																			//
//-------------------------------------------------------------------//
// This function determines if a double has a large enough
// absolute value to be considered non-zero.
//-------------------------------------------------------------------//
inline bool IsNonZero( 
	double dValue
) {
	return ( fabs( dValue ) > cdAbsoluteMinValue );
}
inline bool IsNonZero( 
	double dValue, 
	double dMinValue
) {
	return ( fabs( dValue ) > dMinValue );
}


//-------------------------------------------------------------------//
// IsZero()																				//
//-------------------------------------------------------------------//
// This function determines if a double is "almost nearly all 
// the way" zero.  
//-------------------------------------------------------------------//
inline bool IsZero( 
	double dValue
) {
	return ( fabs( dValue ) < cdAbsoluteMinValue );
}
inline bool IsZero( 
	double dValue, 
	double dMinValue
) {
	return ( fabs( dValue ) < dMinValue );
}


//-------------------------------------------------------------------//
// AreEqual()																			//
//-------------------------------------------------------------------//
// This function determines if the given values are virtually
// the same.
//-------------------------------------------------------------------//
inline bool AreEqual(
	double dVal1, 
	double dVal2
) {
	return ( fabs( dVal1 - dVal2 ) < cdAbsoluteMinValue );
}
inline bool AreEqual(
	double dVal1, 
	double dVal2, 
	double dMinValue
) {
	return ( fabs( dVal1 - dVal2 ) < dMinValue );
}


//-------------------------------------------------------------------//
// IsGreaterThanOrEqualTo()														//
//-------------------------------------------------------------------//
// Returns true if the values are close, or the first is larger.
//-------------------------------------------------------------------//
inline bool IsGreaterThanOrEqualTo(
	double dVal1, 
	double dVal2
) {
	return dVal1 > dVal2 - cdAbsoluteMinValue;
}
inline bool IsGreaterThanOrEqualTo(
	double dVal1, 
	double dVal2, 
	double dMinValue
) {
	return dVal1 > dVal2 - dMinValue;
}


//-------------------------------------------------------------------//
// IsLessThanOrEqualTo()															//
//-------------------------------------------------------------------//
// Returns true if the values are close, or the first is smaller.
//-------------------------------------------------------------------//
inline bool IsLessThanOrEqualTo(
	double dVal1, 
	double dVal2
) {
	return dVal1 < dVal2 + cdAbsoluteMinValue;
}
inline bool IsLessThanOrEqualTo(
	double dVal1, 
	double dVal2, 
	double dMinValue
) {
	return dVal1 < dVal2 + dMinValue;
}


//-------------------------------------------------------------------//
// IsGreaterThan()																	//
//-------------------------------------------------------------------//
// Returns true if the first value is (significantly) larger.
//-------------------------------------------------------------------//
inline bool IsGreaterThan(
	double dVal1, 
	double dVal2
) {
	return dVal1 > dVal2 + cdAbsoluteMinValue;
}
inline bool IsGreaterThan(
	double dVal1, 
	double dVal2, 
	double dMinValue
) {
	return dVal1 > dVal2 + dMinValue;
}


//-------------------------------------------------------------------//
// IsLessThan()																		//
//-------------------------------------------------------------------//
// Returns true if the first value is (significantly) smaller.
//-------------------------------------------------------------------//
inline bool IsLessThan(
	double dVal1, 
	double dVal2
) {
	return dVal1 < dVal2 - cdAbsoluteMinValue;
}
inline bool IsLessThan(
	double dVal1, 
	double dVal2, 
	double dMinValue
) {
	return dVal1 < dVal2 - dMinValue;
}


//-------------------------------------------------------------------//
// SafeDivide()																		//
//-------------------------------------------------------------------//
// This function performs a division, but only after assuring that
// the divisor is not zero.  If the divisor is zero, we assert and
// return zero.
//-------------------------------------------------------------------//
inline double SafeDivide( 
	double	dNum,
	double	dDenom
) {
	if ( IsNonZero( dDenom ) )
		return dNum / dDenom;
	else {
		ASSERT( false );
		return 0.0;
	}
}


//-------------------------------------------------------------------//
// dRound()																				//
//-------------------------------------------------------------------//
// Precision rounding function.  Rounds off at the decimal point.
//-------------------------------------------------------------------//
inline double dRound( double dNum )
{
	return floor( 0.5 + dNum );
}

//-------------------------------------------------------------------//
// dRound()																				//
//-------------------------------------------------------------------//
// Precision rounding function.
// Can round above or below the decimal point ( + below, - above )
//-------------------------------------------------------------------//
inline double dRound( double dNum, int nPrecision )
{
	double dRndPrec = pow( 10.0, nPrecision );
	dNum = ( dRound( dNum * dRndPrec ) / dRndPrec );
	return dNum;
}

#endif	// DOUBLE_HELPERS_H
