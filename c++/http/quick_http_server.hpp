// ------------------------------------------
// quick_http_server.hpp
// ------------------------------------------
// This class allows you to quickly stand up an http(s) server with a handler loop that lets you easily
//
//      1) asynchronously handle incoming requests as they are queued up by the http(s) server
//      2) fire off your own timed events as needed
//
// It makes it very quick to set up a RESTful-webservices-based server process.
//
// There are two main requirements:
//
//      1) Create a server_handler to do your processing that includes vectors for your includes, and your RESTful API calls.
//      2) Use one of the server constructors below, with your derived processing class, using boost::asio standards, to start the server.
//
//          Server<HTTP>    HTTP-specific socket handling
//          Server<HTTPS>   HTTPS-specific socket handling, including certificates and handshaking
//
// Example:
//
//      TODO
//
// ------------------------------------------
// This code is based on the HTTP server boost example from:
//     http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/examples.html
// 
// This code borrows excellent template handling of sockets, ssl certificates, etc., from this project:
//      https://github.com/eidheim/Simple-Web-Server
// ------------------------------------------

#ifndef quick_http_server_hpp
#define quick_http_server_hpp

#include "quick_http_components/server_base.hpp"


namespace QuickHttp
{

    template<>
    class Server<HTTP> : public ServerBase<HTTP> {
    public:
        inline Server(
            boost::asio::io_service& io_service,
            server_handler& handler,
            const std::string& address,
            const std::string& port,
            std::size_t thread_pool_size,
            const long timeout_request=5,
            const long timeout_content=300
        );

    protected:
        inline virtual void start_accept();
        inline virtual void handle_accept(const boost::system::error_code& e);
    };

    template<>
    class Server<HTTPS> : public ServerBase<HTTPS> {
    public:
        inline Server(boost::asio::io_service& io_service,
            server_handler& handler,
            const std::string& address,
            const std::string& port,
            std::size_t thread_pool_size,
            const std::string& cert_file,
            const std::string& private_key_file,
            const long timeout_request=5,
            const long timeout_content=300,
            const std::string& verify_file=std::string()
        );

    protected:
        inline virtual void start_accept();
        inline virtual void handle_accept(const boost::system::error_code& e);

        boost::asio::ssl::context context;
    };


    // =================== IMPLEMENTATION ========================

    Server<HTTP>::Server(
        boost::asio::io_service& io_service,
        server_handler& handler,
        const std::string& address,
        const std::string& port,
        std::size_t thread_pool_size,
        const long timeout_request,
        const long timeout_content
    ) :
        // Call base class
        ServerBase<HTTP>::ServerBase(io_service, handler, address, port, thread_pool_size, timeout_request, timeout_content)
    {
        start_accept();
    }


    /// Initiate an asynchronous accept operation.
    void Server<HTTP>::start_accept()
    {
        // Give the connection both our service and our handler, and it will be ready to go.
        new_connection_.reset(new connection(io_service_, m_handler));

        acceptor_.async_accept(new_connection_->socket(),
          boost::bind(&Server<HTTP>::handle_accept, this,
            boost::asio::placeholders::error));
    }

    /// Handle completion of an asynchronous accept operation.
    void Server<HTTP>::handle_accept(const boost::system::error_code& e)
    {
      if (!e)
      {
        new_connection_->start();
      }

      start_accept();
    }

    Server<HTTPS>::Server(
        boost::asio::io_service& io_service,
        server_handler& handler,
        const std::string& address,
        const std::string& port,
        std::size_t thread_pool_size,
        const std::string& cert_file,
        const std::string& private_key_file,
        const long timeout_request,
        const long timeout_content,
        const std::string& verify_file
    ) :
        // Call base class
        ServerBase<HTTPS>::ServerBase(io_service, handler, address, port, thread_pool_size, timeout_request, timeout_content),

        // Init vars

        // MDM Only allow TLS
        // Check this at: https://www.ssllabs.com/ssltest/
        // context(boost::asio::ssl::context::sslv23)
        context(boost::asio::ssl::context::tlsv12)
    {
        context.use_certificate_chain_file(cert_file);
        context.use_private_key_file(private_key_file, boost::asio::ssl::context::pem);

        if(verify_file.size()>0)
            context.load_verify_file(verify_file);

        start_accept();
    }

    /// Initiate an asynchronous accept operation.
    void Server<HTTPS>::start_accept()
    {
        // MDM I was originally using bind with a callback for HTTP.
        // Simple-Web-Server is using lambdas with different parameters for HTTPS.
        // The signature is different enough that I'm switching to that.

        // Give the connection both our service and our handler.
        new_connection_.reset(new connection(io_service_, m_handler));

        acceptor_.async_accept(new_connection_->socket(),
          boost::bind(&Server<HTTPS>::handle_accept, this,
            boost::asio::placeholders::error));

        /*
        //Create new socket for this connection
        //Shared_ptr is used to pass temporary objects to the asynchronous functions
        std::shared_ptr<HTTPS> socket(new HTTPS(io_service_, context));

        acceptor_.async_accept((*socket).lowest_layer(), [this, socket](const boost::system::error_code& ec) {
            //Immediately start accepting a new connection
            start_accept();

            if(!ec) {
                boost::asio::ip::tcp::no_delay option(true);
                socket->lowest_layer().set_option(option);

                //Set timeout on the following boost::asio::ssl::stream::async_handshake
                std::shared_ptr<boost::asio::deadline_timer> timer;
                if(timeout_request_>0)
                    timer=set_timeout_on_socket(socket, timeout_request_);

                // MDM may we change this signature here?
                // (*socket).async_handshake(boost::asio::ssl::stream_base::server, [this, socket, timer]
                (*socket).async_handshake(boost::asio::ssl::stream_base::server, [this, socket, timer]
                        (const boost::system::error_code& ec) {


                    if(timeout_request_>0)
                        timer->cancel();
                    if(!ec)
                    {
                        // TODO
                        log(LV_ALWAYS,"Received request");
                        // read_request_and_content(socket);
                    }
                });
            }
        });
        */

    }

    // TODO with newer https code
    /// Handle completion of an asynchronous accept operation.
    void Server<HTTPS>::handle_accept(const boost::system::error_code& e)
    {
        if (!e)
        {
            // MDM Do I *want* this option?
            boost::asio::ip::tcp::no_delay option(true);
            // socket->lowest_layer().set_option(option);
            new_connection_->socket().lowest_layer().set_option(option);

            // MDM Set this up to mirror SWS variable to ease code migration.
            // NOPE not the same thing
            // boost::asio::ip::tcp::socket* socket = &(new_connection_->socket());

            // MDM INstead let's just try a f'in shared ptr...
            // THIS IS NOT RIGHT, WE ALREADY HAVE A FUCKING SOCKET:
            //      std::shared_ptr<HTTPS> socket(new HTTPS(io_service_, context));
            std::shared_ptr<HTTPS> socket(&(new_connection_->ssl_socket()));

            //Set timeout on the following boost::asio::ssl::stream::async_handshake
            std::shared_ptr<boost::asio::deadline_timer> timer;
            if(timeout_request_>0)
                timer=set_timeout_on_socket(socket, timeout_request_);

            (*socket).async_handshake(boost::asio::ssl::stream_base::server, [this, socket, timer]
                    (const boost::system::error_code& ec) {
                if(timeout_request_>0)
                    timer->cancel();
                if(!ec)
                {
                    // TODO
                    log(LV_ALWAYS,"Received request");
                    // read_request_and_content(socket);
                    new_connection_->start();
                }
            });

        }

        start_accept();
    }

}   // namespace QuickHttp

#endif
