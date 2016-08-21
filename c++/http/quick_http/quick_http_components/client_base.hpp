// ------------------------------------------
// client_base.hpp
// ------------------------------------------
// There are three classes involved with clients:
//
//      ClientBase      common functionality
//      Client<HTTP>    HTTP-specific socket handling
//      Client<HTTPS>   HTTPS-specific socket handling, including certificates and handshaking
//
// All you really need to know about to use this class are the Client constructors.
// See quick_http_client.hpp for details.
//
// ------------------------------------------
// This code is based on the HTTP Client boost example from:
//     http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/examples.html
// 
// This code borrows excellent template handling of sockets, ssl certificates, etc., from this project:
//      https://github.com/eidheim/Simple-Web-Server
// ------------------------------------------

#ifndef client_base_hpp
#define client_base_hpp

#include <utilities.hpp>
#include <base64.hpp>


namespace QuickHttp
{

    // ==================================
    // ClientBase
    // ==================================
    // This abstract base class is not used directly.
    // See Client<> implementations in quick_http_client.hpp.
    // ==================================

    template <class socket_type>
    class ClientBase
    {
    public:

    	// MDM We collect up the response for post-processing after the call is made.
        unsigned int status_code_;
        Strings response_headers_;
        string response_body_;

    protected:

        ClientBase( boost::asio::io_service& io_service, string header, bool b_binary_result = false )
        :
            // Init vars
            resolver_(io_service),
            b_binary_result_(b_binary_result),
            status_code_(503)                       // default to 503 Service Unavailable until we know differently

        {}

        virtual void aconnect(tcp::resolver::iterator endpoint_iterator) = 0;

        inline void aresolve(
    		http_action ha,
    		const std::string& server,
    		const std::string& port,
    		const std::string& path,
            const std::string& header,
    		const std::string& body,
    		const string& username,
    		const string& password
        );

        inline void handle_resolve(
            const boost::system::error_code& err,
            tcp::resolver::iterator endpoint_iterator
        );

        virtual void handshake() {}  // only needed for HTTPS but we have to call it in base at the right time

        inline void handle_connect(const boost::system::error_code& err);

        inline void handle_write_request(const boost::system::error_code& err);

        inline void handle_read_status_line(const boost::system::error_code& err);

        inline void handle_read_headers(const boost::system::error_code& err);

        inline void handle_read_content(const boost::system::error_code& err);

        inline void add_next_response_chunk_to_body();

        // The object associated with this pointer is allocated in derived classes.
        std::shared_ptr<socket_type> socket_;

        tcp::resolver resolver_;
        boost::asio::streambuf request_;
        boost::asio::streambuf response_;

        bool b_binary_result_;

        bool socket_error_;
    };

    // GENERIC TEMPLATE specification of the class.
    // Then we'll do specializations (HTTP, HTTPS) next.
    template<class socket_type>
    class Client : public ClientBase<socket_type> {};

    // The types we'll be using to specialize.
    typedef boost::asio::ip::tcp::socket HTTP;
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> HTTPS;


    // =================== IMPLEMENTATION ========================

    template <class socket_type>
    void ClientBase<socket_type>::aresolve(
        http_action ha,
        const std::string& server,
        const std::string& port,
        const std::string& path,
        const std::string& header,
        const std::string& body,
        const string& username,
        const string& password
    ) {
        std::string str_action;
        assert(http_action_count == 5);
        switch (ha)
        {
            case ha_get     : str_action = "GET"    ; break;
            case ha_post    : str_action = "POST"   ; break;
            case ha_put     : str_action = "PUT"    ; break;
            case ha_delete  : str_action = "DELETE" ; break;
            case ha_patch   : str_action = "PATCH"  ; break;
            default         : str_action = "GET"    ; break;
        }

        // Form the request.
        std::ostream request_stream(&request_);

        request_stream << str_action << " " << path << " HTTP/1.1\r\n";
        request_stream << "Host: " << server << ":" << port << "\r\n";
        request_stream << "Accept: */*\r\n";

        // Stuff any header provided by the caller.
        request_stream << header;

        // We will provide a basic authentication header if the optional username parameter was provided.
        if (!username.empty())
        {
            // This asks the server to send a request back that would require us to pop a u/p dialog:
            // request_stream << "WWW-Authenticate: Basic realm=\"MY basic realm\"\r\n";

            // This lets us pass authentication directly:
            request_stream << "Authorization: Basic ";
            stringstream u_and_p;
            u_and_p << username;
            u_and_p <<  ":";
            u_and_p << password;
            request_stream << base64_encode(
                (unsigned char const*)u_and_p.str().c_str(),
                (uint_fast32_t)u_and_p.str().length()
            );
            request_stream << "\r\n";
        }

        // MDM I've seen this hang if we don't provide this header, even if it's zero.
        // if (!body.empty())
        {
            request_stream << "Content-Length: ";
            request_stream << body.size();
            request_stream << "\r\n";
        }

        // We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        request_stream << "Connection: close\r\n";

        // The empty line created by this newline marks the end of headers and start of body.
        request_stream << "\r\n";

        // And finally, append any provided body.
        request_stream << body;

        // Output the whole http request to the log, but only if the user has requested DEBUG logging.
        if (g_current_log_verbosity == LV_DEBUG)
        {
            // This is hanging - how do you simply copy a stream?
            // C++ IOStream classes suck!  :P
            /*
            size_t req_length = request_stream.tellp();
            request_stream.seekp(0, std::ios::beg);
            std::stringstream ss_request;
            ss_request << request_stream.rdbuf();
            string str_request = ss_request.str();
            request_stream.seekp(req_length);
            */

            // Screw it I'll just rebuild it ALL here.
            // TODO fix above code so we only have one stream.
            // This is absolute nonsense.  But it got us going.

            g_ss.str(std::string());
            g_ss << "HTTP REQUEST:" << endl << "==============" << endl;

            g_ss << str_action << " " << path << " HTTP/1.1\r\n";
            g_ss << "Host: " << server << ":" << port << "\r\n";
            g_ss << "Accept: */*\r\n";
            g_ss << header;
            if (!username.empty())
            {
                g_ss << "Authorization: Basic ";
                stringstream u_and_p;
                u_and_p << username;
                u_and_p <<  ":";
                u_and_p << password;
                g_ss << base64_encode(
                    (unsigned char const*)u_and_p.str().c_str(),
                    (uint_fast32_t)u_and_p.str().length()
                );
                g_ss << "\r\n";
            }
            g_ss << "Content-Length: ";
            g_ss << body.size();
            g_ss << "\r\n";
            g_ss << "Connection: close\r\n";
            g_ss << "\r\n";
            g_ss << body;

            g_ss << "==============" << endl;
            log(LV_DEBUG,g_ss.str(),true);
        }

        // Start an asynchronous resolve to translate the server and service names
        // into a list of endpoints.
        tcp::resolver::query query(server, port);
        resolver_.async_resolve(
            query,
            boost::bind(
                &ClientBase<socket_type>::handle_resolve,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::iterator
            )
        );
    }

    template <class socket_type>
    void ClientBase<socket_type>::handle_resolve(const boost::system::error_code& err,
            tcp::resolver::iterator endpoint_iterator) {
        if (!err)
        {
            // Attempt a connection to each endpoint in the list until we
            // successfully establish a connection.
            aconnect(endpoint_iterator);

        } else {
            std::cout << "Error: " << err.message() << "\n";
        }
    }

    template <class socket_type>
    void ClientBase<socket_type>::handle_connect(const boost::system::error_code& err)
    {
        log(LV_DEBUG,"handle_connect start",true);

        if (!err)
        {
            // This is needed at this time for SSL.
            handshake();

            // The connection was successful. Send the request.
            boost::asio::async_write(
                    *socket_,
                    request_,
                    boost::bind(&ClientBase<socket_type>::handle_write_request, this,
                            boost::asio::placeholders::error));

        } else {
            std::cout << "Error: " << err.message() << "\n";
        }

        log(LV_DEBUG,"handle_connect end",true);
    }


    // MDM Top level response handler (poorly named IMHO but directly from boost example).
    template <class socket_type>
    void ClientBase<socket_type>::handle_write_request(
            const boost::system::error_code& err) {

        if (!err) {
            // Read the response status line. The response_ streambuf will
            // automatically grow to accommodate the entire line. The growth may be
            // limited by passing a maximum size to the streambuf constructor.
            boost::asio::async_read_until(
                    *socket_,
                    response_,
                    "\r\n",
                    boost::bind(&ClientBase<socket_type>::handle_read_status_line, this,
                            boost::asio::placeholders::error));

        } else {
            std::cout << "Error: " << err.message() << "\n";
        }
    }

    // MDM This is called once during http processing, and then calls handle_read_headers to finish.
    template <class socket_type>
    void ClientBase<socket_type>::handle_read_status_line(
            const boost::system::error_code& err)
    {
        log(LV_DEBUG,"handle_read_status_line start",true);

        if (!err) {
            // Check that response is OK.
            std::istream response_stream(&response_);
            std::string http_version;
            response_stream >> http_version;
            response_stream >> status_code_;
            std::string status_message;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
                log(LV_ERROR,"Invalid HTTP response");
                return;
            }

            if (status_code_ != 200)
            {
                stringstream ss;
                ss << "WARNING: Response did not return with 200 status code (" << status_code_ << ")\r\n";
                log(LV_WARNING,ss.str());

                // We could stop on incorrect status codes here.
                // But sometimes (eg) a 400 response code will come with more meaningful information in the body.
                // So let's always continue on to get the body.
            }

            // Read the response headers, which are terminated by a blank line.
            boost::asio::async_read_until(
                    *socket_,
                    response_,
                    "\r\n\r\n",
                    boost::bind(&ClientBase<socket_type>::handle_read_headers, this,
                            boost::asio::placeholders::error));
        } else {
            std::cout << "Error: " << err << "\n";
        }

        log(LV_DEBUG,"handle_read_status_line end",true);
    }


    // MDM This is called by handle_read_status_line.
    // It keeps reading the input until it hits a blank line.
    // Then it considers the rest to be the body, and calls handle_read_content to collect the rest of it.
    template <class socket_type>
    void ClientBase<socket_type>::handle_read_headers(
        const boost::system::error_code& err
    ) {
        log(LV_DEBUG,"handle_read_headers start",true);

        if (!err)
        {
            // Process the response headers.
            // Save them up in a vector.
            response_headers_.clear();
            std::istream response_stream(&response_);
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
            {
                response_headers_.push_back(header);
            }

            // Collect whatever content we already have.
            response_body_.clear();
            if (response_.size() > 0)
                add_next_response_chunk_to_body();

            // Start reading remaining data until EOF.
            boost::asio::async_read(
                *socket_,
                response_,
                boost::asio::transfer_at_least(1),
                boost::bind(
                    &ClientBase<socket_type>::handle_read_content,
                    this,
                    boost::asio::placeholders::error
                )
            );

        } else
        {
            std::cout << "Error: " << err << "\n";
        }

        log(LV_DEBUG,"handle_read_headers end",true);
    }

    // MDM This handles the rest of the body.
    template <class socket_type>
    void ClientBase<socket_type>::handle_read_content(
        const boost::system::error_code& err
    ) {
        if (!err)
        {
            // Write all of the data that has been read so far.
            add_next_response_chunk_to_body();

            // Continue reading remaining data until EOF.
            boost::asio::async_read(
                *socket_,
                response_,
                boost::asio::transfer_at_least(1),
                boost::bind(
                    &ClientBase<socket_type>::handle_read_content,
                    this,
                    boost::asio::placeholders::error
                )
            );

        } else if (err != boost::asio::error::eof)
        {
            std::cout << "Error: " << err << "\n";
        }
    }


    template <class socket_type>
    void ClientBase<socket_type>::add_next_response_chunk_to_body()
    {
        if (b_binary_result_)
        {
            std::istream response_stream(&response_);
            std::istreambuf_iterator<char> eos;
            std::string s(std::istreambuf_iterator<char>(response_stream), eos);
            response_body_ += s;
        }
        else
        {
            string body_part;
            std::istream response_stream(&response_);
            while (std::getline(response_stream, body_part))
            {
                response_body_ += body_part;

                // This makes the output prettier but can TRASH it
                // on any non-newline-terminated chunks (ie, the first).
                // response_body_ += "\r\n";
            }
        }
    }

}   // namespace QuickHttp

#endif  // client_base_hpp
