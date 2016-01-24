//-------------------------------------------------------------------//
// CompInfo.h
//
// The CCompInfo class provided here provides information about
// the program's heap.  It is for use on Windows NT/2000/XP.
//
// This wrapper also provides simplified versions of Win32 heap
// management functions.
//
// Originally posted on CodeGuru by
//		Yarantsau Andrei, QA
//		Minsk, Belarus
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef COMP_INFO_H_
#define COMP_INFO_H_

#include <windows.h>
#include <stdio.h>

#include <STLExtensions.h>


class CLogfile
{
public:
	CLogfile(char* filename) 
	{
		m_hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS , NULL, NULL );
		InitializeCriticalSection(&m_CriticalSection); 
	};
	~CLogfile() 
	{ 
		if (NULL != m_hFile)
			CloseHandle(m_hFile); 
		DeleteCriticalSection(&m_CriticalSection);
	};
	void print(LPCTSTR lpszFormat, ...)
	{
		EnterCriticalSection(&m_CriticalSection);

		va_list args;
		va_start(args, lpszFormat);

		int		nBuf;
		TCHAR	szBuffer[1024];
		DWORD dwOut;
		nBuf = wvsprintf(szBuffer, lpszFormat, args);
		WriteFile(m_hFile, (LPCVOID)szBuffer, lstrlen(szBuffer) , &dwOut , NULL);
		va_end(args);

		LeaveCriticalSection(&m_CriticalSection);
	};
public:
	HANDLE m_hFile;
	CRITICAL_SECTION m_CriticalSection; 

};


#define TIME_TEXT_SIZE				12

class CCompInfo
{
public:
	CCompInfo(char* filename);
	CCompInfo();
	~CCompInfo();
	void SetAnalyzeTime(UINT nTime);
	UINT GetCPUInfo();
	DWORD HeapMakeSnapShot();
	DWORD HeapCommitedBytes();
	DWORD HeapCompareSnapShots();
	void HeapStoreDumpToFile();
	void HeapCompareDumpWithFile(BOOL bShowContent);
	void HeapPrintDump(BOOL bShowContent);
	char* GetTimeString();

	ustring GetHeapAllocations( int nMinAllocSize = 0 );

private:
	void DoInit();
	void GetHeaps();
	void GetHeapWalk();
public:
	unsigned			m_nHeaps;
	HANDLE			m_aHeaps[ 256 ];
	DWORD				m_dwCurrentSize;
	DWORD				m_dwSnapShotSize;
	UINT				m_Sleep;
	CLogfile*		m_log;
	char				m_sTime[ TIME_TEXT_SIZE ];
};


unsigned int CompactHeap( int* pnHeapCount = 0 );

bool bSetLowFragmentationHeap();


#endif // COMP_INFO_H_