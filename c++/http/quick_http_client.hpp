// ------------------------------------------
// quick_http_client.hpp
// ------------------------------------------
// All you really need to know about to use this class are the Client constructors, below.
//
//      Client<HTTP>    HTTP-specific socket handling
//      Client<HTTPS>   HTTPS-specific socket handling, including certificates and handshaking
//
// Example:
//
//      TODO
//
// ------------------------------------------
// This code is based on the HTTP Client boost example from:
//     http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/examples.html
// 
// This code borrows excellent template handling of sockets, ssl certificates, etc., from this project:
//      https://github.com/eidheim/Simple-Web-Server
// ------------------------------------------

#ifndef quick_http_client_hpp
#define quick_http_client_hpp

#include "quick_http_components/client_base.hpp"


namespace QuickHttp
{
    // ==================================
    // Client<HTTP>
    // ==================================

    template<>
    class Client<HTTP> : public ClientBase<HTTP>
    {
    public:

        inline Client(
            boost::asio::io_service& io_service,
            http_action ha,
            const std::string& server,
            const std::string& port,        // examples: "default", "8080", "2345", "http", "ftp", etc.
                                            // "default" will become "80"
            const std::string& path,
            const std::string& header = "", // directly placed into request; may include OAuth headers, etc.
            const std::string& body = "",   // directly placed into request
            bool b_binary_result = false,
            const string& username = "",
            const string& password = ""
        );
        // NOTE: Results are returned in these member vars from the base class:
        //       unsigned int status_code_;
        //       Strings response_headers_;
        //       string response_body_;

    private:
        inline void aconnect(tcp::resolver::iterator endpoint_iterator);
    };


    // ==================================
    // Client<HTTPS>
    // ==================================

    template<>
    class Client<HTTPS> : public ClientBase<HTTPS>
    {
    public:
        inline Client(
            boost::asio::io_service& io_service,
            http_action ha,
            const std::string& server,
            const std::string& port,        // examples: "default", "8080", "2345", "http", "ftp", etc.
                                            // "default" will become "80" if b_use_ssl is false, or "443" if true
            const std::string& path,
            const std::string& header = "",
            const std::string& body = "",   // directly placed into request
            bool b_binary_result = false,
            const string& username = "",
            const string& password = "",
            bool verify_certificate=false,
            const std::string& cert_file=std::string(),
            const std::string& private_key_file=std::string(),
            const std::string& verify_file=std::string()
        );
        // NOTE: Results are returned in these member vars from the base class:
        //       unsigned int status_code_;
        //       Strings response_headers_;
        //       string response_body_;

    private:
        inline void aconnect(tcp::resolver::iterator endpoint_iterator);
        inline void handshake();
        boost::asio::ssl::context asio_context_;
    };


    // =================== IMPLEMENTATION ========================

    Client<HTTP>::Client(
        boost::asio::io_service& io_service,
        http_action ha,
        const std::string& server,
        const std::string& port,        // examples: "default", "8080", "2345", "http", "ftp", etc.
                                        // "default" will become "80" if b_use_ssl is false, or "443" if true
        const std::string& path,
        const std::string& header,
        const std::string& body,        // directly placed into request
        bool b_binary_result,
        const string& username,
        const string& password
    ) :

        // Call base class
        ClientBase<HTTP>::ClientBase(io_service, header, b_binary_result)

    {
        // Set up the type-specific socket.
        socket_=std::make_shared<HTTP>(io_service);

        // Configure the correct default port.
        string actual_port = port;
        if (strings_are_equal(actual_port,"default"))
        {
            actual_port = "80";
        }

        aresolve(ha, server, actual_port, path, header, body, username, password);
    }


    void Client<HTTP>::aconnect(tcp::resolver::iterator endpoint_iterator)
    {
        boost::asio::async_connect(
                *socket_,
                endpoint_iterator,
                boost::bind(&Client<HTTP>::handle_connect, this,
                        boost::asio::placeholders::error));

    }

    Client<HTTPS>::Client(
        boost::asio::io_service& io_service,
        http_action ha,
        const std::string& server,
        const std::string& port,
        const std::string& path,
        const std::string& header,
        const std::string& body,
        bool b_binary_result,
        const string& username,
        const string& password,
        bool verify_certificate,
        const std::string& cert_file,
        const std::string& private_key_file,
        const std::string& verify_file
    ) :

        // Call base class
        ClientBase<HTTPS>::ClientBase(io_service, header, b_binary_result),

        // Init vars
        // MDM Have the whole world really gone from ssl2 to tls1?
        // asio_context_(boost::asio::ssl::context::sslv23)
        asio_context_(boost::asio::ssl::context::tlsv1)

    {
        if(verify_certificate)
            asio_context_.set_verify_mode(boost::asio::ssl::verify_peer);
        else
            asio_context_.set_verify_mode(boost::asio::ssl::verify_none);

        if(cert_file.size()>0 && private_key_file.size()>0) {
            asio_context_.use_certificate_chain_file(cert_file);
            asio_context_.use_private_key_file(private_key_file, boost::asio::ssl::context::pem);
        }

        if(verify_file.size()>0)
            asio_context_.load_verify_file(verify_file);

        socket_=std::make_shared<HTTPS>(io_service, asio_context_);

        // Configure the correct default port.
        string actual_port = port;
        if (strings_are_equal(actual_port,"default"))
        {
            actual_port = "443";
        }

        aresolve(ha, server, actual_port, path, header, body, username, password);
    }

    void Client<HTTPS>::aconnect(tcp::resolver::iterator endpoint_iterator)
    {
        boost::asio::async_connect(
                socket_->lowest_layer(),
                endpoint_iterator,
                boost::bind(&Client<HTTPS>::handle_connect, this,
                        boost::asio::placeholders::error));

    }

    void Client<HTTPS>::handshake()
    {
        socket_->handshake(boost::asio::ssl::stream_base::client);
        socket_error_=false;
    }

}   // namespace QuickHttp

#endif
