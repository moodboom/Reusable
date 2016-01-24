//-------------------------------------------------------------------//
// StringHelpers.h																	//
//
//	This module contains generic functions to help with Microsoft's
// CString class.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef STRING_HELPERS_H
	#define STRING_HELPERS_H

#include "StandardTypes.h"


//-------------------------------------------------------------------//
// GLOBALS / CONSTS / STATICS
//-------------------------------------------------------------------//

const CString cstrUnknown	( _T("[unknown]") );
const CString cstrBlank		( _T("[blank]") );

//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// ResStr()																				//
//-------------------------------------------------------------------//
// Silly function that returns a CString & pointing to static CString
// of the requested resource string.  Used so that you don't have to
// create local temp CStrings all the time in order to use
// LoadString to get a resource string from the rc file.
//-------------------------------------------------------------------//
// There is a back-door way to create CStrings from resource IDs
// that isn't in the CString documentation:  If the
// CString constuctor argument is cast as an LPCSTR, then CString
// takes it as a UINT of a resouce ID.
//
#define ResStr( a ) CString( (LPCSTR) a )
//
// And then I renamed the ResStr function below to ResStrStatic()
// because is is still needed in a lot of cases, such as functions
// that return a CString reference.
//
CString & ResStrStatic( UINT uiStringNum );
//
//-------------------------------------------------------------------//

// Reduces all whitespace to single spaces.
CString& Pack_CString( CString &str );

// Strips file extentions from strings.
CString& StripExtension( CString& strName );

bool ExtractFileNameFromPath( 
	CString* pstrFullName,
	CString* pstrFileName
);

bool extract_extension_from_filename( 
	CString* pstrFilename,
	CString* pstrExtension
);

bool copy_CString_to_uHuge(
	CString*	pstrSource,
	uHuge*	puhDest
);

void copy_uHuge_to_CString(
	uHuge*	puhSource,
	CString*	pstrDest
);

// This copies with length restriction.
// NOTE: This was for wchar_t sources.  You can add new versions as needed.
CString strNCopy( wchar_t* pwcSource, int nMaxLength );

// Utility function for IsComplete() functions - created to reduce
// some duplicate code throughout EMComponent derived classes.
CString strIndentString( int nIndent );

void MakeTitleCase( CString* pstrName );

CString strAlphaNumeric( CString strSource );

void SmartStringCopy(LPSTR outString, LPCSTR inString, uLong size);

// String helpers for file names.
CString&			AddTrailingBackslash		( CString& strPath		);
CString&			RemoveTrailingBackslash	( CString& strPath		);
CString			strGetRootPathName		( CString& strFilename	);		// e.g., "D:\" or "\\MICHAEL\Shared\"
CString			GetPathFromFilename		( CString& strFilename	);

// This class allows you to do fast searches for a substring
// in a set of source strings.
class SubstringSearcher
{
public:

	SubstringSearcher(
		CString	strSubstring,
		int		nMaxSourceStringLength	= 255
	);

	bool bContainsSubstring( CString& strSource );

protected:
	CString	m_strSubstring;
	TCHAR*	m_szSubstringBuffer;
	int		m_nSubstringLength;
	int		m_nMaxSourceStringLength;

};


BOOL SendTextToClipboard( CString& source );


#endif	// STRING_HELPERS_H
