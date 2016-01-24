//-------------------------------------------------------------------//
// DisplayMessage
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef DISPLAY_MESSAGE_H
	#define DISPLAY_MESSAGE_H


/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module:
//

#define IDL_WARNING								6000		// "Warning"

#define IDD_KILLABLE_WARNING					6001		
#define IDC_CONTINUE								6002		
#define IDC_MESSAGE								6003		
#define IDD_KILLABLE_MESSAGE					6004		
#define IDD_KILLABLE_MESSAGE_WITH_LIST		6005
#define IDC_MESSAGE_LIST						6006
																	
// Backup Copy of IDD_KILLABLE_WARNING:
//
// IDD_KILLABLE_WARNING DIALOGEX 0, 0, 229, 135                              
// STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION                               
// CAPTION "Warning"                                                         
// FONT 8, "MS Sans Serif", 0, 0, 0x1                                        
// BEGIN                                                                     
// 	 DEFPUSHBUTTON   "OK",IDOK,172,114,50,14                               																	
// 	 CONTROL         "Display this message in the future.",IDC_CONTINUE,   																	
// 						  "Button",BS_AUTOCHECKBOX | BS_LEFT | WS_TABSTOP,7,117,																	
// 						  121,11                                                																	
// 	 LTEXT           "Static",IDC_MESSAGE,7,7,215,98,0,WS_EX_CLIENTEDGE    																	
// END                                                                       																	

// Backup Copy of IDD_KILLABLE_MESSAGE_WITH_LIST:
//
// IDD_KILLABLE_MESSAGE_WITH_LIST DIALOG DISCARDABLE  0, 0, 229, 138
// STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION
// CAPTION "Warning"
// FONT 8, "MS Sans Serif"
// BEGIN
//     DEFPUSHBUTTON   "&Yes",IDOK,104,104,50,14
//     PUSHBUTTON      "&No",IDCANCEL,162,104,50,14
//     CONTROL         "Don't ask me this again.",IDC_CONTINUE,"Button",
//                     BS_AUTOCHECKBOX | BS_LEFT | WS_TABSTOP,7,122,95,11
//     CTEXT           "Static",IDC_MESSAGE,7,3,215,31
//     EDITTEXT        IDC_MESSAGE_LIST,5,36,218,62,ES_MULTILINE | 
//                     ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY | 
//                     WS_VSCROLL
// END

// Ideally, resources would be specified on a class-by-class basis.  Unfortunately,
// Visual Studio does not handle that well.  Different projects have different resource
// files that contain all of the project's resources in one place.  Make sure
// you provide the resources matching the above defines in your resource file.
// You must also include this file in the resource file's "Read-only symbol
// directives" ( see "Resource Includes" on the View menu ).
//
// We want to exclude the remainder of the include file when dealing with App Studio.
#ifndef RC_INVOKED
/////////////////////////////////////////////////////////////////////////


// MFC OR NON-MFC versions.

//-------------------------------------------------------------------//
// DisplayMessage()																	//
//-------------------------------------------------------------------//
// Message passed as LPCTSTR.														//
// This version is very similar to a straight ::MessageBox() call,
// but it gives us a common location to provide extensions, as needed.
//-------------------------------------------------------------------//
int DisplayMessage( 
	LPCTSTR	szMsg,
	LPCTSTR	szLabel,
	HWND		hParent		= NULL,
	UINT		nType			= MB_OK | MB_APPLMODAL | MB_ICONINFORMATION,
	bool		bBeep			= false,
	bool		bNoButtons	= false
);


int DisplayMessage( 
	UINT		MsgResourceID,
	UINT		LabelResourceID,	// = IDL_WARNING,		These are not provided to avoid ambiguities
	HWND		hParent,				// = NULL,
	UINT		nType					= MB_OK | MB_APPLMODAL | MB_ICONINFORMATION,
	bool		bBeep					= false,
	bool		bNoButtons			= false
);


// MFC-only versions.
#ifdef	_MFC_VER

//-------------------------------------------------------------------//
// DisplayMessage()																	//
//-------------------------------------------------------------------//
// This function is used to display a message.
// It wraps the MessageBox function.
// We could make it a class but we need to return a
// result, and that would require more overhead (e.g.
// storage of the return value within the class).
//-------------------------------------------------------------------//
int DisplayMessage( 
	UINT		MsgResourceID,
	UINT		LabelResourceID	= IDL_WARNING,
	CWnd*		pParent				= NULL,
	UINT		nType					= MB_OK | MB_APPLMODAL | MB_ICONINFORMATION,
	bool		bBeep					= true,
	bool		bNoButtons			= false
);

int DisplayMessage( 
	CString&	strMsg,
	UINT		LabelResourceID	= IDL_WARNING,
	CWnd*		pParent				= NULL,
	UINT		nType					= MB_OK | MB_APPLMODAL | MB_ICONINFORMATION,
	bool		bBeep					= true,
	bool		bNoButtons			= false
);

int DisplayMessage( 
	CString&	strMsg,
	CString&	strLabel,
	CWnd*		pParent				= NULL,
	UINT		nType					= MB_OK | MB_APPLMODAL | MB_ICONINFORMATION,
	bool		bBeep					= true,
	bool		bNoButtons			= false
);

int DisplayMessage( 
	LPCTSTR	szMsg,
	UINT		LabelResourceID	= IDL_WARNING,
	CWnd*		pParent				= NULL,
	UINT		nType					= MB_OK | MB_APPLMODAL | MB_ICONINFORMATION,
	bool		bBeep					= true,
	bool		bNoButtons			= false
);

//-------------------------------------------------------------------//
// KillableWarning()																	//
//-------------------------------------------------------------------//
// This pops up a warning dialog with a checkbox that can be
// clicked off to turn off display of the warning on consecutive
// calls.
//-------------------------------------------------------------------//
void KillableWarning(
	CString&	strMsg,
	CString&	strLabel,
	bool&		bContinue
);


//-------------------------------------------------------------------//
// KillableMessage()																	//
//-------------------------------------------------------------------//
// This pops up a Yes/No message dialog with a checkbox that can be
// clicked off to turn off the prompt on consecutive calls.
//-------------------------------------------------------------------//
bool KillableMessage(
	CString	&strMsg,
	CString	&strLabel,
	bool		&bContinue,
	bool		&bFutureResponse,
	bool		bUseOKCancel			= false,
	bool		bDontAllowFutureNo	= false,
	bool		bUseList					= false,
	CString	strListText				= CString()		// CString is smart enough not to copy contents here,
																// even tho we can't use a reference.
);

#endif	// _MFC_VER
#endif	// RC_INVOKED
#endif	// DISPLAY_MESSAGE_H



