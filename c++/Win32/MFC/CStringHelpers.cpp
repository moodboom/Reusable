//-------------------------------------------------------------------//
// StringHelpers.cpp																	//
// 
//	This module contains generic functions to help with Microsoft's
// CString class.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "ustring.h"							// For ustring

#include "CStringHelpers.h"


//-------------------------------------------------------------------//
// ResStr()																				//
//-------------------------------------------------------------------//
CString & ResStrStatic( UINT uiStringNum )
{
	// Max number of CStrings needed at any given instance.
	const int cnMaxResStr = 8;		

	// CString array to hold the strings returned from the string table.
	static CString astrResStr[ cnMaxResStr ];	

	// Wrap around index into strResStr table.
	static int nResIndex = 0;

	if ( ++nResIndex >= cnMaxResStr )
		nResIndex = 0;

	astrResStr[ nResIndex ].LoadString( uiStringNum );

	return astrResStr[ nResIndex ];
}

//-------------------------------------------------------------------//
// Pack_CString()																		//
//-------------------------------------------------------------------//
// Removes multiple spaces within a CString.
//-------------------------------------------------------------------//
CString& Pack_CString( CString &str ) {
	
	int nLength = str.GetLength();
	int nPos = 0;
	int nSpacesRemoved = 0;
	bool bPreviousWasSpace = false;
	
	LPTSTR pBuffer = str.GetBuffer( nLength );
	
	while ( nPos < nLength ) {
		
		if ( bPreviousWasSpace && pBuffer[nPos] == _T(' ') )
			nSpacesRemoved++;
		
		if ( nSpacesRemoved )
			pBuffer[nPos - nSpacesRemoved] = pBuffer[nPos];
		
		bPreviousWasSpace = ( pBuffer[nPos] == _T(' ') );
		
		nPos++;
		
	}
	
	if ( pBuffer[ nLength - nSpacesRemoved - 1 ] == _T(' ') )
		nSpacesRemoved++;
	
	str.ReleaseBuffer( nLength - nSpacesRemoved );

	return str;
	
}


//-------------------------------------------------------------------//
// StripExtension()																	//
//-------------------------------------------------------------------//
// This function strips anything in the given string after the LAST
// period ('.').
//-------------------------------------------------------------------//
CString& StripExtension( CString& strName )
{
	int nStop = strName.ReverseFind( _T('.') );
	strName = strName.Left( nStop );
	return strName;
}


//-------------------------------------------------------------------//
// ExtractFileNameFromPath()														//
//-------------------------------------------------------------------//
// This function extracts whatever follows the last slash in the 
// given string.  The entire string is extracted if there is no 
// slash in the source.  
//
// The function returns true if the source name differs from the 
// result (because it had a slash in it), and false otherwise.
// Note that it is okay to use the source string as the result
// string; it will be truncated as needed.
//-------------------------------------------------------------------//
bool ExtractFileNameFromPath( 
	CString* pstrFullName,
	CString* pstrFileName
) {

	// Get just the filename.  Check forward and back slashes.

	int nTrunc;
	if (	
			( nTrunc = pstrFullName->ReverseFind( _T('\\') ) ) != -1 
		|| ( nTrunc = pstrFullName->ReverseFind( _T('/' ) ) ) != -1 
	) {

		// Strip it.
		*pstrFileName = pstrFullName->Right( pstrFullName->GetLength() - nTrunc - 1 );
		return true;

	} else {

		// Copy it.
		*pstrFileName = *pstrFullName;
		return false;

	}

}


//-------------------------------------------------------------------//
// extract_extension_from_filename()											//
//-------------------------------------------------------------------//
// This function extracts whatever follows the last period in the 
// given string.  If there is no period in the source, the return
// string is empty.
//
// Note that it is okay to use the source string as the result
// string; it will be truncated as needed.
//-------------------------------------------------------------------//
bool extract_extension_from_filename( 
	CString* pstrFilename,
	CString* pstrExtension
) {

	// Get just the extension.
	int nTrunc;
	if ( ( nTrunc = pstrFilename->ReverseFind( _T('.') ) ) != -1 ) {

		// Strip it.
		*pstrExtension = pstrFilename->Right( pstrFilename->GetLength() - nTrunc - 1 );
		return true;

	} else {

		// Empty it.
		*pstrExtension = _T("");
		return false;

	}

}


//-------------------------------------------------------------------//
// copy_uHuge_to_CString()															//
//-------------------------------------------------------------------//
// This utility does what it says.
// It is useful for storing our uHuge numbers in systems that do not
// recognize that byte width (e.g., Access).
// The resulting string will include up to 16 characters (0-9, or A-F) 
// and nothing else.
// We use CString::Format() for the basic conversion.
//-------------------------------------------------------------------//
void copy_uHuge_to_CString(
	uHuge*	puhSource,
	CString*	pstrDest
) {

	// Clear the string.
	*pstrDest = _T("");

	// We do one long at a time.  
	// Because of Intel's bassackwards byte ordering,
	// the first half of the uHuge should be the last part
	// of the string.  So we do the second half of the string first.
	uLong* pulHalf = (uLong*) puhSource;
	
	// Convert the second half using Format().
	pstrDest->Format( _T("%X"), *pulHalf );
	
		// We need to stuff the second half with leading zero's
	// until it is 8 characters long.
	while ( pstrDest->GetLength() < 8 )
		*pstrDest = _T("0") + *pstrDest;

	// Go to the first half.
	pulHalf++;

	// Convert the first half.
	CString strTemp;
	strTemp.Format( _T("%X"), *pulHalf );
	
	// We stuff the first half with leading zero's
	// until it is 8 characters long.  This is not
	// necessary, but it makes sure the resulting
	// string is always 16 characters long.
	while ( strTemp.GetLength() < 8 )
		strTemp = _T("0") + strTemp;

	// Add up the two halves.
	*pstrDest = strTemp + *pstrDest;

	// Check the length.
	ASSERT( pstrDest->GetLength() == 16 );

}


//-------------------------------------------------------------------//
// copy_CString_to_uHuge()															//
//-------------------------------------------------------------------//
// This utility does what it says.
// It is useful for storing our uHuge numbers in systems that do not
// recognize that byte width (e.g., Access).
// The string must include up to 16 characters (0-9, or A-F) and
// nothing else.
// We use _tcstoul() for the basic conversion.
//-------------------------------------------------------------------//
bool copy_CString_to_uHuge(
	CString*	pstrSource,
	uHuge*	puhDest
) {


	// If we are longer than 16 characters, we try to convert, 
	// but return false, since the convertion may be wrong.
	int nLength = pstrSource->GetLength();
	bool bReturn = ( nLength < 17 );
	ASSERT( bReturn );

	// We do one half at a time.
	// Because of Intel's bassackwards byte ordering,
	// the first half of the uHuge should be the last part
	// of the string.  So we do the second half of the string first.
	uLong* pulHalf = (uLong*) puhDest;
	CString strHalf;
	TCHAR* pStopPoint;
	
	// Do the second half as needed.
	if ( nLength > 0 ) {

		// Get the second half string and prefix with "0X".
		strHalf = pstrSource->Right( min( nLength, 8 ) );
		strHalf = CString( _T("0X") ) + strHalf;

		// Convert the second half.
		*pulHalf = _tcstoul(
			LPCTSTR( strHalf ),
			&pStopPoint,
			16
		);

	} else
		*pulHalf = 0;

	// Go from the second half to the first.
	pulHalf++;

	// Do the first half as needed.
	if ( nLength > 8 ) {

		// Get the first half string and prefix with "0X".
		strHalf = pstrSource->Left( nLength - 8 );
		strHalf = CString( _T("0X") ) + strHalf;
		
		// Convert using _tcstoul().		
		// Originally, we thought about using this Shell Utility Function, 
		// but it is not available in VC++ 5.0 yet.  SHLWAPI.DLL is a part 
		// of NT 5.0, and it was on my Win95 system, but I did not have the 
		// corresponding include or library files, and the function name
		// was not contained in any VC++ files.
		// We should keep our eyes open for the Shell Utility Functions
		// showing up in the future!
		/*
		bReturn = ( 
			StrToIntEx(  
				LPCTSTR( strHalf ),
				STIF_SUPPORT_HEX,
				pulHalf
			) = TRUE 
		);
		*/
		*pulHalf = _tcstoul(
			LPCTSTR( strHalf ),
			&pStopPoint,
			16
		);

	} else
		*pulHalf = 0;

	return bReturn;

}


//-------------------------------------------------------------------//
// strNCopy()																			//
//-------------------------------------------------------------------//
CString strNCopy( wchar_t* pwcSource, int nMaxLength )
{
	CString strDest;

	LPTSTR szDest = strDest.GetBufferSetLength( nMaxLength + 1 );

	#if defined(UNICODE)
		wcsncpy( szDest, pwcSource, nMaxLength );
	#else
		wcstombs( szDest, pwcSource, nMaxLength );
	#endif
	
	strDest.ReleaseBuffer();

	return strDest;
}


//-------------------------------------------------------------------//
// strIndentString()																	//
//-------------------------------------------------------------------//
// Utility function for IsComplete() functions - created to reduce	//
// some duplicate code throughout EMComponent derived classes.			//
//-------------------------------------------------------------------//
CString strIndentString( int nIndent )
{
	CString strIndent;

	// If we are creating error strings, create a string of
	// tabs so that the indention on the error string matches
	// the level we have dug down to to find this error.
	for ( int nA = 0; nA < nIndent; nA++ )
		strIndent += _T("\t");

	return strIndent;
}


//-------------------------------------------------------------------//
// MakeTitleCase()																	//
//-------------------------------------------------------------------//
// This function sets the string to have title case (the first letter
// is uppercase and the rest is lowercase).  It uses MakeUpper() and
// MakeLower() for all conversions (figured this was safer when
// dealing with Unicode), but nicely avoids doing a full string copy
// by using Get/SetAt().
//-------------------------------------------------------------------//
void MakeTitleCase( CString* pstrName )
{
	pstrName->MakeLower();
	CString strFirstLetter = pstrName->Left( 1 );
	strFirstLetter.MakeUpper();
	pstrName->SetAt( 0, strFirstLetter[0] );
}


//-------------------------------------------------------------------//
// strAlphaNumeric()																	//
//-------------------------------------------------------------------//
// This function "standardizes" the given string by removing 
// everything except A-z, 0-9, and forcing title case.
//-------------------------------------------------------------------//
CString strAlphaNumeric( CString strSource )
{
	const TCHAR szAlphaNumerics[] = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

	CString strResult;
	CString strTemp = strSource;
	CString strTemp2;
	
	int nIndex = 0;
	while ( nIndex < strSource.GetLength() )
	{
		strTemp2 = strTemp.SpanIncluding( szAlphaNumerics );
		strResult += strTemp2;

		nIndex += strTemp2.GetLength() + 1;
		strTemp = strTemp.Right( strTemp.GetLength() - strTemp2.GetLength() - 1 );
	}

	MakeTitleCase( &strResult );
	return strResult;
}


//-------------------------------------------------------------------//
// SmartStringCopy()																	//
//-------------------------------------------------------------------//
// SmartStringCopy copies either 'inString' in its entirety to 'outString', 
// or 'size' chars of 'inString', whichever is smaller, and appends a NULL.
//-------------------------------------------------------------------//
void SmartStringCopy(LPSTR outString, LPCSTR inString, uLong size)
{
	uLong numToCopy = min( (int)strlen(inString), (int)size - 1 );

	strncpy(outString, inString, numToCopy);
	outString[numToCopy] = _T('\0');
}


//-------------------------------------------------------------------//
// AddTrailingBackslash()															//
//-------------------------------------------------------------------//
// This function makes sure that the given path ends in a backslash.
// The given string is modified as needed, and returned for 
// immediate use.
//-------------------------------------------------------------------//
CString& AddTrailingBackslash( CString& strPath )
{

	// Add a trailing backslash as needed.
	if ( strPath.Right( 1 ) != _T("\\") )
		strPath += _T("\\");

	return strPath;

}


//-------------------------------------------------------------------//
// RemoveTrailingBackslash()														//
//-------------------------------------------------------------------//
// This function makes sure that the given path ends in a backslash.
// The given string is modified as needed, and returned for 
// immediate use.
//-------------------------------------------------------------------//
CString& RemoveTrailingBackslash( CString& strPath )
{

	// Remove all trailing backslashes as needed.
	while ( strPath.GetLength() > 0 && strPath.Right( 1 ) == _T("\\") )
		strPath = strPath.Left( strPath.GetLength() - 1 );

	return strPath;

}


//-------------------------------------------------------------------//
// strGetRootPathName()																//
//-------------------------------------------------------------------//
// This extracts the "Root Path Name, that is, either a local
// drive designation (e.g., "d:\...") or a UNC share 
// (e.g., "\\Machine\Share\..."), from the beginning of the
// provided string.
//-------------------------------------------------------------------//
CString strGetRootPathName( CString& strFilename )
{

	int nStrIdx;
	
	// We need to get the "root path name".
	// For local drives, this is "X:\".  
	// For network drives, this is "\\Machine\SharedFolder\".

	// Make sure that the last backslash has been added.
	// This fixes bare shares and bare drive letters, and I don't
	// believe it will mess up any other requests.
	AddTrailingBackslash( strFilename );
	
	// Is this a network UNC name (starts with "\\")?
	CString strRootPathName;
	if ( strFilename.Left( 2 ) == _T("\\\\") )
	{
		// Get all the text up to the fourth backslash,
		// i.e., "\\Machine\SharedFolder\".
		nStrIdx = strFilename.Find( _T('\\'), 2 );
		nStrIdx = strFilename.Find( _T('\\'), nStrIdx + 1 );
		
		// If we didn't find what we needed, return the Unknown string.
		if ( nStrIdx == -1 )
			return cstrUnknown;

		// Extract the root path name.
		strRootPathName = strFilename.Left( nStrIdx + 1 );

	} else
	{
		// Let's look for the ':'.
		nStrIdx = strFilename.Find( _T(':') );
		
		// If we didn't find it in the right place, return the Unknown string.
		if ( nStrIdx != 1 && nStrIdx != 2 )
			return cstrUnknown;

		// Build the root path name.
		strRootPathName = strFilename.Left( nStrIdx ) + _T(":\\");

	}

	return strRootPathName;
}


//-------------------------------------------------------------------//
// GetPathFromFilename()															//
//-------------------------------------------------------------------//
// This function gives you the path of the filename.
//-------------------------------------------------------------------//
CString GetPathFromFilename( CString& strFilename )
{

	return strFilename.Left( strFilename.ReverseFind( _T('\\') ) );

}


//-------------------------------------------------------------------//
// SubstringSearcher()																//
//-------------------------------------------------------------------//
// Constructor to set up for subsequent substring searches - say that
// 10 times fast.
//-------------------------------------------------------------------//
SubstringSearcher::SubstringSearcher( 
	CString	strSubstring,
	int		nMaxSourceStringLength
) :
	// Init vars.
	m_strSubstring				(	strSubstring				),
	m_nMaxSourceStringLength(	nMaxSourceStringLength	)
{
	// Set up the length of the substring.
	m_nSubstringLength = strSubstring.GetLength();

	// Set up the substring buffer.
	// This contains the substring, AS WELL AS ENOUGH ROOM FOR THE SEARCH ALGORITHM!!
	// See the algorithm documentation for details.
	// NOTE: we cast away the const, as we need to modify the buffer.
	// No worries there, we made a copy of the originally supplied string.
	// TO DO
	// I cranked it up to be safe 'cause I didn't feel like determining the need better.
	// Reduce this size later, if ya feel up to it, punk.
	m_szSubstringBuffer = m_strSubstring.GetBufferSetLength( m_nMaxSourceStringLength * 2 + 1 );
}

//-------------------------------------------------------------------//
// bContainsSubstring()																//
//-------------------------------------------------------------------//
// This function uses a "Tuned Boyer-Moore" generic search algorithm.
// 
// I believe we have a pretty fast algorithm here, but I have not
// actually compared it to others.  Here are what seem like the best:
//	
//		Tuned Boyer-Moore (TBM)						The classic benchmark
//		"FastGenericSearchAlgorithm" (HAL)		better worst-case	(who cares)
//		"Self-Tuning Boyer-Moore" (STBM)			potentially faster!!  used by grep
//
// See Software Development\Documentation\SubstringSearching\
// for details.
//-------------------------------------------------------------------//
bool SubstringSearcher::bContainsSubstring(
	CString& strSource
) {
	int m = strSource.GetLength();

	const TCHAR*	x	= LPCTSTR( strSource		);
	TCHAR*			y	= m_szSubstringBuffer;

	int i, j, k, shift;
	
	// Fix this some time...we need a dynamic creation.
	// int bc[ m_nMaxSourceStringLength ];
	// int bc[ 255 ];
	int* bc = new int[ m_nMaxSourceStringLength ];

	/* Preprocessing */
	
	// TO DO
	// What the hell is going on here?  This HAS to be moved to preprocessing
	// code in constructor.  But it uses m!!!
	// TO DO
	// memcpy should be faster, but we need a Unicode-capable version.
	// memcpy( 
	for (i = 0; i < m_nMaxSourceStringLength; ++i) 
		bc[i] = m;

	for (i = 0; i < m - 1; ++i) 
		bc[x[i]] = m - i - 1;

	shift = bc[x[m - 1]];
	bc[x[m - 1]] = 0;

	for (i = m_nSubstringLength; i < m_nSubstringLength + m; ++i) 
		y[i] = x[m - 1];

	// DEBUG
	y[i] = _T('\0');
	ustring strM;
	// for (i = 0; i < m_nMaxSourceStringLength; ++i) 
	for (i = 0; i < 100; ++i) 
	{
		strM.AppendNumber( bc[i] );
		strM += _T(",");
	}
	TRACE( _T("X=%s\nY=%s\nBC=%s\n\n"), x, y, strM.c_str() );
	/*
	ustring strMsg;
	strMsg  = (LPTSTR)x;
	strMsg += y;
	strMsg += strM;
	DisplayMessage( strMsg.c_str() );
	*/

	/* Searching */
	j = m - 1;
	while ( j < m_nSubstringLength ) 
	{
		k = bc[ y[j] ];
		while (k !=  0) 
		{
			j += k; k = bc[y[j]];
			j += k; k = bc[y[j]];
			j += k; k = bc[y[j]];
		}
		i = 0;
		while (i < m - 1 && x[i] == y[j - m + 1 + i] ) 
			++i;
		if (j < m_nSubstringLength && i >= m - 1) 
		{
			// We found a match!
			// That's all we care about in this version, return immediately.
			// OUTPUT(j - m + 1);
			delete bc;
			return true;
		}

		j += shift;                                  /* shift */
	}

	delete bc;
	return false;

}


BOOL SendTextToClipboard(CString& source)
{
  // Return value is TRUE if the text was sent
  // Return value is FALSE if something went wrong
  if(OpenClipboard(0))
  {
    HGLOBAL clipbuffer;
    char* buffer;

    EmptyClipboard(); // Empty whatever's already there

    clipbuffer = GlobalAlloc(GMEM_DDESHARE, source.GetLength()+1);
    buffer = (char*)GlobalLock(clipbuffer);
    strcpy(buffer, LPCSTR(source));
    GlobalUnlock(clipbuffer);

    SetClipboardData(CF_TEXT, clipbuffer); // Send the data

    CloseClipboard(); // VERY IMPORTANT
    return TRUE;
  }
  return FALSE;
}