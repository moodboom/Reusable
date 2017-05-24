#include "utilities.hpp"

// LOGGING GLOBAL SETTINGS
// These are the default values.
// The base log filename should be changed before use, in the application code.
// The current log verbosity should also be set in application code (perhaps via command line param),
// but note that it can be changed on the fly as needed.
// g_ss is available to (re)use when logging.
//
// LOGGING EXAMPLE:
//
//      g_ss.str(std::string());
//      g_ss << "header: \r\n" << str_header;
//      log(LV_DEBUG,g_ss.str());
//
string g_base_log_filename = "app";
LOG_TO_FILE_VERBOSITY g_current_log_verbosity = LV_INFO;
stringstream g_ss;


//=========================================================
// JWT
//=========================================================
// I'm putting these here so we don't have to include encryption headers everywhere.
// And because we need time functions.

// get
string JWT::url_encode()
{
  iat_ = get_current_time_t();

  /*
  // Now, hash the signature base string using HMAC_SHA1 class
  CHMAC_SHA1 objHMACSHA1;
  std::string secretSigningKey;
  unsigned char strDigest[oAuthLibDefaults::OAUTHLIB_BUFFSIZE_LARGE];

  memset( strDigest, 0, oAuthLibDefaults::OAUTHLIB_BUFFSIZE_LARGE );

  // Signing key is composed of consumer_secret&token_secret
  secretSigningKey.assign( m_consumerSecret );
  secretSigningKey.append( "&" );

  if( m_oAuthTokenSecret.length() )
  {
      secretSigningKey.append( m_oAuthTokenSecret );
  }

  // DEBUG
  // ss.str(std::string()); ss << "key imploded:  " << secretSigningKey; log(LV_DEBUG,ss.str(),true);

  objHMACSHA1.HMAC_SHA1( (unsigned char*)sigBase.c_str(),
                         sigBase.length(),
                         (unsigned char*)secretSigningKey.c_str(),
                         secretSigningKey.length(),
                         strDigest ); 

  // DEBUG
  // ss.str(std::string()); ss << "hash_hmac:     " << strDigest; log(LV_DEBUG,ss.str(),true);

  // Do a base64 encode of signature 
  std::string base64Str = base64_encode( strDigest, 20  ); // SHA 1 digest is 160 bits

  // DEBUG
  // ss.str(std::string()); ss << "base64:        " << base64Str; log(LV_DEBUG,ss.str(),true);

  // Do an url encode 
  m_oAuthSignature = urlencode( base64Str );
  */

  return "TODO";
}

// set
void JWT::url_decode(string input)
{
  bOK_ = false;

  string jwt_unencoded;
  if (!::url_decode(input,jwt_unencoded)) return;


  bOK_ = true;  
}
