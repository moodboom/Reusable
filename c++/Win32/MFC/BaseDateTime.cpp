//-------------------------------------------------------------------//
// BaseDateTime.cpp
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "BaseDateTime.h"

//-------------------------------------------------------------------//
// strConciseDateTimeOutput()														//
//-------------------------------------------------------------------//
// Examples:
//
//		
//
//-------------------------------------------------------------------//
CString BaseDateTime::strConciseDateTimeOutput()
{
	CString strOutput;
	
	strOutput = Format( _T("%Y %m/%d %H:%M") );				// Numerically sortable version.
	// strOutput = Format( _T("%m/%d/%Y %I:%M &p") );		// Visually appropriate version - don't use until sorting mechanism is up to the task.

	return strOutput;
}


//-------------------------------------------------------------------//
// strNiceOutput()																	//
//-------------------------------------------------------------------//
// COleDateTime has some functionality for output, but we want to
// make it more automatic.  Output will be in the following form:
//
//		3 d, 5 hr, 23 min, 3 sec
//		5 hr, 23 min, 3 sec
//		23 min, 3 sec
//		3 sec
//		3 d, 3 sec
//		etc.
//
//-------------------------------------------------------------------//
CString BaseDateTimeSpan::strNiceOutput()
{
	CString strOutput;
	CString strTemp;

	if ( GetDays() )
	{
		strTemp.Format( _T("%d d, "), GetDays() );
		strOutput += strTemp;
	}

	if ( GetHours() )
	{
		strTemp.Format( _T("%d hr, "), GetHours() );
		strOutput += strTemp;
	}

	if ( GetMinutes() )
	{
		strTemp.Format( _T("%d min, "), GetMinutes() );
		strOutput += strTemp;
	}

	if ( GetSeconds() )
	{
		strTemp.Format( _T("%d sec, "), GetSeconds() );
		strOutput += strTemp;
	}

	if ( strOutput.GetLength() > 0 )
	{
		strOutput = strOutput.Left( strOutput.GetLength() - 2 );
	} else
	{ 
		strOutput = _T("0 s");
	}

	return strOutput;
}

//-------------------------------------------------------------------//
// strConciseOutput()																//
//-------------------------------------------------------------------//
// A concise output format.
//
// Right-justify this output to get numbers to align.
//
//		"253:05:03:03"
//		"13:05:03:03"
//		"3:05:03:03"
//		"15:03:03"
//		"5:03:03"
//		"13:03"
//		"3:03"
//		":03"
//
//-------------------------------------------------------------------//
CString BaseDateTimeSpan::strConciseOutput()
{
	CString strOutput;
	CString strTemp;

	if ( GetDays() )
	{
		strTemp.Format( _T("%d:"), GetDays() );
		strOutput += strTemp;
	}

	if ( GetHours() && !strOutput.IsEmpty() )
	{
		strTemp.Format( _T("%02d:"), GetHours() );
		strOutput += strTemp;
	} else if ( GetHours() )
	{
		strTemp.Format( _T("%d:"), GetHours() );
		strOutput += strTemp;
	}

	if ( GetMinutes() && !strOutput.IsEmpty() )
	{
		strTemp.Format( _T("%02d:"), GetMinutes() );
		strOutput += strTemp;
	} else if ( GetMinutes() )
	{
		strTemp.Format( _T("%d:"), GetMinutes() );
		strOutput += strTemp;
	}

	// Always add the pre-seconds colon.
	if ( strOutput.IsEmpty() )
		strOutput = _T(":");

	// Always add seconds.
	strTemp.Format( _T("%02d"), GetSeconds() );
	strOutput += strTemp;

	return strOutput;
}


