//-------------------------------------------------------------------//
// DllLoader class
//-------------------------------------------------------------------//
// MDM	4/2/2004 3:10:58 PM
// This allows us to dynamically load uxtheme.dll as needed, which 
// allows us to AVOID loading it under anything less than WinXP,
// which allows IEToolbar to work under both WinXP and Win2000
// (and hopefully anything more recent).
//
//
// NOTE: Derived from code originally posted to CodeGuru.com by 
// Nikolay Denisov (acnick@mail.lanck.net).
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"
#include "DllLoader.h"

#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDllLoader

CDllLoader::CDllLoader( LPCTSTR lpFileName, bool bAutoFree /*=true*/ )
{
    m_hModule   = ::LoadLibrary( lpFileName );
    m_bAutoFree = bAutoFree;
}

CDllLoader::~CDllLoader()
{
    if ( m_bAutoFree && ( m_hModule != 0 ) )
    {
        VERIFY( ::FreeLibrary( m_hModule ) );
    }
}

/////////////////////////////////////////////////////////////////////////////
// Operations

bool CDllLoader::IsLoaded()
{
    return ( m_hModule != 0 );
}

bool CDllLoader::GetVersion( DWORD* pdwVersion )
{
    DLLGETVERSIONPROC pfDllGetVersion = ( DLLGETVERSIONPROC )GetProcAddress( "DllGetVersion" );
    if ( pfDllGetVersion != 0 )
    {
        DLLVERSIONINFO dvi;
        dvi.cbSize = sizeof( dvi );

        HRESULT hr = pfDllGetVersion( &dvi );
        if ( SUCCEEDED( hr ) )
        {
            *pdwVersion = PACKVERSION( dvi.dwMajorVersion, dvi.dwMinorVersion );
            return true;
        }
    }

    return false;
}

FARPROC CDllLoader::GetProcAddress( LPCSTR lpProcName )
{
    return ( m_hModule != 0 ) ? ::GetProcAddress( m_hModule, lpProcName ) : 0;
}
