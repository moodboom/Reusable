//-------------------------------------------------------------------//
// BrowseForMachine
//
// This class provides a wrapper around SHBrowseForFolder().  That
// base in turn is used here to browse a machine.  
//
// See BrowseForHelpers.*, which contains functions that you can drop 
// right into your code.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "BrowseForMachine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//-------------------------------------------------------------------//
// BrowseForMachine()																//
//-------------------------------------------------------------------//
BrowseForMachine::BrowseForMachine(
	const CString*			pstrTitle,
	const CWnd*				pParent,
	const LPITEMIDLIST	pidl
) : 

	// Call base class.
	SHBrowseWrapper(
		pParent,
		pidl, 
		pstrTitle
	)

	// Init vars.
{
	// Set flags to browse for a machine.
	SetFlags( BIF_BROWSEFORCOMPUTER | BIF_STATUSTEXT );
}


//-------------------------------------------------------------------//
// ~BrowseForMachine()																//
//-------------------------------------------------------------------//
BrowseForMachine::~BrowseForMachine()
{
}

