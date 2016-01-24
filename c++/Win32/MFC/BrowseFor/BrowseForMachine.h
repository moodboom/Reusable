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

#ifndef BROWSE_FOR_MACHINE_H
#define BROWSE_FOR_MACHINE_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "SHBrowseWrapper.h"

class BrowseForMachine : public SHBrowseWrapper  
{
public:
	BrowseForMachine(
		const CString*			pstrTitle	= 0,
		const CWnd*				pParent		= NULL,
		const LPITEMIDLIST	pidl			= NULL
	);
	~BrowseForMachine();

};

#endif // BROWSE_FOR_MACHINE_H
