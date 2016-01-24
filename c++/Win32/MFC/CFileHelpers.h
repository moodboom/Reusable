//-------------------------------------------------------------------//
// CFileHelpers.cpp : MFC file helper functions
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef CFILE_HELPERS_H
	#define CFILE_HELPERS_H

#include "..\..\StandardTypes.h"

CString	GetShortFilename			( CString& strFilename,	bool bIgnoreErrors = false );
bool		bFileExists					( CString& strFilename	);
bool		bIsFileWritable			( CString& strFilename	);
void		MakeWritable				( CString& strFilename	);
bool		MakeDir						( CString  strPath		);

bool bHasPlainLocalRoot(LPCTSTR path);
bool bIsPlainLocalRoot(LPCTSTR path);
bool bIsShortCut(LPCTSTR path);
bool bIsUNCPath(LPCTSTR path);
bool bIsNetworkedPath(LPCTSTR path);

bool GetShare(LPCTSTR path, CString *share);
bool GetRoot(LPCTSTR path, CString *root);
bool StripRoot(LPCTSTR path, CString *nonRoot);

inline uShort DriveLetToNum( char drivelet );
inline char DriveNumToLet(uShort drive);

/*
bool TranslateDriveToUNC(LPCSTR inPath, CString *outPath);
bool TranslateUNCToLocal(
	LPCSTR		inPath,
	CString*		outPath,
	bool*			pbIsLoopedBack
);
bool CheckIfLoopedBack(
	LPCSTR		inPath, 
	CString*		outPath, 
	bool*			pbIsLoopedBack
);
bool IsDriveNetworkMapped( uShort drive );

static BOOL Win95GetUniversalName( LPCTSTR szDrive, LPTSTR szUniv  );
*/

// This can rename files AND move files.  Specify the full file names.
// Make sure the source exists, and the dest does not, or nothing will
// happen and this will return false.
bool				RenameFile( 
	CString	strSourceName,
	CString	strDestName
);

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
int				GetDriveTypeFromFilename( CString& strFilename	);		

CString strGetDriveVolumeName( CString& strFilename );

CString cstrFileSizeFormat( Huge hSize );

bool VerifyFileIsWritable(
	const TCHAR*	pszFilename,
	bool				bNoPrompt		= true,
	UINT				MessageID		= 0
);

#endif	// CFILE_HELPERS_H
