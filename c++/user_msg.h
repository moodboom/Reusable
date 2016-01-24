//-------------------------------------------------------------------//
// user_msg.h
//-------------------------------------------------------------------//
//	Abstracts the display of user messages across platforms.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef USER_MSG_H
#define USER_MSG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(WIN32) && !defined(CXFS_NT_NATIVE)

	/////////////////////////////////////////////////////////////////////////
	// ** NOTICE **
	/////////////////////////////////////////////////////////////////////////
	// The following resources are required by this module (or submodules):
	//
	#define IDS_MSG_LABEL			950
	#define IDS_YES_NO_LABEL		951
	// 
	// Ideally, resources would be specified on a class-by-class basis.  Unfortunately,
	// Visual Studio does not handle that well.  Different projects have different resource
	// files that contain all of the project's resources in one place.  Make sure
	// you provide the resources matching the above defines in your resource file.
	// You must also include this file in the resource file's "Resource Set Includes"
	// ( see "Resource Includes" on the View menu ).
	//
	/////////////////////////////////////////////////////////////////////////

#endif

// We want to exclude the remainder of the include file when dealing with App Studio.
// (It's a "Windows thing".)
#ifndef RC_INVOKED


#include "TCHAR_convert.h"			// for TCHAR definition


/* Display a msg to the user in platform-dependant standard fashion. */
WINEXPORT void WINMANGLE user_msg( TCHAR* szMsg, ... );

/* Prompt user.  This returns 1 for yes, 0 for no. */
WINEXPORT int WINMANGLE user_msg_yes_no( TCHAR* szMsg, ... );

/* Provide progress feedback. */
WINEXPORT void WINMANGLE update_progress( int pct, TCHAR* task, ... );

/* Use this when finished with progress. */
WINEXPORT void WINMANGLE progress_complete();


#endif	/* RC_INVOKED */

#endif	/* USER_MSG_H */
