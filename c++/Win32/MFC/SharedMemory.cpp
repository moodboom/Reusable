#include <stdafx.h> 

#include "SharedMemory.h" 

// NOTE: This code was originally taken from MSJ, November 1998, Wicked Code.
 

//-------------------------------------------------------------------//
// CSharedMemory()																	//
//-------------------------------------------------------------------//
CSharedMemory::CSharedMemory() 
{ 
	m_nSize = 0; 
	m_pMutex = NULL; 
	m_hMapping = NULL; 
	m_bFirstInstantiation = false; 
	p = NULL; 
 
	#ifdef _DEBUG 
		m_nLockCount = 0; 
	#endif 
} 
 

//-------------------------------------------------------------------//
// CSharedMemory()																	//
//-------------------------------------------------------------------//
CSharedMemory::CSharedMemory( UINT nSize, LPCTSTR pszName ) 
{ 
	m_nSize = 0; 
	m_pMutex = NULL; 
	m_hMapping = NULL; 
	m_bFirstInstantiation = false; 
	p = NULL; 
 
	#ifdef _DEBUG 
		m_nLockCount = 0; 
	#endif 
 
	if (!Create (nSize, pszName)) 
		AfxThrowMemoryException (); 
} 
 

//-------------------------------------------------------------------//
// ~CSharedMemory()																	//
//-------------------------------------------------------------------//
CSharedMemory::~CSharedMemory() 
{ 
	Delete (); 
} 
 

//-------------------------------------------------------------------//
// Create()																				//
//-------------------------------------------------------------------//
bool CSharedMemory::Create( UINT nSize, LPCTSTR pszName ) 
{ 
	// 
	// If the following ASSERT fires, you're calling Create on a shared 
	// memory object that is already initialized. If you're going to do 
	// that, you must call Delete first. 
	// 
	ASSERT (m_hMapping == NULL); 
 
	// 
	// If pszName is NULL, manufacture an object name. 
	// 
	if (pszName == NULL) { 
		GUID guid; 
		::CoCreateGuid (&guid); 
 
		WCHAR wszName[64]; 
		::StringFromGUID2 (guid, wszName, sizeof (wszName) / sizeof (WCHAR)); 
 
		TCHAR szName[64]; 

		#ifdef UNICODE 
				::lstrcpy (szName, wszName); 
		#else 
				int nCount = ::WideCharToMultiByte (CP_ACP, 0, wszName, -1, 
					szName, sizeof (szName) / sizeof (TCHAR), NULL, NULL); 
				ASSERT (nCount = sizeof (szName) / sizeof (TCHAR)); 
 
		#endif 

		pszName = szName; 
	} 
 
	// 
	// Create a file mapping object. 
	// 
	HANDLE hMapping = ::CreateFileMapping ( 
		(HANDLE) 0xFFFFFFFF, 
		NULL, 
		PAGE_READWRITE, 
		0, 
		nSize, 
		pszName 
	); 
 
	DWORD dwResult = ::GetLastError (); 
	m_bFirstInstantiation = (dwResult == ERROR_ALREADY_EXISTS) ? false : true; 
 
	ASSERT (hMapping != NULL); 
	ASSERT (hMapping != INVALID_HANDLE_VALUE); 
 
	if (hMapping == NULL || hMapping == INVALID_HANDLE_VALUE) 
		return false;
 
	// 
	// Convert the handle into a pointer. 
	// 
	void* pVoid = ::MapViewOfFile ( 
		hMapping, 
		FILE_MAP_ALL_ACCESS, 
		0, 
		0, 
		0 
	); 
 
	ASSERT (pVoid != NULL); 
 
	if (pVoid == NULL) 
	{ 
		::CloseHandle (hMapping); 
		return false; 
	} 
 
	// 
	// Create (or connect to) a shared mutex for locking and unlocking. 
	// 
	CMutex* pMutex = new CMutex 
	( 
		TRUE,											// Initially owned 

		////////////////////////////////////////////////////////////////
		// MDM	7/25/2000 12:15:09 PM
		// This was updated to use the provided name, which should
		// be unique to the usage of the shared memory, instead of
		// a generic name.  Wouldn't the generic name collide with
		// two simultaneous CSharedMemory objects???
		// _T ("2A4F4F41-2FC1-11D2-8E53-006008A82731")		// Mutex name 
		// pszName 
		//
		// MDM	7/26/2000 6:19:29 PM
		// Using pszName fails.  Why??  Is the name stored in the
		// same array as names used by CreateFileMapping()??  Let's
		// try munging it up a bit here.
		LPCTSTR( CString( pszName ) + _T("ShMem") )
		////////////////////////////////////////////////////////////////

	); 
 
	ASSERT ((HANDLE) *pMutex != NULL); 
 
	if ((HANDLE) *pMutex == NULL) 
	{ 
		::UnmapViewOfFile (pVoid); 
		::CloseHandle (hMapping); 
		return false; 
	} 
 
	// Store pertinent data for later. 
	m_nSize = nSize; 
	m_strName = pszName; 
	m_pMutex = pMutex; 
	m_hMapping = hMapping; 
	p = pVoid; 
 
	// Unlock the mutex and return. 
	m_pMutex->Unlock(); 
	return true; 
} 
 

//-------------------------------------------------------------------//
// Delete()																				//
//-------------------------------------------------------------------//
bool CSharedMemory::Delete() 
{ 
	// If the following ASSERT fires, you're calling Delete on a shared 
	// memory object that has not been initialized. 
	ASSERT (m_hMapping != NULL); 
 
	// If either of the following ASSERTs fires, you're changing the 
	// value of p. Don't do that! 
	ASSERT( p != NULL ); 
	ASSERT( AfxIsValidAddress( p, m_nSize, FALSE ) ); 
 
	// This shouldn't happen, but let's check anyway. 
	ASSERT( m_pMutex != NULL ); 
 
	// Clean up. 
	m_nSize = 0; 
	m_strName = _T (""); 
 
	delete m_pMutex; 
	m_pMutex = NULL; 
	::UnmapViewOfFile (p); 
	p = NULL; 
	::CloseHandle (m_hMapping); 
	m_hMapping = NULL; 
	m_bFirstInstantiation = false; 
 
	#ifdef _DEBUG 
		if (m_nLockCount != 0) 
			TRACE0 (_T ("Warning: Deleting shared memory object with a nonzero lock count\n")); 
		m_nLockCount = 0; 
	#endif 
 
	return true;
} 
 

//-------------------------------------------------------------------//
// GetName()																			//
//-------------------------------------------------------------------//
CString CSharedMemory::GetName() 
{ 
	ASSERT (m_hMapping != NULL); 
	return m_strName; 
} 
 

//-------------------------------------------------------------------//
// GetSize()																			//
//-------------------------------------------------------------------//
UINT CSharedMemory::GetSize() 
{ 
	ASSERT (m_hMapping != NULL); 
	return m_nSize; 
} 
 

//-------------------------------------------------------------------//
// Write()																				//
//-------------------------------------------------------------------//
bool CSharedMemory::Write(
	LPCVOID	pBuffer, 
	DWORD		dwBytesToWrite, 
	DWORD*	pdwBytesWritten, 
	DWORD		dwOffset
) { 
	ASSERT (m_hMapping != NULL); 
	ASSERT (AfxIsValidAddress (p, m_nSize, TRUE)); 
 
	if (dwOffset >= m_nSize) { 
		*pdwBytesWritten = 0; 
		return false; 
	} 
 
	DWORD dwCount = __min( dwBytesToWrite, m_nSize - dwOffset ); 
	::CopyMemory ((LPBYTE) p + dwOffset, pBuffer, dwCount); 
 
	if (pdwBytesWritten != NULL) 
		*pdwBytesWritten = dwCount; 
 
	return true; 
} 
 

//-------------------------------------------------------------------//
// Read()																				//
//-------------------------------------------------------------------//
bool CSharedMemory::Read(
	LPVOID	pBuffer, 
	DWORD		dwBytesToRead, 
	DWORD*	pdwBytesRead, 
	DWORD		dwOffset
) { 
	ASSERT( m_hMapping != NULL); 
	ASSERT( AfxIsValidAddress (p, m_nSize, FALSE ) ); 
 
	if (dwOffset >= m_nSize) { 
		*pdwBytesRead = 0; 
		return false;
	} 
 
	DWORD dwCount = __min( dwBytesToRead, m_nSize - dwOffset );
	::CopyMemory (pBuffer, (LPBYTE) p + dwOffset, dwCount); 
 
	if (pdwBytesRead != NULL) 
		*pdwBytesRead = dwCount; 
 
	return true; 
} 
 

//-------------------------------------------------------------------//
// Lock()																				//
//-------------------------------------------------------------------//
// Be careful, this sucker will hang on until we can lock the
// memory!
//-------------------------------------------------------------------//
bool CSharedMemory::Lock() 
{ 
	ASSERT (m_hMapping != NULL); 
	m_pMutex->Lock(); 

	#ifdef _DEBUG 
		m_nLockCount++; 
	#endif 

	return true;
} 
 

//-------------------------------------------------------------------//
// Unlock()																				//
//-------------------------------------------------------------------//
bool CSharedMemory::Unlock() 
{ 
	ASSERT (m_hMapping != NULL); 
	m_pMutex->Unlock (); 

	#ifdef _DEBUG 
		m_nLockCount--; 
	#endif 

	return true; 
} 
