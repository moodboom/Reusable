// MDM 1/2/2007
// Is this redundant in VC8?  No, but we need to set Project->Properties->Config Properties->Linker->Input->Ignore all default libs = YES.
// No, that didn't quite work out.  Hrmph.

// MDM	1/22/2001 11:43:08 AM
// This file forces the correct library linkage order, which can be
// a problem in MFC projects when combining MFC and C code.  To summarize
// the problem (this is from MSDN Q148653):
// 
//		The CRT libraries use weak external linkage for the new, delete, and 
//		DllMain functions. The MFC libraries also contain new, delete, and 
//		DllMain functions, which requires MFC to be linked before the CRT 
//		libraries.
//
// There are no less than six versions of the libraries, depending on
// settings for debug/retail, static/dynamic MFC linking, and unicode/ANSI.
// Also single/multi-threaded!
//
#ifndef FORCE_CORRECT_MFC_C_LINKAGE_H
#define FORCE_CORRECT_MFC_C_LINKAGE_H


#ifndef _AFX_NOFORCE_LIBS

/////////////////////////////////////////////////////////////////////////////
// Win32 libraries

#ifndef _AFXDLL
	#ifndef _UNICODE
		#ifdef _DEBUG
			#pragma comment(lib, "nafxcwd.lib")
		#else
			#pragma comment(lib, "nafxcw.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "uafxcwd.lib")
		#else
			#pragma comment(lib, "uafxcw.lib")
		#endif
	#endif
#else
	#ifndef _UNICODE
		#ifdef _DEBUG
			#pragma comment(lib, "mfc42d.lib")
			#pragma comment(lib, "mfcs42d.lib")
		#else
			#pragma comment(lib, "mfc42.lib")
			#pragma comment(lib, "mfcs42.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "mfc42ud.lib")
			#pragma comment(lib, "mfcs42ud.lib")
		#else
			#pragma comment(lib, "mfc42u.lib")
			#pragma comment(lib, "mfcs42u.lib")
		#endif
	#endif
#endif

#ifdef _DLL
	#if !defined(_AFX_NO_DEBUG_CRT) && defined(_DEBUG)
		#pragma comment(lib, "msvcrtd.lib")
	#else
		#pragma comment(lib, "msvcrt.lib")
	#endif
#else
#ifdef _MT
	#if !defined(_AFX_NO_DEBUG_CRT) && defined(_DEBUG)
		#pragma comment(lib, "libcmtd.lib")
		#pragma comment(lib, "libcpmtd.lib")	// MDM Added 1/2/2006
	#else
		#pragma comment(lib, "libcmt.lib")
	#endif
#else
	#if !defined(_AFX_NO_DEBUG_CRT) && defined(_DEBUG)
		#pragma comment(lib, "libcd.lib")
	#else
		#pragma comment(lib, "libc.lib")
	#endif
#endif
#endif

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comctl32.lib")

// force inclusion of NOLIB.OBJ for /disallowlib directives
#pragma comment(linker, "/include:__afxForceEXCLUDE")

// force inclusion of DLLMODUL.OBJ for _USRDLL
#ifdef _USRDLL
#pragma comment(linker, "/include:__afxForceUSRDLL")
#endif

// force inclusion of STDAFX.OBJ for precompiled types
#ifdef _AFXDLL
#pragma comment(linker, "/include:__afxForceSTDAFX")
#endif

#endif //!_AFX_NOFORCE_LIBS


#endif	// FORCE_CORRECT_MFC_C_LINKAGE_H
