//-------------------------------------------------------------------//
//	RegistryHelpers
//
//	This module contains generic functions to read from and save to
// the Windows registry.  TCHAR strings, DWORD's, and binary data
// are handled.  MFC is NOT required.  See the AppRegistryData class
// for a nice MFC WinApp-associated wrapper for these functions.
//
// NOTE: MS registry terminology is as follows (don't blame me):
//
//		Key		Branch which may have a value and under which there may
//					be other branches and leaves.
//		Value		Leaf slot which contains one item of data.
//		Data		Contents of the leaf.
//		Profile	obsolete term used to refer to an *.ini file; these
//					were the predecesors of the Windows Registry.
//
// In our functions, the word Registry was used instead of Profile.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//


#ifndef REGISTRY_HELPERS_H
#define REGISTRY_HELPERS_H

#include "StandardTypes.h"

// If not found and default is supplied, it is added to the 
// registry.  Otherwise this does nothing.
void	WindowsRegistry_GetString( 
	HKEY				hBaseKey,						// IN
	const TCHAR*	pszLabel,						// IN
	DWORD&			dwResultBufferSize,			// IN, OUT
	TCHAR*			pszResult,						// OUT (you must supply the buffer space)
	TCHAR*			pszDefault				= 0,	// IN  (optional)
	bool*				pbFound					= 0	// OUT (optional)
);

// Here, we extract the size of the string from the registry,
// and allocate an appropriately sized buffer to load.
// As the caller, you will need to clean up the allocation
// provided in the returned pointer.
TCHAR* WindowsRegistry_GetNewString( 
	HKEY				hBaseKey,						// IN
	const TCHAR*	pszLabel,						// IN
	TCHAR*			pszDefault				= 0,	// IN  (optional)
	DWORD*			pdwResultBufferSize	= 0,	// OUT (optional)
	bool*				pbFound					= 0	// OUT (optional)
);

bool WindowsRegistry_SetString( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	const TCHAR*	pszString
);

// This version sets a default if not found.
DWORD WindowsRegistry_GetDWord( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	DWORD				dwDefault,
	bool*				pbFound
);

// This version does nothing if not found.
DWORD WindowsRegistry_GetDWord( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	bool*				pbFound		= 0
);

bool WindowsRegistry_SetDWord( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	DWORD				dwNumber						
);

// This helper will get you the size
// so you know what to allocate
// before calling GetBinary.  Note
// that you don't need to use this when
// you are get/setting a constant-sized
// structure.
DWORD WindowsRegistry_GetBinarySize(
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	bool*				pbFound		= 0
);

// If not found and default is supplied, it is added to the 
// registry.  Otherwise this does nothing.
void WindowsRegistry_GetBinary(
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	int				nSize,
	void*				pResult,
	void*				pDefault		= 0,	
	bool*				pbFound		= 0
);

bool WindowsRegistry_SetBinary( 
	HKEY				hBaseKey,
	const TCHAR*	pszLabel,
	void*				pBinary,		
	int				nSize	
);

bool WindowsRegistry_OpenKey(
	HKEY				hBaseKey,
	LPCTSTR			lpszSubKey,
	PHKEY				phSubKey,
	bool				bCreateIfNotFound,
	REGSAM			samDesired = KEY_READ | KEY_WRITE
);

#endif	// REGISTRY_HELPERS_H
