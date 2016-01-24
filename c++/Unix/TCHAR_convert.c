//-------------------------------------------------------------------//
// TCHAR_convert.c
//-------------------------------------------------------------------//
//	These utility functions helps us go to/from TCHAR, typically
//	needed for Windows display of strings coming from the *nix side 
//	of the moon.																		
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "TCHAR_convert.h"

#include <string.h>							/* for strncpy						*/


#if defined(_UNICODE)

WINEXPORT void WINMANGLE convert_char_to_TCHAR( wchar_t* wzDest, char* szSource, int nMaxBytes )
{
	mbstowcs( wzDest, szSource, nMaxBytes );
}

WINEXPORT void WINMANGLE convert_TCHAR_to_char( char* szDest, wchar_t* wzSource, int nMaxBytes )
{
	wcstombs( szDest, wzSource, nMaxBytes );
}

WINEXPORT void WINMANGLE convert_wchar_to_TCHAR( wchar_t* wzDest, wchar_t* wzSource, int nMaxBytes )
{
	wcsncpy( wzDest, wzSource, nMaxBytes );
	if ( (int)wcslen( wzSource ) >= nMaxBytes )
		wzDest[nMaxBytes-1] = _T('\0');
}

WINEXPORT void WINMANGLE convert_TCHAR_to_wchar( wchar_t* wzDest, wchar_t* wzSource, int nMaxBytes )
{
	wcsncpy( wzDest, wzSource, nMaxBytes );
	if ( (int)wcslen( wzSource ) >= nMaxBytes )
		wzDest[nMaxBytes-1] = _T('\0');
}

#else

WINEXPORT void WINMANGLE convert_char_to_TCHAR( char* szDest, char* szSource, int nMaxBytes )
{
	strncpy( szDest, szSource, nMaxBytes );
	if ( (int)strlen( szSource ) >= nMaxBytes )
		szDest[nMaxBytes-1] = '\0';
}

WINEXPORT void WINMANGLE convert_TCHAR_to_char( char* szDest, char* szSource, int nMaxBytes )
{
	strncpy( szDest, szSource, nMaxBytes );
	if ( (int)strlen( szSource ) >= nMaxBytes )
		szDest[nMaxBytes-1] = '\0';
}

WINEXPORT void WINMANGLE convert_wchar_to_TCHAR( char* szDest, wchar_t* wzSource, int nMaxBytes )
{
	wcstombs( szDest, wzSource, nMaxBytes );
}

WINEXPORT void WINMANGLE convert_TCHAR_to_wchar( wchar_t* wzDest, char* szSource, int nMaxBytes )
{
	mbstowcs( wzDest, szSource, nMaxBytes );
}

#endif

