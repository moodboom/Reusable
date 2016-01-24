//-------------------------------------------------------------------//
// ustring class
//-------------------------------------------------------------------//
// Here's one way to handle UNICODE in STL.
// We create the [ustring] string class.
// It is derived from either the [string] or [wstring] STL class,
// depending on the current state of the _UNICODE #define.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined(USTRING_H)
#define USTRING_H

#ifdef WIN32
   #include <min_max_fix.h>
#endif

#include "STLExtensions.h"			// For vector, string, StandardTypes, etc.
#include <string.h>                             // for strlen

#ifdef WIN32

   #include <stdio.h>					// These are for va_arg et al, used by ustring::printf()
   #include <stdarg.h>

   #include <windows.h>
   #include <TCHAR.h>

#else
// #elif defined LINUX

   // MDM Time to patch some MS nastiness...

   #include <stdlib.h>
   #include <stdarg.h>

   #ifdef _UNICODE

      typedef wchar_t TCHAR;
      #define _T(a) aL

      #define _ltot( n, str, radix ) wsprintf( str, "%d", n )
      #define _tcsclen wstrlen

   #else

      typedef char TCHAR;
      #define _T(a) a

      #define _ltot( n, str, radix ) sprintf( str, "%d", n )
      #define _tcsclen strlen

   #endif

   typedef const TCHAR* LPCTSTR;

#endif


class ustring: 
	public 
		#ifdef _UNICODE
			std::wstring
		#else
			std::string
		#endif
{
	typedef 
		#ifdef _UNICODE
			std::wstring
		#else
			std::string
		#endif
		inherited;

public:
	
	//-------------------------------------------------------------------//
	// ustring()																			//
	//-------------------------------------------------------------------//
	// I SOLVED THE INHERITANCE LOSS-OF-FUNCTIONALITY ISSUE!
	// 
	//	When you inherit, base class operator+() et al, as well as constructors, will 
	// not work with derived class, because you have a different class type.
	// 
	// *IF* you provide this function, it will "patch up" a lot of the functionality.  
	// The conversions can be done for you then.
	// 
	//		Derived::Derived( Base );
	// 
	// For example, this lets you do things like:
	//
	//		ustring( strSub + _T("append") ).c_str()
	//
	// without having to define a specific operator+() et al.
	//
	// WARNING: Obviously, you will lose any data stored only in the derived class 
	// when you go through this function.  This only makes sense when the derived 
	// class is for functionality expansion only, not data storage expansion.
	//-------------------------------------------------------------------//
	ustring( 
		#ifdef _UNICODE
			std::wstring strSource
		#else
			std::string strSource
		#endif
	) :
		// Call base class.
		inherited( strSource )
	{
	}


	//-------------------------------------------------------------------//
	// ustring()																			//
	//-------------------------------------------------------------------//
	ustring( LPCTSTR szInit = 0 )
	:
		// Call base class (with a little more smarts added...)
		inherited( szInit ? szInit : _T("") )
	
	{}


	//-------------------------------------------------------------------//
	// operator=()																			//
	//-------------------------------------------------------------------//
	// This prevents the "operator = is ambiguous" error.  Not sure
	// exactly which versions are colliding...
	// We also snuck in the NULL check to avoid crashes in MS's STL...
	//-------------------------------------------------------------------//
	const ustring& operator=( LPCTSTR pszSource )
	{
		if ( pszSource )
			assign( pszSource );
		else
			clear();

		return *this;
	}


	//-------------------------------------------------------------------//
	// clear()																				//
	//-------------------------------------------------------------------//
	// MDM	4/17/2003 2:16:23 PM
	// Why doesn't inherited::clear() work??
	// Did MS not provide it??
	//-------------------------------------------------------------------//
	void clear()
	{
		resize( 0 );
	}


        #ifdef WIN32
            // This lets ya chuck numbers at the end
            // of the string.  I liked using an
            // operator+= override, but it interfered
            // with the version for strings, nicht gut.
            ustring& AppendNumber( Long lNumber )
            {
                    TCHAR szNumberBuffer[20];
                    _ltot( lNumber, szNumberBuffer, 10 );
                    append( (const TCHAR*)szNumberBuffer );
                    return *this;
            }
        #endif

	bool operator==( ustring& strCompare )
	{
		// Use a case-insensitive compare for ==.
		return ( ucompare( strCompare ) == 0 );
	}

	int ucompare( 
		ustring& strCompare,
		bool		bCaseSensitive	= false
	) {
            // Q_UNUSED(bCaseSensitive);
		// TO DO
		// Switch this to a case-insensitive version.
		return compare( strCompare );
	}

        #ifdef WIN32
            int printf( int nCharactersNeeded, TCHAR* format ... )
            {
                // We allocate the space we will need.
                nCharactersNeeded = std::max( nCharactersNeeded, (int)_tcsclen( format ) + 20 );

                va_list variableList;
                va_start(variableList, format);
                TCHAR* workingString = new TCHAR[ nCharactersNeeded + 1 + 1 ];

                // Do the work!
                // Note that _vs[n]tprintf() is available in WIN32 but perhaps not elsewhere...
                int nReturn;
                #ifdef _UNICODE
                        nReturn = vswprintf(workingString,format,variableList);
                #else
                        nReturn = vsprintf(workingString,format,variableList);
                #endif

                // Did we have enough space?
                ASSERT( nReturn >= 0 );

                // Clean the array first!
                erase();

              // NOTE: nReturn will be 0 for empty strings, that is OK,
              // but we don't need to copy anything.
                if ( nReturn > 0 )
                {
                    // Copy the data back.

                    // Make sure we already have reserved at least exactly what we need.
                    if ( capacity() < (unsigned int)nReturn + 1 )
                           reserve( nReturn + 1 );

                    replace( 0, capacity(), workingString );
                }

                va_end(variableList);
                delete [] workingString;

                return nReturn;
            }
        #endif

	// MFC-only functionality
	#ifdef	_MFC_VER

                /*
		ustring( CString& strSource )
		{
			assign( LPCTSTR( strSource ) );
		}

		const ustring& operator=( CString& strSource )
		{
			assign( LPCTSTR( strSource ) );
			return *this;
		}
		*/

		bool LoadString( UINT uID )
		{
			// TO DO
			// This allocates a separate buffer, can
			// we use the ustring buffer directly?
			// c_str() will give us --const-- access
			// but then how do we update the new 
			// used and allocated sizes?
			const int cnBufferMax = 2000;
			TCHAR szBuffer[ cnBufferMax + 1 ];
			bool bReturn = (
				::LoadString( 
					::GetModuleHandle(NULL), 
					uID, 
					szBuffer, 
					cnBufferMax 
				) > 0
			);
			assign( szBuffer );
			return bReturn;
		}

	#endif	// _MFC_VER

};



//-------------------------------------------------------------------//
// new_XXX_from_XXX()																//
//-------------------------------------------------------------------//
// These conversion functions create a new buffer that contains
// the provided string, translated as needed.
// 
// These functions are the heart of the conversion classes that 
// follow.  The classes provide an internal buffer that you
// don't have to worry about in your conversion code.
//-------------------------------------------------------------------//
inline char* new_pchar_from_pwchar( const wchar_t* szSource )
{
    // Q_UNUSED(szSource);

    char* szBuffer;

   #ifdef WIN32

	// Call MBTWC with no output to get size needed.
	int nReqdSize = WideCharToMultiByte( 
		CP_ACP,		// ANSI
		0,				// dwFlags
		szSource,
		-1,
		0,				// o for output string, we just want length.
		0,
		0,
		0
	);
	
	// From the docs, the number returned "includes the byte for 
	// the null terminator".
	szBuffer = new char[ nReqdSize ];

	WideCharToMultiByte( 
		CP_ACP,		// code page = ANSI
		0,				// dwFlags
		szSource,
		-1,
		szBuffer,
		nReqdSize,
		"?",
		0
	);

   #elif defined LINUX

      // TO DO
      ASSERT( false );
      szBuffer = 0;

   #elif defined OSX

      // TO DO
      ASSERT( false );
      szBuffer = 0;

   #endif

	return szBuffer;

};


inline char* new_UTF8_from_pwchar( const wchar_t* szSource )
{
    // Q_UNUSED(szSource);

    char* szBuffer;

   #ifdef WIN32

	// Call MBTWC with no output to get size needed.		
	// From the docs, the number returned "includes the byte for 
	// the null terminator".
	int nReqdSize = WideCharToMultiByte( 
		CP_UTF8,		// code page = UTF8
		0,				// dwFlags
		szSource,
		-1,
		0,				// no output buffer, we just want length
		0,				// output buffer size, 0 to get length
		0,
		0
	);

	   // Allocate an appropriately-sized buffer.
	   szBuffer = new char[ nReqdSize ];

	   WideCharToMultiByte( 
		   CP_UTF8,		// code page = UTF8
		   0,				// dwFlags
		   szSource,
		   -1,
		   szBuffer,
		   nReqdSize,
		   0,
		   0
	   );


   #elif defined LINUX

      // TO DO
      ASSERT( false );
      szBuffer = 0;

   #elif defined OSX

      // TO DO
      ASSERT( false );
      szBuffer = 0;

   #endif

	return szBuffer;

};


inline wchar_t* new_pwchar_from_pchar( const char* szSource )
{
	wchar_t* szBuffer;
	
	// Get the size needed and allocate.
        int nReqdSize = strlen( szSource ) + 1;
	szBuffer = new wchar_t[ nReqdSize ];

   #ifdef WIN32

	   MultiByteToWideChar( 
		   CP_ACP,		// code page = ANSI
		   0,				// dwFlags
		   szSource,
		   -1,
		   szBuffer,
		   nReqdSize
	   );

   #elif defined LINUX

      // TO DO
      ASSERT( false );

   #elif defined OSX

      // TO DO
      ASSERT( false );

   #endif

	return szBuffer;

};


inline wchar_t* new_pwchar_from_UTF8( const char* szSource )
{
	wchar_t* szBuffer;

	// Get the size needed and allocate.
	int nReqdSize = strlen( szSource ) + 1;
	szBuffer = new wchar_t[ nReqdSize ];

   #ifdef WIN32

	   MultiByteToWideChar( 
		   CP_UTF8,		// code page = UTF8
		   0,				// dwFlags
		   szSource,
		   -1,
		   szBuffer,
		   nReqdSize
	   );

   #elif defined LINUX

      // TO DO
      ASSERT( false );

   #elif defined OSX

      // TO DO
      ASSERT( false );

   #endif

	return szBuffer;

};


// We need these functions ONLY FOR HEBREW environments.
inline wchar_t* new_pwchar_from_Hebrew( const char* szSource )
{
	wchar_t* szBuffer;
	
	// Get the size needed and allocate.
	int nReqdSize = strlen( szSource ) + 1;
	szBuffer = new wchar_t[ nReqdSize ];

   #ifdef WIN32

	   MultiByteToWideChar( 
		   1255,			// code page = HEBREW, from DBDAOINT.H
		   0,				// dwFlags
		   szSource,
		   -1,
		   szBuffer,
		   nReqdSize
	   );

   #elif defined LINUX

      // TO DO
      ASSERT( false );

   #elif defined OSX

      // TO DO
      ASSERT( false );

   #endif

	return szBuffer;
};
inline char* new_Hebrew_from_pwchar( const wchar_t* szSource )
{
    // Q_UNUSED(szSource);

    char* szBuffer;

   #ifdef WIN32

	   // Call MBTWC with no output to get size needed.		
	   // From the docs, the number returned "includes the byte for 
	   // the null terminator".
	   int nReqdSize = WideCharToMultiByte( 
		   1255,			// code page = HEBREW, from DBDAOINT.H
		   0,				// dwFlags
		   szSource,
		   -1,
		   0,				// no output buffer, we just want length
		   0,				// output buffer size, 0 to get length
		   0,
		   0
	   );

	   // Allocate an appropriately-sized buffer.
	   szBuffer = new char[ nReqdSize ];

	   WideCharToMultiByte( 
		   1255,			// code page = HEBREW, from DBDAOINT.H
		   0,				// dwFlags
		   szSource,
		   -1,
		   szBuffer,
		   nReqdSize,
		   0,
		   0
	   );

   #elif defined LINUX

      // TO DO
      ASSERT( false );

   #elif defined OSX

      // TO DO
      ASSERT( false );

   #endif

	return szBuffer;

};


//-------------------------------------------------------------------//
// XXX_to_XXX()																		//
//-------------------------------------------------------------------//
// These string conversion classes are lightweight wrappers
// around all combinations of string conversions.  You can use them
// for dynamic conversion as needed - they create and delete their
// own internal buffers as needed.
//
//	SUMMARY OF AVAILABLE CLASSES
//	-------------------------------------------------------------------
//	pwchar_to_pchar		The true converters.
// pwchar_to_UTF8
// pchar_to_pwchar
// UTF8_to_pwchar
//
// pchar_to_UTF8			Pseudo-conversion.
// UTF8_to_pchar
//
// TCHAR_to_pchar			These all map to other versions depending
//	TCHAR_to_pwchar		on whether we have [#define _UNICODE] or not.
//	TCHAR_to_UTF8	
//	TCHAR_to_TUTF8	
//	pchar_to_TCHAR	
//	pchar_to_TUTF8	
//	pwchar_to_TCHAR	
//	pwchar_to_TUTF8	
//	UTF8_to_TCHAR	
//	UTF8_to_TUTF8	
//	TUTF8_to_TCHAR	
//	TUTF8_to_UTF8	
//	TUTF8_to_pchar	
//	TUTF8_to_pwchar	
//	-------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------
// Example usage:
//
//		CString strTest = _T("Test");
//		sendSQLcharStr( TCHAR_to_pchar( LPCTSTR( strTest ) ) );
//
// The TCHAR_to_pchar object here creates a buffer, converts the string
// into it as needed, returns a pointer to the buffer, and cleans it 
// up when the object goes out of scope.  In the example code, this is as
// soon as sendSQLcharStr() returns.  Nice and clean!
// ----------------------------------------------------------------------
//
//
// Three distinct string types are handled:
//
//		Type				Nomenclature	Desc
//		-------------------------------------------
//		char*				pchar				Old-school
//		wchar_t*			pwchar			UCS-2 - New-school (2 bytes per char, provides 
//													mult-lang. support in all Windows post-9x)
//		char* (UTF-8)	UTF8				Non-M$ style, used by Oracle, browsers, etc. 
//													- null-terminated, optional mult-byte per char
//
// In addition, two "morphing" types are provided.  These merely
// do one of the above conversions, depending on whether _UNICODE is
// #define'd or not.
//
//		Type				Nomenclature	Desc
//		-------------------------------------------
//		TCHAR				TCHAR				Typical Windows strings - they morph from char 
//													to wchar_t if _UNICODE is defined
//		char*	(TUTF8)	TUTF8				This is used to morph from char to UTF8 if 
//													_UNICODE is defined
//
// Note that some conversions need to "fall through"; eg converting
// from TCHAR to char when UNICODE is not defined.  In these
// cases, the class name is #define'd away.  This is the lightest-weight
// solution, but it means in those cases, you can't truly create
// a conversion class object.  Dynamic conversions (as in the example)
// work fine, which is the real intention.
//
// Note that the pchar <-> UTF-8 conversions use char* on both
// sides.  But one allows multi-byte encoding and one does not.
//
// Note that whenever a wide to single-byte conversion "fails",
// Windows will attempt a best-fit, and if that fails, we typically 
// insert the '?' char.
//
// Note that the guts of these classes call our global functions that
// can also be used directly, as needed.
//
//-------------------------------------------------------------------//
class pwchar_to_pchar
{
public:
	pwchar_to_pchar( const wchar_t* szSource )
	{
		m_szBuffer = new_pchar_from_pwchar( szSource );
	}

	operator const char*() const
	{
		return m_szBuffer;
	}

	// For badboy code that asks for non-const param.
	operator char*()
	{
		return m_szBuffer;
	}

	~pwchar_to_pchar()
	{
		delete[] m_szBuffer;
	}

protected:
	char* m_szBuffer;
};


class pwchar_to_UTF8
{
public:
	pwchar_to_UTF8( const wchar_t* szSource )
	{
		m_szBuffer = new_UTF8_from_pwchar( szSource );
	}

	operator const char*() const
	{
		return m_szBuffer;
	}

	~pwchar_to_UTF8()
	{
		delete[] m_szBuffer;
	}

protected:
	char* m_szBuffer;
};


class pchar_to_pwchar
{
public:
	pchar_to_pwchar( const char* szSource )
	{
		m_szBuffer = new_pwchar_from_pchar( szSource );
	}

	operator wchar_t*() const
	{
		return m_szBuffer;
	}

	~pchar_to_pwchar()
	{
		delete[] m_szBuffer;
	}

protected:
	wchar_t* m_szBuffer;
};


class UTF8_to_pwchar
{
public:
	UTF8_to_pwchar( const char* szSource )
	{
		m_szBuffer = new_pwchar_from_UTF8( szSource );
	}

	operator wchar_t*() const
	{
		return m_szBuffer;
	}

	~UTF8_to_pwchar()
	{
		delete[] m_szBuffer;
	}

protected:
	wchar_t* m_szBuffer;
};


// ------------------------------------------------------------------
// Here, we #define different string conversions that map to either
// nothing, or another form of string conversion, depending on the
// UNICODE #define.

// Eventually, we could set this up so it actually does a conversion,
// converting multi-byte chars to '?' chars.  Not very useful though.
#define pchar_to_UTF8
#define UTF8_to_pchar

#ifdef _UNICODE
	#define TCHAR_to_pchar		pwchar_to_pchar	
	#define TCHAR_to_pwchar		
	#define TCHAR_to_UTF8		pwchar_to_UTF8     
	#define TCHAR_to_TUTF8		pwchar_to_UTF8
	#define pchar_to_TCHAR		pchar_to_pwchar    
	#define pchar_to_TUTF8		pchar_to_UTF8
	#define pwchar_to_TCHAR		
	#define pwchar_to_TUTF8		pwchar_to_UTF8
	#define UTF8_to_TCHAR		UTF8_to_pwchar     
	#define UTF8_to_TUTF8		     
	#define TUTF8_to_TCHAR		UTF8_to_pwchar
	#define TUTF8_to_UTF8		
	#define TUTF8_to_pchar		UTF8_to_pchar    
	#define TUTF8_to_pwchar		UTF8_to_pwchar
#else
	#define TCHAR_to_pchar		
	#define TCHAR_to_pwchar		pchar_to_pwchar    
	#define TCHAR_to_UTF8		pchar_to_UTF8      
	#define TCHAR_to_TUTF8		
	#define pchar_to_TCHAR		
	#define pchar_to_TUTF8		
	#define pwchar_to_TCHAR		pwchar_to_pchar    
	#define pwchar_to_TUTF8		pwchar_to_pchar    
	#define UTF8_to_TCHAR		UTF8_to_pchar      
	#define UTF8_to_TUTF8		UTF8_to_pchar      
	#define TUTF8_to_TCHAR		
	#define TUTF8_to_UTF8		pchar_to_UTF8      
	#define TUTF8_to_pchar		
	#define TUTF8_to_pwchar		pchar_to_pwchar    
#endif
// ------------------------------------------------------------------


// This nice little helper fixes the deficiencies of strncpy(), 
// which is my BIGGEST PET PEEVE IN C.  You have to make it
// a macro, since it needs to do a sizeof() on the actual
// destination string array.
#define AutoStrncpy(dest,src) _tcsncpy(dest,src,sizeof(dest)/sizeof TCHAR);dest[sizeof(dest)/sizeof TCHAR-1]=_T('\0');


#endif // !defined(USTRING_H)
