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

#ifndef __DLLLOADER_H__
#define __DLLLOADER_H___

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CDllLoader

#define PACKVERSION( major, minor ) MAKELONG( minor, major )

class /*GUILIB_EXT_CLASS*/ CDllLoader
{
// Constructors
public:
    CDllLoader( LPCTSTR lpFileName, bool bAutoFree = true );
    ~CDllLoader();

// Operations
public:
    bool IsLoaded();
    bool GetVersion( DWORD* pdwVersion );
    FARPROC GetProcAddress( LPCSTR lpProcName );

// Implementation data
protected:
    HMODULE m_hModule;
    bool    m_bAutoFree;
};

/////////////////////////////////////////////////////////////////////////////
#endif  // !__DLLLOADER_H___
