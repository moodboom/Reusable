#pragma once

// This is a wrapper class around the third-party header-only Simple-Web-Server library (which in turn uses boost asio).
// https://github.com/eidheim/Simple-Web-Server
// You can add it to your CMakeLists.txt via a relative path, like this:
//
//      set(ThirdParty "../../../")
//      INCLUDE_DIRECTORIES(${ThirdParty})
//
#include <Simple-Web-Server/client_https.hpp>

#include "HttpsConstants.hpp"

using namespace SimpleWeb;


// We derive from SimpleWeb::HttpsClient to add our base functionality.
class HttpsClient : public Client<HTTPS>
{
public:
    HttpsClient(const std::string& url, bool bVerifyCert = true)
    :
        // Call base class
        Client<HTTPS>::Client(url, bVerifyCert)
    {}

    boost::asio::io_service& get_io_service() { return io_service; }
};



