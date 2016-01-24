//-------------------------------------------------------------------//
//	RegistryHelpers
//
//	This module contains generic functions to read from and save to
// the Windows registry.  TCHAR strings, DWORD's, and binary data
// are handled.  MFC is NOT required.  See the AppRegistryData class
// for a nice MFC WinApp-associated wrapper for these functions.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

// #include "stdafx.h"
#include <windows.h>
#include <TCHAR.H>

#include "AssertHelpers.h"				// For ASSERT() and VERIFY()

#include "RegistryHelpers.h"


//-------------------------------------------------------------------//
// WindowsRegistry_GetString()													//
//-------------------------------------------------------------------//
void	WindowsRegistry_GetString( 
	HKEY				hBaseKey,				// IN
	const TCHAR*	pszLabel,				// IN
	DWORD&			dwResultBufferSize,	// IN, OUT
	TCHAR*			pszResult,				// OUT (you must supply the buffer space)
	TCHAR*			pszDefault,				// IN	 (optional)
	bool*				pbFound					// OUT (optional)
) {
	bool		bFound = false;

	ASSERT( pszResult );
	pszResult[0] = _T('\0');
	
	// Attempt to get the path from the registry.
	LONG lResult = ::RegQueryValueEx(
						hBaseKey,					// HKEY		hKey,				handle of key to query            
						pszLabel,				// LPTSTR	lpValueName,	address of name of value to query 
						0,							// LPDWORD	lpReserved,		reserved                          
						0,							// LPDWORD	lpType,			address of buffer for value type  
		(PBYTE)		pszResult,				// LPBYTE	lpData,			address of data buffer            
		(LPDWORD)	&dwResultBufferSize	// LPDWORD	lpcbData			address of data buffer size       
	);

  	// If we found a registry "key" "value" "data"...
	// ( who came UP with that garbage terminology?? )
	errno = 0;
	if ( lResult == ERROR_SUCCESS ) 
	{
		bFound = true;

	// If we didn't find anything and the default was specified...
	} else if ( pszDefault ) 
	{
		// Place it in the registry.
		// We pragma away the warning that "== has no effect, did u intend =?"
		#pragma warning(disable : 4553)
		VERIFY( 
			RegSetValueEx(
				hBaseKey,
				pszLabel,
				0,
				REG_SZ,
				(CONST BYTE*) pszDefault,
				( _tcslen( pszDefault ) + 1 ) * sizeof TCHAR		// BYTES INCLUDING NULL TERM!
			) == ERROR_SUCCESS
		);
		#pragma warning(default : 4553)

		// Copy the default to the result.
		_tcscpy( pszResult, pszDefault );

	}

	// We USED to clean up here, now we leave that to the caller.
	// VERIFY( RegCloseKey( hBaseKey ) == ERROR_SUCCESS );

	if ( pbFound )
		*pbFound = bFound;

}


//-------------------------------------------------------------------//
// WindowsRegistry_GetNewString()												//
//-------------------------------------------------------------------//
// Here, we extract the size of the string from the registry,
// and allocate an appropriately sized buffer to load.
// As the caller, you will need to clean up the allocation
// provided for pszResult.
//-------------------------------------------------------------------//
TCHAR* WindowsRegistry_GetNewString( 
	HKEY				hBaseKey,				// IN
	const TCHAR*	pszLabel,				// IN
	TCHAR*			pszDefault,				// IN
	DWORD*			pdwResultBufferSize,	// OUT
	bool*				pbFound					// OUT
) {
	TCHAR*	pszResult;
	bool		bFound = false;

	// Get the SIZE of the string, if it exists.
	DWORD dwResultBufferSize;
	LONG lResult = ::RegQueryValueEx(
						hBaseKey,				// HKEY		hKey,				handle of key to query            
						pszLabel,				// LPTSTR	lpValueName,	address of name of value to query 
						0,							// LPDWORD	lpReserved,		reserved                          
						0,							// LPDWORD	lpType,			address of buffer for value type  
						NULL,						// LPBYTE	lpData,			address of data buffer            
		(LPDWORD)	&dwResultBufferSize	// LPDWORD	lpcbData			address of data buffer size       
	);

	if ( lResult != ERROR_SUCCESS ) 
	{
		if ( pszDefault )
		{
			// If we didn't find a registry "key" "value" "data",
			// but there is a default, use its size.
			dwResultBufferSize = _tcsclen( pszDefault );

		} else
		{
			// There is no key and a default was not supplied.
			// WindowsRegistry_GetString() will need a minimal buffer.
			// It will set the buffer to empty and just return a 
			// "not found" result.
			dwResultBufferSize = 0;
		}
	}

	// Allocate the array now that we know its size.
	pszResult = new TCHAR[ dwResultBufferSize + 1 ];	

	// We are now set to let WindowsRegistry_GetString() do the work.
	WindowsRegistry_GetString(
		hBaseKey,
		pszLabel,
		dwResultBufferSize,
		pszResult,
		pszDefault,
		pbFound
	);

	// Return the size, if requested.
	if ( pdwResultBufferSize )
		*pdwResultBufferSize = dwResultBufferSize;

	return pszResult;

}


//-------------------------------------------------------------------//
// WindowsRegistry_SetString()													//
//-------------------------------------------------------------------//
// This function sets the given data into the given value.
// It returns false if we don't succeed for any reason.
// It is an improvement over WriteProfileString in that it does 
// not require a "section".
//-------------------------------------------------------------------//
bool WindowsRegistry_SetString( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	const TCHAR*	pszString
) {
	bool bReturn;

	ASSERT( pszString );
	if ( !pszString )
		return false;

	// Place string in the registry.
	bReturn = ( 
		RegSetValueEx(
			hBaseKey,
			pszLabel,		// LPTSTR	lpValueName,	address of name of value
			0,
			REG_SZ,
			(CONST BYTE*)(LPCTSTR)pszString,
			( _tcslen( pszString ) + 1 ) * sizeof TCHAR		// BYTES INCLUDING NULL TERMINATOR!
		) == ERROR_SUCCESS
	);

	// We USED to clean up here, now we leave that to the caller.
	// VERIFY( RegCloseKey( hBaseKey ) == ERROR_SUCCESS );

	return bReturn;

}

	
//-------------------------------------------------------------------//
// WindowsRegistry_GetDWord()														//
//-------------------------------------------------------------------//
// This version sets a default if not found.
//-------------------------------------------------------------------//
DWORD WindowsRegistry_GetDWord( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	DWORD				dwDefault,
	bool*				pbFound
) {

	DWORD		dwResult;
	bool		bFound = false;
	DWORD		dwBufferSize = sizeof DWORD;

	// Attempt to get the path from the registry.
	LONG lResult = ::RegQueryValueEx(
						hBaseKey,				// HKEY		hKey,				handle of key to query            
						pszLabel,			// LPTSTR	lpValueName,	address of name of value to query 
						0,						// LPDWORD	lpReserved,		reserved                          
						0,						// LPDWORD	lpType,			address of buffer for value type  
		(PBYTE)		&dwResult,			// LPBYTE	lpData,			address of data buffer            
		(LPDWORD)	&dwBufferSize		// LPDWORD	lpcbData			address of data buffer size       
	);

  	// If we found a registry "key" "value" "data"...
	// ( who came UP with that garbage terminology?? )
	errno = 0;
	if ( lResult == ERROR_SUCCESS ) {

		bFound = true;

		// Make sure we got the right size.
		ASSERT( dwBufferSize == sizeof DWORD );

	} else {

		// A default is always provided in this version of the function.
		// Place it in the registry.
		// We pragma away the warning that "== has no effect, did u intend =?"
		#pragma warning(disable : 4553)
		VERIFY( 
			RegSetValueEx(
				hBaseKey,
				pszLabel,
				0,
				REG_DWORD,
				(CONST BYTE*) &dwDefault,
				sizeof DWORD
			) == ERROR_SUCCESS
		);
		#pragma warning(default : 4553)

		// Copy the default to the result.
		dwResult = dwDefault;

	}

	// We USED to clean up here, now we leave that to the caller.
	// VERIFY( RegCloseKey( hBaseKey ) == ERROR_SUCCESS );

	if ( pbFound )
		*pbFound = bFound;

	return dwResult;

}


//-------------------------------------------------------------------//
// WindowsRegistry_GetDWord()														//
//-------------------------------------------------------------------//
// This version does nothing if not found.
//-------------------------------------------------------------------//
DWORD WindowsRegistry_GetDWord( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	bool*				pbFound
) {

	DWORD		dwResult;
	bool		bFound = false;
	DWORD		dwBufferSize = sizeof DWORD;

	// Attempt to get the path from the registry.
	LONG lResult = ::RegQueryValueEx(
						hBaseKey,				// HKEY		hKey,				handle of key to query            
						pszLabel,			// LPTSTR	lpValueName,	address of name of value to query 
						0,						// LPDWORD	lpReserved,		reserved                          
						0,						// LPDWORD	lpType,			address of buffer for value type  
		(PBYTE)		&dwResult,			// LPBYTE	lpData,			address of data buffer            
		(LPDWORD)	&dwBufferSize		// LPDWORD	lpcbData			address of data buffer size       
	);

  	// If we found a registry "key" "value" "data"...
	// ( who came UP with that garbage terminology?? )
	errno = 0;
	if ( lResult == ERROR_SUCCESS ) {

		bFound = true;

		// Make sure we got the right size.
		ASSERT( dwBufferSize == sizeof DWORD );

	} else {

		// We didn't find it, don't do anything in this function version.
		dwResult = 0;

	}

	// We USED to clean up here, now we leave that to the caller.
	// VERIFY( RegCloseKey( hBaseKey ) == ERROR_SUCCESS );

	if ( pbFound )
		*pbFound = bFound;

	return dwResult;

}


//-------------------------------------------------------------------//
// SetDWord()																			//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
bool WindowsRegistry_SetDWord( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	DWORD				dwNumber						
) {

	bool bReturn;

	// Place it in the registry.
	bReturn = ( 
		RegSetValueEx(
			hBaseKey,
			pszLabel,		// LPTSTR	lpValueName,	address of name of value
			0,
			REG_DWORD,
			(CONST BYTE*) &dwNumber,
			sizeof DWORD
		) == ERROR_SUCCESS
	);

	// We USED to clean up here, now we leave that to the caller.
	// VERIFY( RegCloseKey( hBaseKey ) == ERROR_SUCCESS );

	return bReturn;

}
	

//-------------------------------------------------------------------//
// WindowsRegistry_GetBinarySize()												//
//-------------------------------------------------------------------//
DWORD WindowsRegistry_GetBinarySize(
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	bool*				pbFound
) {
	
	DWORD		dwBufferSize;

	// Attempt to get the size of the data from the registry.
	LONG lResult = ::RegQueryValueEx(
						hBaseKey,			// HKEY		hKey,				handle of key to query            
						pszLabel,			// LPTSTR	lpValueName,	address of name of value to query 
						0,						// LPDWORD	lpReserved,		reserved                          
						0,						// LPDWORD	lpType,			address of buffer for value type  
						0,						// LPBYTE	lpData,			address of data buffer            
		(LPDWORD)	&dwBufferSize		// LPDWORD	lpcbData			address of data buffer size       
	);

	if ( lResult == ERROR_SUCCESS ) 
	{
		if ( pbFound )
			*pbFound = true;

		return dwBufferSize;
	}

	if ( pbFound )
		*pbFound = false;

	return 0;

}


//-------------------------------------------------------------------//
// WindowsRegistry_GetBinary()													//
//-------------------------------------------------------------------//
void WindowsRegistry_GetBinary(
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	int				nSize,
	void*				pResult,
	void*				pDefault,
	bool*				pbFound
) {
	
	bool		bFound = false;
	DWORD		dwBufferSize = nSize;

	// Attempt to get the data from the registry.
	LONG lResult = ::RegQueryValueEx(
						hBaseKey,				// HKEY		hKey,				handle of key to query            
						pszLabel,			// LPTSTR	lpValueName,	address of name of value to query 
						0,						// LPDWORD	lpReserved,		reserved                          
						0,						// LPDWORD	lpType,			address of buffer for value type  
		(PBYTE)		pResult,				// LPBYTE	lpData,			address of data buffer            
		(LPDWORD)	&dwBufferSize		// LPDWORD	lpcbData			address of data buffer size       
	);

	// Make sure we got the right size.
	ASSERT( dwBufferSize == (DWORD) nSize );

  	// If we found a registry "key" "value" "data"...
	// ( who came UP with that garbage terminology?? )
	errno = 0;
	if ( lResult == ERROR_SUCCESS ) {

		bFound = true;

	// If we didn't find anything and the default was specified...
	} else if ( pDefault ) {

		// Place it in the registry.
		// We pragma away the warning that "== has no effect, did u intend =?"
		#pragma warning(disable : 4553)
		VERIFY( 
			RegSetValueEx(
				hBaseKey,
				pszLabel,
				0,
				REG_BINARY,
				(CONST BYTE*) pDefault,
				nSize
			) == ERROR_SUCCESS
		);
		#pragma warning(default : 4553)

		// Copy the default to the result.
		memcpy( pResult, pDefault, nSize );

	// Otherwise, don't do anything.
	// } else {
	
	}

	// We USED to clean up here, now we leave that to the caller.
	// VERIFY( RegCloseKey( hBaseKey ) == ERROR_SUCCESS );

	if ( pbFound )
		*pbFound = bFound;

}


//-------------------------------------------------------------------//
// WindowsRegistry_SetBinary()													//
//-------------------------------------------------------------------//
// This function sets the given data into the given value.
// It returns false if we don't succeed for any reason.
// It is an improvement over WriteProfileString in that it does 
// not require a "section".
//-------------------------------------------------------------------//
bool WindowsRegistry_SetBinary( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	void*				pBinary,
	int				nSize	
) {
	bool bReturn;

	// Place it in the registry.
	bReturn = ( 
		RegSetValueEx(
			hBaseKey,
			pszLabel,		// LPTSTR	lpValueName,	address of name of value
			0,
			REG_BINARY,
			(CONST BYTE*) pBinary,
			nSize
		) == ERROR_SUCCESS
	);

	// We USED to clean up here, now we leave that to the caller.
	// VERIFY( RegCloseKey( hBaseKey ) == ERROR_SUCCESS );

	return bReturn;

}


//-------------------------------------------------------------------//
// WindowsRegistry_OpenKey()														//
//-------------------------------------------------------------------//
// Opens the requested key.  If asked, this function will create
// keys that do not exist.
//
// Returns true if the key was opened.
//-------------------------------------------------------------------//
bool WindowsRegistry_OpenKey(
	HKEY				hBaseKey,
	LPCTSTR			lpszSubKey,
	PHKEY				phSubKey,
	bool				bCreateIfNotFound,
	REGSAM			samDesired
) {

	LONG lErr = RegOpenKeyEx(
		hBaseKey,					// handle to open key
		lpszSubKey,					// subkey name
		0,								// reserved, must be 0
		KEY_READ | KEY_WRITE,	// security access mask
		phSubKey						// handle to open key
	);

	if ( lErr != ERROR_SUCCESS )
	{
		if ( bCreateIfNotFound )
		{
			lErr = RegCreateKeyEx(
				hBaseKey,						// handle to open key
				lpszSubKey,						// subkey name
				0,									// reserved, must be 0
				0,									// class string - "none currently defined - pass a NULL STRING"
				REG_OPTION_NON_VOLATILE,	// special options
				samDesired,		            // security access mask
				NULL,								// inheritance
				phSubKey,						// handle to open key
				NULL								// disposition value buffer - on success, filled with REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY 
			);

			return ( lErr == ERROR_SUCCESS );
		}

		return false;
	
	}
	
	return true;

}