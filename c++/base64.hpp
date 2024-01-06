#include <cstdint>      // for uint_fast32_t
#include <string>

std::string base64_encode(unsigned char const* , uint_fast32_t len);
std::string base64_decode(std::string const& s);
