//-------------------------------------------------------------------//
//	BrowseFor... Helpers
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "BrowseForDirectory.h"		// For GetDirFromUser()
#include "BrowseForMachine.h"			// For GetMachineFromUser()

#include "BrowseForHelpers.h"


// Replaced with the SHBrowseWrapper class.
/*
//-------------------------------------------------------------------//
// BrowseCallbackProc()																//
//-------------------------------------------------------------------//
// Call back function required by SHBrowseForFolder().  Used by
// GetDirFromUser(), below.
// This function allows us to specify a starting directory that is
// other than the default (the desktop).
//-------------------------------------------------------------------//
#include <windows.h>      
#include <shlobj.h>

// Keep a pointer to the following strings around globally, so
// the callback function can find them.
CString* g_pstrStartPath;
CString* g_pstrBrowseMsg;

int CALLBACK BrowseCallbackProc(
	HWND		hwnd,
	UINT		uMsg,
	LPARAM	lp, 
	LPARAM	pData
) {

	TCHAR szDir[MAX_PATH];         
	
	switch ( uMsg ) {
      case BFFM_INITIALIZED: 
		{

			// SHBrowseForFolder() uses paths without
			// ending backslashs, except for drive letters.  
			// Strip off any trailing backslashes.
			if ( 
					g_pstrStartPath->Right( 1 ) == _T("\\") 
				&&	g_pstrStartPath->Right( 2 ) != _T(":\\") 
			)
				*g_pstrStartPath = g_pstrStartPath->Left( g_pstrStartPath->GetLength() - 1 );

			// Copy to the array.
			_tcsncpy( 
				szDir,
				LPCTSTR( *g_pstrStartPath ),
				sizeof szDir / sizeof TCHAR
			);
			szDir[MAX_PATH-1] = _T('\0');
			
			// WParam is TRUE since you are passing a path.
			// It would be FALSE if you were passing a pidl.
			SendMessage(
				hwnd,
				BFFM_SETSELECTION,
				TRUE,
				(LPARAM) szDir
			);

			if ( g_pstrBrowseMsg )
			{
				// See if we can be sneaky and get the undocumented static
				// text control in the common dialog.  We got its ID using Spy++.
				// We ASSERT if not.
				HWND hUndocStatic = GetDlgItem( hwnd, 0x3742	);
				ASSERT( hUndocStatic );
				SendMessage( hUndocStatic, WM_SETTEXT, 0, (LONG) LPCTSTR( *g_pstrBrowseMsg ) );
			}

			break;            
		}

      case BFFM_SELCHANGED: 
		{
      
		   // Set the status window to the currently selected path.
         if ( 
				SHGetPathFromIDList(
					(LPITEMIDLIST) lp,
					szDir
				) 
			) {

				SendMessage(
					hwnd,
					BFFM_SETSTATUSTEXT,
					0,
					(LPARAM)szDir
				);
         }
			break;
		}
		
		default:
			break;         
	
	}         
	
	return 0;

}      


//-------------------------------------------------------------------//
// GetDirFromUser()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
bool GetDirFromUser( 
	CString*		pstrDir,
	CWnd*			pParent,
	CString*		pstrMsg
) {

	// Make a copy of the string.  We don't want to modify the original
	// until done.  Set our global pointer to the copy.
	CString strDirCopy( *pstrDir );
	g_pstrStartPath = &strDirCopy;

	// Set our message string pointer.
	g_pstrBrowseMsg = pstrMsg;
	
	BROWSEINFO bi;         
	TCHAR szDir[MAX_PATH];
   LPITEMIDLIST pidl;         
	LPMALLOC pMalloc;
   if ( SUCCEEDED( SHGetMalloc(&pMalloc ) ) ) {
      
		// Set up our browse info.
		memset( &bi, 0, sizeof(bi) );
		bi.hwndOwner = pParent->GetSafeHwnd();
      bi.pszDisplayName = 0;            
		bi.pidlRoot = 0;
      bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
      bi.lpfn = BrowseCallbackProc;
      
		pidl = SHBrowseForFolder(&bi);            
		if (pidl) {
         if ( 
				SHGetPathFromIDList(
					pidl,
					szDir
				)
			) {

				// Get the result.
				*pstrDir = szDir;
				
				// Add "\" as needed.
				if ( pstrDir->Right( 1 ) != _T("\\") )
					*pstrDir += _T("\\");
				
				return true;

			}
         
         // In C:	pMalloc->lpVtbl->Free(pMalloc,pidl);
         //			pMalloc->lpVtbl->Release(pMalloc);            
			pMalloc->Free(pidl); pMalloc->Release();

		}         
	}

	// No luck.
	return false;

}
*/


//-------------------------------------------------------------------//
// GetDirFromUser()																	//
//-------------------------------------------------------------------//
bool GetDirFromUser( 
	CString*		pstrDir,		// Fill this param in if you want a specific starting point.
									// It will contain the user's selection on return.
	CWnd*			pParent,
	CString*		pstrTitle,
	bool			bIncludeNewFolderButton

) {

	BrowseForDirectory bfd( 
		pstrTitle, 
		pstrDir,				
		pParent
	);
	if ( bfd.SelectFolder() )
	{
		*pstrDir = bfd.GetSelectedFolder();
		return true;
	}

	return false;
}


//-------------------------------------------------------------------//
// GetMachineFromUser()																//
//-------------------------------------------------------------------//
bool GetMachineFromUser( 
	CString*		pstrMachine,
	CWnd*			pParent,
	CString*		pstrTitle
) {
	// Get the network root pidl.  This will be our browse root.
	LPITEMIDLIST pidl = NULL;
	VERIFY(
		SHGetSpecialFolderLocation(
			NULL,
			CSIDL_NETWORK,
			&pidl
		) == NOERROR
	);
	
	BrowseForMachine bfm( pstrTitle, pParent, pidl );
	
	// Set the initial selection to the root of the network.
	// CAN WE?
	/*
	CString strNetwork;
	SHGetFolderPath(
		NULL,
		CSIDL_NETWORK,


	bfm.SetInitialSelection( strNetwork );
	*/
	
	if ( bfm.SelectFolder() )
	{
		*pstrMachine = bfm.GetSelectedFolder();
		return true;
	}
	return false;
}
