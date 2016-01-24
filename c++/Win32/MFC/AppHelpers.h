//-------------------------------------------------------------------//
// MFC AppHelpers
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef APP_HELPERS_H
	#define APP_HELPERS_H

#include "ustring.h"		// For ustring, etc.

#include <afxdisp.h>				// For COleDateTime

CString			GetProgramFilename();			// "C:\Program Files\CorelDraw\PhotoPaint.exe"
CString			GetProgramPath();					// "C:\Program Files\CorelDraw"
CString			GetProgramName();					// "PhotoPaint"

COleDateTime	GetProgramCreationDate();

// These check the current OS version.
bool				bWin9X();
bool				bWin95OSR2OrGreater();
bool				bWin98();
bool				bWinNT4OrGreater();
bool				bWin2000OrGreater();
bool				bCommonControlsUpToDate( int nReqdVersion );
DWORD				GetDllVersion( LPCTSTR lpszDllName );

// This converts an error number returned by GetLastError()
// into the equivalent string, using Windows' FormatMessage().
ustring ConvertErrorToString( DWORD dwError );
	

#endif	// APP_HELPERS_H
