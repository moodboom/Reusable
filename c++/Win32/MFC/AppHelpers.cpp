//-------------------------------------------------------------------//
// AppHelpers.cpp : MFC App helper functions
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "CStringHelpers.h"			// For MakeTitleCase() in GetProgramName(), GetPathFromFilename(), etc.

#include <lmerr.h>					// For error codes.

#include "AppHelpers.h"


//-------------------------------------------------------------------//
// GLOBALS / CONSTANTS / STATICS / DEFINES
//-------------------------------------------------------------------//

// This is used by the version-checking routines.
#define WINDOWSVERSION( major, minor ) ( (DWORD)MAKELONG( minor, major ) )		

//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// ConvertErrorToString()															//
//-------------------------------------------------------------------//
ustring ConvertErrorToString( DWORD dwError )
{	
	HMODULE 	hModule 	= NULL; // default to system source
	LPTSTR	MessageBuffer	= NULL;
	DWORD		dwBufferLength;

	ustring strError;

	// if dwError is in the network range, load the message source
	if (
			dwError >= NERR_BASE 
		&& dwError <= MAX_NERR
	) {
	    hModule = ::LoadLibraryEx( _TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE );
	}
 
	// call FormatMessage() to allow for message text to be acquired
	// from the system or the supplied module handle
	dwBufferLength = ::FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER 
		|	FORMAT_MESSAGE_IGNORE_INSERTS 
		|	FORMAT_MESSAGE_FROM_SYSTEM 
		|	// always consider system table
			((hModule != NULL) ? FORMAT_MESSAGE_FROM_HMODULE : 0),
		
		// module to get message from (NULL == system)
		hModule,					
		
		dwError,
		
		// default language
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	
		
		(LPTSTR) &MessageBuffer, 
		0, 
		NULL 
	);

	if ( MessageBuffer ) 
	{
		if ( dwBufferLength )
			strError = MessageBuffer;
 
		// free the buffer allocated by the system
		::LocalFree(MessageBuffer);
	}
 
	// if you loaded a message source, unload it
	if (hModule != NULL)
	    ::FreeLibrary(hModule);

	if ( strError.size() == 0 ) 
	{
		strError = _T( "Unknown System Error " );
		strError.AppendNumber( dwError );
	}

	return strError;
}


//-------------------------------------------------------------------//
// GetProgramFilename()																//
//-------------------------------------------------------------------//
// This uses a "Dynamic Link Library Function".  Sounds scary.
//-------------------------------------------------------------------//
CString GetProgramFilename() 
{

	CString strFilename;
	
	const int nMaxPath = 500;
	TCHAR* pszProgramPath = new TCHAR[ nMaxPath ];
	GetModuleFileName(
		0,							// NULL = current
		pszProgramPath,
		nMaxPath
	);
	strFilename = pszProgramPath;
	delete[] pszProgramPath;
	
	return strFilename;

}


//-------------------------------------------------------------------//
// GetProgramPath()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
CString GetProgramPath() 
{

	CString strPath = GetProgramFilename();

	// Trim off the executable name.
	strPath = GetPathFromFilename( strPath );

	/*
	// OLD WAY
	strPath = strPath.Left( 
			strPath.GetLength() 
		-	GetProgramName().GetLength()
		-	4													// for ".EXE"
	);
	*/

	return strPath;
	
}


//-------------------------------------------------------------------//
// GetProgramName()																	//
//-------------------------------------------------------------------//
// This function gets the name of the program.  The app keeps track
// of this name for us.  We pretty it up by making it first-character-
// uppercase.
//-------------------------------------------------------------------//
CString GetProgramName()
{
	CString strProgramName( AfxGetApp()->m_pszExeName );
	MakeTitleCase( &strProgramName );
	return strProgramName;
}


//-------------------------------------------------------------------//
// GetProgramCreationDate()														//
//-------------------------------------------------------------------//
// This function returns the creation date for the program file.
// Note that we use "LastWriteTime", not "CreationTime".  When the
// compiler rebuilds the executable, it updates Write time, not
// create time.  And when the file is copied, create time is updated,
// but last write remains the same.  So last write represents our
// creation time better, from the perspective of when the executable
// was built by the compiler.
// Note that it shows how to get the other available times
// and the file attributes as well.
//-------------------------------------------------------------------//
COleDateTime GetProgramCreationDate()
{

	// This was taken from SCANDIR.C in MSDN.
	BY_HANDLE_FILE_INFORMATION bhfi;         
	HANDLE hfile;  
	hfile = CreateFile(
		LPCTSTR( GetProgramFilename() ), 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
	); 
	
	// Get all the file times.
	GetFileTime(
		hfile, 
		&bhfi.ftCreationTime, 
		&bhfi.ftLastAccessTime, 
		&bhfi.ftLastWriteTime
	); 

	// Note: Here's how we can get the file attributes.
	// bhfi.dwFileAttributes = GetFileAttributes( LPCTSTR( GetProgramFilename() ) ); 

	CloseHandle(hfile);

	// Note that we return the last write time.  This is the most valid date
	// to determine when this executable was last "built" by the compiler.
	COleDateTime Result = bhfi.ftLastWriteTime;
	return Result;

}


enum WindowsVersion 
{
	WV_WinNT3, 
	WV_WinNT4NoSp, 
	WV_WinNT4Sp1, 
	WV_WinNT4Sp2, 
	WV_WinNT4Sp3, 
	WV_WinNT4PostSp3, 
	WV_WinNTPost4, 
	WV_Win95OSR1, 
	WV_Win95OSR2, 
	WV_Win98OrFuture, 
	WV_WinUnknown, 
	WV_Uninitialized
};


// GetWindowsVersion returns the currently running version of Windows.

//-------------------------------------------------------------------//
// GetWindowsVersion()																//
//-------------------------------------------------------------------//
// Here's what the documentation recommends that you use instead
// of GetVersionEx(), an older function.  The only problem
// is that these funtions only EXIST on Win2000!!!!  You stupid 
// ignorant sacks of... <pounding on little Billy Gates' head>
/*
// Set up the version structure.
OSVERSIONINFOEX osVersion;
ZeroMemory( &osVersion, sizeof( OSVERSIONINFOEX ) );
osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

// Init the "condition mask".
DWORDLONG dwlConditionMask = 0;
VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );

// Call VerifyVersionInfo() and return the result.
return ( VerifyVersionInfo( &osVersion, VER_MAJORVERSION, dwlConditionMask ) != FALSE );
*/

//-------------------------------------------------------------------//
WindowsVersion GetWindowsVersion()
{
	OSVERSIONINFO verInfo;

	verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		if (!::GetVersionEx(&verInfo))
		return WV_WinUnknown;

	switch (verInfo.dwPlatformId)
	{

	case VER_PLATFORM_WIN32_WINDOWS:
		if (verInfo.dwMinorVersion == 0)
		{
			// if (GetLowWord(verInfo.dwBuildNumber) > 1080)
			if ( LOWORD(verInfo.dwBuildNumber) > 1080 )
			{
				return WV_Win95OSR2;
			}
			else
			{
				return WV_Win95OSR1;
			}
		}
		else
		{
			return WV_Win98OrFuture;
		}

	case VER_PLATFORM_WIN32_NT:
		if (verInfo.dwMajorVersion == 3)
		{
			return WV_WinNT3;
		}
		else if (verInfo.dwMajorVersion == 4)
		{
			if (_tcslen(verInfo.szCSDVersion) == 0)
			{
				return WV_WinNT4NoSp;
			}
			else if (_tcscmp(verInfo.szCSDVersion, _T("Service Pack 1") ) == 0)
			{
				return WV_WinNT4Sp1;
			}
			else if (_tcscmp(verInfo.szCSDVersion, _T("Service Pack 2") ) == 0)
			{
				return WV_WinNT4Sp2;
			}
			else if (_tcscmp(verInfo.szCSDVersion, _T("Service Pack 3") ) == 0)
			{
				return WV_WinNT4Sp3;
			}
			else
			{
				return WV_WinNT4PostSp3;
			}
		}
		else if ( verInfo.dwMajorVersion > 4 )
		{
			return WV_WinNTPost4;
		}
		else
		{
			return WV_WinUnknown;
		}

	default:
		return WV_WinUnknown;
	}
}


//-------------------------------------------------------------------//
// Is_Win2000_Professional()														//
//-------------------------------------------------------------------//
// This uses the newer VerifyVersionInfo() call.  This gives more
// detailed (and more confusing) results than GetVersionEx().
// This will NOT COMPILE on anything pre-Windows2000, ignorant mofos.
//-------------------------------------------------------------------//
/*
BOOL Is_Win2000_Professional() 
{
   OSVERSIONINFOEX osvi;
   DWORDLONG dwlConditionMask = 0;

   // Initialize the OSVERSIONINFOEX structure.

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   osvi.dwMajorVersion = 5;
   osvi.wProductType = VER_NT_WORKSTATION;

   // Initialize the condition mask.

   VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, 
      VER_GREATER_EQUAL );
   VER_SET_CONDITION( dwlConditionMask, VER_PRODUCT_TYPE, 
      VER_EQUAL );

   // Perform the test.

   return VerifyVersionInfo(
      &osvi, 
      VER_MAJORVERSION | VER_PRODUCT_TYPE,
      dwlConditionMask);
}
*/


//-------------------------------------------------------------------//
// bWin9XOS()																			//
//-------------------------------------------------------------------//
// Returns true if the user is running a Win95
// compatible machine.
//-------------------------------------------------------------------//
bool bWin9X()
{
	switch (GetWindowsVersion())
	{
		case WV_Win95OSR1:
		case WV_Win95OSR2:
		case WV_Win98OrFuture:
			return true;

		default:
			return false;
	}
}


//-------------------------------------------------------------------//
// bWin95OSR2OrGreater()															//
//-------------------------------------------------------------------//
// Returns true if the user is running a Win95
// OSR2 compatible machine.
//-------------------------------------------------------------------//
bool bWin95OSR2OrGreater()
{
	switch (GetWindowsVersion())
	{
		case WV_Win95OSR2:
		case WV_Win98OrFuture:
			return true;

		default:
			return false;
	}
}


//-------------------------------------------------------------------//
// bWin98()																				//
//-------------------------------------------------------------------//
// Returns true if the user is running a Win98
// compatible machine.
//-------------------------------------------------------------------//
bool bWin98()
{
	switch (GetWindowsVersion())
	{
		case WV_Win98OrFuture:
			return true;

		default:
			return false;
	}
}


//-------------------------------------------------------------------//
// IsWinNT4CompatibleMachine()													//
//-------------------------------------------------------------------//
// Returns true if the user is running a version of
// Windows NT greater than version 3.
//-------------------------------------------------------------------//
bool bWinNT4OrGreater()
{
	switch (GetWindowsVersion())
	{
		case WV_WinNT4NoSp:
		case WV_WinNT4Sp1:
		case WV_WinNT4Sp2:
		case WV_WinNT4Sp3:
		case WV_WinNT4PostSp3:
		case WV_WinNTPost4:
			return true;

		default:
			return false;
	}
}


//-------------------------------------------------------------------//
// bWin2000OrGreater()																//
//-------------------------------------------------------------------//
// Returns true if the user is running a version of
// Windows NT greater than version 4.
//-------------------------------------------------------------------//
bool bWin2000OrGreater()
{
	switch (GetWindowsVersion())
	{
		case WV_WinNTPost4:
			return true;

		default:
			return false;
	}
}


//-------------------------------------------------------------------//
// bCommonControlsUpToDate()														//
//-------------------------------------------------------------------//
// Check for a minimum common control version.
// NOTE: Use this as a specific example of how to call GetDllVersion.
//-------------------------------------------------------------------//
bool bCommonControlsUpToDate( int nReqdVersion )
{
	// Check to make sure that the requested version of the common controls
	// is available.
	return ( 
			GetDllVersion( TEXT( "comctl32.dll" ) ) 
		>= WINDOWSVERSION( nReqdVersion, 0 )
	);
}


//-------------------------------------------------------------------//
// GetDllVersion()																	//
//-------------------------------------------------------------------//
// Checks the version of the indicated dll.  We use the PACKVERSION
// macro with the return value, as outlined in the MSDN article:
// "Shell and Common Controls Versions"
//-------------------------------------------------------------------//
#include <shlwapi.h>
DWORD GetDllVersion( LPCTSTR lpszDllName )
{
	HINSTANCE hinstDll;
	DWORD dwVersion = 0;

	hinstDll = LoadLibrary( lpszDllName );
	
	if( hinstDll )
	{
		DLLGETVERSIONPROC pDllGetVersion;

		pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress( hinstDll, "DllGetVersion" );

		// Because some DLLs may not implement this function, you
		// must test for it explicitly. Depending on the particular 
		// DLL, the lack of a DllGetVersion function may
		// be a useful indicator of the version.
		if( pDllGetVersion )
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;

			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			hr = (*pDllGetVersion)(&dvi);

			if( SUCCEEDED( hr ) )
				dwVersion = WINDOWSVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
		}
		
		FreeLibrary( hinstDll );
	}
	return dwVersion;
}


