#ifndef SCRAP_HPP_
#define SCRAP_HPP_

#include <boost/config.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <sstream>

#include <utilities.hpp>  // For logging globals

using namespace std;
using namespace boost;


// ----------------------------------------------------------
// 1
namespace casty
{
    struct Base
    {
      virtual char kind() { return 'B'; }
    };
    struct Base2
    {
      virtual char kind2() { return '2'; }
    };
    struct Derived : public Base, Base2
    {
        virtual char kind() { return 'D'; }
    };
}
// ----------------------------------------------------------
// 2
// 3
#include <boost/cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>
// ----------------------------------------------------------
// 6
class blah
{
public:
    blah(int a1, int b1, string name = "unspecified" ) : a(a1), b(b1), name_(name) {}
    int a;
    int b;
    string name_;
    bool operator<(const blah& right) const { return (a==right.a)?(b<right.b):(a<right.a); }
};
struct blah_ptr_lessthan
{
    bool operator()(const blah* left, const blah* right)
    { return *left < *right; }
};
// ----------------------------------------------------------
// 7
class Value 
{
public:
    Value(const wstring& value) 
        {
            v = value;
        }

    Value(Value& other)
        {
            this->v = other.v; 
        }

    template<typename T>
    operator T() const
        {
            T reply;
            std::wistringstream is;
            is.str(v);
            is >> reply;
            return reply;
        } 

    operator wstring() const 
        {
            return v;
        }


private:
    wstring v;
};
class Config
{
public:
  virtual Value getValue(const string& key) const = 0;

  Value operator()(const string& key) 
  {
    return getValue(key);
  }
};
class ArgsConfig : public Config{   Value getValue(const string& key) const {    return Value(L"test"); }};
// ----------------------------------------------------------
// 8 
#include <unordered_map>		// boost and C++11 overlap is getting tricky; this is for C++11 unordered_map and unordered_multimap
typedef std::unordered_map<std::string, int> hashmap;
typedef std::unordered_multimap<std::string, int> hashmmap;
// ----------------------------------------------------------
// 9 
// BGL support
#include <boost/config.hpp>
#include <boost/utility.hpp>                // for boost::tie
#include <boost/graph/adjacency_list.hpp>
// ----------------------------------------------------------
// 11
// unordered set of pointers
// To use c++11 undordered containers with object keys, we need hash_value and equal_to.
// When using pointers to objects, we need the functions to use pointers, and we have to specify them when defining the set or map.
// hash_value and equal_to functors work perfectly with pointer parameters.
// Remember to keep them synced on the same vars or you will get hash problems.
struct blah_pointers_equal
{
    bool operator()(const blah* l, const blah* r) const
    {
        return l->a == r->a && l->b == r->b;
    }
};
struct blah_pointer_hash
{
    size_t operator()(const blah* const& pb) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, pb->a);
        boost::hash_combine(seed, pb->b);
        return seed;
    }
};
typedef boost::unordered_set<blah*,blah_pointer_hash,blah_pointers_equal> BlahPointerSet;

struct blah_pointers_alt_equal
{
    bool operator()(const blah* l, const blah* r) const
    {
        return l->a == r->a;
    }
};
struct blah_pointer_alt_hash
{
    size_t operator()(const blah* const& pb) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, pb->a);
        return seed;
    }
};
typedef boost::unordered_set<blah*,blah_pointer_alt_hash,blah_pointers_alt_equal> BlahPointerAltSet;
// ----------------------------------------------------------
// 12
#include <boost/asio.hpp>
// ----------------------------------------------------------


#endif // SCRAP_HPP_
