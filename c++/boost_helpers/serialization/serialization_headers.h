#ifndef SERIALIZATION_HEADERS_H
#define SERIALIZATION_HEADERS_H


// =======================
// BOOST SERIALIZE HEADERS
// =======================
// MDM NOTE that the order of these includes comes
// directly from the serialize demo code - I don't
// recommend changing them unless absolutely necessary,
// since they work as they are for cross-platform.
// =======================
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>

// MDM Not needed, and causes "already defined" errors if
// included in more than one module.
// #include <boost/archive/tmpdir.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/is_abstract.hpp>
// =======================


// MDM This will more often than not be needed.
#include <ustring.h>
#include <boost_helpers/serialization/serialize_ustring.hpp>


#endif  // SERIALIZATION_HEADERS_H
