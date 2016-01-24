// ------------------------------------------
// server_base.hpp
// ------------------------------------------
// This class is the base for quick_http_server, see that header for usage documentation.
//
// ------------------------------------------
// This code is based on the HTTP server boost example from:
//     http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/examples.html
// 
// This code borrows excellent template handling of sockets, ssl certificates, etc., from this project:
//      https://github.com/eidheim/Simple-Web-Server
// ------------------------------------------

#ifndef server_base_hpp
#define server_base_hpp

#include <utilities.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include "connection.hpp"
#include "../quick_http_server_handler.hpp"


namespace QuickHttp
{
    // ==================================
    // ServerBase
    // ==================================
    // This abstract base class is not used directly.
    // See Server<> implementations in quick_http_server.hpp.
    // ==================================

    template <class socket_type>
    class ServerBase : private boost::noncopyable
    {
    public:

        /// Run the server's io_service loop.
        /// This will get called directly by the user.
        void run();

    protected:

        inline ServerBase(
                boost::asio::io_service& io_service,
                base_server_handler& custom_handler,
                const std::string& address,
                const std::string& port,
                std::size_t thread_pool_size
        );

        /// Initiate an asynchronous accept operation.
        virtual void start_accept() = 0;

        /// Handle completion of an asynchronous accept operation.
        virtual void handle_accept(const boost::system::error_code& e) = 0;

        /// Handle a request to stop the server.
        void handle_stop();

        // The caller should derive a custom handler, we'll track it here.
        base_server_handler& m_custom_handler;

        /// The number of threads that will call io_service::run().
        std::size_t thread_pool_size_;

        /// The io_service used to perform asynchronous operations.
        boost::asio::io_service& io_service_;

        /// The signal_set is used to register for process termination notifications.
        boost::asio::signal_set signals_;

        /// Acceptor used to listen for incoming connections.
        boost::asio::ip::tcp::acceptor acceptor_;

        /// The next connection to be accepted.
        connection_ptr new_connection_;
    };


    // =================== IMPLEMENTATION ========================

    template <class socket_type>
    ServerBase<socket_type>::ServerBase(
            boost::asio::io_service& io_service,
            base_server_handler& custom_handler,
            const std::string& address,
            const std::string& port,
            std::size_t thread_pool_size
    ) :

        // Init vars
        io_service_(io_service),
        thread_pool_size_(thread_pool_size),
        signals_(io_service),
        acceptor_(io_service),
        new_connection_(),
        m_custom_handler(custom_handler)

    {
        // Register to handle the signals that indicate when the server should exit.
        // It is safe to register for the same signal multiple times in a program,
        // provided all registration for the specified signal is made through Asio.
        signals_.add(SIGINT);
        signals_.add(SIGTERM);
        #if defined(SIGQUIT)
          signals_.add(SIGQUIT);
        #endif // defined(SIGQUIT)
        signals_.async_wait(boost::bind(&ServerBase<socket_type>::handle_stop, this));

        // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(address, port);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();
    }

    /// Run the server's io_service loop.
    template <class socket_type>
    void ServerBase<socket_type>::run()
    {
      // Create a pool of threads to run all of the io_services.
      std::vector<boost::shared_ptr<boost::thread> > threads;
      for (std::size_t i = 0; i < thread_pool_size_; ++i)
      {
        boost::shared_ptr<boost::thread> thread(new boost::thread(
              boost::bind(&boost::asio::io_service::run, &io_service_)));
        threads.push_back(thread);
      }

      // Wait for all threads in the pool to exit.
      for (std::size_t i = 0; i < threads.size(); ++i)
      {
        threads[i]->join();
      }
    }

    /// Handle a request to stop the server.
    template <class socket_type>
    void ServerBase<socket_type>::handle_stop()
    {
        io_service_.stop();
        log(LV_INFO,"Stop signal received, stopping...");
    }


    // GENERIC TEMPLATE specification of the class.
    // Then we'll do specializations (HTTP, HTTPS) next.
    template<class socket_type>
    class Server : public ServerBase<socket_type> {};

    // The types we'll be using to specialize.
    typedef boost::asio::ip::tcp::socket HTTP;
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> HTTPS;

}   // namespace QuickHttp

#endif  // server_base_hpp
