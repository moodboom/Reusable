//-------------------------------------------------------------------//
// BrowseForDirectory
//
// This class provides a wrapper around SHBrowseForFolder().  That
// base in turn is used here to browse a dir.  
//
// See BrowseForHelpers.*, which contains functions that you can drop 
// right into your code.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "BrowseForDirectory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//-------------------------------------------------------------------//
// BrowseForDirectory()																//
//-------------------------------------------------------------------//
BrowseForDirectory::BrowseForDirectory(
	const CString*			pstrTitle,
	const CString*			pstrInitialDirectory,
	const CWnd*				pParent
) : 

	// Call base class.
	SHBrowseWrapper(
		pParent,
		0, 
		pstrTitle
	)

	// Init vars.
{
	// Set the initial directory, if provided.
	if ( pstrInitialDirectory )
		SetInitialSelection( *pstrInitialDirectory );
	
	// Set default flags.
	SetFlags( BIF_STATUSTEXT );
}


//-------------------------------------------------------------------//
// ~BrowseForDirectory()															//
//-------------------------------------------------------------------//
BrowseForDirectory::~BrowseForDirectory()
{
}

