#include <base64.hpp>         // For base64_en/decode
#include <oauth/HMAC_SHA1.h>  // For HS256
#include <oauth/oauthlib.h>   // For OAUTHLIB_BUFFSIZE_LARGE
#include <json/json.hpp>      // 2016 JSON handling

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

 // SHA 1 digest is 160 bits
 const int cn_digest_length = 20;
 
 // get
string JWT::url_encode()
{
  bOK_ = false;
  
  iat_ = get_utc_current_time_t();

  string header   = R"({"alg":"HS256","typ":"JWT"})";

  // sub_ ("subject") is typically an email address.  
  // Email addresses are very tricky to validate, all kinds of crazy shit is allowed: 
  //    https://emailregex.com/
  //    https://stackoverflow.com/questions/201323/using-a-regular-expression-to-validate-an-email-address
  // They should at least conform to: /\S+@[-0-9A-Za-z]+\.[-0-9A-Za-z]+/
  // BUT it's impossible to get perfect.  So don't bother at all (say most people).
  
  // For the payload, we should use the official method so the JSON is properly escaped!
  string payload;
  try {
    using json = nlohmann::json;
    json j;
    j["sub"] = sub_;
    j["role"] = role_;
    j["iat"] = iat_;
    payload = j.dump();
  }
  catch(const std::exception& se)
  {
      stringstream ss;
      ss << "Error [" << se.what() << "] encoding JWT for subject " << sub_;
      log(LV_ERROR, ss.str());
      return "";
  }
  catch (...)
  {
      stringstream ss;
      ss << "Error encoding JWT for subject " << sub_;
      log(LV_ERROR, ss.str());
      return "";
  }
  
  string headerbase64   = base64_encode( (unsigned char const*)header.c_str(),  header.length()  );
  string payloadbase64  = base64_encode( (unsigned char const*)payload.c_str(), payload.length() );

  string unsigned_token = headerbase64 + "." + payloadbase64;

  // string signature = HMAC-SHA256(key, unsignedToken) 
  CHMAC_SHA1 objHMACSHA1;
  unsigned char strDigest[oAuthLibDefaults::OAUTHLIB_BUFFSIZE_LARGE];
  memset( strDigest, 0, oAuthLibDefaults::OAUTHLIB_BUFFSIZE_LARGE );
  objHMACSHA1.HMAC_SHA1( 
    (unsigned char*)unsigned_token.c_str(),
    unsigned_token.length(),
    (unsigned char*)shared_secret_.c_str(),
    shared_secret_.length(),
    strDigest 
  ); 
  
  bOK_ = true;
  return ::urlencode(headerbase64 + "." + payloadbase64 + "." + base64_encode(strDigest, cn_digest_length));
}

// set
void JWT::url_decode(string input)
{
  bOK_ = false;

  // Split out the three parts.
  string headerbase64;
  string payloadbase64;
  string signaturebase64;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(".");
  tokenizer tokens(input, sep);
  for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
  {
         if (headerbase64.empty())    { ::url_decode(string(*tok_iter),headerbase64   ); }
    else if (payloadbase64.empty())   { ::url_decode(string(*tok_iter),payloadbase64  ); }
    else if (signaturebase64.empty()) { ::url_decode(string(*tok_iter),signaturebase64); }
    else return;
  }
  if (signaturebase64.empty()) return;
  
  string unsigned_token = headerbase64 + "." + payloadbase64;
  
  // string signature = HMAC-SHA256(key, unsignedToken) 
  CHMAC_SHA1 objHMACSHA1;
  unsigned char strDigest[oAuthLibDefaults::OAUTHLIB_BUFFSIZE_LARGE];
  memset( strDigest, 0, oAuthLibDefaults::OAUTHLIB_BUFFSIZE_LARGE );
  objHMACSHA1.HMAC_SHA1( 
    (unsigned char*)unsigned_token.c_str(),
    unsigned_token.length(),
    (unsigned char*)shared_secret_.c_str(),
    shared_secret_.length(),
    strDigest 
  ); 
  string signaturebase64_check = base64_encode(strDigest,cn_digest_length);

  if (signaturebase64_check != signaturebase64) return;  
  
  string header   = base64_decode(headerbase64);
  string payload  = base64_decode(payloadbase64);

  // Parse the JSON.  
  try {

      // sub_ ("subject") is typically an email address.  
      // Email addresses are very tricky to validate, all kinds of crazy shit is allowed: 
      //    https://emailregex.com/
      //    https://stackoverflow.com/questions/201323/using-a-regular-expression-to-validate-an-email-address
      // They should at least conform to: /\S+@[-0-9A-Za-z]+\.[-0-9A-Za-z]+/
      // BUT it's impossible to get perfect.  So don't bother at all (say most people).

      using json = nlohmann::json;
      json jBody = json::parse(payload);

      if (jBody.count("sub" )== 0) return;
      if (jBody.count("role")== 0) return;
      
      sub_  = jBody["sub" ].get<string>();
      role_ = jBody["role"].get<int>()   ;
  }
  catch(const std::exception& se)
  {
      stringstream ss;
      ss << "Error [" << se.what() << "] parsing JWT: " << endl << input << endl;
      log(LV_ERROR, ss.str());
      return;
  }
  catch (...)
  {
      stringstream ss;
      ss << "Error parsing JWT: " << endl << input << endl;
      log(LV_ERROR, ss.str());
      return;
  }

  bOK_ = true;  
}
