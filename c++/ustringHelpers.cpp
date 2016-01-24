//-------------------------------------------------------------------//
// ustringHelpers
//-------------------------------------------------------------------//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include <windows.h>				// For WideCharToMultiByte(), etc.  - some day we may make this cross-platform

#include "ustringHelpers.h"


//-------------------------------------------------------------------//
// strFileSizeFormat()																//
//-------------------------------------------------------------------//
// This formats the given number of bytes into a nice output as
// follows:
//
//		"  3.5 TB   "
//		"  2.4 GB   "
//		" 12.3 MB   "
//		" 32.3 KB   "
//		"125 bytes"
//
// Note the extra spaces on the right of first items, to facilitate
// right-justification where possible.  Also note the extra
// spaces on the left to facilitate sorting (somewhat).
//
// NOTE: we could not use unsigned values here because lame-duck
// MS has not implemented conversion from unsigned __int64 to double!!
// (only __int64 to double).  As of VC6, Jan 2001.
//-------------------------------------------------------------------//
ustring strFileSizeFormat( Huge hSize )
{
	const Huge	chBytesPerKB	=						1024;
	const Huge	chBytesPerMB	= chBytesPerKB *	1024;
	const Huge	chBytesPerGB	= chBytesPerMB *	1024;
	Huge			hBytesPerTB		= chBytesPerGB; hBytesPerTB *= 1024;	// const values limited to max integer value.
	Huge			hBytesPerPB		= hBytesPerTB;  hBytesPerPB *= 1024;	// const values limited to max integer value.

	const Huge	chKBLimit		=						1000;
	const Huge	chMBLimit		= chKBLimit		*	1000;
	const Huge	chGBLimit		= chMBLimit		*	1000;
	Huge			hTBLimit			= chGBLimit; hTBLimit *= 1000;			// const values limited to max integer value.
	Huge			hPBLimit			= hTBLimit;  hPBLimit *= 1000;			// const values limited to max integer value.

	ustring strResult;
	if ( hSize > hPBLimit )
		strResult.printf( 12, _T("%5.1f PB"), (double)hSize / (double)hBytesPerPB );
	else if ( hSize > hTBLimit )
		strResult.printf( 12, _T("%5.1f TB"), (double)hSize / (double)hBytesPerTB );
	else if ( hSize > chGBLimit )
		strResult.printf( 12, _T("%5.1f GB"), (double)hSize / (double)chBytesPerGB );
	else if ( hSize > chMBLimit )
		strResult.printf( 12, _T("%5.1f MB"), (double)hSize / (double)chBytesPerMB );
	else if ( hSize > chKBLimit )
		strResult.printf( 12, _T("%5.1f KB"), (double)hSize / (double)chBytesPerKB );
	else
		strResult.printf( 12, _T("%5d bytes"), (int) hSize );
		
	return strResult;
}


//-------------------------------------------------------------------//
// RemoveTrailingBackslash()														//
//-------------------------------------------------------------------//
// This function makes sure that the given path does not end in a 
// backslash.  The given string is modified as needed, and returned 
// for immediate use.
//-------------------------------------------------------------------//
ustring& RemoveTrailingBackslash( ustring& strPath )
{
	// Remove all trailing backslashes as needed.
	int nLength;

	while ( 
			( nLength = strPath.size() ) > 0 
		&& ( strPath[ nLength - 1 ] == _T('\\') )
	)
		// Whack the last character.
		strPath.resize( nLength - 1 );

	return strPath;

}


//-------------------------------------------------------------------//
// GetPathFromFilename()															//
//-------------------------------------------------------------------//
// This function gives you the path of the filename.
//-------------------------------------------------------------------//
ustring GetPathFromFilename( ustring& strFilename )
{
	// return strFilename.Left( strFilename.ReverseFind( _T('\\') ) );
	
	ustring strPath( strFilename );
	int nLastSlash = strPath.rfind( _T('\\') );
	if ( nLastSlash != -1 )
		strPath.resize( nLastSlash );
	return strPath;
}


