//-------------------------------------------------------------------//
// CRegistryHelpers interface														//
//-------------------------------------------------------------------//
// This module adds helpers that make it a bit easier
// to use CStrings to get registry data.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef C_REGISTRY_HELPERS_H
#define C_REGISTRY_HELPERS_H

#include "StandardTypes.h"

#include "..\RegistryHelpers.h"				// This does the nitty gritty work for us.  
														// We include it here since it is almost
														// always needed by the caller as well.


CString	WindowsRegistry_GetCString( 
	HKEY				hBaseKey,						// IN
	const TCHAR*	pszLabel,						// IN
	TCHAR*			pszDefault				= 0,	// IN  (optional)
	bool*				pbFound					= 0	// OUT (optional)
);


/////////////////////////////////////////////////////////////////////////////

#endif	// C_REGISTRY_HELPERS_H
