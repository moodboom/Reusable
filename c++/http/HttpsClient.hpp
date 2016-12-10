#pragma once

#include <Simple-Web-Server/client_https.hpp>           // See HttpsServer.hpp for instructions on including Simple-Web-Server in CMake.

using namespace SimpleWeb;
using namespace std;


// We derive from SimpleWeb::HttpsClient to add our base functionality.
class HttpsClient : public Client<HTTPS>
{
public:
    HttpsClient(const string& url, bool bVerifyCert = true)
    :
        // Call base class
        Client<HTTPS>::Client(url, bVerifyCert)
    {}

    boost::asio::io_service& get_io_service() { return io_service; }
};

// Results extraction helpers
static string get_body(std::shared_ptr<Client<HTTPS>::Response> response)           { return string(istreambuf_iterator<char>(response->content), {}); }
static int32_t get_status_code(std::shared_ptr<Client<HTTPS>::Response> response)   { return boost::lexical_cast<int32_t>(response->status_code.substr(0,3));; }
