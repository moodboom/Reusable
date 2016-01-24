#ifndef BLOWFISH_H
	#define BLOWFISH_H


/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module (or submodules):
//

#define BLOWFISHDATA                    21000

// Since this is a reusable class, we don't have access to a resource file.
//
// 1) Make sure that the above defines are "rolled into" your project's
//		resource file.  Select "Resource Set Includes" on the View menu and add 
//		this header file to the "Read-only symbol directives" listbox.
// 2) Make sure your project includes the associated resources in its
//		resource file.
//
// Also...
//
//	BLOWFISHDATA            BINARY  MOVEABLE PURE   "<path>Blowfish\\BLOWFISH.DAT"
// 
//	This line should be added to the project's resource file, in a
// custom section called "BINARY" (or you can cut and paste the item
// using the resource viewer, and then edit the path in its properties.)
// Or just import a resource, use BINARY type, and browse for the file, then
// rename the resource from IDR_... to BLOWFISHDATA.

// We want to exclude the remainder of the include file when dealing with App Studio.
#ifndef RC_INVOKED
/////////////////////////////////////////////////////////////////////////


#include "..\..\StandardTypes.h"				// For uHuge type


// Max key size for algorithms is 56 bytes/448 bits.
const short MAXKEYBYTES = 56;

// N represents the number of iterations through F()
// for each encrypted block.  16 is extremely secure.
const short N = 16;


typedef struct {

	unsigned long P[N + 2];
	unsigned long S[4][256];

} BLOWFISH_SUBKEYS;


class BlowFish {

protected:

	BLOWFISH_SUBKEYS SubKeys;

	inline unsigned long F(unsigned long x);

	inline void Encipher(
		unsigned long *xl, 
		unsigned long *xr
	);

	inline void Decipher(
		unsigned long *xl, 
		unsigned long *xr
	);

public:

	BlowFish()
	:
		
		// Init vars.
		bInitialized( false )

	{}

	~BlowFish() {}

	// Make sure that key is UNSIGNED!
	bool InitializeSubKeys(
		unsigned char* pszKey,
		unsigned int   nKeyLen 
	);

	bool bInitialized;

	bool Encrypt( 
		void*		pBuffer,
		int		nBufferLength
	);

	bool Decrypt( 
		void*		pBuffer,
		int		nBufferLength
	);

	bool StreamEncrypt( 
		void*		pBuffer,
		int		nBufferLength
	);

	bool StreamDecrypt( 
		void*		pBuffer,
		int		nBufferLength
	);

};

#endif	// RC_INVOKED
#endif // BLOWFISH
