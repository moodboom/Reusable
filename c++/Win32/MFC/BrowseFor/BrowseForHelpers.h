//-------------------------------------------------------------------//
//	BrowseFor... Helpers
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef BROWSE_FOR_HELPERS_H
	#define BROWSE_FOR_HELPERS_H


bool GetDirFromUser( 
	CString*		pstrDir,
	CWnd*			pParent,
	CString*		pstrMsg						= 0,
	bool			bIncludeNewFolderButton	= false
);


bool GetMachineFromUser( 
	CString*		pstrMachine,
	CWnd*			pParent	= NULL,
	CString*		pstrMsg	= 0
);


#endif	// BROWSE_FOR_HELPERS_H
