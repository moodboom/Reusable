// BaseChatUI.cpp : implementation file
//

#include "stdafx.h"
#include "DirectPlayChat.h"
#include "BaseChatUI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// BaseChatUI dialog

BaseChatUI::BaseChatUI(
	HINSTANCE hInstance
) {

	// Create the chat object.
	pChat = new DirectPlayChat(
		this,
		hInstance
	);
	
}


BaseChatUI::~BaseChatUI()
{

	// Clean up the chat object.
	delete pChat;

}


//-------------------------------------------------------------------//
// SendChatMessage()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void BaseChatUI::SendChatMessage( CString& strMsg )
{

	pChat->SendChatMessage( ASM_NORMAL, strMsg );

}


//-------------------------------------------------------------------//
// SendAppSysMessage()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void BaseChatUI::SendAppSysMessage( DWORD dwType, CString& strMsg )
{

	pChat->SendChatMessage( dwType, strMsg );

}
