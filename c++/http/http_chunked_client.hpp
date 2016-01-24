//
// This code is based on the HTTP Client boost example from:
//     http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/examples.html
// 
// Description:
//     HTTP CClient
//     Example programs implementing simple HTTP 1.0 clients. These examples show how to use the read_until and async_read_until functions. 
// 
// We converted that into a REST-cabable client.  And then converted that into a client that can process a chunked response.


#ifndef http_chunked_client_hpp
#define http_chunked_client_hpp

#include "../basic_types.hpp"
#include "base_chunked_handler.hpp"

using boost::asio::ip::tcp;


class http_chunked_client
{
public:
	http_chunked_client(
		boost::asio::io_service& io_service,
		base_chunked_handler& custom_handler,
		const std::string& server,
		const std::string& port, 
		const std::string& path,
        const std::string& body = "",
		const string& username = "",
        const string& password = ""
	);

    // MDM We collect up the response for post-processing after the call is made.
	unsigned int status_code_;
	Strings response_headers_;

private:
	
	void handle_resolve(
		const boost::system::error_code& err,
		tcp::resolver::iterator endpoint_iterator
	);

	void handle_connect(const boost::system::error_code& err);

	void handle_write_request(const boost::system::error_code& err);

	void handle_read_status_line(const boost::system::error_code& err);

	void handle_read_headers(const boost::system::error_code& err);

	void handle_read_content(const boost::system::error_code& err);

	void add_next_response_chunk_to_body();

    void process_chunk(string str_chunk);
		
	tcp::resolver resolver_;
	tcp::socket socket_;
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;

    // Track partial chunks, and only report "complete" ones to the caller.
    string str_partial_hex_;
    string str_event_chunk_;
    size_t n_event_chunk_bytes_needed_;

    // The caller should derive a custom handler, we'll track it here and call it after events fully arrive.
	base_chunked_handler& custom_handler_;
};


#endif      // http_chunked_client_hpp
