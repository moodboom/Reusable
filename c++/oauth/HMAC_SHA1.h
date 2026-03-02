/*
	HMAC-SHA1 using OpenSSL — drop-in replacement for the original hand-rolled implementation.
	Keeps the CHMAC_SHA1 class interface so call sites need zero changes.
*/

#ifndef __HMAC_SHA1_H__
#define __HMAC_SHA1_H__

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <cstring>

typedef unsigned char BYTE;

class CHMAC_SHA1
{
public:
	enum {
		SHA1_DIGEST_LENGTH	= 20,
		SHA1_BLOCK_SIZE		= 64,
		HMAC_BUF_LEN		= 4096
	};

	void HMAC_SHA1(BYTE *text, int text_len, BYTE *key, int key_len, BYTE *digest)
	{
		unsigned int out_len = SHA1_DIGEST_LENGTH;
		::HMAC(EVP_sha1(), key, key_len, text, text_len, digest, &out_len);
	}
};

#endif /* __HMAC_SHA1_H__ */
