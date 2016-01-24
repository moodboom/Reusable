//-------------------------------------------------------------------//
// ustringHelpers
//-------------------------------------------------------------------//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef USTRING_HELPERS_H
	#define USTRING_HELPERS_H

#include "ustring.h"


// This formats the given number of bytes into a nice output.
ustring strFileSizeFormat( Huge hSize );

// These manipulate strings related to file names.
ustring& RemoveTrailingBackslash( ustring& strDir );
ustring GetPathFromFilename( ustring& strFilename );

#endif		// USTRING_HELPERS_H
