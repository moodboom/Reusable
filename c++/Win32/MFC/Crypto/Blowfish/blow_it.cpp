#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "blowfish.h"
#include "blow_it.h"

// These var's need to be global.  HANDLE only needs
// to be visible to this module.
HGLOBAL                 hSubKeys = NULL;
BLOWFISH_SUBKEYS far *  lpSubKeys;


// Blowfish is optimized for many operations with one key.  It requires
// initialization, in the form of setting up a large database of
// precomputed subkeys manipulated with a specific key.  The subkey
// array is then used for subsequent en/decryption.
//
// This routine initializes blowfish for the given key; that is, it creates
// the subkeys array and manipulates it with the given key.  It should be
// called any time en/decryption is desired with a new key.  However,
// once the subkey array has been generated for a given key, the blowfish
// en/decryption routines can be called without need for initialization
// until the key needs to be changed.
//
// Note that initialization is accomplished through the loading of a
// Windows user defined resource, hence the Win in the name.

BOOL  BlowInitialize(
         unsigned char* lpszKey,
         unsigned int   nKeyLen )
{

   short             i;
   short             j;

   BOOL              bReturn;


   // We need to get the standard pre-computed subkey arrays.
   // They are located in a Windows user-defined resource.
   // See blowfish.rc.

   // Deallocate any previous subkeys, as they are specific to the old key.
   BlowFree();

   // Load the user defined resource.
   hSubKeys = LoadResource( AfxGetInstanceHandle(),
              FindResource( AfxGetInstanceHandle(), "BlowfishData", "BLOWFISHDATA" ) );

   // Lock it.
   lpSubKeys = (BLOWFISH_SUBKEYS far *)LockResource( hSubKeys );

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


   bReturn = InitializeBlowfish( lpszKey, nKeyLen );

   // Unlock subkey arrays 'til we need 'em.
   UnlockResource( hSubKeys );

   return bReturn;

}

// This routine encrypts the data in lpszSourceData and outputs it to the
// lpszDestData buffer.  These buffers can occupy the same memory.  The
// data can be of any type.  The end of the source data is determined
// exclsively by the value in lSourceLen.  The data itself may contain
// NULLs without risk of truncation.

// WARNING! Blowfish processing requires the cyphertext to have a length that
// is a multiple of 8 bytes.  Yes, this means that
//
//       ** THE CYPHERTEXT MAY BE LONGER THAN THE PLAINTEXT! **
//
// If the plaintext was not a mult of 8 bytes in length, make sure you
// specify  a dest buffer that can be expanded to fit the source
// buffer plus ( 8 - sourcelen % 8 ).

BOOL  BlowEncryptString(
         LPSTR          lpszSourceData,
         unsigned long  lSourceLen,
         LPSTR          lpszDestData )
{

   BOOL           bReturn = TRUE;

   unsigned long  lLeft32Bits;
   unsigned long  lRight32Bits;

   unsigned long  lStringPos;

   int            nNumLeftBytes;
   int            nNumRightBytes;


   // Lock subkey arrays.
   lpSubKeys = (BLOWFISH_SUBKEYS far *)LockResource( hSubKeys );

   if (     !lpszSourceData
         || !lpszDestData
         || lSourceLen <= 0               ) {

      bReturn = FALSE;

   } else {

      // Begin encrypting.

      // Loop through plaintext string, outputting to cyphertext.
      for ( lStringPos = 0; lStringPos < lSourceLen; lStringPos += 8 ) {

         lLeft32Bits  = 0;
         lRight32Bits = 0;

         // If we don't have full 8 bytes...
         if ( lSourceLen - lStringPos < 8 ) {

            // Determine number of bytes remaining.
            nNumLeftBytes = (int) ( lSourceLen - lStringPos );
            nNumRightBytes = nNumLeftBytes - 4;
            if ( nNumLeftBytes > 4 )
               nNumLeftBytes = 4;
            if ( nNumRightBytes < 0 )
               nNumRightBytes = 0;

         } else {

            // We have a full 8 bytes.
            nNumLeftBytes = 4;
            nNumRightBytes = 4;

         }

         // Copy "left" bytes to numerics.
         memmove(
            &lLeft32Bits,
            &lpszSourceData[lStringPos],
            nNumLeftBytes );

         // If there are right bytes...
         if ( nNumRightBytes )

            // Copy "right" bytes to numerics.
            memmove(
               &lRight32Bits,
               &lpszSourceData[lStringPos + 4],
               nNumRightBytes );

         BlowfishEncipher( &lLeft32Bits, &lRight32Bits );

         // Copy everything to output.
         // Remember that if plaintext was not mult of 8 bytes in length,
         // the cyphertext dest buffer must be
         // able to hold the plaintext source plus ( 8 - sourcelen % 8 ).
         memmove(
            &lpszDestData[lStringPos],
            &lLeft32Bits,
            4 );
         memmove(
            &lpszDestData[lStringPos + 4],
            &lRight32Bits,
            4 );

      }

      // End encrypting.

   }

   // Unlock subkey arrays 'til we need 'em again.
   UnlockResource( hSubKeys );

   return bReturn;

}


// WARNING! Blowfish processing requires the cyphertext to have a length that
// is a multiple of 8 bytes.  Yes, this means that
//
//       ** THE CYPHERTEXT MAY BE LONGER THAN THE PLAINTEXT! **
//
// This means that lDestLen will be expanded to be a multiple of 8 when looking
// at the source.  Make sure that the cyphertext buffer is the size of
// lDestLen plus ( 8 - lDestLen % 8 ).

BOOL  BlowDecryptString(
         LPSTR          lpszSourceData,
         LPSTR          lpszDestData,
         unsigned long  lDestLen )

{

   BOOL           bReturn = TRUE;

   unsigned long  lLeft32Bits;
   unsigned long  lRight32Bits;

   unsigned long  lSourceLen;
   unsigned long  lStringPos;

   int            nNumLeftBytes;
   int            nNumRightBytes;


   // Lock subkey arrays.
   lpSubKeys = (BLOWFISH_SUBKEYS far *)LockResource( hSubKeys );

   if (     !lpszSourceData
         || !lpszDestData
         || lDestLen <= 0               ) {

      bReturn = FALSE;

   } else {

      // Check if target is not a multiple of 8.
      if ( lDestLen % 8 != 0 ) {

         // Make sure the source length is a multiple of 8.
         lSourceLen = lDestLen + ( 8 - lDestLen % 8 );
         bReturn = FALSE;

      } else {

         lSourceLen = lDestLen;

      }

      // Begin decrypting.

      // Loop through plaintext string, outputting to cyphertext.
      for ( lStringPos = 0; lStringPos < lSourceLen; lStringPos += 8 ) {

         lLeft32Bits  = 0;
         lRight32Bits = 0;

         // Copy "left" bytes to numerics.
         memmove(
            &lLeft32Bits,
            &lpszSourceData[lStringPos],
            4 );

         // Copy "right" bytes to numerics.
         memmove(
            &lRight32Bits,
            &lpszSourceData[lStringPos + 4],
            4 );

         BlowfishDecipher( &lLeft32Bits, &lRight32Bits );

         // Now that we have deciphered the full 8 bytes, we
         // see if we need all of them.

         // If we don't want full 8 bytes...
         if ( lDestLen - lStringPos < 8 ) {

            // Determine number of bytes we want.
            nNumLeftBytes = (int) ( lDestLen - lStringPos );
            nNumRightBytes = nNumLeftBytes - 4;
            if ( nNumLeftBytes > 4 )
               nNumLeftBytes = 4;
            if ( nNumRightBytes < 0 )
               nNumRightBytes = 0;

         } else {

            // We want full 8 bytes.
            nNumLeftBytes = 4;
            nNumRightBytes = 4;

         }

         // Copy everything to output.
         memmove(
            &lpszDestData[lStringPos],
            &lLeft32Bits,
            nNumLeftBytes );
         if ( nNumRightBytes )
            memmove(
               &lpszDestData[lStringPos + 4],
               &lRight32Bits,
               nNumRightBytes );

      }

      // End decrypting.

   }

   // Unlock subkey arrays 'til we need 'em again.
   UnlockResource( hSubKeys );

   return bReturn;

}

BOOL  BlowFree( void )
{

   // If we have the resource...
   if ( hSubKeys ) {

      // Deallocate previous subkeys.
      UnlockResource( hSubKeys );
      FreeResource( hSubKeys );

   }

   return TRUE;

}

BOOL  BlowEncryptFile (
         char *         szSourceFile,
         char *         szDestFile )

{

   FILE *         SourceFile;
   FILE *         DestFile;

   unsigned long  lLeft32Bits;
   unsigned long  lRight32Bits;

   BOOL           bReturn = TRUE;


   // Lock subkey arrays.
   lpSubKeys = (BLOWFISH_SUBKEYS far *)LockResource( hSubKeys );

   if ( ( SourceFile = fopen( szSourceFile,"rb" ) ) == NULL ) {

      bReturn = FALSE;

   } else if ( ( DestFile = fopen( szDestFile,"wb" ) ) == NULL ) {

      bReturn = FALSE;

   } else {

      // Begin with a message.
      // printf( "\nEncrypting..." );

      // Loop through plaintext file, outputting to cyphertext.
      // A plaintext file that is not an even multiple of 8 bytes
      // will be padded with 0 by this process.
      while ( !feof( SourceFile ) ) {

         lLeft32Bits  = 0;
         lRight32Bits = 0;

         fread( (char *) &lLeft32Bits,  4, 1, SourceFile ) ;
         fread( (char *) &lRight32Bits, 4, 1, SourceFile ) ;

         BlowfishEncipher( &lLeft32Bits, &lRight32Bits );

         fwrite( (char *) &lLeft32Bits,  4, 1, DestFile );
         fwrite( (char *) &lRight32Bits, 4, 1, DestFile );

      }

      // End with a message.
      // printf( "complete.\n" );

   }

   fclose( SourceFile ) ;
   fclose( DestFile ) ;

   // Unlock subkey arrays 'til we need 'em again.
   UnlockResource( hSubKeys );

   return bReturn;

}


BOOL  BlowDecryptFile (
         char *         szSourceFile,
         char *         szDestFile )

{

   FILE *         SourceFile;
   FILE *         DestFile;

   unsigned long  lSourceFileLen;
   unsigned long  lFilePos;

   unsigned long  lLeft32Bits;
   unsigned long  lRight32Bits;

   BOOL           bReturn = TRUE;


   // Lock subkey arrays.
   lpSubKeys = (BLOWFISH_SUBKEYS far *)LockResource( hSubKeys );

   if ( ( SourceFile = fopen( szSourceFile,"rb" ) ) == NULL ) {

      bReturn = FALSE;

   } else if ( ( DestFile = fopen( szDestFile,"wb" ) ) == NULL ) {

      bReturn = FALSE;

   } else {

      // Begin with a message.
      // printf( "\nDecrypting..." );

      // Get length of source file.
      fseek( SourceFile, 0, SEEK_END );
      lSourceFileLen = ftell( SourceFile );

      // Warn if not a multiple of 8.
      if ( lSourceFileLen % 8 != 0 ) {

         // printf( "Possible error! -> Cyphertext length [%.0f] is not a multiple of 8.\n", (double) lSourceFileLen );

         bReturn = FALSE;

      }

      // Reset.
      fseek( SourceFile, 0, SEEK_SET );
      lFilePos = 0;

      // Loop through cyphertext file, outputting to plaintext.
      // A plaintext file that was not an even multiple of 8 bytes long
      // should have been padded with 0.  We should now have a length
      // that is an exact multiple of 8 in the cyphertext file.
      while ( lFilePos + 8 <= lSourceFileLen && !feof( SourceFile ) ) {

         lLeft32Bits  = 0;
         lRight32Bits = 0;

         lFilePos += 4 * fread( (char *) &lLeft32Bits,  4, 1, SourceFile ) ;
         lFilePos += 4 * fread( (char *) &lRight32Bits, 4, 1, SourceFile ) ;

         BlowfishDecipher( &lLeft32Bits, &lRight32Bits );

         fwrite( (char *) &lLeft32Bits,  4, 1, DestFile );
         fwrite( (char *) &lRight32Bits, 4, 1, DestFile );

      }

      // End with a message.
      // printf( "complete.\n" );

   }

   fclose( SourceFile ) ;
   fclose( DestFile ) ;

   // Unlock subkey arrays 'til we need 'em again.
   UnlockResource( hSubKeys );

   return bReturn;

}


// "Fluffs up" the values in the buffer to be valid ASCII string values.
// Char values of 0-34 are replaced with letters.  The buffer doubles in
// size, plus the terminating NULL, so be careful to preallocate enough
// space in lpszBuffer.  The buffer len is the length of the compressed
// data to be expanded.
BOOL  ExpandToASCII(
         LPSTR          lpszTightBuffer,
         unsigned int   nBufferLen    )
{

   unsigned int   i;

   GLOBALHANDLE   ghTempBuffer;
   LPSTR          lpszTempBuffer;


   ghTempBuffer = GlobalAlloc( GHND, nBufferLen * 2 + 1 );
   lpszTempBuffer = (LPSTR) GlobalLock( ghTempBuffer );

   for ( i = 0; i < nBufferLen; i++ ) {

      // There are two bytes for each incoming byte.
      // If we have a below-range value,
      if ( (unsigned char)lpszTightBuffer[i] < 35 ) {

         // Convert it to a letter, store it in the first byte.
         // Plop a space in the second.
         lpszTempBuffer[ i*2 ]      = lpszTightBuffer [i] + 65;
         lpszTempBuffer[ i*2 + 1 ]  = 32;

      } else {

         // Otherwise, just use the second byte.  Put a space in first.
         lpszTempBuffer[ i*2 ]      = 32;
         lpszTempBuffer[ i*2 + 1 ]  = lpszTightBuffer [i];

      }
   }

   lpszTempBuffer[ nBufferLen * 2 ] = '\0';
   strcpy( lpszTightBuffer, lpszTempBuffer );

   GlobalUnlock( ghTempBuffer );
   GlobalFree( ghTempBuffer );

   return TRUE;

}


// This won't work in DOS due to Global Alloc's, etc.  Just comment it out.
BOOL  CompressFromASCII(
         LPSTR lpszLooseBuffer )
{

   unsigned int   i;
   unsigned int   nBufferLen;

   GLOBALHANDLE   ghTempBuffer;
   LPSTR          lpszTempBuffer;


   nBufferLen = strlen( lpszLooseBuffer ) / 2;
   ghTempBuffer = GlobalAlloc( GHND, nBufferLen );
   lpszTempBuffer = (LPSTR) GlobalLock( ghTempBuffer );

   for ( i = 0; i < nBufferLen; i++ ) {

      // If we have a space in first byte,
      if ( lpszLooseBuffer[ i*2 ] == 32 ) {

         // We have a normal value, just use second byte, ignoring first.
         lpszTempBuffer[i] = lpszLooseBuffer[ i*2 + 1 ];

      // The second byte should be a space, or we are out of synch.
      } else if ( lpszLooseBuffer[ i*2 + 1 ] == 32 ) {

         // Adjust the first byte back to its original value.
         lpszTempBuffer[i] = lpszLooseBuffer[ i*2 ] - 65;

      }
   }

   memmove( lpszLooseBuffer, lpszTempBuffer, nBufferLen );

   GlobalUnlock( ghTempBuffer );
   GlobalFree( ghTempBuffer );

   return TRUE;


}
