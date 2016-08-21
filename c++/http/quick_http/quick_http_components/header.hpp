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
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>


namespace QuickHttp {

struct header
{
  std::string name;
  std::string value;
};

} // namespace QuickHttp


#endif // HEADER_HPP
