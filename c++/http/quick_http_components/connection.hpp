//
// This code is based on the HTTP Server 3 boost example from:
//     http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/examples.html
//
// Description:
//     HTTP Server 3
//     An HTTP server using a single io_service and a thread pool calling io_service::run().
//
// Ideally we could use the example as-is, but it uses a file-based scheme that is not at all RESTful.
// To update this code when a new example becomes available,
// do a diff between the old and new example code and paste in the changes.
//
//
// Original copyright notice:
//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "../quick_http_server_handler.hpp"


namespace QuickHttp
{
    /// Represents a single connection from a client.
    class connection : public boost::enable_shared_from_this<connection>, private boost::noncopyable
    {
    public:

        /// Construct a connection with the given io_service.
        inline explicit connection(
            boost::asio::io_service& io_service,
            server_handler& handler
        );

        /// Get the socket associated with the connection.
        inline boost::asio::ip::tcp::socket& socket();
        inline boost::asio::ip::tcp::socket& ssl_socket();

        /// Start the first asynchronous operation for the connection.
        inline void start();

    private:
        /// Handle completion of a read operation.
        inline void handle_read(const boost::system::error_code& e,
          std::size_t bytes_transferred);

        /// Handle completion of a write operation.
        inline void handle_write(const boost::system::error_code& e);

        /// Strand to ensure the connection's handlers are not called concurrently.
        boost::asio::io_service::strand strand_;

        /// Socket for the connection.
        // MDM NOTE: THIS IS NOT GOOD ENOUGH to work in base, it will not work for HTTPS.
        // For that, we need boost::asio::ssl::stream<boost::asio::ip::tcp::socket>
        boost::asio::ip::tcp::socket socket_;
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_;

        /// The handler used to process the incoming request.
        server_handler& request_handler_;

        /// Buffer for incoming data.
        boost::array<char, 8192> buffer_;

        /// The incoming request.
        Request request_;

        /// The parser for the incoming request.
        request_parser request_parser_;

        /// The reply to be sent back to the client.
        reply reply_;
    };

    typedef boost::shared_ptr<connection> connection_ptr;


    // =================== IMPLEMENTATION ========================

    connection::connection(
        boost::asio::io_service& io_service,
        server_handler& handler
    ) :
        strand_(io_service),
        socket_(io_service),
        request_handler_(handler)
    {
    }

    boost::asio::ip::tcp::socket& connection::socket()
    {
      return socket_;
    }

    boost::asio::ip::tcp::socket& connection::ssl_socket()
    {
      return ssl_socket_;
    }

    void connection::start()
    {
      socket_.async_read_some(boost::asio::buffer(buffer_),
          strand_.wrap(
            boost::bind(&connection::handle_read, shared_from_this(),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred)));
    }

    void connection::handle_read(const boost::system::error_code& e,
        std::size_t bytes_transferred)
    {
      if (!e)
      {
        boost::tribool result;

        // MDM Capture the body.
        char* body;
        boost::tie(result, body) = request_parser_.parse(
            request_, buffer_.data(), buffer_.data() + bytes_transferred);

        // (old way)
        // boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
        //     request_, buffer_.data(), buffer_.data() + bytes_transferred);

        if (result)
        {
            // MDM Assign the body if any.
            std::size_t body_length = std::min(request_handler_.max_body_size_,buffer_.data() - body + bytes_transferred);
            if (body_length > 0)
                request_.body_.assign(body,body_length);

            // use the functor to handle the request
            request_handler_(request_, reply_);

          boost::asio::async_write(socket_, reply_.to_buffers(),
              strand_.wrap(
                boost::bind(&connection::handle_write, shared_from_this(),
                  boost::asio::placeholders::error)));
        }
        else if (!result)
        {
          reply_ = reply::stock_reply(reply::bad_request);
          boost::asio::async_write(socket_, reply_.to_buffers(),
              strand_.wrap(
                boost::bind(&connection::handle_write, shared_from_this(),
                  boost::asio::placeholders::error)));
        }
        else
        {
          socket_.async_read_some(boost::asio::buffer(buffer_),
              strand_.wrap(
                boost::bind(&connection::handle_read, shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred)));
        }
      }

      // If an error occurs then no new asynchronous operations are started. This
      // means that all shared_ptr references to the connection object will
      // disappear and the object will be destroyed automatically after this
      // handler returns. The connection class's destructor closes the socket.
    }

    void connection::handle_write(const boost::system::error_code& e)
    {
      if (!e)
      {
        // Initiate graceful connection closure.
        boost::system::error_code ignored_ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
      }

      // No new asynchronous operations are started. This means that all shared_ptr
      // references to the connection object will disappear and the object will be
      // destroyed automatically after this handler returns. The connection class's
      // destructor closes the socket.
    }

} // namespace QuickHttp

#endif // CONNECTION_HPP
