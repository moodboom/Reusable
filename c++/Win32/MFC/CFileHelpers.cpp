//-------------------------------------------------------------------//
// CFileHelpers.cpp : MFC file helper functions
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include <SYS\STAT.H>					// For _S_IREAD etc.

#include <io.h>							// For _access()

#include "CStringHelpers.h"			// For strGetRootPathName() etc.

#include "DisplayMessage.h"

#include "CFileHelpers.h"


//-------------------------------------------------------------------//
// bFileExists()																		//
//-------------------------------------------------------------------//
// This function determines if the given file exists.
//-------------------------------------------------------------------//
bool bFileExists( CString& strFilename )
{
	return ( _taccess( LPCTSTR( strFilename ), 0 ) != -1 );
}


//-------------------------------------------------------------------//
// bIsFileWritable()																	//
//-------------------------------------------------------------------//
// This function determines if the given file is writable 
// (and readable!).
//-------------------------------------------------------------------//
bool bIsFileWritable( CString& strFilename )
{
	return ( _taccess( LPCTSTR( strFilename ), 6 ) != -1 );
}


//-------------------------------------------------------------------//
// MakeWritable()																		//
//-------------------------------------------------------------------//
void MakeWritable( CString& strFilename )
{
	if ( !bIsFileWritable( strFilename ) )
	{
		_tchmod( LPCTSTR( strFilename ), _S_IREAD | _S_IWRITE );
	}
}


//-------------------------------------------------------------------//
// MakeDir()																			//
//-------------------------------------------------------------------//
// This function makes the requested dir.
// It can handle requests that require creation of more than one
// directory (I think  :> ).
//
// It only returns false if the dir does not exist after the call.
// More specifically, if CreateDirectory() fails because the dir
// already exists, we still return true.
//-------------------------------------------------------------------//
bool MakeDir( CString strPath )
{
	bool bReturn = true;

	// Recurse as needed.
	RemoveTrailingBackslash( strPath );
	CString strParent = GetPathFromFilename( strPath );
	if ( 
			strParent != strPath
		&&	!bFileExists( strParent ) 
	)
		bReturn = MakeDir( strParent );
	
	if ( bReturn )
	{
		bReturn = ( 
			CreateDirectory( 
				LPCTSTR( strPath ),						// pointer to directory path string
				0 												// pointer to security descriptor (NT only), NULL = use default
			) != FALSE
		);

		if ( !bReturn )
		{
			// If we failed because the dir exists, we want to return true.
			DWORD dwError = GetLastError();
			if ( dwError == ERROR_ALREADY_EXISTS )
				bReturn = true;
		}
	}

	return bReturn;

}


//-------------------------------------------------------------------//
// GetShortFilename()																//
//-------------------------------------------------------------------//
// This function converts the given filename into its short 
// equivalent.  It also converts the string to lower case,
// and clears any ending backslash. This is helpful in comparing 
// two filenames for effective equivalence.
//
// Failure to find the supplied path or file will result in
// returning a blank string.  
// 
// Check for a returned blank string if you anticipate that you 
// may be passing a bad file/path and you want to handle it.
// Don't forget to set bIgnoreErrors = true.
//
// NOTE: CD-ROM drive paths will fail with no media inserted!
//-------------------------------------------------------------------//
CString GetShortFilename( 
	CString& strFilename,
	bool		bIgnoreErrors
) {

	CString strReturn;

	// Create the file buffer.
	// The docs say that short names sometimes are LONGER than long
	// names.  A possible example:  
	//	
	//		"c:\Get it\file.exe" -> "c:\Getit~1\file.exe"
	//
	// So we double the size for safety.

	size_t BufferSize = strFilename.GetLength() * 2;
	TCHAR* pszBuffer = new TCHAR[ BufferSize ];
	TCHAR* pszBufferOut = new TCHAR[ BufferSize ];
	
	// Fill it.
	_tcsncpy(
		pszBuffer, 
		LPCTSTR( strFilename ),
		BufferSize
	);

	// Convert it.
	DWORD dwResult = GetShortPathName(
		pszBuffer,
		pszBufferOut,
		BufferSize
	);

	// The short filename was VERY LONG, and 
	// didn't fit in the buffer????
	ASSERT( dwResult < (DWORD) BufferSize );

	// Check for failure.
	// If dwResult == 0, the filename prolly wasn't valid.
	// We leave the result BLANK and assert (unless anticipated).
	// Note that CD-ROM drive paths will fail with no media inserted!
	// In that case, we will end up returning a blank string, as
	// we do with all other errors.
	if ( dwResult == 0 )
	{
		if ( !bIgnoreErrors )
		{
			// DEBUG
			// Common error = 2, "file not found".
			// int nErr = GetLastError();
			ASSERT( false );
		}
	
	} else 
	{
		// Copy it out.
		strReturn = pszBufferOut;

		// Convert to lower case.
		strReturn.MakeLower();

		// Remove any trailing backslash as needed.
		// We remove instead of add because we may be dealing
		// with a filename, not a dir name.
		RemoveTrailingBackslash( strReturn );
	}

	// Clean up.
	delete[] pszBuffer;
	delete[] pszBufferOut;

	return strReturn;

}


//-------------------------------------------------------------------//
// bHasPlainLocalRoot()																//
//-------------------------------------------------------------------//
// Returns TRUE if the specified path begins with a form
// "C:\", FALSE otherwise.
//-------------------------------------------------------------------//
inline bool bHasPlainLocalRoot(LPCTSTR path)
{
	return ((path[1] == _T(':') ) && (path[2] == _T('\\') ));
}


//-------------------------------------------------------------------//
// bIsPlainLocalRoot()																//
//-------------------------------------------------------------------//
// Returns TRUE if the specified path is of the form "C:\",
// FALSE otherwise.
//-------------------------------------------------------------------//
inline bool bIsPlainLocalRoot(LPCTSTR path)

{
	return ( 
			( _tcslen( path ) == 3 ) 
		&& bHasPlainLocalRoot(path) 
	);
}


//-------------------------------------------------------------------//
// bIsShortCut()																		//
//-------------------------------------------------------------------//
// Returns TRUE if the specified path is of the form "*.lnk", 
// FALSE otherwise.
//-------------------------------------------------------------------//
inline bool bIsShortCut(LPCSTR path)
{
	char		ext[4];
	uLong		length;

	length = strlen(path);

	if (strlen(path) < 4)
	{
		return FALSE;
	}
	else
	{
		// There is no string 'right' command in Ring-0.
		ext[0] = path[length - 4];
		ext[1] = path[length - 3];
		ext[2] = path[length - 2];
		ext[3] = path[length - 1];

		return (_strnicmp(ext, ".lnk", 4) == 0);
	}
}


//-------------------------------------------------------------------//
// bIsUNCPath()																		//
//-------------------------------------------------------------------//
// Returns TRUE if the specified path is in UNC format, FALSE
// otherwise.
//-------------------------------------------------------------------//
inline bool bIsUNCPath(LPCTSTR path)
{
	uLong i, length, numSlashes;

	if (_tcsncmp(path, _T("\\\\"), 2) != 0)
		return FALSE;

	length = _tcslen(path);
	numSlashes = 0;

	for (i = 0; i < length; i++)
	{
		if (
				( path[i] == _T('\\') ) 
			|| ( path[i] == _T('/' ) )
		)
			numSlashes++;

		if (numSlashes == 4)
			return TRUE;
	}

	return FALSE;
}


//-------------------------------------------------------------------//
// bIsNetworkedPath()																//
//-------------------------------------------------------------------//
// Returns TRUE if the specified path resides on a networked
// drive, FALSE otherwise.
//-------------------------------------------------------------------//
inline bool bIsNetworkedPath(LPCTSTR path)
{
	if (_tcslen(path) < 3)
		return FALSE;

	if (bIsUNCPath(path))
		return TRUE;

	TCHAR head[4];

	_tcsncpy( head, path, 3 );
	head[3] = _T('\0');

	return (GetDriveType(head) == DRIVE_REMOTE);

}


/*
//-------------------------------------------------------------------//
// Win95GetUniversalName()															//
//-------------------------------------------------------------------//
// Win95GetUniversalName implements a hack under Windows95 that simulates
// the non-working WNetGetUniversalName (from KB Article ID: Q131416).
//-------------------------------------------------------------------//
static BOOL Win95GetUniversalName( LPCTSTR szDrive, LPTSTR szUniv  )   
{
	// get the local drive letter
	char chLocal = toupper( szDrive[0] );

	// cursory validation
	if ( chLocal < 'A' || chLocal > 'Z' )
		return FALSE;

	if ( szDrive[1] != ':' || szDrive[2] != '\\' )
		return FALSE;

	HANDLE hEnum;
	DWORD dwResult = WNetOpenEnum( RESOURCE_CONNECTED, RESOURCETYPE_DISK,
									 0, NULL, &hEnum );

	if ( dwResult != NO_ERROR )
		return FALSE;

	// request all available entries
	const int    c_cEntries   = 0xFFFFFFFF;
	// start with a reasonable buffer size
	DWORD        cbBuffer     = 50 * sizeof( NETRESOURCE );
	NETRESOURCE *pNetResource = (NETRESOURCE*) malloc( cbBuffer );

	BOOL fResult = FALSE;

	while ( TRUE )
	{
		DWORD dwSize   = cbBuffer,
		cEntries = c_cEntries;

		dwResult = WNetEnumResource( hEnum, &cEntries, pNetResource,
									  &dwSize );

		if ( dwResult == ERROR_MORE_DATA )
		{
			// the buffer was too small, enlarge
			cbBuffer = dwSize;
			pNetResource = (NETRESOURCE*) realloc(pNetResource, cbBuffer);
			continue;
		}

		if ( dwResult != NO_ERROR )
			goto done;

		// search for the specified drive letter
		for ( int i = 0; i < (int) cEntries; i++ )
			if ( pNetResource[i].lpLocalName &&
				chLocal == toupper(pNetResource[i].lpLocalName[0]) )
			{
				// match
				fResult = TRUE;

				// build a UNC name
				strcpy( szUniv, pNetResource[i].lpRemoteName );
				strcat( szUniv, szDrive + 2 );
				_strupr( szUniv );
				goto done;
			}
		}

done:
	// cleanup
	WNetCloseEnum( hEnum );
	free( pNetResource );

	return fResult;
}


//-------------------------------------------------------------------//
// TranslateDriveToUNC()															//
//-------------------------------------------------------------------//
// TranslateDriveToUNC translates a mapped networked drive-based pathname to
// its UNC form.
//-------------------------------------------------------------------//
bool TranslateDriveToUNC(LPCSTR inPath, CString *outPath)
{
	bool bReturn = true;

	try
	{
		uLong result;

		if ( bWin9X() && !bWin98())
		{
			bool	bResultWin95;
			char	tempOutPath[MAX_PATH];

			// Special hack function for Windows95.
			bResultWin95 = Win95GetUniversalName(inPath, tempOutPath) == FALSE ? false : true;

			if (bResultWin95)
			{
				(*outPath) = tempOutPath;
			}
		}
		else
		{
			UNIVERSAL_NAME_INFO	*pUNI;
			unsigned long		bufSize;

			// Handle normally for Win98 and WinNT.
			bufSize = sizeof(UNIVERSAL_NAME_INFO) + 
				MAX_PATH * sizeof(WCHAR);

			pUNI = (UNIVERSAL_NAME_INFO *) new uShort[bufSize];

			result = WNetGetUniversalName(inPath, 
				UNIVERSAL_NAME_INFO_LEVEL, pUNI, &bufSize);

			if (result == NO_ERROR)
			{
				(* outPath) = pUNI->lpUniversalName;
			}

			delete[] (uShort*) pUNI;
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		bReturn = false;
	}

	return bReturn;
}
*/


//-------------------------------------------------------------------//
// GetShare()																			//
//-------------------------------------------------------------------//
// Takes a UNC path and returns the share component.
//-------------------------------------------------------------------//
bool GetShare(LPCTSTR path, CString *share)
{
	bool bReturn = true;

	try
	{
		CString tempString;

		tempString = path;
		share->Empty();

		if ( bIsUNCPath(path) )
		{
			Long	i;
			uLong	numSlashes;

			for ( i = 0, numSlashes = 0; i < tempString.GetLength(); i++ )
			{
				if ( path[i] == _T('\\') )
					numSlashes++;

				if (numSlashes == 3)
				{
					tempString = path + i + 1;
					(* share) = tempString.Left(tempString.Find( _T('\\') ));

					break;
				}
			}
		}
	}
	catch (CMemoryException *ex)
	{
		bReturn = false;
		ex->Delete();
	}

	return bReturn;
}


//-------------------------------------------------------------------//
// GetRoot()																			//
//-------------------------------------------------------------------//
// GetRoot takes any legal path and returns a string of the form 
// "C:\" or "\\UncVolumeName\share\".
//-------------------------------------------------------------------//
bool GetRoot(LPCTSTR path, CString *root)
{
	bool bReturn = true;

	try
	{
		CString tempString;

		tempString = path;
		root->Empty();

		if ( bHasPlainLocalRoot(path) )
		{
			(* root) = tempString.Left(3);
		}
		else if ( bIsUNCPath(path) )
		{
			Long	i;
			uLong	numSlashes;

			for (i = 0, numSlashes = 0; i < tempString.GetLength(); i++)
			{
				if (path[i] == _T('\\') )
					numSlashes++;

				if (numSlashes == 4)
				{
					(* root) = tempString.Left(i + 1);
					break;
				}
			}
		}
	}
	catch (CMemoryException *ex)
	{
		bReturn = false;
		ex->Delete();
	}

	return bReturn;
}


//-------------------------------------------------------------------//
// StripRoot()																			//
//-------------------------------------------------------------------//
// StripRoot removes the root from a pathname.
//-------------------------------------------------------------------//
bool StripRoot(LPCTSTR path, CString *nonRoot)
{
	bool bReturn = true;

	try
	{
		CString csInPath, root;

		csInPath = path;

		if ( GetRoot(path, &root) )
		{
			(* nonRoot) = csInPath.Right(csInPath.GetLength() - 
				root.GetLength());
		}
	}
	catch (CMemoryException *ex)
	{
		bReturn = false;
		ex->Delete();
	}

	return bReturn;
}


//-------------------------------------------------------------------//
// DriveLetToNum()																	//
//-------------------------------------------------------------------//
// DriveNumToLet converts a 0-based drive number to an uppercase drive letter.
//-------------------------------------------------------------------//
inline uShort DriveLetToNum(
	char drivelet
) {
	return (toupper(drivelet) - 'A');
}
inline char DriveNumToLet(uShort drive)
{
	return (drive + 'A');
}


/*
//-------------------------------------------------------------------//
// TranslateUNCToLocal()															//
//-------------------------------------------------------------------//
// TranslateUNCToLocal translates a 'looped-back' path to its local 
// form.
//-------------------------------------------------------------------//

// From Svrapi.h.
#define	LM20_NNLEN		12		// LM 2.0 Net name length
#define	MAXCOMMENTSZ	256	// Multipurpose comment length
#define	SHPWLEN			8		// Share password length (bytes)
struct share_info_50 
{
	char			shi50_netname[LM20_NNLEN+1];
	unsigned char 	shi50_type; 
    unsigned short	shi50_flags;
	char FAR *		shi50_remark;
	char FAR *		shi50_path;
	char			shi50_rw_password[SHPWLEN+1];
	char			shi50_ro_password[SHPWLEN+1];

};

bool TranslateUNCToLocal(
	LPCSTR		inPath,
	CString*		outPath,
	bool*			pbIsLoopedBack
) {
	bool bReturn = true;

	ASSERT(
			bWin9X() 
		|| bWinNT4OrGreater()
	);

	try
	{
		CString newRoot, nonRoot, share;

		if ( GetShare( inPath, &share ) )
		{
			bReturn = StripRoot(inPath, &nonRoot);
		}

		if ( bReturn )
		{
			if ( bWin9X() )
			{
				uLong				bufSize, result;
				share_info_50*	pSI50;
				unsigned short	cbTotalAvail;

				bufSize = sizeof(share_info_50) + MAX_PATH + MAXCOMMENTSZ + 2;
				pSI50 = (share_info_50 *) new uShort[bufSize];

				result = Win95_NetShareGetInfo(NULL, share, 50, 
					(char *) pSI50, bufSize, &cbTotalAvail);

				if (result == NERR_Success)
				{
					newRoot = pSI50->shi50_path;
				} else
				{
					bReturn = false;
				}

				delete[] (uShort*) pSI50;
			}
			else if ( bWinNT4OrGreater() )
			{
				uLong				result;
				SHARE_INFO_2*	pSI2;

				USES_CONVERSION;

				result = WinNT_NetShareGetInfo(NULL, A2W(share), 2, 
					(LPBYTE *) &pSI2);

				if (result == NERR_Success)
				{
					newRoot = W2A((LPWSTR) pSI2->shi2_path);
					WinNT_NetApiBufferFree(pSI2);
				} else
				{
					bReturn = false;
				}
			}
		}

		if ( bReturn )
		{
			(* outPath) = newRoot + "\\" + nonRoot;
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		bReturn = false;
	}

	(*pbIsLoopedBack) = bReturn;

	return derr;
}


//-------------------------------------------------------------------//
// CheckIfLoopedBack()																//
//-------------------------------------------------------------------//
// Checks if the given networked path is looped back to a
// local drive.
//-------------------------------------------------------------------//
bool CheckIfLoopedBack(
	LPCSTR		inPath, 
	CString*		outPath, 
	bool*			pbIsLoopedBack
) {
	bool bReturn = true;

	if ( !bIsNetworkedPath( inPath ) )
	{
		(*pbIsLoopedBack) = false;
	}
	else if ( bIsUNCPath( inPath ) )
	{
		bReturn = TranslateUNCToLocal(inPath, outPath, pbIsLoopedBack);
	}
	else
	{
		CString uncPath, share;

		bReturn = TranslateDriveToUNC(inPath, &uncPath);
		
		if ( bReturn )
		{
			bReturn = TranslateUNCToLocal(uncPath, outPath, pbIsLoopedBack);
		}
	}

	return bReturn;
}


//-------------------------------------------------------------------//
// IsDriveNetworkMapped()															//
//-------------------------------------------------------------------//
// IsDriveNetworkMapped checks if a given drive letter represents a currently
// mapped, or a mapped but non-working networked drive. (Needed because
// mapped but non-working drives are seen in the Explorer but not indicated
// by GetLogicalDrives).
//-------------------------------------------------------------------//
bool IsDriveNetworkMapped(uShort drive)
{
	char			keyName[2];
	CString		regPathToMapped;
	HKEY			regHandle;
	bool			bIsMapped	= FALSE;
	uLong			nResult;

	// ASSERT( IsLegalDriveNumber(drive) );

	keyName[0] = DriveNumToLet(drive);
	keyName[1] = '\0';

	if ( bWin9X() )
		regPathToMapped = kRegistryMappedDrive98Section;
	else
		regPathToMapped = kRegistryMappedDriveNTSection;

	regPathToMapped += "\\";
	regPathToMapped += keyName;

	result = RegOpenKeyEx(kRegistryMappedDriveRoot, regPathToMapped, NULL, 
		KEY_READ, &regHandle);

	if (result == ERROR_SUCCESS)
	{
		isMapped = TRUE;
		RegCloseKey(regHandle);
	}
	else
	{
		isMapped = FALSE;
	}

	return isMapped;
}
*/


//-------------------------------------------------------------------//
// RenameFile()																		//
//-------------------------------------------------------------------//
// This function renames a file.  It returns false if the file does
// not exist, or if the target file DOES exist.
//-------------------------------------------------------------------//
bool RenameFile( 
	CString	strSourceName,
	CString	strDestName
) {

	if (
			!bFileExists( strSourceName	)
		||	 bFileExists( strDestName		)
	)
		return false;

	MakeDir( GetPathFromFilename( strDestName ) );

	return 
	(
		MoveFile(
			LPCTSTR( strSourceName	),	// Existing file
			LPCTSTR( strDestName		)
		) != FALSE
	);

}


//-------------------------------------------------------------------//
// GetDriveTypeFromFilename()														//
//-------------------------------------------------------------------//
// This function extracts the root path name from the supplied
// filename, and returns the drive type, as specified by 
// GetDriveType():
//
//		DRIVE_UNKNOWN		The drive type cannot be determined. 
//		DRIVE_NO_ROOT_DIR The root path is invalid. For example, no volume is mounted at the path. 
//		DRIVE_REMOVABLE	The disk can be removed from the drive. 
//		DRIVE_FIXED			The disk cannot be removed from the drive. 
//		DRIVE_REMOTE		The drive is a remote (network) drive. 
//		DRIVE_CDROM			The drive is a CD-ROM drive. 
//		DRIVE_RAMDISK		The drive is a RAM disk.
//
//-------------------------------------------------------------------//
int GetDriveTypeFromFilename( CString& strFilename	)
{
	CString strDrive = strGetRootPathName( strFilename );
	return GetDriveType( LPCTSTR( strDrive ) );
}


//-------------------------------------------------------------------//
// strGetDriveVolumeName()															//
//-------------------------------------------------------------------//
// This gets the volume name for the drive that the file 
// resides on.  You can provide any string that starts with a
// valid drive designation, that is, either a local
// drive designation (e.g., "d:\...") or a UNC share 
// (e.g., "\\Machine\Share\...").
//-------------------------------------------------------------------//
CString strGetDriveVolumeName( CString& strFilename )
{
	CString strRootPathName = strGetRootPathName( strFilename );

	// This prevents the STUPID GetVolumeInfo() function from
	// prompting for removable media to be inserted.
	UINT nPrevMode = SetErrorMode( SEM_FAILCRITICALERRORS );
	
	const int cnVolMaxSize = 200;
	TCHAR szVolumeName[ cnVolMaxSize ];
	if( 
		!GetVolumeInformation(
			LPCTSTR( strRootPathName ),
			szVolumeName,
			cnVolMaxSize,
			NULL,NULL,NULL,NULL,0
		)
	)
	{
		SetErrorMode( nPrevMode );
		return cstrUnknown;
	}

	SetErrorMode( nPrevMode );
	CString strResult( szVolumeName );
	if ( strResult.IsEmpty() )
		strResult = cstrBlank;

	return strResult;
}


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
CString cstrFileSizeFormat( Huge hSize )
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

	CString strResult;
	if ( hSize > hPBLimit )
		strResult.Format( _T("%5.1f PB"), (double)hSize / (double)hBytesPerPB );
	else if ( hSize > hTBLimit )
		strResult.Format( _T("%5.1f TB"), (double)hSize / (double)hBytesPerTB );
	else if ( hSize > chGBLimit )
		strResult.Format( _T("%5.1f GB"), (double)hSize / (double)chBytesPerGB );
	else if ( hSize > chMBLimit )
		strResult.Format( _T("%5.1f MB"), (double)hSize / (double)chBytesPerMB );
	else if ( hSize > chKBLimit )
		strResult.Format( _T("%5.1f KB"), (double)hSize / (double)chBytesPerKB );
	else
		strResult.Format( _T("%5d bytes"), (int) hSize );
		
	return strResult;
}


//-------------------------------------------------------------------//
// VerifyFileIsWritable()															//
//-------------------------------------------------------------------//
// This function makes sure that the given file is not 
// read-only.
// 
// If bNoPrompt is true, this function will attempt to update 
// read-only files automatically.  If bNoPrompt is false, the 
// provided MessageID will be used to display a Yes/No message to
// the user.  If they select Yes, the files will be made writable.
//
//	The return value will be true if the files are writable, false
// if not.
//-------------------------------------------------------------------//
bool VerifyFileIsWritable(
	const TCHAR*	pszFilename,
	bool				bNoPrompt,
	UINT				MessageID 
) {

	bool bResult = ( _access( pszFilename, 6 ) == 0 );
	
	if ( !bResult )
	{

		if ( 
			bNoPrompt																						||
			DisplayMessage( MessageID, IDL_WARNING, (CWnd*)NULL, MB_YESNO ) == IDYES )
		{
		
			// Change the files' attributes.
			bResult = ( _chmod( pszFilename, _S_IREAD | _S_IWRITE ) == 0 );
			
		}

	}

	return bResult;

}


