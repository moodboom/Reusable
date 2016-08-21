//
// This code is based on the HTTP Server 3 boost example from:
//     http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/examples.html
//
// Description:
//     HTTP Server 3
//     An HTTP server using a single io_service and a thread pool calling io_service::run().
//
// Original copyright notice:
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BASE_CHUNKED_HANDLER_HPP
#define BASE_CHUNKED_HANDLER_HPP

#include "http_server_components/mime_types.hpp"
#include "http_server_components/reply.hpp"
#include "http_server_components/request.hpp"


// This functor creates the server reply for the given request.
// You must override it to provide custom server replies.
class base_chunked_handler
  : private boost::noncopyable
{
public:

	explicit base_chunked_handler()
	{}

	virtual void operator() (std::string response_chunk)
	{
		// The default handler does nothing.
	}
};


#endif // BASE_CHUNKED_HANDLER_HPP
