#include "stdafx.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#include "Blowfish.h"


//-------------------------------------------------------------------//
// InitializeSubkeys()																//
//-------------------------------------------------------------------//
// Blowfish is optimized for many operations with one key.  It requires
// initialization, in the form of setting up a large database of
// precomputed subkeys manipulated with a specific key.  The subkey
// array is then used for subsequent en/decryption.
// This routine initializes blowfish for the given key; that is, it creates
// the subkeys array and manipulates it with the given key.  It should be
// called any time en/decryption is desired with a new key.  However,
// once the subkey array has been generated for a given key, the blowfish
// en/decryption routines can be called without need for initialization
// until the key needs to be changed.
// Note that initialization is accomplished through the loading of a
// Windows user defined resource.
//-------------------------------------------------------------------//
bool BlowFish::InitializeSubKeys(
	unsigned char* pszKey,
	unsigned int   nKeyLen 
) {

	bool bResult;
	
	ASSERT( !bInitialized );
	bInitialized = true;

   // Load the standard pre-computed subkey arrays.
	// These arrays are composed of the hexidecimal digits of
	// pi (bit-mangled by Intel byte ordering, oh well).
   GLOBALHANDLE hStdSubKeys = LoadResource( 
		NULL,
      FindResource( 
			NULL, 
         MAKEINTRESOURCE(BLOWFISHDATA),	// Used to be ["BLOWFISHDATA",] for VC6
			_T( "BINARY" )
		) 
	);
	
	// If this fails, check the resource file to make sure
	// it includes the BLOWFISHDATA binary data block.
	VERIFY( bResult = ( hStdSubKeys != NULL ) );

   if ( bResult ) {

		// Lock it.
		BLOWFISH_SUBKEYS* pStdSubKeys = (BLOWFISH_SUBKEYS*) LockResource( 
			hStdSubKeys
		);

		// Copy to our internal subkey array.
		memcpy(
			&SubKeys,
			pStdSubKeys,
			sizeof BLOWFISH_SUBKEYS
		);

		// Unlock and release std subkey array.
		// According to the docs, we don't need to do this, but
		// hopefully we'll be freeing it sooner than it would
		// otherwise be freed.
		UnlockResource( hStdSubKeys );
		FreeResource( hStdSubKeys );

		// The std subkey array consists of the decimals of pi.  Who
		// cares if they are in Intel byte order??!?	 As long as we 
		// are consistent.  Theoretically, this means we are not using
		// the official BlowFish algorithm, but the change should not
		// affect the strength of the algorithm.
		/*
		// For the given data file, we want to process things in byte
		// order, not as rearranged in a longword.
		// TO DO: Convert subkey source file so there
		// is no need for this malarchy.
		for ( i = 0; i < N + 2; ++i ) {

			// Resort P arrray data for Intel.
			lpSubKeys->P[i] = ((lpSubKeys->P[i] & 0xFF000000) >> 24) |
									((lpSubKeys->P[i] & 0x00FF0000) >>  8) |
									((lpSubKeys->P[i] & 0x0000FF00) <<  8) |
									((lpSubKeys->P[i] & 0x000000FF) << 24);

		}

		for (i = 0; i < 4; ++i) {
			for (j = 0; j < 256; ++j) {

				// Resort S arrray data for Intel.
				lpSubKeys->S[i][j] = ((lpSubKeys->S[i][j] & 0xFF000000) >> 24) |
											((lpSubKeys->S[i][j] & 0x00FF0000) >>  8) |
											((lpSubKeys->S[i][j] & 0x0000FF00) <<  8) |
											((lpSubKeys->S[i][j] & 0x000000FF) << 24);

			}
		}
		*/

		unsigned short	i;
		unsigned short j;
		unsigned short k;
		unsigned long  data;
		unsigned long  datal;
		unsigned long  datar;
   
		j = 0;
		for (i = 0; i < N + 2; ++i) {
      
			data = 0x00000000;
      
			for (k = 0; k < 4; ++k) {

				// pszKey must be unsigned!
				data = (data << 8) | pszKey[j];

				j = j + 1;
				if (j >= nKeyLen ) {
					j = 0;
				}
			}
			SubKeys.P[i] = SubKeys.P[i] ^ data;
		}

		datal = 0x00000000;
		datar = 0x00000000;

		for (i = 0; i < N + 2; i += 2) {
      
			Encipher(&datal, &datar);

			SubKeys.P[i] = datal;
			SubKeys.P[i + 1] = datar;
		}

		for (i = 0; i < 4; ++i) {
			for (j = 0; j < 256; j += 2) {

				Encipher(&datal, &datar);

				SubKeys.S[i][j] = datal;
				SubKeys.S[i][j + 1] = datar;
			}
		}

	}
   
   return bResult;

}


//-------------------------------------------------------------------//
// Encipher()																			//
//-------------------------------------------------------------------//
void BlowFish::Encipher(unsigned long *xl, unsigned long *xr)
{
   unsigned long  Xl;
   unsigned long  Xr;
   unsigned long  temp;
   short          i;

   Xl = *xl;
   Xr = *xr;

   for (i = 0; i < N; ++i) {
      Xl = Xl ^ SubKeys.P[i];
      Xr = F(Xl) ^ Xr;

      temp = Xl;
      Xl = Xr;
      Xr = temp;
   }

   temp = Xl;
   Xl = Xr;
   Xr = temp;

   Xr = Xr ^ SubKeys.P[N];
   Xl = Xl ^ SubKeys.P[N + 1];

   *xl = Xl;
   *xr = Xr;
}


//-------------------------------------------------------------------//
// Decipher()																			//
//-------------------------------------------------------------------//
void BlowFish::Decipher(unsigned long *xl, unsigned long *xr)
{
   unsigned long  Xl;
   unsigned long  Xr;
   unsigned long  temp;
   short          i;

   Xl = *xl;
   Xr = *xr;

   for (i = N + 1; i > 1; --i) {
      Xl = Xl ^ SubKeys.P[i];
      Xr = F(Xl) ^ Xr;

      /* Exchange Xl and Xr */
      temp = Xl;
      Xl = Xr;
      Xr = temp;
   }

   /* Exchange Xl and Xr */
   temp = Xl;
   Xl = Xr;
   Xr = temp;

   Xr = Xr ^ SubKeys.P[1];
   Xl = Xl ^ SubKeys.P[0];

   *xl = Xl;
   *xr = Xr;
}


//-------------------------------------------------------------------//
// F()																					//
//-------------------------------------------------------------------//
unsigned long BlowFish::F(unsigned long x)
{
   unsigned short a;
   unsigned short b;
   unsigned short c;
   unsigned short d;
   unsigned long  y;

   // MDM Added typecast.
   d = (unsigned short) ( x & 0x00FF );

   x >>= 8;

   // MDM Added typecast.
   c = (unsigned short) ( x & 0x00FF );

   x >>= 8;

   // MDM Added typecast.
   b = (unsigned short) ( x & 0x00FF );

   x >>= 8;

   // MDM Added typecast.
   a = (unsigned short) ( x & 0x00FF );

   y = SubKeys.S[0][a] + SubKeys.S[1][b];
   y = y ^ SubKeys.S[2][c];
   y = y + SubKeys.S[3][d];

   return y;
}


//-------------------------------------------------------------------//
// Encrypt()																			//
//-------------------------------------------------------------------//
// This function directly encrypts a buffer using the current
// subkey arrays.
// It does not encrypt and returns false if the buffer length 
// is not a multiple of 8.
//-------------------------------------------------------------------//
bool BlowFish::Encrypt( 
	void*		pBuffer,
	int		nBufferLength
) {

	if ( nBufferLength % 8 != 0 )
		return false;

	unsigned char* pCharBuffer = (unsigned char*) pBuffer;
	for ( int i = 0; i < nBufferLength; i += 8 )

		// Encrypt.
		Encipher(
			(unsigned long *) ( pCharBuffer + i ), 
			(unsigned long *) ( pCharBuffer + i + 4 )
		);
	
	return true;

}


//-------------------------------------------------------------------//
// Decrypt()																			//
//-------------------------------------------------------------------//
// This function directly decrypts a buffer using the current
// subkey arrays.
// It does not decrypt and returns false if the buffer length 
// is not a multiple of 8.
//-------------------------------------------------------------------//
bool BlowFish::Decrypt( 
	void*		pBuffer,
	int		nBufferLength
) {

	if ( nBufferLength % 8 != 0 )
		return false;

	unsigned char* pCharBuffer = (unsigned char*) pBuffer;
	for ( int i = 0; i < nBufferLength; i += 8 )

		// Decrypt.
		Decipher(
			(unsigned long *) ( pCharBuffer + i ), 
			(unsigned long *) ( pCharBuffer + i + 4 )
		);
	
	return true;

}


//-------------------------------------------------------------------//
// StreamEncrypt()																			//
//-------------------------------------------------------------------//
// This function directly encrypts a buffer using the current
// subkey arrays.
// 
// In addition, this function XOR's the previous block with
// the current, simulating a stream cipher with Blowfish.
// There's really no true additional security added here (from the
// standpoint that the security lies in the key); this is just an 
// easy way to make it more difficult to randomly change bits in
// the encrypted buffer without frying the entire buffer downstream.
// 
// You must en/decrypt the entire buffer as a single
// block each time, since this is a pseudo-stream cipher.
//
// It does not encrypt and returns false if the buffer length 
// is not a multiple of 8.
//-------------------------------------------------------------------//
bool BlowFish::StreamEncrypt( 
	void*		pBuffer,
	int		nBufferLength
) {

	if ( nBufferLength < 8 || nBufferLength % 8 != 0 )
		return false;

	unsigned char* pCharBuffer = (unsigned char*) pBuffer;

	// Encrypt the first block.
	Encipher(
		(unsigned long *) ( pCharBuffer ), 
		(unsigned long *) ( pCharBuffer + 4 )
	);

	for ( int i = 8; i < nBufferLength; i += 8 ) {

		// XOR the current block with
		// the previously encrypted block.
		*((uHuge*)( pCharBuffer + i )) = 
				*((uHuge*)( pCharBuffer + i		)) 
			^	*((uHuge*)( pCharBuffer + i - 8	));
		
		// Encrypt.
		Encipher(
			(unsigned long *) ( pCharBuffer + i ), 
			(unsigned long *) ( pCharBuffer + i + 4 )
		);

	}
	
	return true;

}


//-------------------------------------------------------------------//
// StreamDecrypt()																	//
//-------------------------------------------------------------------//
// This function directly decrypts a buffer using the current
// subkey arrays.
// 
// In addition, this function XOR's the previous block with
// the current, simulating a stream cipher with Blowfish.
// There's really no true additional security added here (from the
// standpoint that the security lies in the key); this is just an 
// easy way to make it more difficult to randomly change bits in
// the encrypted buffer without frying the entire buffer downstream.
// 
// You must en/decrypt the entire buffer as a single
// block each time, since this is a pseudo-stream cipher.
//
// It does not decrypt and returns false if the buffer length 
// is not a multiple of 8.
//
// DEBUG
// This static can be used if you need to modify the database format
// of an existing database.  
//
//		static bool bOldRead = false;
//
// Use the following steps:
//
//		1) Add the var.
//		2) Update stream encryption function.
//		3) Create two sections of code within this function, one that 
//			is the current contents and one that is the updated
//			function.  Use the above bool to determine which to use.
//		4) Wipe your personal db.  Run the updated program (with the 
//			bool false ) to set up a new blank database that you can 
//			append to from the old database.  Then exit the program 
//			and copy your personal db to a new name or dir.
//		5) Add the // DEBUG line in to bool YourDatabase::Append().
//		6) Put a breakpoint on DBAppendDlg::OnAppendFrom().  Run the 
//			program and select db append.  Select the target db you
//			created in 4).  Then select the old db as the source.  When 
//			you hit the breakpoint, set bOldRead to true.  Then let 'er rip!
//
//-------------------------------------------------------------------//
bool BlowFish::StreamDecrypt( 
	void*		pBuffer,
	int		nBufferLength
) {

	if ( nBufferLength < 8 || nBufferLength % 8 != 0 )
		return false;

	unsigned char* pCharBuffer = (unsigned char*) pBuffer;

	uHuge uhThisEncryptedBlock;

	// Preserve the first block, encrypted.
	uHuge uhPreviousEncryptedBlock = *((uHuge*)( pCharBuffer ));

	// Decrypt the first block.  It is a special case that we
	// don't XOR.
	Decipher(
		(unsigned long *) ( pCharBuffer ), 
		(unsigned long *) ( pCharBuffer + 4 )
	);

	for ( int i = 8; i < nBufferLength; i += 8 ) {

		// Preserve this block, encrypted.
		uhThisEncryptedBlock = *((uHuge*)( pCharBuffer + i ));

		// Decrypt.
		Decipher(
			(unsigned long *) ( pCharBuffer + i ), 
			(unsigned long *) ( pCharBuffer + i + 4 )
		);
	
		// XOR the current block with
		// the previously encrypted block.
		*((uHuge*)( pCharBuffer + i )) = 
				*((uHuge*)( pCharBuffer + i		)) 
			^	uhPreviousEncryptedBlock;

		// Set up for the next block.
		uhPreviousEncryptedBlock = uhThisEncryptedBlock;

	}

	return true;

}
