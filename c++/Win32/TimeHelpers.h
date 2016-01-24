//-------------------------------------------------------------------//
//	TimeHelpers.h
//	This module contains generic functions to help with time management.
//
// It relies on <windows.h>, which is probably a part of your
// precompiled header [stdafx.h].  Include one of the following
// before including this:
//
//			#include "stdafx.h"
//		OR	
//			#define _WIN32_WINNT 0x0400	(or greater)
//			#include <windows.h>
//			#include <TCHAR.h>
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined( TIME_HELPERS_H )
#define TIME_HELPERS_H

#include "AssertHelpers.h"	// For ASSERT(), VERIFY()
#include <sys/timeb.h>		// For struct _timeb
#include <ustring.h>       // For ConvertSecondsToTime

//-------------------------------------------------------------------//
// sleep()																				//
//-------------------------------------------------------------------//
// Pause the current thread for the specified length of time.
// We use a "waitable timer" so that the thread should go
// to near-zero CPU time.
//-------------------------------------------------------------------//
inline void sleep( DWORD dwMilliseconds )
{
	// Now why didn't somebody tell me this was done already?
	// Thanks Zac.
	
	Sleep( dwMilliseconds );
	/*
	HANDLE hTimer = NULL;
	LARGE_INTEGER liDueTime;

	// This value is supposed to be in "100 nanosecond intervals" WTF!  :>
	// It must be negative if it is a relative time.  See SetWaitableTimer()
	// for details.
	liDueTime.QuadPart = -10000 * dwMilliseconds;

	// Create a waitable timer.
	hTimer = CreateWaitableTimer(NULL, TRUE, _T("WaitableTimer") );
	ASSERT( hTimer );
	VERIFY( 
		SetWaitableTimer(
			hTimer, 
			&liDueTime, 
			0, 
			NULL, 
			NULL, 
			0
		)
	);

	// Wait for the timer.
	VERIFY( WaitForSingleObject( hTimer, INFINITE ) == WAIT_OBJECT_0 );
	*/
}


//-------------------------------------------------------------------//
// CalcTimeSpan()																		//
//-------------------------------------------------------------------//
// Determine the time that has elapsed, in seconds, between two
// time structures that are returned from _ftime().
//-------------------------------------------------------------------//
inline double CalcTimeSpan( 
	struct _timeb& tbStart,
	struct _timeb& tbStop
) {
	double dReturn = double( tbStop.time - tbStart.time );
	dReturn += ( tbStop.millitm - tbStart.millitm ) / 1000.0;
	return dReturn;
}


//-------------------------------------------------------------------//
// CalcTimeSpanMS()																	//
//-------------------------------------------------------------------//
// Determine the time that has elapsed, in milliseconds, between two
// time structures that are returned from _ftime().
//-------------------------------------------------------------------//
inline double CalcTimeSpanMS( 
	struct _timeb& tbStart,
	struct _timeb& tbStop
) {
	double dReturn = double( tbStop.time - tbStart.time ) * 1000;
	dReturn += ( tbStop.millitm - tbStart.millitm );
	return dReturn;
}


//-------------------------------------------------------------------//
// ConvertSecondsToTime()															//
//-------------------------------------------------------------------//
// Tries to make a nice readable display of the given seconds.
// The wide parameter forces hours:minutes:seconds for all values.
//-------------------------------------------------------------------//
inline ustring ConvertSecondsToTime( int nSeconds, bool bWide = false )
{
   const int cnSecPerMinute   = 60;
   const int cnSecPerHour     = cnSecPerMinute * 60;
   const int cnSecPerDay      = cnSecPerHour * 24;

   ustring strReturn;
   ustring strTemp;

   int nRemainingSeconds = nSeconds;
   if ( nSeconds > cnSecPerDay )
   {
      strReturn.AppendNumber( nRemainingSeconds / cnSecPerDay );
      strReturn += _T(" days ");
      nRemainingSeconds = nRemainingSeconds % cnSecPerDay;
   }

   if ( bWide || ( nSeconds > cnSecPerHour ) )
   {
      strTemp.printf( 50, _T("%02d"), nRemainingSeconds / cnSecPerHour );
      strReturn += strTemp;
      nRemainingSeconds = nRemainingSeconds % cnSecPerHour;
   }

   if ( bWide || ( nSeconds > cnSecPerMinute ) )
   {
      strTemp.printf( 50, _T(":%02d"), nRemainingSeconds / cnSecPerMinute );
      strReturn += strTemp;
      nRemainingSeconds = nRemainingSeconds % cnSecPerMinute;
   }

   strTemp.printf( 50, _T(":%02d"), nRemainingSeconds );
   strReturn += strTemp;

   return strReturn;
}


//-------------------------------------------------------------------//
// CalcMSUntilNextDailyRunTime()                                     //
//-------------------------------------------------------------------//
// This function performs a specific task: given any SYSTEMTIME, 
// return the number of ms until we hit the SYSTEMTIME's time of day.
// It's useful when a SYSTEMTIME is used to specify a daily timer.
//-------------------------------------------------------------------//
inline int CalcMSUntilNextDailyRunTime( SYSTEMTIME& stRun )
{
   // These are the only fields we care about, so we'll just worry about them.
   // See the notes below before attempting any FILETIME-based math...
   //
   //    stRun.wHour
   //    stRun.wMinute
   //    stRun.wSecond
   //    stRun.wMilliseconds
   //

   // Get the current time.
   SYSTEMTIME stNow;
   GetLocalTime( &stNow );

   int nRunMS = 
   (
         stRun.wMilliseconds
      +  stRun.wSecond        * 1000
      +  stRun.wMinute        * 1000 * 60
      +  stRun.wHour          * 1000 * 60 * 60     // (should max out at 86400000, which int can handle)
   );

   int nNowMS = 
   (
         stNow.wMilliseconds
      +  stNow.wSecond        * 1000
      +  stNow.wMinute        * 1000 * 60
      +  stNow.wHour          * 1000 * 60 * 60     // (should max out at 86400000, which int can handle)
   );


   // Make sure the next run time is AFTER the current time.
   // If it's less, just add a day.
   if ( nRunMS < nNowMS )
      nRunMS += (24*60*60*1000);

   return ( nRunMS - nNowMS );


   // NOTE ABOUT DEALING WITH FILETIME
   //
   // According to MSDN, we should...
   //
   //    1) convert SYSTEMTIME to FILETIME
   //    2) convert FILETIME to ULARGE_INTEGER (don't cast! copy members!)
   //    3) do math to calc relative time using ULARGE_INTEGER values
   //
   // NOTE: FILETIME consists of two DWORDs.
   // They have the same bit layout as an __int64.
   // But because two DWORDs may byte-align differently
   // than an __int64 (which must be on 8-byte alignment),
   // you shouldn't cast from one to the other.  You have
   // to copy.  See:
   //
   //    http://blogs.msdn.com/oldnewthing/archive/2004/08/25/220195.aspx
   //
   // Example:
   //
   //    FILETIME ft;
   //    ULARGE_INTEGER temp;
   //    temp.HighPart = ft.dwHighDateTime;
   //    temp.LowPart = ft.dwLowDateTime;
   //    // now ready to use temp.QuadPart
   // 
}


//-------------------------------------------------------------------//
// GetSeason()                                                       //
//-------------------------------------------------------------------//
// Here, we return:
//
//    0 for spring 
//    1 for summer 
//    2 for fall
//    3 for winter
//
// NOTES
// Spring begins on MAR 21, Summer begins on JUN 21, 
// Fall begins on SEP 23, and Winter begins on DEC 21.
//-------------------------------------------------------------------//
inline int GetSeason( COleDateTime& date )
{
   if ( date.GetMonth() < 3  || ( date.GetMonth() ==  3 && date.GetDay() < 21 ) )
      return 3;

   if ( date.GetMonth() < 6  || ( date.GetMonth() ==  6 && date.GetDay() < 21 ) )
      return 0;

   if ( date.GetMonth() < 9  || ( date.GetMonth() ==  9 && date.GetDay() < 23 ) )
      return 1;                                      

   if ( date.GetMonth() < 12 || ( date.GetMonth() == 12 && date.GetDay() < 21 ) )
      return 2;

   // Back to winter...
   return 3;
}

#endif // !defined(TIME_HELPERS_H)
