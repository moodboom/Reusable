//-------------------------------------------------------------------//
//	Math Helpers
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef MATH_HELPERS_H
	#define MATH_HELPERS_H

#include <math.h>					// For atan2()

static double s_pi = acos(-1);


//-------------------------------------------------------------------//
// bPointIsWithinPolygon()															//
//-------------------------------------------------------------------//
// Point-in-a-polygon solutions:
//		http://www.ecse.rpi.edu/Homepages/wrf/research/geom/pnpoly.html
//		http://mathforum.org/dr.math/problems/scott5.31.96.html
//-------------------------------------------------------------------//
inline bool bPointIsWithinPolygon(
	double	dX,
	double	dY,
	int nPoints,
	double*	pdXPoints,
	double*	pdYPoints
) {
	bool bReturn = false;
	for ( int i = 0, j = nPoints - 1; i < nPoints; j = i++ )
	{
		if (
					(
							(
									( pdYPoints[i] <= dY ) 
								&& ( dY < pdYPoints[j] )
							) 
						||	(
									( pdYPoints[j] <= dY ) 
								&&	( dY < pdYPoints[i] )
							)
					) 
				&&
					( dX < ( pdXPoints[j] - pdXPoints[i] ) * ( dY - pdYPoints[i] ) / ( pdYPoints[j] - pdYPoints[i] ) + pdXPoints[i] ) 
		) {

			bReturn = !bReturn;
		}
	}
	
	return bReturn;

}


//-------------------------------------------------------------------//
// GetLineSlope()																		//
//-------------------------------------------------------------------//
// Returns the slope of the line defined by the two input points.
// Note that this function correctly returns a result that can fall
// anywhere within a full 360-degrees, depending on the order of
// the input points.
//-------------------------------------------------------------------//
inline double GetLineSlope(
	double dP1_x, 
	double dP1_y,
	double dP2_x, 
	double dP2_y
) {
	// atan2() is nice, it will consider the sign of the two
	// input values and return a result in the correct quadrant
	// (so we don't have to do this after calling atan():)
	//		if ( dP2_x < dP1_x )
	//			fAlpha += 3.1416;		// 180 degrees
	return atan2( dP2_y - dP1_y, dP2_x - dP1_x );
}

#endif	// MATH_HELPERS_H