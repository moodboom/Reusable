//-------------------------------------------------------------------//
// FileScan implementation
//-------------------------------------------------------------------//
// This class makes file scanning trivial.
//
//	Copyright © 2001-2004 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"						// For CFileFind class.

#include "CStringHelpers.h"			// For extract_extension_from_filename()

#include "FileScan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-------------------------------------------------------------------//
// FileScan()																			//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
FileScan::FileScan( 
	CString* pstrStartPath,
	TCHAR*	pszFilter,
	bool		bScanSubdirectories
) :

	// Init vars.
	m_strStartPath			( *pstrStartPath			),
	m_strFilter				( pszFilter					),
	m_bScanSubdirectories( bScanSubdirectories	)
	
{
	// Make sure start path ends with backslash.
	if ( 
			m_strStartPath.Right( 1 ) != _T("\\")
		&&	m_strStartPath.Right( 1 ) != _T("/")
	)
		m_strStartPath += _T("\\");
	
	// Create the first member of our CFileFind array,
	// using the constructor params.
	Finders.Add( new CFileFind );
	Results.Add( Finders[0]->FindFile( LPCTSTR( m_strStartPath + _T("*.*") ) ) != FALSE );

	// TO DO
	// Do a proper filter check.  For now, we just compare extensions.
	// Set up our filter extension string.
	extract_extension_from_filename( 
		&m_strFilter,
		&m_strFilterExt
	);

}


//-------------------------------------------------------------------//
// ~FileScan()																			//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
FileScan::~FileScan()
{
	int FindersSize = Finders.GetSize();

	// If we had more than one left, the user did not finish looking.
	// Why not?
	ASSERT( FindersSize == 1 );

	// Delete the leftovers.
	for ( int i = 0; i < FindersSize; i++ )
		delete Finders[i];
}


//-------------------------------------------------------------------//
// GetNextFile()																		//
//-------------------------------------------------------------------//
// This function is called by the end user to find the next file.
//
// We get the most-recently successful finder object and try it
// again.  If it fails, we need to walk back up the previously
// created finder objects in the array.
//	Each time GetNextFile() returns false, we are responsible for
// deleting a finder object.
//-------------------------------------------------------------------//
bool FileScan::GetNextFile()
{
	// Try the last successful finder object.
	int nFinderIndex = Finders.GetUpperBound();
	bool bReturn = GetNextFile( nFinderIndex );

	// We will be done only when we have come back up past the first Finders array item.
	// We recurse back upwards as far as possible before giving up.
	while ( !bReturn && nFinderIndex > 0 ) 
	{	
		// Unwind a finder from the array.
		delete Finders[ nFinderIndex ];
		Finders.RemoveAt( nFinderIndex );
		Results.RemoveAt( nFinderIndex );
		nFinderIndex--;

		bReturn = GetNextFile( nFinderIndex );
			
	}

	return bReturn;

}


//-------------------------------------------------------------------//
// GetNextFile()																		//
//-------------------------------------------------------------------//
// This is our recursive function.  We return the first match of
// file in the dir specified by the finder object at the given
// index, or any of its subdirs.
// Upon returning false, the caller is responsible for deleting
// the starting finder object.
//-------------------------------------------------------------------//
bool FileScan::GetNextFile( 
	int nFinderIndex 
) {
	
	while ( Results[ nFinderIndex ] ) 
	{
		// This will actually get us a file name.
		Results[ nFinderIndex ] = 
			( Finders[ nFinderIndex ]->FindNextFile() != FALSE );

		// If file == subdir
		if ( Finders[ nFinderIndex ]->IsDirectory() ) 
		{
			if ( m_bScanSubdirectories ) 
			{
				// Skip "." and "..".
				m_strDirName = Finders[nFinderIndex]->GetFileName();
				if ( 
						m_strDirName != _T(".")
					&& m_strDirName != _T("..")
				) {

					// Add a new finder to our array, using subdir, and recurse.
					// If recursion finds something, return true.
					ASSERT( Finders.GetUpperBound() == nFinderIndex );
					Finders.Add( new CFileFind );
					Results.Add( 
						Finders[ nFinderIndex + 1 ]->FindFile( 
							LPCTSTR( 
								Finders[nFinderIndex]->GetFilePath() + _T("\\*.*") 
							) 
						) != FALSE
					);
					if ( GetNextFile( nFinderIndex + 1 ) )
						return true;
					
					else 
					{						
						// Unwind a finder from the array.
						delete Finders[ nFinderIndex + 1 ];
						Finders.RemoveAt( nFinderIndex + 1 );
						Results.RemoveAt( nFinderIndex + 1 );

					}

				}

			}

		// else if file == filter match, return true.
		} else 
		{

			// TO DO
			// Do a proper filter check.
			// For now, we just compare extensions.
			CString strExtension = Finders[ nFinderIndex ]->GetFileName();
			extract_extension_from_filename( 
				&strExtension,
				&strExtension
			);
			if ( strExtension.CompareNoCase( m_strFilterExt ) == 0 ) 
			{			
				// We found one!
				return true;
			}

		}

		// Loop back to get next file

	}

	// Give it up, we're out of files.  :>
	return false;

}

//-------------------------------------------------------------------//
// These functions return results from the last successful find.
// They just pass along the info that they root out of CFileFind.
//-------------------------------------------------------------------//
CString FileScan::strFilename()
{
	return Finders[ Finders.GetUpperBound() ]->GetFileName();
}
CString FileScan::strTitle()
{
	return Finders[ Finders.GetUpperBound() ]->GetFileTitle();
}
CString FileScan::strPath()
{
	CString strPath;
	strPath = Finders[ Finders.GetUpperBound() ]->GetFilePath();
	strPath = strPath.Left( strPath.ReverseFind( _T('\\') ) + 1 );
	return strPath;
}
CString FileScan::strPathAndFilename()
{
	CString strPath;
	strPath = Finders[ Finders.GetUpperBound() ]->GetFilePath();
	return strPath;
}
Huge FileScan::hSize()
{
	// MDM 64-bit result is built-in, now.
	// return Finders[ Finders.GetUpperBound() ]->GetLength64();
	return Finders[ Finders.GetUpperBound() ]->GetLength();
}
CTime FileScan::ctCreatedTime()
{
	CTime ctCreated;
	Finders[ Finders.GetUpperBound() ]->GetCreationTime( ctCreated );
	return ctCreated;
}
CTime FileScan::ctModifiedTime()
{
	CTime ctModified;
	Finders[ Finders.GetUpperBound() ]->GetLastWriteTime( ctModified );
	return ctModified;
}
CTime FileScan::ctAccessedTime()
{
	CTime ctModified;
	Finders[ Finders.GetUpperBound() ]->GetLastAccessTime( ctModified );
	return ctModified;
}
