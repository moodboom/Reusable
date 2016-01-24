// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__62E26F40_A5F5_11D5_BCC1_00D0B7E64F2B__INCLUDED_)
#define AFX_STDAFX_H__62E26F40_A5F5_11D5_BCC1_00D0B7E64F2B__INCLUDED_


// -------------------------------------------------
// MDM This sets the target OS of the application.
// -------------------------------------------------

// MDM	2/24/2005 1:30:34 PM
// Used to target Win98 and Win NT 4 (0x0400).
// We're moving up to requiring Windows 2000 (0x0500).
#define _WIN32_WINNT 0x0500 
#undef WINVER
#define WINVER 0x0500
// -------------------------------------------------


// MDM 12/29/05 Added.
#define _MFC_VER 0x0700
#include <min_max_fix.h>

#include <ForceCorrect_MFC_C_Linkage.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__62E26F40_A5F5_11D5_BCC1_00D0B7E64F2B__INCLUDED_)
