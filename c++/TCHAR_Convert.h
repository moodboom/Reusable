//-------------------------------------------------------------------//
// TCHAR_convert.h
//-------------------------------------------------------------------//
//	These utility functions helps us go to/from TCHAR, typically
//	needed for Windows display of strings coming from the *nix side 
//	of the moon.																		
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef TCHAR_CONVERT_H
#define TCHAR_CONVERT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(WIN32)
	
	#include <TCHAR.H>

	#define vsnprintf _vsnprintf 
	#define strcasecmp _stricmp

#else
	
	/* Windows UNICODE mumbo-jumbo.
		Under Windows, TCHAR and the _T() macro allow conditional compiles to produce 
		either char, wchar_t, or MBCS executables.  wchar_t builds can be internationalized
		to any language supported by Unicode.																*/
	#define _T( szStr ) szStr
	typedef char TCHAR;

#endif


#include <stdlib.h>				// for wchar_t
#include "export.h"				// for WINXxx export macros


WINEXPORT void WINMANGLE convert_char_to_TCHAR( TCHAR* szDest, char* szSource, int nMaxBytes );

WINEXPORT void WINMANGLE convert_TCHAR_to_char( char* szDest, TCHAR* szSource, int nMaxBytes );

WINEXPORT void WINMANGLE convert_wchar_to_TCHAR( TCHAR* szDest, wchar_t* szSource, int nMaxBytes );

WINEXPORT void WINMANGLE convert_TCHAR_to_wchar( wchar_t* szDest, TCHAR* szSource, int nMaxBytes );


#endif	/* TCHAR_CONVERT_H */

