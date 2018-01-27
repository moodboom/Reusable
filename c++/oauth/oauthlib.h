#ifndef __OAUTHLIB_H__
#define __OAUTHLIB_H__

#include "time.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>

// For stupid CaseInsensitiveMultimap
#include <Simple-Web-Server/utility.hpp>
using namespace SimpleWeb;

// ================================================================================================
// MDM these are defined by client in the sample code
// ================================================================================================
namespace oAuthLibDefaults
{
    /* Constants */
    const int OAUTHLIB_BUFFSIZE = 1024;
    const int OAUTHLIB_BUFFSIZE_LARGE = 1024;
    const std::string OAUTHLIB_CONSUMERKEY_KEY = "oauth_consumer_key";
    const std::string OAUTHLIB_CALLBACK_KEY = "oauth_callback";
    const std::string OAUTHLIB_VERSION_KEY = "oauth_version";
    const std::string OAUTHLIB_SIGNATUREMETHOD_KEY = "oauth_signature_method";
    const std::string OAUTHLIB_SIGNATURE_KEY = "oauth_signature";
    const std::string OAUTHLIB_TIMESTAMP_KEY = "oauth_timestamp";
    const std::string OAUTHLIB_NONCE_KEY = "oauth_nonce";
    const std::string OAUTHLIB_TOKEN_KEY = "oauth_token";
    const std::string OAUTHLIB_TOKENSECRET_KEY = "oauth_token_secret";
    const std::string OAUTHLIB_VERIFIER_KEY = "oauth_verifier";
    const std::string OAUTHLIB_SCREENNAME_KEY = "screen_name";
    const std::string OAUTHLIB_AUTHENTICITY_TOKEN_KEY = "authenticity_token";
    const std::string OAUTHLIB_SESSIONUSERNAME_KEY = "session[username_or_email]";
    const std::string OAUTHLIB_SESSIONPASSWORD_KEY = "session[password]";
    const std::string OAUTHLIB_AUTHENTICITY_TOKEN_TWITTER_RESP_KEY = "authenticity_token\" type=\"hidden\" value=\"";
    const std::string OAUTHLIB_TOKEN_TWITTER_RESP_KEY = "oauth_token\" type=\"hidden\" value=\"";
    const std::string OAUTHLIB_PIN_TWITTER_RESP_KEY = "code-desc\"><code>";
    const std::string OAUTHLIB_TOKEN_END_TAG_TWITTER_RESP = "\" />";
    const std::string OAUTHLIB_PIN_END_TAG_TWITTER_RESP = "</code>";

    const std::string OAUTHLIB_AUTHHEADER_STRING = "Authorization: OAuth ";
};

namespace oAuthTwitterApiUrls
{
    /* Twitter OAuth API URLs */
    const std::string OAUTHLIB_TWITTER_REQUEST_TOKEN_URL = "api.twitter.com/oauth/request_token";
    const std::string OAUTHLIB_TWITTER_AUTHORIZE_URL = "api.twitter.com/oauth/authorize?oauth_token=";
    const std::string OAUTHLIB_TWITTER_ACCESS_TOKEN_URL = "api.twitter.com/oauth/access_token";
};
// ================================================================================================


typedef enum _eOAuthHttpRequestType
{
    eOAuthHttpInvalid = 0,
    eOAuthHttpGet,
    eOAuthHttpPost,
    eOAuthHttpDelete
} eOAuthHttpRequestType;

typedef std::list<std::string> oAuthKeyValueList;
typedef std::map<std::string, std::string> oAuthKeyValuePairs;

class oAuth
{
public:
    oAuth(bool bIncludeVersion = true);
    ~oAuth();

    /* OAuth public methods used by twitCurl */
    void getConsumerKey( std::string& consumerKey /* out */ );
    void setConsumerKey( const std::string& consumerKey /* in */ );

    void getConsumerSecret( std::string& consumerSecret /* out */ );
    void setConsumerSecret( const std::string& consumerSecret /* in */ );

    void getOAuthTokenKey( std::string& oAuthTokenKey /* out */ );
    void setOAuthTokenKey( const std::string& oAuthTokenKey /* in */ );

    void getOAuthTokenSecret( std::string& oAuthTokenSecret /* out */ );
    void setOAuthTokenSecret( const std::string& oAuthTokenSecret /* in */ );

    void getOAuthScreenName( std::string& oAuthScreenName /* out */ );
    void setOAuthScreenName( const std::string& oAuthScreenName /* in */ );

    // MDM why was support for m_oAuthCallbackKey missing?  Etrade requires it.
    void getCallbackKey( std::string& oAuthCallbackKey /* out */ );
    void setCallbackKey( const std::string& oAuthCallbackKey /* in */ );

    void getOAuthPin( std::string& oAuthPin /* out */ );
    void setOAuthPin( const std::string& oAuthPin /* in */ );

    bool getOAuthHeader(const eOAuthHttpRequestType eType, /* in */
                         const std::string& rawUrl, /* in */
                         const std::string& rawData, /* in */                       // MDM: can be empty for GETs, or include request key1=value1&key2=value2 pairs for POSTs
                         CaseInsensitiveMultimap& oAuthHttpHeader, /* out */
                         const bool includeOAuthVerifierPin = false, /* in */
                         const bool bUpdateHeader = true  /* in */
    );
    bool getEtradeHeader(
        const eOAuthHttpRequestType eType, /* in */
        const std::string& rawUrl, /* in */
        const std::string& rawData, /* in */
        CaseInsensitiveMultimap& oAuthHttpHeader, /* out */
        const bool includeOAuthVerifierPin = false, /* in */
        const bool includeOAuthTokenPin = false /* in */
    );

    bool extractOAuthTokenKeySecret( const std::string& requestTokenResponse /* in */ );

    oAuth clone();

    // MDM non-stupid getters
    const std::string getConsumerKey() { return m_consumerKey; }
    const std::string getNonce() { return m_nonce; }
    const std::string getTimeStamp() { return m_timeStamp; }
    const std::string getOAuthSignature() { return m_oAuthSignature; }
    
private:

    /* OAuth data */
    bool m_bIncludeVersion;
    std::string m_consumerKey;
    std::string m_consumerSecret;
    std::string m_oAuthTokenKey;
    std::string m_oAuthTokenSecret;
    std::string m_oAuthCallbackKey;
    std::string m_oAuthPin;
    std::string m_nonce;
    std::string m_timeStamp;
    std::string m_oAuthScreenName;
    std::string m_oAuthSignature;

    /* OAuth twitter related utility methods */
    void buildOAuthRawDataKeyValPairs( const std::string& rawData, /* in */
                                       bool urlencodeData, /* in */
                                       oAuthKeyValuePairs& rawDataKeyValuePairs /* out */ );

    bool buildOAuthTokenKeyValuePairs( const bool includeOAuthVerifierPin, /* in */
                                       // MDM made this a member
				       // const std::string& oauthSignature, /* in */
                                       oAuthKeyValuePairs& keyValueMap /* out */,
                                       const bool generateTimestamp /* in */ );

    bool getStringFromOAuthKeyValuePairs( const oAuthKeyValuePairs& rawParamMap, /* in */
                                          std::string& rawParams, /* out */
                                          const std::string& paramsSeperator /* in */ );

    bool getSignature( const eOAuthHttpRequestType eType, /* in */
                       const std::string& rawUrl, /* in */
                       const oAuthKeyValuePairs& rawKeyValuePairs /* in */
		       // MDM Made this a member
		       // std::string& oAuthSignature /* out */
		       );

    void generateNonceTimeStamp();
};

#endif // __OAUTHLIB_H__
