//-------------------------------------------------------------------//
// BaseDateTime.h
//
// This class builds on the COleDateTime classes of MFC.  See them
// for more information.  Feel free to add to this as you need
// additional functionality.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//
#ifndef BASE_DATE_TIME_H
#define BASE_DATE_TIME_H

#include <afxdisp.h>				// Base classes COleDateTime, COleDateTimeSpan


class BaseDateTime : public COleDateTime
{
public:

// Construction
	
	// Here we duplicate all the base class constructors.  We just call the base class.  A minor PITA.
	BaseDateTime(																						) : COleDateTime(														) {}
	BaseDateTime( const COleDateTime& dateSrc													) : COleDateTime( dateSrc											) {}
	BaseDateTime( const VARIANT& varSrc															) : COleDateTime( varSrc											) {}
	BaseDateTime( DATE dtSrc																		) : COleDateTime( dtSrc												) {}
	BaseDateTime( time_t timeSrc																	) : COleDateTime( timeSrc											) {}
	BaseDateTime( const SYSTEMTIME& systimeSrc												) : COleDateTime( systimeSrc										) {}
	BaseDateTime( const FILETIME& filetimeSrc													) : COleDateTime( filetimeSrc										) {}
	BaseDateTime( int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec	) : COleDateTime( nYear, nMonth, nDay, nHour, nMin, nSec	) {}
	BaseDateTime( WORD wDosDate, WORD wDosTime												) : COleDateTime( wDosDate, wDosTime							) {}

	// We needed to add the "!=" operator.  MS's version returns BOOL, which pops
	// a whole mess of compiler warnings.  Fools.
	const bool operator != (const BaseDateTime& ctCompare)	{ return !( *this == ctCompare ); }
	
// ADD YOUR OWN FUNCTIONALITY HERE.

	// See implementation for output examples.
	CString strConciseDateTimeOutput();

};


class BaseDateTimeSpan : public COleDateTimeSpan
{
public:

// Construction

	// Here we duplicate all the base class constructors.  We just call the base class.  A minor PITA.
	BaseDateTimeSpan(																) : COleDateTimeSpan( 										) {}
	BaseDateTimeSpan( double dblSpanSrc										) : COleDateTimeSpan( dblSpanSrc							) {}
	BaseDateTimeSpan( const COleDateTimeSpan& dateSpanSrc				) : COleDateTimeSpan( dateSpanSrc						) {}
	BaseDateTimeSpan( long lDays, int nHours, int nMins, int nSecs	) : COleDateTimeSpan( lDays, nHours, nMins, nSecs	) {}

// ADD YOUR OWN FUNCTIONALITY HERE.

	// See implementation for output examples.
	CString strNiceOutput();
	CString strConciseOutput();

};

//-------------------------------------------------------------------//
#endif	// BASE_DATE_TIME_H
//-------------------------------------------------------------------//













