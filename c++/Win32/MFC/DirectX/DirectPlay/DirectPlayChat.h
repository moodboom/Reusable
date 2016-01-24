// DirectPlayChat.h : header file
//

#if !defined(DIRECT_PLAY_CHAT_H)
	#define DIRECT_PLAY_CHAT_H

#include <windowsx.h>							// For DirectX 6 - also, DX6 LIB MUST BE FIRST IN Tools:Options:Dir:Include list!

#define IDIRECTPLAY2_OR_GREATER
#include <dplay.h>

class BaseChatUI;									// Our UI class.


//-------------------------------------------------------------------//
// Globals and Statics																//
//-------------------------------------------------------------------//

const DWORD MAXPLAYERS = 10;					// max no. players in the session

////////////////////////////////
// APPLICATION SYSTEM MESSAGES
////////////////////////////////
// These are the base-class application system messages (non-chat msgs).
// Derived classes can create their own ASM's; just make sure the msg
// values are unique.  All ASM's are routed through BaseChatUI's
// Send/ReceiveAppSysMessage() virtual functions.
//
const DWORD ASM_NORMAL			= 0;						// Message type for "normal" chat string (non-app-system-msg)
const DWORD	ASM_JOINED			= 1;						// Player has joined.
const DWORD ASM_LEFT				= 2;						// Player has left.
const DWORD ASM_YOU_ARE_HOST	= 3;						// You have become the new host.
const DWORD ASM_BASE_LAST		= ASM_YOU_ARE_HOST;
//
////////////////////////////////

// structure used to store DirectPlay information
typedef struct {
	LPDIRECTPLAY4A	lpDirectPlay4A;			// IDirectPlay4A interface pointer
	HANDLE			hPlayerEvent;				// player event to use
	DPID			dpidPlayer;						// ID of player created
	BOOL			bIsHost;							// TRUE if we are hosting the session
} DPLAYINFO, *LPDPLAYINFO;

// guid for this application
// {5BFDB060-06A4-11d0-9C4F-00A0C905425E}
DEFINE_GUID(DPCHAT_GUID, 
0x5bfdb060, 0x6a4, 0x11d0, 0x9c, 0x4f, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);

// prototypes
extern HRESULT	ConnectUsingLobby(LPDPLAYINFO lpDPInfo);
extern HRESULT	ConnectUsingDialog(HINSTANCE hInstance, LPDPLAYINFO lpDPInfo);

//-------------------------------------------------------------------//


class DirectPlayChat
{

public:
	
	DirectPlayChat::DirectPlayChat(
		BaseChatUI*	pUI,
		HINSTANCE	hInstance
	);

	DirectPlayChat::~DirectPlayChat();

	CString GetCurrentPlayerName();

protected:

	BaseChatUI*	m_pUI;

	HRESULT SetupConnection(HINSTANCE hInstance, LPDPLAYINFO lpDPInfo);

	HRESULT ShutdownConnection(LPDPLAYINFO lpDPInfo);

	static DWORD WINAPI ReceiveThread( LPVOID lpThreadParameter );

	HRESULT SendChatMessage(
		DWORD			dwType,
		CString&		strMsg
	);

	static HRESULT ReceiveMessage(LPDPLAYINFO lpDPInfo);

	static void HandleApplicationMessage(
		LPDPLAYINFO lpDPInfo, 
		LPDPMSG_GENERIC lpMsg, 
		DWORD dwMsgSize,
		DPID idFrom, 
		DPID idTo
	);

	static void HandleSystemMessage(
		LPDPLAYINFO lpDPInfo, 
		LPDPMSG_GENERIC lpMsg, 
		DWORD dwMsgSize,
		DPID idFrom, 
		DPID idTo
	);

	static HRESULT GetChatPlayerName(
		LPDIRECTPLAY4A lpDirectPlay4A, 
		DPID dpidPlayer,
		LPDPNAME *lplpName
	);

	static HRESULT NewChatString(LPDIRECTPLAY4A lpDirectPlay4A, DPID dpidPlayer,
					  LPSTR lpszMsg, LPSTR *lplpszStr);
	
	
	char* GetDirectPlayErrStr(
		HRESULT hr
	);
	
	DPLAYINFO	DPInfo;

	friend class BaseChatUI;		// It calls SendChatMessage().

};


#endif // DIRECT_PLAY_CHAT_H