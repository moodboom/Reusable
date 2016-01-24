#include <boost/config.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <sstream>

using namespace std;

// 13 ==================================================================================
//    BOOST ASIO SSL how do we get it into http?  here we go with the ssl example:
//			http://www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/example/cpp03/ssl/client.cpp

// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// #include <cstdlib>
// #include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

enum { max_length = 1024 };

class client
{
public:
  client(boost::asio::io_service& io_service,
      boost::asio::ssl::context& context,
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
    : socket_(io_service, context)
  {
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback(
        boost::bind(&client::verify_certificate, this, _1, _2));

    boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
        boost::bind(&client::handle_connect, this,
          boost::asio::placeholders::error));
  }

  bool verify_certificate(bool preverified,
      boost::asio::ssl::verify_context& ctx)
  {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
  }

  void handle_connect(const boost::system::error_code& error)
  {
    if (!error)
    {
      socket_.async_handshake(boost::asio::ssl::stream_base::client,
          boost::bind(&client::handle_handshake, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Connect failed: " << error.message() << "\n";
    }
  }

  void handle_handshake(const boost::system::error_code& error)
  {
    if (!error)
    {
      std::cout << "Enter message: ";
      std::cin.getline(request_, max_length);
      size_t request_length = strlen(request_);

      boost::asio::async_write(socket_,
          boost::asio::buffer(request_, request_length),
          boost::bind(&client::handle_write, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      std::cout << "Handshake failed: " << error.message() << "\n";
    }
  }

  void handle_write(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
      boost::asio::async_read(socket_,
          boost::asio::buffer(reply_, bytes_transferred),
          boost::bind(&client::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      std::cout << "Write failed: " << error.message() << "\n";
    }
  }

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
      std::cout << "Reply: ";
      std::cout.write(reply_, bytes_transferred);
      std::cout << "\n";
    }
    else
    {
      std::cout << "Read failed: " << error.message() << "\n";
    }
  }

private:
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  char request_[max_length];
  char reply_[max_length];
};



// ==================================================================================
// ==================================================================================
int main( int argc, char * argv[] )
// ==================================================================================
// ==================================================================================
{
    std::cout << "NOTE: NDEBUG is ";
    #ifndef NDEBUG
        std::cout << "NOT";
    #endif
    std::cout << " defined\n\n";


    std::cout << "== 12 === asio ssl =======" << endl;


    //    BOOST ASIO SSL how do we get it into http?  here we go with the ssl example:
    //			http://www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/example/cpp03/ssl/client.cpp
    //

      try
      {
    	  string host("bitpost.com");
    	  string port("443");

          cout << "ASIO setting up " << host << ":" << port << "..." << endl;

			/* boost asio ssl client example code
			boost::asio::io_service io_service;

			boost::asio::ip::tcp::resolver resolver(io_service);
			boost::asio::ip::tcp::resolver::query query(host, port);
			boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

			boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

			// What should we use for this?
			// This file, copied out to a new local file named [ca_startcom.pem]:
			//
			//		/usr/share/ca-certificates/mozilla/StartCom_Certification_Authority.crt
			//
			ctx.load_verify_file("ca_startcom.pem");

			cout << "ASIO requesting " << host << ":" << port << "..." << endl;
			cout << "=================" << endl;
			cout << "Just hit ENTER..." << endl;
			cout << "=================" << endl;

			client c(io_service, ctx, iterator);

			io_service.run();

			cout << "ASIO done." << endl;
			*/

          	// boost docs give a little short snippet to set up the SSL layer
            // this works!

			using boost::asio::ip::tcp;
			namespace ssl = boost::asio::ssl;
			typedef ssl::stream<tcp::socket> ssl_socket;

			// Create a context that uses the default paths for
			// finding CA certificates.
			ssl::context ctx(ssl::context::sslv23);
			ctx.set_default_verify_paths();

			// Open a socket and connect it to the remote host.
			boost::asio::io_service io_service;
			ssl_socket sock(io_service, ctx);
			tcp::resolver resolver(io_service);
			tcp::resolver::query query("bitpost.com", "https");

			boost::asio::connect(sock.lowest_layer(), resolver.resolve(query));
			sock.lowest_layer().set_option(tcp::no_delay(true));

			// Perform SSL handshake and verify the remote host's
			// certificate.
			sock.set_verify_mode(ssl::verify_peer);
			sock.set_verify_callback(ssl::rfc2818_verification("bitpost.com"));
			sock.handshake(ssl_socket::client);

            cout << "sock.handshake() done..." << endl;

      }
      catch (std::exception& e)
      {
        std::cerr << "Exception: " << e.what() << endl;
      }




    // We can keep it running if needed to better see the output.
    // while (true)
    // {}

}
