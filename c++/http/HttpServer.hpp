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
#include <Simple-Web-Server/server_http.hpp>

using namespace SimpleWeb;
using namespace std;


// ------------------
// HttpServer
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
// We lightly derive from Simple-Web-Server's Server<HTTP> class, for these reasons:  
// 1) The base creates a protected io_service, we need to derive to expose io_service, so we can extend it with our timers.
// 2) We provide a default_resource_send() to send the buffer to the client; we have not needed any custom behavior yet, it gets the job done.
// 3) We provide a startServer() function so the user doesn't need to create a thread for the start() base class function, and derived classes can override it.
// ------------------

// HACK OUT SSL as haproxy is in charge now
// class HttpServer : public Server<HTTPS>

class HttpServer : public Server<HTTP>
{
    typedef Server<HTTP> inherited;
    
public:

    HttpServer(
        unsigned short port, 
        size_t thread_pool_size, 
        long timeout_request=5, 
        long timeout_content=300
    ) :
    
      Server<HTTP>::Server()

    {
        config.port = port;
        config.thread_pool_size = thread_pool_size;
        config.timeout_request = timeout_request;
        config.timeout_content = timeout_content;
    }
    virtual ~HttpServer() {}

    std::shared_ptr<boost::asio::io_service>& get_io_service() { return io_service; }

    virtual void startServer() {

        inherited::start();

        // NOTE The base start() call avoids starting the io service
        // because we created it earlier, externally.
        // So once we get back from start(),
        // we still have to start the io service up.
        // If thread_pool_size>1, start m_io_service.run() in (thread_pool_size-1) threads for thread-pooling
        threads.clear();
        for(std::size_t c = 1; c < config.thread_pool_size; c++) {
            threads.emplace_back([this]() {
                this->io_service->run();
            });
        }

        // Main thread
        if(config.thread_pool_size > 0)
            io_service->run();

        // Wait for the rest of the threads, if any, to finish as well
        for(auto &t : threads)
            t.join();
    }
    
    // helpers
    string get_request_content(std::shared_ptr<HttpServer::Request> request) { return request->content.string(); }

};


typedef std::shared_ptr<HttpServer::Response> HRes;
typedef std::shared_ptr<HttpServer::Request>  HReq;

