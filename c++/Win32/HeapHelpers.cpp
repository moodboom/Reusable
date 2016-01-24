//-------------------------------------------------------------------//
// HeapHelpers.cpp 
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "HeapHelpers.h"

#include <conio.h>


#define SystemBasicInformation       0
#define SystemPerformanceInformation 2
#define SystemTimeInformation        3
#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
    DWORD   dwUnknown1;
    ULONG   uKeMaximumIncrement;
    ULONG   uPageSize;
    ULONG   uMmNumberOfPhysicalPages;
    ULONG   uMmLowestPhysicalPage;
    ULONG   uMmHighestPhysicalPage;
    ULONG   uAllocationGranularity;
    PVOID   pLowestUserAddress;
    PVOID   pMmHighestUserAddress;
    ULONG   uKeActiveProcessors;
    BYTE    bKeNumberProcessors;
    BYTE    bUnknown2;
    WORD    wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
    LARGE_INTEGER   liIdleTime;
    DWORD           dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
    LARGE_INTEGER liKeBootTime;
    LARGE_INTEGER liKeSystemTime;
    LARGE_INTEGER liExpTimeZoneBias;
    ULONG         uCurrentTimeZoneId;
    DWORD         dwReserved;
} SYSTEM_TIME_INFORMATION;


// ntdll!NtQuerySystemInformation (NT specific!)
//
// The function copies the system information of the
// specified type into a buffer
//
// NTSYSAPI
// NTSTATUS
// NTAPI
// NtQuerySystemInformation(
//    IN UINT SystemInformationClass,    // information type
//    OUT PVOID SystemInformation,       // pointer to buffer
//    IN ULONG SystemInformationLength,  // buffer size in bytes
//    OUT PULONG ReturnLength OPTIONAL   // pointer to a 32-bit
//                                       // variable that receives
//                                       // the number of bytes
//                                       // written to the buffer 
// );
typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);

PROCNTQSI NtQuerySystemInformation;

#define HEAP_DUMP_FILENAME _T("heap.out")




CCompInfo::CCompInfo(TCHAR* filename) 
{ 
	DoInit();
	m_log = new CLogfile(filename);
};

CCompInfo::CCompInfo() 
{ 
	DoInit();
};
	
CCompInfo::~CCompInfo() 
{
	if (NULL != m_log)
		delete m_log;
	
	DeleteFile(HEAP_DUMP_FILENAME);
};

void CCompInfo::DoInit()
{
	//Sleep for getting results
	m_Sleep = 1000;
	//Switch Crt_heap to Process_heap
	//Variable:__MSVCRT_HEAP_SELECT
	//Value:__GLOBAL_HEAP_SELECTED,1
	if (
		!SetEnvironmentVariable( 
			_T("__MSVCRT_HEAP_SELECT"),
			_T("__GLOBAL_HEAP_SELECTED,1")
		)
	)
		return;

	m_dwCurrentSize = 0;
	m_dwSnapShotSize = 0;	
	m_log = NULL;
}

void CCompInfo::SetAnalyzeTime(UINT nTime) 
{ 
	if (nTime > 0) 
		m_Sleep = nTime; 
};

UINT CCompInfo::GetCPUInfo()
{
    SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
    SYSTEM_TIME_INFORMATION        SysTimeInfo;
    SYSTEM_BASIC_INFORMATION       SysBaseInfo;
    double                         dbIdleTime;
    double                         dbSystemTime;
    LONG                           status;
    LARGE_INTEGER                  liOldIdleTime = {0,0};
    LARGE_INTEGER                  liOldSystemTime = {0,0};

    NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
		GetModuleHandle( _T("ntdll") ),
		"NtQuerySystemInformation"
	);

    if (!NtQuerySystemInformation)
        return 0;

    // get number of processors in the system
    status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL);
    if (status != NO_ERROR)
        return 0;
    
    while(1)
    {
        // get new system time
	    status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
        if (status!=NO_ERROR)
            return 0;

        // get new CPU's idle time
        status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
        if (status != NO_ERROR)
            return 0;

        // if it's a first call - skip it
       if (liOldIdleTime.QuadPart != 0)
       {
            // CurrentValue = NewValue - OldValue
            dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
            dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

            // CurrentCpuIdle = IdleTime / SystemTime
            dbIdleTime = dbIdleTime / dbSystemTime;

            // CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
            dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;

			return (UINT)dbIdleTime;
       }

        // store new CPU's idle and system time
        liOldIdleTime = SysPerfInfo.liIdleTime;
        liOldSystemTime = SysTimeInfo.liKeSystemTime;
		
        // wait one second
        Sleep(m_Sleep);
    }
}

void CCompInfo::GetHeaps()
{
		memset(m_aHeaps,0,sizeof(m_aHeaps));
		m_nHeaps = GetProcessHeaps(sizeof(m_aHeaps)/sizeof(m_aHeaps[0]),m_aHeaps);
};

void CCompInfo::GetHeapWalk() 
{
		PROCESS_HEAP_ENTRY pEntry;
		m_dwCurrentSize = 0;

		GetHeaps();

		for ( unsigned i=0; i < m_nHeaps; i++ )
		{
			pEntry.lpData = NULL;
			while (HeapWalk(m_aHeaps[i], &pEntry ))
			{
			if ((PROCESS_HEAP_ENTRY_BUSY & pEntry.wFlags) == PROCESS_HEAP_ENTRY_BUSY)
				m_dwCurrentSize += pEntry.cbData;
			}
		}
};

DWORD CCompInfo::HeapMakeSnapShot()
{
		GetHeapWalk();
		m_dwSnapShotSize = m_dwCurrentSize;
		return m_dwSnapShotSize;
};

DWORD CCompInfo::HeapCommitedBytes()
{
		GetHeapWalk();
		return m_dwCurrentSize;
}

DWORD CCompInfo::HeapCompareSnapShots()
{
		GetHeapWalk();
		return (m_dwCurrentSize - m_dwSnapShotSize);
}

void CCompInfo::HeapStoreDumpToFile()
{
	PROCESS_HEAP_ENTRY pEntry;
	m_dwCurrentSize = 0;
	HANDLE hFile;
	DWORD dwIn[2], dwOut;

	GetHeaps();
	
	hFile = CreateFile(HEAP_DUMP_FILENAME, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS , NULL, NULL );

	if ( INVALID_HANDLE_VALUE == hFile ) return;

	for ( unsigned i=0; i < m_nHeaps; i++ )
	{
		pEntry.lpData = NULL;
		while (HeapWalk(m_aHeaps[i], &pEntry ))
		{
			if ((PROCESS_HEAP_ENTRY_BUSY & pEntry.wFlags) == PROCESS_HEAP_ENTRY_BUSY)
			{
				dwIn[0] = (DWORD)pEntry.lpData;
				dwIn[1] = (DWORD)pEntry.cbData;
				WriteFile(hFile, &dwIn, sizeof(DWORD)*2 , &dwOut , NULL);
			}
		}
	}

	CloseHandle(hFile);
}

void CCompInfo::HeapCompareDumpWithFile(BOOL bShowContent)
{
	PROCESS_HEAP_ENTRY pEntry;
	m_dwCurrentSize = 0;
	BOOL bIsPresent = FALSE, bResult = FALSE;
	HANDLE hFile;
	DWORD dwPlace = 0, dwSize = 0;
	DWORD dwIn[2], dwOut;

	GetHeaps();
	
	hFile = CreateFile(HEAP_DUMP_FILENAME, GENERIC_READ, 0, NULL, OPEN_EXISTING , FILE_ATTRIBUTE_READONLY , NULL );
	if ( INVALID_HANDLE_VALUE == hFile ) return;

	if (NULL != m_log)
	{
		if (bShowContent)
			m_log->print( _T("\n\n ----- E X T E N D E D  H E A P  D U M P ----- \n\n") );
		else
			m_log->print( _T("\n\n ----- B A S I C  H E A P  D U M P ----- \n\n") );
	}

	for ( unsigned i=0; i < m_nHeaps; i++ )
	{
		pEntry.lpData = NULL;
		while (HeapWalk(m_aHeaps[i], &pEntry ))
		{
			if ((PROCESS_HEAP_ENTRY_BUSY & pEntry.wFlags) == PROCESS_HEAP_ENTRY_BUSY)
			{
				SetFilePointer (hFile, NULL , NULL, FILE_BEGIN) ; 
				bIsPresent = FALSE;
				memset( dwIn , 0 , sizeof(DWORD)*2 );
				dwOut = 0;
				while( 1 )
				{      
					bResult = ReadFile(hFile, &dwIn, sizeof(DWORD)*2 , &dwOut, NULL );
					if (bResult && dwOut == 0)  break;
					if ( dwIn[0] == (DWORD)pEntry.lpData && dwIn[1] == (DWORD)pEntry.cbData )
					{
						bIsPresent = TRUE;
						break;
					}
							
				}
				if (!bIsPresent)
				{
					if (NULL != m_log)
						m_log->print( _T("block at [0x%x] size [%d]\n") , pEntry.lpData , pEntry.cbData );

					if (bShowContent)
					{
						TCHAR* pData = (TCHAR *)pEntry.lpData;
						for ( unsigned k = 0 ; k < pEntry.cbData; k++ )
						{
							if ( NULL != m_log )
								m_log->print( _T("%c") , pData[k] );
						}
						if ( NULL != m_log )
							m_log->print( _T("\n\n") );
					}
				}

			}
		}
	}

	CloseHandle(hFile);
}

void CCompInfo::HeapPrintDump(BOOL bShowContent)
{
	PROCESS_HEAP_ENTRY pEntry;
	m_dwCurrentSize = 0;

	GetHeaps();

	if ( NULL != m_log )
	{
		if (bShowContent)
			m_log->print( _T("--------- Extended heap dump ---------\n") );
		else
			m_log->print( _T("--------- Basic heap dump ---------\n") );
	}

	for ( unsigned i=0; i < m_nHeaps; i++ )
	{
		pEntry.lpData = NULL;
		unsigned j = 0;
		while (HeapWalk(m_aHeaps[i], &pEntry ))
		{
		if ((PROCESS_HEAP_ENTRY_BUSY & pEntry.wFlags) == PROCESS_HEAP_ENTRY_BUSY)
		{
			if ( NULL != m_log )
					m_log->print( _T("heap[%d] block[%d] at [0x%x] size[%d]\n"), i , j , pEntry.lpData , pEntry.cbData );
			if (bShowContent)
			{
				TCHAR* pData = (TCHAR *)pEntry.lpData;
				for ( unsigned k = 0 ; k < pEntry.cbData; k++ )
				{
					if ( NULL != m_log )
						m_log->print( _T("%c") , pData[k] );
				}
				if ( NULL != m_log )
					m_log->print( _T("\n\n") );
			}
		}
		j++;
		}
	}
	if ( NULL != m_log )
		m_log->print( _T("\n------------------------------------\n") );
}

TCHAR* CCompInfo::GetTimeString()
{
	SYSTEMTIME  sysTime;
	GetLocalTime(&sysTime);
	memset(m_sTime, 0, TIME_TEXT_SIZE );
	_stprintf(
		m_sTime,
		_T("%02d:%02d:%02d%s"),
		(sysTime.wHour == 0 ? 12 :
		(sysTime.wHour <= 12 ? sysTime.wHour : sysTime.wHour -12)),
		sysTime.wMinute,
		sysTime.wSecond,
		(sysTime.wHour < 12 ? _T("AM"):_T("PM"))
	);
	return m_sTime;
}


ustring CCompInfo::GetHeapAllocations( int nMinAllocSize )
{
	ustring strResult;
	ustring strLine;
	
	PROCESS_HEAP_ENTRY pEntry;
	m_dwCurrentSize = 0;

	GetHeaps();

	if ( nMinAllocSize > 0 )
		strResult.printf( 1000, _T("Allocations > %d MB:\r\n"), nMinAllocSize / 1024 / 1024 );
	else
		strResult += _T("Allocations:\r\n");

	strResult += _T("-------------------------\r\n");

	int nAllocationCount = 0;
	for ( unsigned i=0; i < m_nHeaps; i++ )
	{
		pEntry.lpData = NULL;
		while (HeapWalk(m_aHeaps[i], &pEntry ))
		{
			if ( (PROCESS_HEAP_ENTRY_BUSY & pEntry.wFlags) == PROCESS_HEAP_ENTRY_BUSY )
			{
				if ( pEntry.cbData >= nMinAllocSize )
				{
					strLine.printf( 1000, _T("%8d MB at 0x%08X\r\n"), (int)pEntry.cbData / 1024 / 1024, (int)( pEntry.lpData ) );
					strResult += strLine;
				}
				m_dwCurrentSize += pEntry.cbData;
				nAllocationCount++;
			}
		}
	}

	strResult += _T("-------------------------\r\n");
	strLine.printf( 1000, _T("%8d MB total in %d allocations\r\n"), m_dwCurrentSize / 1024 / 1024, nAllocationCount );
	strResult += strLine;
	
	return strResult;
}


//-------------------------------------------------------------------//
// CompactHeap()																		//
//-------------------------------------------------------------------//
// This function compacts the application's heap(s) and returns
// the maximum block size available after compaction.
//
// Not sure which of the heaps we encounter is the "default" used by
// the program.  We compact them all.
//
// If you want a heap count, provide the address of the integer
// to set.
//
// NOTE: we only allow 25 heaps max here.
//-------------------------------------------------------------------//
unsigned int CompactHeap( int* pnHeapCount )
{
	// This does ALL heaps.

	unsigned int unLargestBlock = 0;
	
	// How many heaps do we have?
	DWORD dwHeaps = GetProcessHeaps( 0, 0 );

	HANDLE* phHeaps = new HANDLE[ dwHeaps ];
	
	DWORD dwActualHeaps = GetProcessHeaps( dwHeaps, phHeaps );
	ASSERT( dwActualHeaps == dwHeaps );
	if ( dwActualHeaps == dwHeaps )
	{
		for ( int nA = 0; nA < dwHeaps; nA++ )
		{
			unLargestBlock = std::max( unLargestBlock, (unsigned int)HeapCompact( phHeaps[nA], 0 ) );
		}
	}

	if ( pnHeapCount != 0 )
		*pnHeapCount = dwHeaps;

	delete[] phHeaps;

	return unLargestBlock;

	/*
	// This does the default heap (the CRT heap? not according to 
	// M$DN support...).
	return HeapCompact( GetProcessHeap(), 0 );
	*/
}


//-------------------------------------------------------------------//
// bSetLowFragmentationHeap()														//
//-------------------------------------------------------------------//
// This function attempts to set the current process heap to 
// use "low fragmentation".  This is only available on XP and above
// (ie, not any flavor of 2000, NT, 9x, or ME).  
//
// WARNING!  Once you add this in, you can no longer run the app 
// on anything but XP without getting a link failure warning when 
// you run!  You will basically be building an XP-or-greater 
// executable.
//
// Forget it for now!
//-------------------------------------------------------------------//
bool bSetLowFragmentationHeap()
{
	// This define here will make sure the app is built on XP.
	// There's really no need for this, as the app will build, just not run, without XP.
	// #if _WIN32_WINNT >= 0x0501

	/*
	// From MSDN:
	//		The HeapInformation parameter is a pointer to a ULONG variable that can contain the following value:
	//			2: Low-fragmentation heap is enabled
	ULONG ulHeapInformationClass = 2;
	
	return ( 
		HeapSetInformation( 
			GetProcessHeap(), 
			HeapCompatibilityInformation,			// This indicates the next param will be a ULONG*, as defined above.
			&ulHeapInformationClass, 
			sizeof ULONG
		) != FALSE 
	);
	*/

	return true;

}