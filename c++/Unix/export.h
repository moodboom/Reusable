//-------------------------------------------------------------------//
// export.h
//-------------------------------------------------------------------//
// Export macros for C++ / C / Windows / Unix environments.
// 
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef _export_h
#define _export_h


/*
/////////////////////////////////////////////////////
// Set up for  
//
//		[ (cpp && WIN32) || ( c && _MS_VER ) || c ]  
//
// environment as needed.
/////////////////////////////////////////////////////	*/

#if defined( __cplusplus ) && defined( WIN32 )

#define WINEXPORT	extern "C" __declspec (dllexport) 

#define WINMANGLE __cdecl

#define WINEXTERN extern "C" 
#define CFUNCTION extern "C" 

#elif defined(_MS_VER) || defined(WIN32)

#define WINEXPORT extern __declspec (dllexport)

#define WINMANGLE __cdecl

#define WINEXTERN extern 
#define CFUNCTION

#else

#define WINEXPORT 
#define WINMANGLE 
#define WINEXTERN extern
#define CFUNCTION

#endif


#endif	// _export_h