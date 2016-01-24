//-------------------------------------------------------------------//
// Win32Helpers (non-MFC)
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef WIN32_HELPERS_H
	#define WIN32_HELPERS_H

#include "..\ustring.h"


//-------------------------------------------------------------------//
// Error functions.

// This converts an error number returned by GetLastError()
// into the equivalent string, using Windows' FormatMessage().
ustring ConvertErrorToString( DWORD dwError );

//-------------------------------------------------------------------//



#endif	// WIN32_HELPERS_H
