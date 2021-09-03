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
// 19

// Validate that a set of top-level properties of a boost JSON object exist in the expected format.
// Input:
//    // A tuple of < fieldName, jsonType, bOptional >
//    vector< std::tuple< string, boost::json::kind, bool > > check = 
//    {
//        { "myFirstField", boost::json::string, true },
//        { "anotherField", boost::json::int_64, false },
//    }
// Output:
//    { true if all mandatory fields were found, true if all optional fields were found }
pair< bool, bool> validateObject( boost::json::object obj, vector< std::tuple< string, boost::json::kind, bool > > check )
{
    pair< bool, bool > result = { true, true };
    for ( auto& c: check )
        if ( obj[get<0>(c)].kind() != get<1>(c) )
            if ( get<2>(c) )
                result.second = false;
            else
                result.first = false;

    return result;
}
// Boost JSON <-> struct conversion
// THIS IS WHAT YOU WANT TO DO whenever there is a specific set of known fields to extract.
// However, it will fail if any are null or missing!
namespace my_order {

    using namespace boost::json;
    using string = std::string;
    using string_view = boost::json::string_view;

    class OrderConfirmation {
    public:
        // int64_t qty;
        // int64_t filled_qty;
        string qty;
        string filled_qty;
        string side;
        string client_order_id;
    };

    // FILL JSON FROM CLASS
    // --------------------
    void tag_invoke( value_from_tag, value& jv, OrderConfirmation const& o )
    {
        jv = {
            { "qty" , o.qty },
            { "filled_qty" , o.filled_qty },
            { "side", o.side },
            { "client_order_id", o.client_order_id }
        };        
    }

    // FILL CLASS FROM JSON
    // --------------------
    // This helper function deduces the type and assigns the value with the matching key
    template<class T>
    void extract( object const& obj, T& t, string_view key )
    {
        t = value_to<T>( obj.at( key ) );
    }

    OrderConfirmation tag_invoke( value_to_tag< OrderConfirmation >, value const& jv )
    {
        OrderConfirmation o;
        object const& obj = jv.as_object();
        extract( obj, o.qty, "qty" );
        extract( obj, o.filled_qty, "filled_qty" );
        extract( obj, o.side, "side" );
        extract( obj, o.client_order_id, "client_order_id" );
        return o;
    }
}

// ----------------------------------------------------------


#endif // SCRAP_HPP_
