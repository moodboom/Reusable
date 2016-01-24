//-------------------------------------------------------------------//
// ComHelpers.h																		//
//
//	This module contains functions to make generic COM work easier.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef COM_HELPERS_H
	#define COM_HELPERS_H

#include "..\ustring.h"

HRESULT	GetComInterface(
	GUID					guidTarget,								// COM interface ID
	MULTI_QI*			pQI,										// Interfaces structure pointer - this will contain the interfaces on successful return!
	int					nQICount					= 1,			// Number of requested interfaces
	bool					bDisplayErrors			= false,		// Should we attempt to display errors?
	LPCTSTR				szCOMInterfaceName	= _T("")		// Interface name to display in error msg
);


ustring GetComError( 
	HRESULT hInterfaceRequestResult, 
	DWORD dwLastError 
);


#endif	// COM_HELPERS_H
