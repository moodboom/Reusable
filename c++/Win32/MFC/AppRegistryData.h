//-------------------------------------------------------------------//
// AppRegistryData.h interface													//
//-------------------------------------------------------------------//
// This class provides a basis for storage of an App's registry
// data.  All registry data types are handled in this class (CString,
// DWORD, and Binary).
//	
//	USAGE
// To use this class, include an object of this type in your 
// CWinApp-derived class.  The registry object will then be 
// available to any module through theApp, the global app pointer 
// in the default code built by AppWizard.
//
//	1) One methodology is to derive a new class from this one, (named
// MyRegData, for example) and then include an object of this 
// derived class in the project's CWinApp-derived class.
// The derived class should provide Get/Set() members for each data item.
// The member functions should use the generic functions in this
// base class to actually access the registry data.  See GetAppVersion() 
// for an example of a specialized function.
// This method typically results in cleaner-looking code, but ties your 
// classes together at a more-derived level, which is bad.  However,
// tasks like registry key management are easier because the key
// definitions are centralized.
//
//	EXAMPLE
//
//	CWinApp							AppRegistryData
//		MyApp		-contains->			MyRegData
//
// CString strUserName = theApp->RegData.GetUserName();
//
//
// 2) The alternative to this method is to create registry access functions
// in the classes that need the registry data.  You should not need
// to derive from AppRegistryData; just use its members directly.
// Registry code will become more scattered throughout your project, 
// but this is good, as classes will be more self-contained.  Note that 
// any class that needs registry data will still be tied to AppRegistryData,
// but at least this class is a generic "base-class" type.
//
//
// NOTE: MS registry terminology is as follows (don't blame me):
//
//		Key		Branch which may have a value and under which there may
//					be other branches and leaves.
//		Value		Leaf slot which contains one item of data.
//		Data		Contents of the leaf.
//
//
// Note that the following functions are available through the CWinApp object
// for access to single registry values located within "sections"
// under the application key.  They are not very useful.  You should use
// the helpers provided with this class instead.
//
//		UINT CWinApp::GetProfileInt(
//			LPCTSTR lpszSection, 
//			LPCTSTR lpszEntry,
//			int nDefault
//		);
//
//		CString CWinApp::GetProfileString(
//			LPCTSTR lpszSection, 
//			LPCTSTR lpszEntry,
//			LPCTSTR lpszDefault
//		);
//
//		BOOL CWinApp::GetProfileBinary(
//			LPCTSTR lpszSection, 
//			LPCTSTR lpszEntry,
//			BYTE** ppData, 
//			UINT* pBytes
//		);
//
//		BOOL CWinApp::WriteProfileInt(
//			LPCTSTR lpszSection, 
//			LPCTSTR lpszEntry,
//			int nValue
//		);
//
//		BOOL CWinApp::WriteProfileString(
//			LPCTSTR lpszSection, 
//			LPCTSTR lpszEntry,
//			LPCTSTR lpszValue
//		);
//
//		BOOL CWinApp::WriteProfileBinary(
//			LPCTSTR lpszSection, 
//			LPCTSTR lpszEntry,
//			LPBYTE pData, 
//			UINT nBytes
//		);
//
//
// In our functions, the word Registry was used instead of Profile.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef APP_REGISTRY_DATA_H
#define APP_REGISTRY_DATA_H

#include "StandardTypes.h"

class AppRegistryData
{

public:

	// Our constructor.
	AppRegistryData();

	bool GetAppPath( CString* pstrPath );
	bool SetAppPath( CString strPath );

	DWORD	GetAppVersion();
	bool SetAppVersion();

	CString GetAutoRunPath();
	bool SetAutoRunPath( CString* pstrPath );

	// This pseudo-astract function should be derived to provide the 
	// current version number of the program.
	virtual DWORD GetCurrentVersion() { ASSERT( false ); return 0; }

	// This function should be overridden if action is required
	// to update an older version to the current.
	virtual bool UpdateFromOldVersion(
		DWORD dwInstallVersion,
		DWORD dwThisVersion
	);

	// Derived classes can provide functions that Get/Set
	// specific strings or structures.  Those functions
	// can call these generic ones.
	// A Get/Set pair is provided for each type of registry data.
	// Note that most of these are thin wrappers for functions
	// in RegistryHelpers.h.  Therefore, they are inline, except 
	// for ones that use local string buffers.
	
	//------------------------------------------------------------//
	// Note that we are now moving towards greater local control
	// of registry data - use the generic functions whenever
	// possible.
	//------------------------------------------------------------//

	// If not found and default is supplied, it is added to the 
	// registry.  Otherwise this does nothing.
	// Uses a local buffer, don't make it inline.
	CString GetString	( 
		const TCHAR*	pszLabel,
		TCHAR*			pszDefault		= 0,
		bool*				pbFound			= 0	
	);

	bool SetString( 
		const TCHAR*	pszLabel,
		CString*			pstrAppString					
	);

	// This version sets a default if not found.
	DWORD GetDWord( 
		const TCHAR*	pszLabel,
		DWORD				dwDefault,
		bool*				pbFound			= 0	
	);

	// This version does nothing if not found.
	DWORD GetDWord( 
		const TCHAR*	pszLabel,
		bool*				pbFound			= 0	
	);

	bool SetDWord( 
		const TCHAR*	pszLabel,
		DWORD				dwAppNumber						
	);
	
	// If not found and default is supplied, it is added to the 
	// registry.  Otherwise this does nothing.
	void GetBinary(
		const TCHAR*	pszLabel,
		int				nSize,
		void*				pResult,
		void*				pDefault			= 0,
		bool*				pbFound			= 0	
	);
	
	bool SetBinary( 
		const TCHAR*	pszLabel,
		void*				pAppBinary,		
		int				nSize	
	);

	// Uses a local buffer, don't make it inline.
	void GetWinIniItem(
		TCHAR*	pSection,
		TCHAR*	pItemValue,
		CString* pstrResultData,
		TCHAR*	pDefaultData
	);

	// Use this function to set an alternative base key for registry
	// data access.  See the comments at the top of GetBaseKey()
	// for details.
	void SetBaseKey(
		HKEY		hBasePredefinedKey,
		CString	strCategoryKey,
		CString	strCompanyKey,
		CString	strProgramKey
	);
	
	// Use this function to reset registry data access to come from 
	// the standard MFC app base.  See GetBaseKey() for details.
	void ClearBaseKey();

protected:

	// This function returns the "base key".  For apps, this
	// is equivalent to the undocumented AfxGetApp()->GetAppRegistryKey()
	// function.  If you have called SetBaseKey(), that information is
	// used instead.
	HKEY	GetBaseKey();
	
	HKEY		m_hBasePredefinedKey;
	CString	m_strCategoryKey;
	CString	m_strCompanyKey;
	CString	m_strProgramKey;

};


//-------------------------------------------------------------------//
// Globals/Statics																	//
//-------------------------------------------------------------------//
extern AppRegistryData* g_pAppRegData;


/////////////////////////////////////////////////////////////////////////////

#endif	// APP_REGISTRY_DATA_H
