//-------------------------------------------------------------------//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#if !defined(STL_EXTENSIONS_H)
#define STL_EXTENSIONS_H

#include <vector>

#include "StandardTypes.h"

#include "AssertHelpers.h"			// For ASSERT()

#include "STLContainers.h"			// Additional container extensions


typedef std::vector< Byte		>	ByteVector;
typedef std::vector< Short		>	ShortVector;
typedef std::vector< Long		>	LongVector;
typedef std::vector< Huge		>	HugeVector;
typedef std::vector< uByte		>	uByteVector;
typedef std::vector< uShort	>	uShortVector;
typedef std::vector< uLong		>	uLongVector;
typedef std::vector< uHuge		>	uHugeVector;
typedef std::vector< Float		>	FloatVector;
typedef std::vector< Double	>	DoubleVector;
typedef std::vector< bool		>	BoolVector;

// Add other types as needed:

#include <deque>
typedef std::deque< Long >		LongDeque;


// Found this good idea in some post on Experts-Exchange.com...
// Here's a convenient & general utility function
//  You could also use boost::lexical_cast instead...
#include <sstream>
template<typename T>
std::string to_string(const T& t)
{
  std::ostringstream strm;
  strm << t;
  return strm.str();
}


#endif // !defined(STL_EXTENSIONS_H)
