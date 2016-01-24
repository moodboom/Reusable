//-------------------------------------------------------------------//
//	SHBrowseWrapper
//
// This class provides a wrapper around SHBrowseForFolder().  That
// base in turn is used in derived classes specialized for browsing
// folders, machines, etc.  See BrowseForHelpers.*, which contains
// functions that you can drop right into your code.
//
// Originally based on CodeGuru sample code:
// ShellBrowser.h: interface for the CShellBrowser class.
// Copyright 1998 Scott D. Killen
// 
// TO DO
// roll in SwBrowseForFolder.* files
// Before we bother, we might want to check for other prettier
// solutions, including browsers that let you edit the dir
// string while browsing.
// credit:
//
// and for the "new folder" button...
// SwBrowseForFolder.cpp: implementation of the CSwBrowseForFolder class.
// Leonid Kunin, CIST-EAST, SAIC, 1999
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "SHBrowseWrapper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
//
// Construction/Destruction
//

SHBrowseWrapper::SHBrowseWrapper(
	const CWnd*				pParent, 
	const LPITEMIDLIST	pidl, 
	const CString*			pstrTitle
) :

	// Init vars.
	m_hwnd( 0 )

{
	SetOwner( pParent->GetSafeHwnd() );
	SetRoot(pidl);
	SetTitle( pstrTitle );
	m_bi.lpfn = BrowseCallbackProc;
	m_bi.lParam = reinterpret_cast<long>(this);
	m_bi.pszDisplayName = m_szSelected;
}

SHBrowseWrapper::~SHBrowseWrapper()
{

}

//////////////////////////////////////////////////////////////////////
//
// Implementation
//

void SHBrowseWrapper::SetOwner(const HWND hwndOwner)
{
	if (m_hwnd != NULL)
		return;

	m_bi.hwndOwner = hwndOwner;
}

void SHBrowseWrapper::SetRoot(const LPITEMIDLIST pidl)
{
	if (m_hwnd != NULL)
		return;

	m_bi.pidlRoot = pidl;
}

CString SHBrowseWrapper::GetTitle() const
{
	return m_bi.lpszTitle;
}

void SHBrowseWrapper::SetTitle(
	const CString* pstrTitle
) {

	if (m_hwnd != NULL)
		return;

	if ( pstrTitle == 0 )
	{
		m_bi.lpszTitle = 0;
		return;
	}

	m_pchTitle = std::auto_ptr<char>(new char [static_cast<size_t>(pstrTitle->GetLength()) + 1]);
	strcpy(m_pchTitle.get(), *pstrTitle);
	m_bi.lpszTitle = m_pchTitle.get();
}

//-------------------------------------------------------------------//
// SetFlags()																			//
//-------------------------------------------------------------------//
// Here are the flags in the SHLOBJ.H header (check it directly
// for lastest changes):
/*
	#define BIF_RETURNONLYFSDIRS   0x0001  // For finding a folder to start document searching
	#define BIF_DONTGOBELOWDOMAIN  0x0002  // For starting the Find Computer
	#define BIF_STATUSTEXT         0x0004
	#define BIF_RETURNFSANCESTORS  0x0008
	#define BIF_EDITBOX            0x0010
	#define BIF_VALIDATE           0x0020   // insist on valid result (or CANCEL)

	#define BIF_BROWSEFORCOMPUTER  0x1000  // Browsing for Computers.
	#define BIF_BROWSEFORPRINTER   0x2000  // Browsing for Printers
	#define BIF_BROWSEINCLUDEFILES 0x4000  // Browsing for Everything
*/
//-------------------------------------------------------------------//
void SHBrowseWrapper::SetFlags(const UINT ulFlags)
{
	if (m_hwnd != NULL)
		return;

	m_bi.ulFlags = ulFlags;
}

CString SHBrowseWrapper::GetSelectedFolder() const
{
	return m_szSelected;
}

bool SHBrowseWrapper::SelectFolder()
{
	bool bRet = false;

	LPITEMIDLIST pidl;
	if ((pidl = ::SHBrowseForFolder(&m_bi)) != NULL)
	{
		m_strPath.Empty();

		// FIX from CodeGuru commentary.  This allows us to
		// use BIF_BROWSEFORCOMPUTER.
		/*
		if (SUCCEEDED(::SHGetPathFromIDList(pidl, m_szSelected)))
		{
			bRet = true;
			m_strPath = m_szSelected;
		}
		*/
		if ( m_bi.ulFlags & BIF_BROWSEFORCOMPUTER )
		{
			bRet = true;
			m_strPath = m_szSelected;
		}
		else
		{
			if (SUCCEEDED(::SHGetPathFromIDList(pidl, m_szSelected)))
			{
				 bRet = true;
				 m_strPath = m_szSelected;
			}
		}

		LPMALLOC pMalloc;
		//Retrieve a pointer to the shell's IMalloc interface
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			// free the PIDL that SHBrowseForFolder returned to us.
			pMalloc->Free(pidl);
			// release the shell's IMalloc interface
			(void)pMalloc->Release();
		}
	}
	m_hwnd = NULL;

	return bRet;
}


//-------------------------------------------------------------------//
// OnInit()																				//
//-------------------------------------------------------------------//
// Set up the initial selection.
//-------------------------------------------------------------------//
void SHBrowseWrapper::OnInit()
{
	// SHBrowseForFolder() uses paths without
	// ending backslashs, except for drive letters.  
	// Strip off any trailing backslashes.
	if ( 
			m_strInitialSelection.Right( 1 ) == _T("\\") 
		&&	m_strInitialSelection.Right( 2 ) != _T(":\\") 
	)
		m_strInitialSelection = m_strInitialSelection.Left( m_strInitialSelection.GetLength() - 1 );

	SetSelection( m_strInitialSelection );
	SetStatusText( m_strInitialSelection );
}


//-------------------------------------------------------------------//
// SetInitialSelection()															//
//-------------------------------------------------------------------//
void SHBrowseWrapper::SetInitialSelection(const CString & strPath)
{
	m_strInitialSelection = strPath;
}


//-------------------------------------------------------------------//
// OnSelChanged()																		//
//-------------------------------------------------------------------//
// Set the status window on selection change.
//-------------------------------------------------------------------//
void SHBrowseWrapper::OnSelChanged(const LPITEMIDLIST pidl) const
{
	CString strBuffer;
	if (SHGetPathFromIDList(pidl, strBuffer.GetBuffer(MAX_PATH)))
		strBuffer.ReleaseBuffer();
	else
		strBuffer.ReleaseBuffer(0);
	SetStatusText(strBuffer);
}

void SHBrowseWrapper::EnableOK(const bool bEnable) const
{
	if (m_hwnd == NULL)
		return;

	// FIX from CodeGuru commentary.  Wrong param slot was used.
	// (void)SendMessage( m_hwnd, BFFM_ENABLEOK, static_cast<WPARAM>(bEnable), NULL );
	(void)SendMessage( m_hwnd, BFFM_ENABLEOK, NULL, static_cast<LPARAM>(bEnable) );
}

void SHBrowseWrapper::SetSelection(const LPITEMIDLIST pidl) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_SETSELECTION, FALSE, reinterpret_cast<long>(pidl));
}

void SHBrowseWrapper::SetSelection(const CString& strPath) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<long>(LPCTSTR(strPath)));
}

void SHBrowseWrapper::SetStatusText(const CString& strText) const
{
	if (m_hwnd == NULL)
		return;

	(void)SendMessage(m_hwnd, BFFM_SETSTATUSTEXT, NULL, reinterpret_cast<long>(LPCTSTR(strText)));
}


//-------------------------------------------------------------------//
// BrowseCallbackProc()																//
//-------------------------------------------------------------------//
int __stdcall SHBrowseWrapper::BrowseCallbackProc(
	HWND		hwnd,
	UINT		uMsg,
	LPARAM	lParam, 
	LPARAM	lpData
) {

	SHBrowseWrapper* pbff = reinterpret_cast<SHBrowseWrapper*>(lpData);
	pbff->m_hwnd = hwnd;
	switch ( uMsg )
	{
		case BFFM_INITIALIZED:
			pbff->OnInit();
			break;
		case BFFM_SELCHANGED:
			pbff->OnSelChanged(reinterpret_cast<LPITEMIDLIST>(lParam));
			break;
		default:
			break;
	}
	
	return 0;
}


