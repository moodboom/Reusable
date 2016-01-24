#include "../base64.hpp"
#include "http_chunked_client.hpp"

http_chunked_client::http_chunked_client(
	boost::asio::io_service& io_service,
	base_chunked_handler& custom_handler,
	const std::string& server,
	const std::string& port, // Can be "80", "2345", "http", "ftp", etc.
	const std::string& path,
	const std::string& body,    // directly placed into request
	const string& username,
    const string& password
) :

	// Init vars
	custom_handler_(custom_handler),
	resolver_(io_service),
	socket_(io_service),
    n_event_chunk_bytes_needed_(numeric_limits<size_t>::max()),
    status_code_(503)                                               // default to 503 Service Unavailable until we know differently
{
	// Form the request. 
	std::ostream request_stream(&request_);

	request_stream << "GET " << path << " HTTP/1.1\r\n";
	request_stream << "Host: " << server << ":" << port << "\r\n";
	request_stream << "Accept: */*\r\n";

	// DEBUG
	// I don't think these are REQUIRED...
	// request_stream << "Content-Type: application/x-www-form-urlencoded\r\n";
	// request_stream << "Accept-Encoding: gzip, deflate\r\n";

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

	if (!body.empty())
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

	// Start an asynchronous resolve to translate the server and service names
	// into a list of endpoints.
	tcp::resolver::query query(server, port);
	resolver_.async_resolve(
		query,
		boost::bind(
			&http_chunked_client::handle_resolve,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator
		)
	);
}

void http_chunked_client::handle_resolve(const boost::system::error_code& err,
		tcp::resolver::iterator endpoint_iterator) {
	if (!err) {
		// Attempt a connection to each endpoint in the list until we
		// successfully establish a connection.
		boost::asio::async_connect(
				socket_,
				endpoint_iterator,
				boost::bind(&http_chunked_client::handle_connect, this,
						boost::asio::placeholders::error));

	} else {
		std::cout << "Error: " << err.message() << "\n";
	}
}

void http_chunked_client::handle_connect(const boost::system::error_code& err) {
	if (!err) {
		// The connection was successful. Send the request.
		boost::asio::async_write(
				socket_,
				request_,
				boost::bind(&http_chunked_client::handle_write_request, this,
						boost::asio::placeholders::error));

	} else {
		std::cout << "Error: " << err.message() << "\n";
	}
}


// MDM Top level response handler (poorly named IMHO but directly from boost example).
void http_chunked_client::handle_write_request(
		const boost::system::error_code& err) {

	if (!err) {
		// Read the response status line. The response_ streambuf will
		// automatically grow to accommodate the entire line. The growth may be
		// limited by passing a maximum size to the streambuf constructor.
		boost::asio::async_read_until(
				socket_,
				response_,
				"\r\n",
				boost::bind(&http_chunked_client::handle_read_status_line, this,
						boost::asio::placeholders::error));

	} else {
		std::cout << "Error: " << err.message() << "\n";
	}
}

// MDM This is called once during http processing, and then calls handle_read_headers to finish.
void http_chunked_client::handle_read_status_line(
		const boost::system::error_code& err) {
	if (!err) {
		// Check that response is OK.
		std::istream response_stream(&response_);
		std::string http_version;
		response_stream >> http_version;
		response_stream >> status_code_;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
			std::cout << "Invalid response\n";
			return;
		}
		if (status_code_ != 200) {
			std::cout << "Response returned with status code ";
			std::cout << status_code_ << "\n";
			return;
		}

		// Read the response headers, which are terminated by a blank line.
		boost::asio::async_read_until(
				socket_,
				response_,
				"\r\n\r\n",
				boost::bind(&http_chunked_client::handle_read_headers, this,
						boost::asio::placeholders::error));
	} else {
		std::cout << "Error: " << err << "\n";
	}
}

// MDM This is called by handle_read_status_line.
// It keeps reading the input until it hits a blank line.
// NOTE: is that reliable enough???
// Then it considers the rest to be the body, and calls handle_read_content to collect the rest of it.
void http_chunked_client::handle_read_headers(
	const boost::system::error_code& err
) {
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
		if (response_.size() > 0)
			add_next_response_chunk_to_body();

		// Start reading remaining data until EOF.
		boost::asio::async_read(
			socket_,
			response_,
			boost::asio::transfer_at_least(1),
			boost::bind(
				&http_chunked_client::handle_read_content,
				this,
				boost::asio::placeholders::error
			)
		);

	} else
	{
		std::cout << "Error: " << err << "\n";
	}
}

// MDM This handles the rest of the body.
void http_chunked_client::handle_read_content(
	const boost::system::error_code& err
) {
	if (!err)
	{
        // Write all of the data that has been read so far.
		add_next_response_chunk_to_body();

		// Continue reading remaining data until EOF.
		boost::asio::async_read(
			socket_,
			response_,
			boost::asio::transfer_at_least(1),
			boost::bind(
				&http_chunked_client::handle_read_content,
				this,
				boost::asio::placeholders::error
			)
		);

	} else if (err != boost::asio::error::eof)
	{
		std::cout << "Error: " << err << "\n";
	}
}


void http_chunked_client::add_next_response_chunk_to_body()
{
	std::istream response_stream(&response_);
	std::istreambuf_iterator<char> eos;
	std::string s(std::istreambuf_iterator<char>(response_stream), eos);

    process_chunk(s);
}


void http_chunked_client::process_chunk(string str_chunk)
{
    // Here is what complete chunks look like...
    // from http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    /*
       Chunked-Body   = *chunk
                        last-chunk
                        trailer
                        CRLF

       chunk          = chunk-size [ chunk-extension ] CRLF
                        chunk-data CRLF
       chunk-size     = 1*HEX
       last-chunk     = 1*("0") [ chunk-extension ] CRLF

       chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
       chunk-ext-name = token
       chunk-ext-val  = token | quoted-string
       chunk-data     = chunk-size(OCTET)
       trailer        = *(entity-header CRLF)

    */

    if (n_event_chunk_bytes_needed_ == numeric_limits<size_t>::max())
    {
        // This is a new event, pull out the chunk size in bytes.
        // It will be hex digits then CR/LF, according to http chunked RFC.
        
        // NOTE that we MAY have already gotten part of it.
        str_chunk = str_partial_hex_ + str_chunk;

        size_t n_first_line = str_chunk.find_first_of("\r\n");
		if (n_first_line == std::string::npos)
        {
            // We didn't get a full CRLF, we're not ready to look for a chunk yet.
            // Cache what we have and keep processing.
            str_partial_hex_ = str_chunk;
            return;
        }
        str_partial_hex_.clear();

        try
        {
            // This guy crashes on occasion?  
            // Red herring I think, due to other issues - it's probably fine.  
            // But we'll switch to stream for now.
            // n_event_chunk_bytes_needed_ = std::stoul(str_hex_size, nullptr, 16);

            unsigned int x;   
            std::stringstream ss;
            ss << std::hex << str_chunk.substr(0,n_first_line);
            ss >> x;
            n_event_chunk_bytes_needed_ = static_cast<size_t>(x);
        }
        catch(...)
        {
            // TODO not sure what to do here.  
            // We were not able to read what should have been a clean hex value.
            // Perhaps we won't hit this now...
            cout << "ERROR processing http chunked byte count, ignoring chunk..." << endl;
            n_event_chunk_bytes_needed_ = 0;
        }

        if (str_chunk.size() > n_first_line+2)
            str_chunk = str_chunk.substr(n_first_line+2);
        else
            str_chunk.clear(); // no chunk body yet, keep reading
    }

    if (str_chunk.size() < n_event_chunk_bytes_needed_)
    {
        // Add it to our buffer and return, we need to keep listening for more.
        n_event_chunk_bytes_needed_ -= str_chunk.size();
        str_event_chunk_ += str_chunk;
        return;
    }

    string str_remainder;
    if (str_chunk.size() > n_event_chunk_bytes_needed_)
    {
        str_remainder = str_chunk.substr(n_event_chunk_bytes_needed_,str_chunk.size()-n_event_chunk_bytes_needed_);
        str_chunk = str_chunk.substr(0,n_event_chunk_bytes_needed_);

        // If all we have is "\r\n" we're done for now.
        if (str_remainder == "\r\n")
            str_remainder.clear();
    }

    // We now have just what we needed.
    // Let the handler process it.
    str_event_chunk_ += str_chunk;
    custom_handler_(str_event_chunk_);

    // Reset.
    str_event_chunk_.clear();
    n_event_chunk_bytes_needed_ = numeric_limits<size_t>::max();

    // Go again if we have a remainder...
    if (str_remainder.size() > 0)
        process_chunk(str_remainder);
}

