//-------------------------------------------------------------------//
// ComHelpers.cpp																	//
// 
//	This module contains functions to make generic COM work easier.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "..\AssertHelpers.h"			// For ASSERT()

#include "..\..\Win32Helpers.h"		// For ConvertErrorToString()

#include "..\DisplayMessage.h"		// For error feedback

#include "ComHelpers.h"


//-------------------------------------------------------------------//
// GetComError()																		//
//-------------------------------------------------------------------//
// Reports on common errors to COM requests.
// Feel free to add error handling for other values as needed.
// They are typically found in WinError.h.
//-------------------------------------------------------------------//
ustring GetComError( HRESULT hInterfaceRequestResult, DWORD dwLastError )
{
	ustring strMsg;
	switch( hInterfaceRequestResult )
	{
		case E_INVALIDARG:
			strMsg = _T("An invalid argument was provided");
			break;
		case REGDB_E_CLASSNOTREG:
			strMsg = _T("Class not found to be registered");
			break;
		case CLASS_E_NOAGGREGATION:
			strMsg = _T("Illegal aggregation attempted");
			break;
		case CO_S_NOTALLINTERFACES:
			strMsg = _T("Not all interfaces were found");
			break;
		case E_NOINTERFACE:
			strMsg = _T("No interface found");
			break;
		case CO_E_NOTINITIALIZED:
		case RPC_E_THREAD_NOT_INIT:
			strMsg = _T("COM was found to not have been initialized [via CoInitialize()] in your process or thread");
			break;
		
		// --------------
		// TO DO
		
		// I have found this to be the result when a COM interface "disappears out 
		// from under us" due to the underlying COM object being destroyed.
		// If this seems to be the common consensus, add an appropriate msg here.
		// For now, just fall through to the "unknown error".
		case E_OUTOFMEMORY:		
			ASSERT( false );
										
		// --------------
		
		default:
		{
			if ( dwLastError == ERROR_SUCCESS )
			{
				strMsg = _T("There was an unknown COM error (error ");
				strMsg.AppendNumber( (Long)hInterfaceRequestResult );
				strMsg += _T(" - see WinError.h for more details)");

			} else
			{
				// Pull out the stops!  Call GetLastError() etc.
				strMsg = ConvertErrorToString( dwLastError );
				
				// Get rid of the period and newline.
				strMsg.resize( strMsg.size() - 2 );
			}
		}
	}

	return strMsg;	
}


//-------------------------------------------------------------------//
// GetComInterface()																	//
//-------------------------------------------------------------------//
// This simplifies as much as possible the process of grabbing
// a COM interface.  You should add parameters to this function,
// including a default whenever possible, as you need more
// functionality.
//-------------------------------------------------------------------//
HRESULT	GetComInterface(
	GUID					guidTarget,				// COM interface ID
	MULTI_QI*			pQI,						// Interfaces structure pointer - this will contain the interfaces on successful return!
	int					nQICount,				// Number of requested interfaces
	bool					bDisplayErrors,		// Should we attempt to display errors?
	LPCTSTR				szCOMInterfaceName	// Interface name to display in error msg
) {
	
	// Set this stuff up "freshly" each time.
	HRESULT hClientRequest;
	COSERVERINFO serverInfo;   //assumed to be client machine
	COAUTHINFO athn;
	serverInfo.pAuthInfo = &athn;
	
	// Set to NULL for local machine.
	serverInfo.pwszName = NULL;

	serverInfo.dwReserved1 = 0;
	serverInfo.dwReserved2 = 0;

	athn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
	athn.dwAuthnSvc = RPC_C_AUTHN_WINNT;
	athn.dwAuthzSvc = RPC_C_AUTHZ_NONE;
	athn.dwCapabilities = EOAC_NONE;
	athn.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
	athn.pAuthIdentityData = NULL;
	athn.pwszServerPrincName = NULL;

	// Original example:
	/*
	HRESULT hClientRequest;
	COSERVERINFO serverInfo;   //assumed to be client machine
	COAUTHINFO athn;
	long exists = false;

	serverInfo.pAuthInfo = &athn;

	// Set to NULL for local machine.
	serverInfo.pwszName = NULL;

	serverInfo.dwReserved1 = 0;
	serverInfo.dwReserved2 = 0;

	athn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
	athn.dwAuthnSvc = RPC_C_AUTHN_WINNT;
	athn.dwAuthzSvc = RPC_C_AUTHZ_NONE;
	athn.dwCapabilities = EOAC_NONE;
	athn.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
	athn.pAuthIdentityData = NULL;
	athn.pwszServerPrincName = NULL;

	hClientRequest = CoCreateInstanceEx(CLSID_Xxxx,
										NULL,
										CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER,
										&serverInfo,
										1,
										&qi);
	*/

	hClientRequest = CoCreateInstanceEx(
		guidTarget,
		NULL,
		CLSCTX_LOCAL_SERVER |  CLSCTX_REMOTE_SERVER,
		&serverInfo,
		nQICount,				// Number of QI's in MULTI_QI
		pQI						// pointer to array of MULTI_QI structures
	);
	DWORD dwErr = GetLastError();
	
	// Consolidate any bad results into hClientRequest.
	// We PROBABLY don't need to do this, but just in case...
	if ( SUCCEEDED( hClientRequest ) )
		for ( int nA = 0; nA < nQICount; nA++ )
			if ( !SUCCEEDED( pQI->hr ) )
			{
				hClientRequest = pQI->hr;
				break;
			}

	// Now test result and report if there was a problem.
	if ( 
			!SUCCEEDED( hClientRequest ) 
		&&	bDisplayErrors
	) {
		// Why did it fail?
		ustring strMsg = GetComError( hClientRequest, dwErr );
		strMsg += _T(" when connecting to the ");
		strMsg += szCOMInterfaceName;
		strMsg += _T(" COM interface.");
		DisplayMessage( strMsg.c_str() );
	}

	return hClientRequest;
}