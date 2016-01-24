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

#ifndef BROWSE_FOR_DIRECTORY_H
#define BROWSE_FOR_DIRECTORY_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "SHBrowseWrapper.h"

class BrowseForDirectory : public SHBrowseWrapper  
{
public:
	BrowseForDirectory(
		const CString*			pstrTitle				= 0,
		const CString*			pstrInitialDirectory	= 0,
		const CWnd*				pParent					= NULL
	);
	~BrowseForDirectory();

};

#endif // BROWSE_FOR_DIRECTORY_H
