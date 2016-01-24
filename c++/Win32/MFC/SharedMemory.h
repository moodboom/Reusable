#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <afxole.h> 
#include <afxmt.h> 
 
// NOTE: This code is from MSJ, November 1998, Wicked Code.

class CSharedMemory 
{ 
public: 

	CSharedMemory(); 
	CSharedMemory( UINT nSize, LPCTSTR pszName = NULL ); 
	virtual ~CSharedMemory(); 
 
	bool Create(UINT nSize, LPCTSTR pszName = NULL); 
	bool Delete(); 
	
	CString GetName(); 
	UINT GetSize(); 

	bool Write(
		LPCVOID	pBuffer, 
		DWORD		dwBytesToWrite, 
		DWORD*	pdwBytesWritten, 
		DWORD		dwOffset
	); 
	
	bool Read(
		LPVOID	pBuffer, 
		DWORD		dwBytesToRead, 
		DWORD*	pdwBytesRead, 
		DWORD		dwOffset
	); 
	
	bool Lock(); 
	bool Unlock(); 

	bool bFirstInstantiation() 
	{ 
		ASSERT( m_hMapping != NULL ); 
		return m_bFirstInstantiation;
	} 
 
	void* p;			// Address of memory block 
 
protected: 

	// Memory block size 
	UINT			m_nSize;		
	
	// File mapping handle 
	HANDLE		m_hMapping;	
	
	// Object name 
	CString		m_strName;	
	
	// Pointer to mutex for synchronizing reads and writes.
	CMutex*		m_pMutex;	
	
	// TRUE if this is the first connection.
	bool			m_bFirstInstantiation;

	#ifdef _DEBUG 
		
		// Lock count 
		UINT			m_nLockCount;	

	#endif 
}; 

#endif // SHARED_MEMORY_H