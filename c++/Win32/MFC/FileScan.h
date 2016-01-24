//-------------------------------------------------------------------//
// FileScan header
//-------------------------------------------------------------------//
// This class makes file scanning trivial.
//
//	Copyright © 2001-2004 A better Software.
//-------------------------------------------------------------------//

// FileScan.h : header file
//
#if !defined(FILE_SCAN_H)
	#define FILE_SCAN_H

#include <afx.h>						// For CString
#include <afxtempl.h>				// For CTypedPtrArray
#include <StandardTypes.h>			// For Huge

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// FileScan dialog


class FileScan 
{

// Construction
public:
   
	FileScan(
		CString* pstrStartPath,
		TCHAR*	pszFilter				= _T("*.*"),
		bool		bScanSubdirectories	= true
	);

   ~FileScan();
	
	bool GetNextFile();

	// These functions return results from the last successful find.
	CString	strFilename();
	CString	strTitle();
	CString	strPath();
	CString	strPathAndFilename();
	Huge		hSize();
	CTime		ctCreatedTime();
	CTime		ctModifiedTime();
	CTime		ctAccessedTime();

protected:

	// This is our internal recursive version.
	bool GetNextFile( int nFinderArrayIndex );

	CString	m_strStartPath;
	CString	m_strFilter;
	bool		m_bScanSubdirectories;

	CTypedPtrArray< CPtrArray, CFileFind* > Finders;    
	CArray< bool, bool > Results;    

	// Temp strings used during recursion.
	// We just need one instance of each, so they were made member vars.
	// TO DO
	// Do a proper filter check.  For now, we just compare extensions.
	CString m_strDirName;
	CString m_strFilterExt;

};


#endif // !defined(FILE_SCAN_H)
