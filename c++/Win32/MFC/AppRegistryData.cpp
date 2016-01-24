//-------------------------------------------------------------------//
// AppRegistryData Implementation
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "CRegistryHelpers.h"				// This does the work for us.
#include "AppRegistryData.h"


//-------------------------------------------------------------------//
// Globals/Statics																	//
//-------------------------------------------------------------------//
AppRegistryData* g_pAppRegData = 0;


//-------------------------------------------------------------------//
// REGISTRY KEY NAMES
//-------------------------------------------------------------------//
const TCHAR szAppPathKey[]			= _T("ProgramPath");
const TCHAR szAppVersionKey[]		= _T("ProgramVersion");
const TCHAR szAutoRunPathKey[]	= _T("AutoRunPath");
//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
// construction																		//
//-------------------------------------------------------------------//
AppRegistryData::AppRegistryData()
{
	// Assume we are using the standard MFC app base key, until
	// we are told otherwise.
	ClearBaseKey();

	// There should only be one of these, and we should 
	// only hit this once!
	ASSERT( g_pAppRegData == 0 );

	// Set the one and only global pointer.
	g_pAppRegData = this;
}


//-------------------------------------------------------------------//
// GetAppPath()																		//
//-------------------------------------------------------------------//
// This function retrieves the program path as set by the
// program's installation wizard.  If the value is not
// found, we can assume that the program has not been installed,
// and attempt a reinstallation.
// 
// Note that most registry "Get" functions will not need to check
// the bool.  This function is a special case, as we use it to
// determine the installation status.  
//
// Typical "Get" function:
//
//		CString GetNameOfMyMonkey();
//
//-------------------------------------------------------------------//
bool AppRegistryData::GetAppPath( CString* pstrPath )
{

	bool bFound;
	*pstrPath = GetString(
		szAppPathKey,
		0,
		&bFound
	);
	return bFound;

}


//-------------------------------------------------------------------//
// SetAppPath()																		//
//-------------------------------------------------------------------//
// This sets the app's installed path.  It should only be called
// from within an installation wizard, etc.
//-------------------------------------------------------------------//
bool AppRegistryData::SetAppPath( CString strPath ) {

	return SetString(
		szAppPathKey,
		&strPath
	);

}


//-------------------------------------------------------------------//
// GetAppVersion()																	//
//-------------------------------------------------------------------//
// This function gets the program version as set by the program's
//	installation wizard.
// Note that if the value is not found, this function does nothing
// and returns 0.
//-------------------------------------------------------------------//
DWORD AppRegistryData::GetAppVersion()
{
	return GetDWord( szAppVersionKey );
}


//-------------------------------------------------------------------//
// SetAppVersion()																	//
//-------------------------------------------------------------------//
// This function is called to set the installed program version
// number in the registry to the current Exe's version number.
// It should only be called from within an installation wizard, etc.
//-------------------------------------------------------------------//
bool AppRegistryData::SetAppVersion()
{
	return SetDWord( 
		szAppVersionKey,
		GetCurrentVersion() 
	);
}


//-------------------------------------------------------------------//
// GetAutoRunPath()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
CString AppRegistryData::GetAutoRunPath()
{
	return GetString( szAutoRunPathKey );
}


//-------------------------------------------------------------------//
// SetAutoRunPath()																	//
//-------------------------------------------------------------------//
// This function is used to hold the path to the version of the
// program that was first run (typically from an AutoRun).  Then,
// the installed version of the program can get this path and
// do what it needs to with it (e.g., find data).
//-------------------------------------------------------------------//
bool AppRegistryData::SetAutoRunPath( CString* pstrPath ) {

	return SetString(
		szAutoRunPathKey,
		pstrPath
	);

}


//-------------------------------------------------------------------//
// UpdateFromOldVersion()															//
//-------------------------------------------------------------------//
// This function should be overridden if action is required
// to update an older version to the current.  All older
// versions should be handled by the function.  Note that
// the actions may involve the registry or anything to do
// with the program or its data, since the derived class
// will be program-specific.
//
// The base class updates the registry to the current version 
// number.  It should typically be called AFTER the derived
// class has successfully updated things.
//-------------------------------------------------------------------//
bool AppRegistryData::UpdateFromOldVersion(
	DWORD dwInstallVersion,
	DWORD dwThisVersion
) {

	return SetAppVersion();

}


//-------------------------------------------------------------------//
// SetBaseKey()																		//
//-------------------------------------------------------------------//
// Call this function to assign an alternative base key for future
// reg data access.  Without calling this function, or after calling
// ClearBaseKey(), the standard MFC app base key will be used.
// See GetBaseKey() for details.
//-------------------------------------------------------------------//
void AppRegistryData::SetBaseKey(
	HKEY		hBasePredefinedKey,
	CString	strCategoryKey,
	CString	strCompanyKey,
	CString	strProgramKey
) {
	m_hBasePredefinedKey	= hBasePredefinedKey;
	m_strCategoryKey		= strCategoryKey;
	m_strCompanyKey		= strCompanyKey;
	m_strProgramKey		= strProgramKey;
}


//-------------------------------------------------------------------//
// ClearBaseKey()																		//
//-------------------------------------------------------------------//
// This clears the base key, so that future reg data access will
// attempt to get the standard MFC app base key.
// See GetBaseKey() for details.
//-------------------------------------------------------------------//
void AppRegistryData::ClearBaseKey() 
{
	m_hBasePredefinedKey	= 0;
	m_strCompanyKey		= _T("");
	m_strProgramKey		= _T("");
}


//-------------------------------------------------------------------//
// GetBaseKey()																		//
//-------------------------------------------------------------------//
// Here, we get the "base key".  The base key is either the standard
// base MFC app key, used when this class is associated with a std
// MFC app derived from BaseApp, or the key specified in a previous 
// call to SetBaseKey(), which allows us to access registry data
// under any specified key.
//
// For apps, we call 
//
//		AfxGetApp()->GetAppRegistryKey();
//
// This returns the key for:
//
//		HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
//												(Company)	(AppName)
//
// creating it if it doesn't exist.
//
// If you have called SetBaseKey() then we use the following:
//
//		m_hBaseKey\m_strCategoryKey\m_strCompanyKey\m_strProgramKey
//
// to grab the base key.
//
// NOTE: It is the responsibility of the caller to call 
// RegCloseKey() on the returned HKEY.
//-------------------------------------------------------------------//
HKEY AppRegistryData::GetBaseKey()
{
	if ( 
			m_strCategoryKey.IsEmpty() 
		||	m_strCompanyKey.IsEmpty() 
		||	m_strProgramKey.IsEmpty() 
		|| m_hBasePredefinedKey == 0 
	) {
		// This is an undocumented MFC function that returns
		//
		//		HKEY_CURRENT_USER\"Software"\<CompanyName>\<SoftwareProductName>
		//
		// You MUST have previously called CWinApp::SetRegistryKey()
		// to set the "company" name.  The "software product" name is copied
		// from CWinApp::m_pszAppName.
		return AfxGetApp()->GetAppRegistryKey();
	
	} else 
	{
		// NOTE: This is basically a modified GetAppRegistryKey().
		
		HKEY hAppKey = NULL;
		HKEY hSoftKey = NULL;
		HKEY hCompanyKey = NULL;
		if (
			RegOpenKeyEx(
				HKEY_CURRENT_USER, 
				LPCTSTR( m_strCategoryKey ), 
				0, 
				KEY_WRITE|KEY_READ,
				&hSoftKey
			) == ERROR_SUCCESS
		) {

			DWORD dw;
			if (
				RegCreateKeyEx(
					hSoftKey, 
					LPCTSTR( m_strCompanyKey ), 
					0, 
					REG_NONE,
					REG_OPTION_NON_VOLATILE, 
					KEY_WRITE|KEY_READ, 
					NULL,
					&hCompanyKey, 
					&dw
				) == ERROR_SUCCESS 
			) {

				RegCreateKeyEx(
					hCompanyKey, 
					LPCTSTR( m_strProgramKey ), 
					0, 
					REG_NONE,
					REG_OPTION_NON_VOLATILE, 
					KEY_WRITE|KEY_READ, 
					NULL,
					&hAppKey, 
					&dw
				);
			}
		}
		
		if ( hSoftKey != NULL )
			RegCloseKey( hSoftKey );
		
		if ( hCompanyKey != NULL )
			RegCloseKey( hCompanyKey );

		return hAppKey;

	}

}



//-------------------------------------------------------------------//
// BASIC DATA TYPE HANDLING
//-------------------------------------------------------------------//

//-------------------------------------------------------------------//
// GetString()																			//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
CString	AppRegistryData::GetString( 
	const TCHAR*	pszLabel,
	TCHAR*			pszDefault,
	bool*				pbFound
) {
	
	HKEY hAppKey = GetBaseKey();

	CString strResult = WindowsRegistry_GetCString( 
		hAppKey,
		pszLabel,
		pszDefault,
		pbFound
	);
	
	// Clean up.
	VERIFY( RegCloseKey( hAppKey ) == ERROR_SUCCESS );

	return strResult;

}


//-------------------------------------------------------------------//
// SetString()																			//
//-------------------------------------------------------------------//
// This function sets the given data into the given value.
// It returns false if we don't succeed for any reason.
// It is an improvement over WriteProfileString in that it does 
// not require a "section", and uses CStrings.
//-------------------------------------------------------------------//
bool AppRegistryData::SetString( 
	const TCHAR*	pszLabel,
	CString*			pstrAppString					
) {
	HKEY hAppKey = GetBaseKey();

	bool bReturn = WindowsRegistry_SetString( 
		hAppKey,
		pszLabel,
		*pstrAppString
	);

	// Clean up.
	VERIFY( RegCloseKey( hAppKey ) == ERROR_SUCCESS );
	
	return bReturn;
}

	
//-------------------------------------------------------------------//
// GetDWord()																			//
//-------------------------------------------------------------------//
// This version sets a default if not found.
//-------------------------------------------------------------------//
DWORD AppRegistryData::GetDWord( 
	const TCHAR*	pszLabel,
	DWORD				dwDefault,
	bool*				pbFound
) {
	HKEY hAppKey = GetBaseKey();

	DWORD dwReturn = WindowsRegistry_GetDWord( 
		hAppKey,
		pszLabel,
		dwDefault,
		pbFound
	);

	// Clean up.
	VERIFY( RegCloseKey( hAppKey ) == ERROR_SUCCESS );

	return dwReturn;
}


//-------------------------------------------------------------------//
// GetDWord()																			//
//-------------------------------------------------------------------//
// This version does nothing if not found.
//-------------------------------------------------------------------//
DWORD AppRegistryData::GetDWord( 
	const TCHAR*	pszLabel,
	bool*				pbFound
) {
	HKEY hAppKey = GetBaseKey();

	DWORD dwReturn = WindowsRegistry_GetDWord( 
		hAppKey,
		pszLabel,
		pbFound
	);

	// Clean up.
	VERIFY( RegCloseKey( hAppKey ) == ERROR_SUCCESS );

	return dwReturn;
}


//-------------------------------------------------------------------//
// SetDWord()																			//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
bool AppRegistryData::SetDWord( 
	const TCHAR*	pszLabel,
	DWORD				dwAppNumber						
) {
	HKEY hAppKey = GetBaseKey();

	bool bReturn = WindowsRegistry_SetDWord( 
		hAppKey,
		pszLabel,
		dwAppNumber						
	);

	// Clean up.
	VERIFY( RegCloseKey( hAppKey ) == ERROR_SUCCESS );

	return bReturn;
}
	

//-------------------------------------------------------------------//
// GetBinary()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void AppRegistryData::GetBinary(
	const TCHAR*	pszLabel,
	int				nSize,
	void*				pResult,
	void*				pDefault,
	bool*				pbFound
) {
	HKEY hAppKey = GetBaseKey();

	WindowsRegistry_GetBinary(
		hAppKey,
		pszLabel,
		nSize,
		pResult,
		pDefault,
		pbFound
	);

	// Clean up.
	VERIFY( RegCloseKey( hAppKey ) == ERROR_SUCCESS );

}


//-------------------------------------------------------------------//
// SetBinary()																			//
//-------------------------------------------------------------------//
// This function sets the given data into the given value.
// It returns false if we don't succeed for any reason.
// It is an improvement over WriteProfileString in that it does 
// not require a "section", and uses CStrings.
//-------------------------------------------------------------------//
bool AppRegistryData::SetBinary( 
	const TCHAR*	pszLabel,
	void*				pBinary,
	int				nSize	
) {
	HKEY hAppKey = GetBaseKey();

	bool bReturn = WindowsRegistry_SetBinary( 
		hAppKey,
		pszLabel,
		pBinary,		
		nSize	
	);

	// Clean up.
	VERIFY( RegCloseKey( hAppKey ) == ERROR_SUCCESS );

	return bReturn;
}


//-------------------------------------------------------------------//
// GetWinIniItem()																	//
//-------------------------------------------------------------------//
// This function retreives data in the WIN.INI file.
// Note that some configuration info (such as fonts, ports, and
// regional settings) is still stored in WIN.INI, for
// "compatibility reasons".  See "WIN.INI Settings in the Registry".
// We use GetProfileString to get at WIN.INI (even though it is 
// documented as obsolete).
//
// The advantage of this function over GetProfileString is that it
// returns a CString (and is not slandered with the "obsolete" tag :> ).
//
// Note that GetProfileInt is available for extracting integer items
// from WIN.INI.
//-------------------------------------------------------------------//
void AppRegistryData::GetWinIniItem(
	TCHAR*	pSection,
	TCHAR*	pItemValue,
	CString* pstrResultData,
	TCHAR*	pDefaultData
) {

	TCHAR		Buffer[_MAX_PATH * 2];
	GetProfileString( 
		pSection,
		pItemValue,
		pDefaultData,
		Buffer,
		_MAX_PATH * 2
	);
	
	*pstrResultData = Buffer;
	
}


