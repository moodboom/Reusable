// This routine encrypts or decrypts a file with a supplied key 
// using the BLOWFISH algorithm written by Bruce Schneier (and
// placed in the public domain).  The security of the encryption
// lies entirely within the variable length key.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>


#include "blowfish.h"

#include "blow_it.h"

void main(
      int      argc,
      char *   argv[]   )
{
   
   char     szMode[10];
   char     szKey[55];
   char     szSourceFile[256];
   char     szDestFile[256];
   
   
   // Initialize.
   
   // Get command line arguments.
   strncpy( szMode, argv[1], 9 );
   szMode[9] = '\0';
   strncpy( szKey, argv[2], 54 ) ;
   szKey[54] = '\0';
   strncpy( szSourceFile, argv[3], 255 ) ;
   szSourceFile[255] = '\0';
   strncpy( szDestFile, argv[4], 255 ) ;
   szDestFile[255] = '\0';
   
   if ( strcmp( szMode, "ENCRYPT" ) == 0 ) {
   
      BlowEncryptFile( szSourceFile, szDestFile, szKey );
   
   } else if ( strcmp( szMode, "DECRYPT" ) == 0 ) {
   
      BlowDecryptFile( szSourceFile, szDestFile, szKey );
   
   } else {
   
      printf( "\n SYNTAX:\n   blowfish ENCRYPT|DECRYPT <Key> <Sourcefile> <Destfile>\n" );
      
   }
   
   return;
   
   
}


