#ifndef BLOW_IT_H
	#define BLOW_IT_H

BOOL  BlowInitialize(
         unsigned char* lpszKey,
         unsigned int   nKeyLen );

BOOL  BlowEncryptString(
         LPSTR          lpszSourceData,
         unsigned long  lSourceLen,
         LPSTR          lpszDestData );

BOOL  BlowDecryptString(
         LPSTR          lpszSourceData,
         LPSTR          lpszDestData,
         unsigned long  lDestLen );

BOOL  BlowFree( void );

BOOL  BlowEncryptFile (
         char *         szSourceFile,
         char *         szDestFile );

BOOL  BlowDecryptFile (
         char *         szSourceFile,
         char *         szDestFile );

BOOL  CompressFromASCII(
         LPSTR lpszLooseBuffer );

BOOL  ExpandToASCII(
         LPSTR          lpszTightBuffer,
         unsigned int   nBufferLen    );

#endif // BLOW_IT_H