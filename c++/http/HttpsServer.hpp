#pragma once


// Simple-Web-Server is required but easy to include.
// 
// Instructions:
//
//  1)  Get the code and put it somewhere within reach of your project:
//
//          https://github.com/eidheim/Simple-Web-Server
//
//  2)  Include it in your project.
//      It's header-only, so you can easily add it to your CMakeLists.txt with INCLUDE_DIRECTORIES.
//      You can even add it via a relative path, like this:
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
// 
// This is a wrapper class around the third-party header-only Simple-Web-Server library (which in turn uses boost asio).
// We lightly derive from SimpleWeb::HttpsServer, for two reasons:  
// 1) The base creates a protected io_service, we need to derive to expose io_service, so we can extend it with our timers.
// 2) We provide a default_resource_send() to send the buffer to the client; we have not needed any custom behavior yet, it gets the job done.
// ------------------
class HttpsServer : public Server<HTTPS>
{
    // THEFT!  Steal the base class constructor, as-is.
    using Server<HTTPS>::Server;

public:
    
    boost::asio::io_service& get_io_service() { return io_service; }

protected:
    
    // This function sends the response to the client.
    // The response buffer was created previously by a handler.
    // This code is from Simple-Web-Server https_examples.cpp.
    // It is all we have needed so far, so we have not needed to make it virtual.
    void default_resource_send(/*const HttpsServer &server,*/ std::shared_ptr<HttpsServer::Response> response,
                               std::shared_ptr<ifstream> ifs, std::shared_ptr<vector<char> > buffer) {
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


