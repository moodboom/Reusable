// BaseChatUI.h : header file
//

#if !defined(BASE_CHAT_UI_H)
#define BASE_CHAT_UI_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DirectPlayChat;					// This contains the guts of DirectX interfacing.


/////////////////////////////////////////////////////////////////////////////
// BaseChatUI dialog

class BaseChatUI
{
public:

	// Construction
	BaseChatUI(
		HINSTANCE hInstance
	);

	~BaseChatUI();


// Implementation
protected:

	virtual void ReceiveChatMessage	( CString& strMsg ) {}
	virtual void SendChatMessage		( CString& strMsg );

	virtual void ReceiveAppSysMessage( DWORD dwType, CString& strMsg ) {}
	virtual void SendAppSysMessage	( DWORD dwType, CString& strMsg );

	DirectPlayChat*		pChat;

	friend class DirectPlayChat;			// Calls ReceiveChatMessage.

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(BASE_CHAT_UI_H)
