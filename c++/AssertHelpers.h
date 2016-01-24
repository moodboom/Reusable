//-------------------------------------------------------------------//
//	AssertHelpers
//
//	This module attempts to provide ASSERT(), VERIFY(), and TRACE()
// functionality under a wide range of builds and platforms.
//
// ASSERT( test ) will break and notify if (test != true) in debug builds.
// It will evaporate away under release builds.

// VERIFY( test ) will always execute <test> but will only break
// and notify the user if (test != true) under debug builds.
//
//	TRACE( szFormat, ... ) will output to whatever std error output is
//	available.
// 
//	Copyright © 2011 A better Software.
//-------------------------------------------------------------------//

#ifndef ASSERT_HELPERS_H
#define ASSERT_HELPERS_H

#include "PragmaMessages.h"			// For convenience, often used together.

#ifndef ASSERT

	#ifdef _WIN32

        // #ifdef false	// what was wrong with ASSERT() on windows?  Q_WS_WIN
        #ifndef _DEBUG

            #define ASSERT(test) 
			#define TRACE(test)

		#else

			// Use _ASSERT()
			#ifndef _INC_CRTDBG
				#include <crtdbg.h>
			#endif // _INC_CRTDBG

			#define ASSERT(test) _ASSERT(test)
		#endif

    #else

        // QT for X11, Mac OS X, Embedded Linux
        // #if defined(Q_WS_X11) || defined(Q_WS_MAC) || defined(Q_WS_QWS)

        #ifdef QT_NO_DEBUG

            #define ASSERT(test)
            #define TRACE(test)

        #else

            // Sadly, Q_ASSERT kills off the app!  doh.
            // #define ASSERT(test) Q_ASSERT(test)
            // #define ASSERT(test) if (!(test)) {DebugBreak();}
            # define ASSERT(x) ((x) ? (void)0 : qWarning("ASSERT: \"%s\" in %s (%d)",#x,__FILE__,__LINE__))
            #define TRACE(test)

        #endif

	#endif

#endif // ASSERT

#ifndef VERIFY

	#ifdef _WIN32

		#ifdef _DEBUG
			#define VERIFY(test) ASSERT(test)
		#else
		
			// This pragma prevents warnings about "(test) == true;" type statements under release mode.
			#pragma warning(disable : 4553 )
		
			#define VERIFY(test) test

		#endif // _DEBUG

	#endif // _WIN32

#endif // VERIFY

#ifndef TRACE

	#ifdef _WIN32

           #ifndef _INC_CRTDBG
		#include <crtdbg.h>
	   #endif // _INC_CRTDBG

   	   #ifdef _DEBUG
		   #define TRACE(test) 	_CrtDbgReport( _CRT_WARN, NULL, NULL, NULL, test )
	   #else
		   #define TRACE(test)
	   #endif

        #endif // _WIN32

#endif // TRACE

#endif	// ASSERT_HELPERS_H
