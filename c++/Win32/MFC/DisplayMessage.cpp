//-------------------------------------------------------------------//
//	DisplayMessage
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//
#include "stdafx.h"

#include <FontDlg.h>							// Base class, allows custom font.

#include "AssertHelpers.h"					// For ASSERT() of course :>

#include "DisplayMessage.h"


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
	HWND		hParent,
	UINT		nType,
	bool		bBeep,
	bool		bNoButtons
) {

	if ( bBeep )
		MessageBeep( nType );

	// TO DO
	ASSERT( !bNoButtons );

	// Make the message box TASK modal if it has no parent,
	// so it does not get lost underneath other windows.
	if ( hParent == 0 )
		nType = ( nType & ~MB_APPLMODAL ) | MB_TASKMODAL;
	
	// MDM	4/22/2003 3:06:58 PM
	// Is this any better for any reason?  It's definitely worse in some ways...
	// return AfxMessageBox( szMsg, nType );

	return ::MessageBox( hParent, szMsg, szLabel, nType );

}


//-------------------------------------------------------------------//
// DisplayMessage()																	//
//-------------------------------------------------------------------//
// Message passed as resource ID.												//
//-------------------------------------------------------------------//
int DisplayMessage( 
	UINT		MsgResourceID,
	UINT		LabelResourceID,
	HWND		hParent,
	UINT		nType,
	bool		bBeep,
	bool		bNoButtons
) {

	// Load the strings.
	const int cnMsgBufferSize = _MAX_PATH * 2;
	const int cnLabelBufferSize = _MAX_PATH;
	TCHAR szMsg[ cnMsgBufferSize ];
	TCHAR szLabel[ cnLabelBufferSize ];
	::LoadString( ::GetModuleHandle(NULL), MsgResourceID, szMsg, cnMsgBufferSize );
	::LoadString( ::GetModuleHandle(NULL), LabelResourceID, szLabel, cnLabelBufferSize );

	return DisplayMessage( szMsg, szLabel, hParent, nType, bBeep, bNoButtons );

}


// MFC-only versions.
#ifdef	_MFC_VER

#include "CStringHelpers.h"

//-------------------------------------------------------------------//
// DisplayMessage()																	//
//-------------------------------------------------------------------//
// Message passed as CString.														//
//-------------------------------------------------------------------//
int DisplayMessage( 
	CString&	strMsg,
	CString&	strLabel,
	CWnd*		pParent,
	UINT		nType,
	bool		bBeep,
	bool		bNoButtons
) {

	if ( bBeep )
		MessageBeep( nType );

	// If no parent, use the main app window.
	if ( !pParent )
		pParent = AfxGetMainWnd();
	
	// NOTE: Even if this is NULL, the call below
	// to MessageBox() will still work.  Scary, eh?
	ASSERT( pParent != NULL );
	
	// TO DO
	ASSERT( !bNoButtons );
	/*

  int nReturn;

	if ( bNoButtons ) 
	{
		// Create a modeless dialog to display the message.
		// Keep the handle in a static so we can update the
		// message in UpdateDisplayMessage() and close the
		// dialog down in CloseDisplayMessage().
		// Use a derived CDialog with msg string params in
		// constructor.  Call Create in constructor.
	
	} else 
	{	
		// Standard message box.
		nReturn = pParent->MessageBox(
			LPCTSTR( strMsg ),
			LPCTSTR( strLabel ),
			nType
		);

	}

  return nReturn;

	*/
	
	// Standard message box.
	return pParent->MessageBox(
		LPCTSTR( strMsg ),
		LPCTSTR( strLabel ),
		nType
	);

}

//-------------------------------------------------------------------//
// DisplayMessage() - overloaded version										//
//-------------------------------------------------------------------//
// Message & label passed as resource string ID.							//
//-------------------------------------------------------------------//
int DisplayMessage( 
	UINT	MsgResourceID,
	UINT	LabelResourceID,
	CWnd*	pParent,
	UINT	nType,
	bool	bBeep,
	bool	bNoButtons
) {
	return DisplayMessage( 
		ResStr( MsgResourceID ),
		ResStr( LabelResourceID ), 
		pParent, 
		nType, 
		bBeep, 
		bNoButtons 
	);
}

//-------------------------------------------------------------------//
// DisplayMessage() - overloaded version										//
//-------------------------------------------------------------------//
// Message passed as CString, label passed as resource string ID.		//
//-------------------------------------------------------------------//
int DisplayMessage( 
	CString&	strMsg,
	UINT		LabelResourceID,
	CWnd*		pParent,
	UINT		nType,
	bool		bBeep,
	bool		bNoButtons
) {
	return DisplayMessage( 
		strMsg, 
		ResStr( LabelResourceID ),
		pParent, 
		nType, 
		bBeep, 
		bNoButtons 
	);
}


//-------------------------------------------------------------------//
// DisplayMessage() - overloaded version										//
//-------------------------------------------------------------------//
// Message passed as TCHAR, label passed as resource string ID.		//
//-------------------------------------------------------------------//
int DisplayMessage( 
	LPCTSTR	szMsg,
	UINT		LabelResourceID,
	CWnd*		pParent,
	UINT		nType,
	bool		bBeep,
	bool		bNoButtons
) {
	return DisplayMessage( 
		CString( szMsg ), 
		ResStr( LabelResourceID ),
		pParent, 
		nType, 
		bBeep, 
		bNoButtons 
	);
}


//-------------------------------------------------------------------//
// KillableMessageDlg																//
//-------------------------------------------------------------------//
// TO DO
// At this point in time, the msg is in a list box, so we did not
// have to bother with resizing the msg window.  Update to use a
// label for the msg and size the window to fit the label.
//-------------------------------------------------------------------//
class KillableMessageDlg : public FontDlg
{
	typedef FontDlg inherited;

public:
	KillableMessageDlg( 
		UINT	DlgID
	) : 
		// Call base class.
		inherited( DlgID ),

		// Init vars.
		m_bUseOKCancel	( false ),
		m_bUseList		( false )

	{}

	BOOL		bContinue;
	CString	strMsg;
	CString	strListText;
	CString	strLabel;
	bool		m_bUseOKCancel;
	bool		m_bUseList;

	void DoDataExchange( CDataExchange* pDX )
	{
		DDX_Check(pDX, IDC_CONTINUE, bContinue );
		DDX_Text(pDX, IDC_MESSAGE, strMsg );
		if ( m_bUseList )
			DDX_Text(pDX, IDC_MESSAGE_LIST, strListText );
	}

	virtual BOOL OnInitDialog()
	{
		CDialog::OnInitDialog();

		SetWindowText( LPCTSTR( strLabel ) );
		if ( m_bUseOKCancel )
		{
			GetDlgItem( IDOK		)->SetWindowText( _T("&OK") );
			GetDlgItem( IDCANCEL	)->SetWindowText( _T("&Cancel") );
		}

		return TRUE;
	}

	virtual void OnCancel() 
	{
		// Do a DDX before we drop out.  We want to 
		// actually get the bContinue status before we bail.
		UpdateData();

		CDialog::OnCancel();
	}

};



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
) {
	if ( !bContinue )
	{
		KillableMessageDlg dlg( IDD_KILLABLE_WARNING );

		dlg.bContinue	= (BOOL) bContinue;
		dlg.strMsg		= strMsg;
		dlg.strLabel	= strLabel;

		// If you hit this, you prolly need to add the dlg template
		// resource.  See the top of the header file for details.
		ASSERT( dlg.DoModal() != -1 );

		bContinue = ( dlg.bContinue != 0 );
	}
}


//-------------------------------------------------------------------//
// KillableMessage()																	//
//-------------------------------------------------------------------//
// This pops up a Yes/No message dialog with a checkbox that can be
// clicked off to turn off the prompt on consecutive calls.
//
// You supply the message, the dialog will provide either Yes/No
// buttons or OK/Cancel buttons, depending on the value of bUseOKCancel.
//
// Turning off the prompt will cause future calls to this function
// to return the result from the call when the prompt was turned off.
//
// This future response can be either Yes or No; however,
// if bDontAllowFutureNo is true, then this function will ignore the
// user's request to "Don't ask in the future" if she selects No/Cancel.
//-------------------------------------------------------------------//
bool KillableMessage(
	CString&	strMsg,
	CString&	strLabel,
	bool&		bContinue,
	bool&		bFutureResponse,
	bool		bUseOKCancel,
	bool		bDontAllowFutureNo,
	bool		bUseList,
	CString	strListText
) {
	if ( !bContinue )
	{
		KillableMessageDlg dlg( bUseList? IDD_KILLABLE_MESSAGE_WITH_LIST : IDD_KILLABLE_MESSAGE );

		dlg.bContinue			= (BOOL) bContinue;
		dlg.strMsg				= strMsg;
		dlg.strLabel			= strLabel;
		dlg.m_bUseOKCancel	= bUseOKCancel;
		dlg.m_bUseList			= bUseList;
		dlg.strListText		= strListText;

		int nDlgResult = dlg.DoModal();

		// If the dialog failed, make sure you have supplied the dlg template
		// in your *.rc file!!  See the note at the top of DisplayMessage.h.
		ASSERT( nDlgResult != -1 );
		
		bFutureResponse = ( nDlgResult == IDOK );

		bContinue = 
				( dlg.bContinue != 0 )
			&& ( bFutureResponse || !bDontAllowFutureNo );

	}

	return bFutureResponse;
}


#endif // _MFC_VER
