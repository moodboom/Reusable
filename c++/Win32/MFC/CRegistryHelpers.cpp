//-------------------------------------------------------------------//
// CRegistryHelpers Implementation
//-------------------------------------------------------------------//
// This module adds helpers that make it a bit easier
// to use CStrings to get registry data.  They follow the 
// strategy of GetProfileString().
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "CRegistryHelpers.h"


//-------------------------------------------------------------------//
// GetString()																			//
//-------------------------------------------------------------------//
CString	WindowsRegistry_GetCString( 
	HKEY				hBaseKey,		// IN
	const TCHAR*	pszLabel,		// IN
	TCHAR*			pszDefault,		// IN  (optional)
	bool*				pbFound			// OUT (optional)
) {
	
	DWORD dwType, dwCount;
	LONG lResult = RegQueryValueEx(
		hBaseKey, 
		pszLabel, 
		NULL, 
		&dwType,
		NULL, 
		&dwCount
	);
	if ( lResult != ERROR_SUCCESS )
	{
		// Since we did NOT find the registry setting, we will need
		// to set up a buffer with just enough room for the default
		// value, if one was provided.  BYTES INCLUDING NULL TERM!
		if ( pszDefault )
			dwCount = ( _tcsclen( pszDefault ) + 1 ) * sizeof TCHAR;
		else
			dwCount = sizeof TCHAR;
		
	}

	// Set up the correct size buffer and call WindowsRegistry_GetString().
	CString strResult;
	WindowsRegistry_GetString(
		hBaseKey,
		pszLabel,
		dwCount,
		strResult.GetBuffer( dwCount / sizeof(TCHAR) + 1 ),
		pszDefault,
		pbFound
	);
	strResult.ReleaseBuffer();
	return strResult;
	
}


