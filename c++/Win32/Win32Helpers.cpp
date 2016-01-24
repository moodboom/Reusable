//-------------------------------------------------------------------//
// Win32Helpers.cpp : Win32 (non-MFC) helper functions
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include <windows.h>

#include <lmerr.h>											// For error codes.

#include "Win32Helpers.h"


//-------------------------------------------------------------------//
// ConvertErrorToString()															//
//-------------------------------------------------------------------//
ustring ConvertErrorToString( DWORD dwError )
{	
	HMODULE 	hModule 	= NULL; // default to system source
	LPTSTR	MessageBuffer	= NULL;
	DWORD		dwBufferLength;

	ustring strError;

	// if dwError is in the network range, load the message source
	if (
			dwError >= NERR_BASE 
		&& dwError <= MAX_NERR
	) {
	    hModule = ::LoadLibraryEx( _TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE );
	}
 
	// call FormatMessage() to allow for message text to be acquired
	// from the system or the supplied module handle
	dwBufferLength = ::FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER 
		|	FORMAT_MESSAGE_IGNORE_INSERTS 
		|	FORMAT_MESSAGE_FROM_SYSTEM 
		|	// always consider system table
			((hModule != NULL) ? FORMAT_MESSAGE_FROM_HMODULE : 0),
		
		// module to get message from (NULL == system)
		hModule,					
		
		dwError,
		
		// default language
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	
		
		(LPTSTR) &MessageBuffer, 
		0, 
		NULL 
	);

	if ( MessageBuffer ) 
	{
		if ( dwBufferLength )
			strError = MessageBuffer;
 
		// free the buffer allocated by the system
		::LocalFree(MessageBuffer);
	}
 
	// if you loaded a message source, unload it
	if (hModule != NULL)
	    ::FreeLibrary(hModule);

	if ( strError.size() == 0 ) 
	{
		strError = _T( "Unknown System Error " );
		strError.AppendNumber( dwError );
	}

	return strError;
}


