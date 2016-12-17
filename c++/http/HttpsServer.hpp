#pragma once

// Simple-Web-Server is required but easy to include, it's header-only.  It uses boost::asio so you'll need that too.
// Instructions:
//
//  1)  Get the code and put it somewhere within reach of your project: https://github.com/eidheim/Simple-Web-Server
//  2)  Include it in your project.  You can easily add it to your CMakeLists.txt with INCLUDE_DIRECTORIES, eg:
//
//          set(ThirdParty "../../../")
//          INCLUDE_DIRECTORIES(${ThirdParty})
//
#include <Simple-Web-Server/server_https.hpp>

using namespace SimpleWeb;
using namespace std;


// ------------------
// HttpsServer
// ------------------
// A very simple generic web server meant to be easy to use out of the box.
// You can derive from this class and create your handlers before calling startServer().  Eg:
//
//  void createHandlers() {
//      resource["^/test.html"]["GET"]=[this](HRes response, HReq request) {
//          log(LV_ALWAYS,string("Received GET request: ") + get_request_content(request));
//          string content="we testin";
//          *response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
//      };
//  }
//
// Alternatively, you can provide handlers from outside the class.
// See Simple-Web-Server https_examples.cpp for details on that more-direct but less-contained approach.
//
// We lightly derive from Simple-Web-Server's Server<HTTPS> class, for these reasons:  
// 1) The base creates a protected io_service, we need to derive to expose io_service, so we can extend it with our timers.
// 2) We provide a default_resource_send() to send the buffer to the client; we have not needed any custom behavior yet, it gets the job done.
// 3) We provide a startServer() function so the user doesn't need to create a thread for the start() base class function, and derived classes can override it.
// ------------------
class HttpsServer : public Server<HTTPS>
{
    typedef Server<HTTPS> inherited;
    
    // THEFT!  Steal the base class constructor, as-is.
    using Server<HTTPS>::Server;

public:

    boost::asio::io_service& get_io_service() { return io_service; }

    virtual void startServer() {
        std::thread server_thread([this](){
            inherited::start();
        });
        std::this_thread::sleep_for(std::chrono::seconds(1));
        server_thread.join();
    }
    
    // helpers
    string get_request_content(std::shared_ptr<HttpsServer::Request> request) { return request->content.string(); }

protected:
    
    // This function sends the response to the client.
    // The response buffer was created previously by a handler.
    // This code is from Simple-Web-Server https_examples.cpp.
    // It is all we have needed so far, so we have not needed to make it virtual.
    void default_resource_send(
        std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<ifstream> ifs, 
        std::shared_ptr<vector<char> > buffer
    ) {
        streamsize read_length;
        if((read_length=ifs->read(&(*buffer)[0], buffer->size()).gcount())>0) {
            response->write(&(*buffer)[0], read_length);
            if(read_length==static_cast<streamsize>(buffer->size())) {
                send(response, [this, response, ifs, buffer](const boost::system::error_code &ec) {
                    if(!ec)
                        default_resource_send(response, ifs, buffer);
                    else
                        cerr << "Connection interrupted" << endl;
                });
            }
        }
    }
};


typedef std::shared_ptr<HttpsServer::Response> HRes;
typedef std::shared_ptr<HttpsServer::Request>  HReq;

