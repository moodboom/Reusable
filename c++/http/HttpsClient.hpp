#pragma once

#include <Simple-Web-Server/client_https.hpp>           // See HttpsServer.hpp for instructions on including Simple-Web-Server in CMake.

using namespace SimpleWeb;
using namespace std;


// We derive from SimpleWeb::HttpsClient to add our base functionality.

class HttpsClient : public Client<HTTPS>
{
    typedef Client<HTTPS> inherited;
    
public:
    HttpsClient(const string& url, bool bVerifyCert = true)
    :
        // Call base class
        inherited::Client(url, bVerifyCert)
    {}

    std::shared_ptr<Response> make_request(
        const std::string& request_type, 
        const std::string& path="/",
        string_view content="",
        const CaseInsensitiveMultimap& header = CaseInsensitiveMultimap()
    ) {
        try {
            return inherited::request(
                request_type,     // verb
                path,             // path
                content,          // content
                header            // header
            );        
        } catch(const std::exception& e) {
        
            // SWS throws exceptions, which is not helpful.
            // A bad call is a bad call and needs immediate handling.
            
            // TODO SWS makes status_ccode private so we can't set it here.
            // Determine a good internal way to return bad status due to exception.
            // std::shared_ptr<Response> response(new Response);
            // response->status_code = "503 Not Availalbe -- HttpsClient exception caught";
            
            return std::shared_ptr<Response>();
        }
    }
};

// Results extraction helpers
static string get_body(std::shared_ptr<Client<HTTPS>::Response> response)           { if (response) return string(istreambuf_iterator<char>(response->content), {}); return ""; }
static int32_t get_status_code(std::shared_ptr<Client<HTTPS>::Response> response)   { if (response) return boost::lexical_cast<int32_t>(response->status_code.substr(0,3));; return 500; }



// It's become pretty annoying that SWS separates out http from https so severely.
// This is nearly an exact clone of the above class, for http.

class HttpClient : public Client<HTTP>
{
  typedef Client<HTTP> inherited;

public:
  HttpClient(const string& url)
    :
  // Call base class
    inherited::Client(url)
  {}

  std::shared_ptr<Response> make_request(
    const std::string& request_type,
    const std::string& path="/",
    string_view content="",
    const CaseInsensitiveMultimap& header = CaseInsensitiveMultimap()
  ) {
      try {
          return inherited::request(
            request_type,     // verb
            path,             // path
            content,          // content
            header            // header
          );
      } catch(const std::exception& e) {

          // SWS throws exceptions, which is not helpful.
          // A bad call is a bad call and needs immediate handling.

          // TODO SWS makes status_ccode private so we can't set it here.
          // Determine a good internal way to return bad status due to exception.
          // std::shared_ptr<Response> response(new Response);
          // response->status_code = "503 Not Availalbe -- HttpsClient exception caught";

          return std::shared_ptr<Response>();
      }
  }
};

// Results extraction helpers
static string get_body(std::shared_ptr<Client<HTTP>::Response> response)           { if (response) return response->content.string(); return ""; }
static int32_t get_status_code(std::shared_ptr<Client<HTTP>::Response> response)   { if (response) return boost::lexical_cast<int32_t>(response->status_code.substr(0,3));; return 500; }


