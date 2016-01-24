#ifndef MIN_MAX_FIX_H
#define MIN_MAX_FIX_H

//--------------------------------------------------------------------//
// min/max have been problematic in the past, as there are multiple 
// definitions in MS and STL headers.  This section fixes things up
// for VC 6 and VC 7 and beyond.  From here on out, ALWAYS USE THESE 
// for min/max:
//
//		std::min( 1, 2 )
//		std::max( 1, 2 )
//
// Got help on this from here:
//
//		http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&safe=active&threadm=3C5024EB.5CC4D8BC%40yahoo.co.uk&rnum=1&prev=/groups%3Fq%3Dafxtempl.h%2Bmin%26hl%3Den%26lr%3D%26ie%3DUTF-8%26safe%3Dactive%26as_qdr%3Dall%26selm%3D3C5024EB.5CC4D8BC%2540yahoo.co.uk%26rnum%3D1
//
//-------------------------------------------------------------------//
#ifdef WIN32

	#if _MFC_VER < 0x0700

      // Bypass the non-standard version in Microsoft SDK\include\WinDef.h...
      #ifndef NOMINMAX
		   #define NOMINMAX
      #endif
		#ifdef max
			#undef max
			#undef min
		#endif

		#pragma warning(disable : 4786)	// Silence annoying STL long-names whining from MS Compiler

		// VS 6 does not have std::min/max because it collided with
		// some internal MS macro.  Here, we define it, but only
		// inside the std namespace.  Then, using std::min/max()
		// works fine...
		#include <algorithm>

      #define max(a,b)            (((a) > (b)) ? (a) : (b))
      #define min(a,b)            (((a) < (b)) ? (a) : (b))

      /*
		using std::_cpp_max;
		using std::_cpp_min;

		namespace std
		{
			#define max _cpp_max
			#define min _cpp_min
		}
      */

	#else

		#define NOMINMAX

		// VS 7 and above properly define std::min/max(), but 
		// there is apparently still legacy code that does not
		// include the std:: qualifier.  This helps that code
		// compile.  Use std::min/max() in new code, for compatibility.
		#include <algorithm>
		using std::min;
		using std::max;
		
		// Other stuff we've needed.  Not working under VS 7?
		// using std::acos;

	#endif

#else
	
	#include <algorithm>
	using std::min;
	using std::max;

#endif
//-------------------------------------------------------------------//


#endif // #define MIN_MAX_FIX_H
