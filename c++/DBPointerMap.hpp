//-------------------------------------------------------------------//
//	Copyright � 2015 A better Software.
//-------------------------------------------------------------------//

#pragma once

#include <utilities.hpp>


//-------------------------------------------------------------------//
// DBPointerMap
//-------------------------------------------------------------------//
// History
// -------
// v1.0 7/02/2015   Created for A better Trader
// v1.1 7/03/2015   Updated to recommend storing references not iterators; added notes on why
// ---- 8/06/2016   >>>>>> DEPRECATED >>> refer to PersistentIDObject in STLContainers.hpp instead.
//-------------------------------------------------------------------//
// Why a map of pointers?
//
//    when a container needs to move its contents in memory, pointers make that efficient
//    the majority of object contents need to be mutable
//
//    three options:
//        use a key in the set, derive a class from it with the contents
//            + small key
//            + encapsulation
//            - requires mutable to solve the const problem
//        use a key in the set, key includes a mutable object
//            + encapsulation
//            - weird bc everything uses a const object but we have const functions like save() that change the mutable subobject
//        use a map
//            + small key
//            + solves const problem bc value is totally mutable by design
//            + simpler equal and hash functions
//            - no encapsulation, have to deal with a pair instead of an object
//            - the variable names (especially using "first" for the key) are imprecise
//            + we can have multiple keys (although in this case the object should definitely include const copies of them)
//
// Why include DB behavior?
//
//      major objects need to be stored
//      major objects need to reference other major objects
//      it is very efficient to use an integer-based primary key for foreign keys, eg in sqlite, so we bake it in
//      we can standardize:
//          for new objects, we must first write to db to get a unique db id, then store that id in memory
//          id [-1] = not yet assigned by db
//
// USAGE:
//
//  1) Derive a class from DPBase.
//
//  2) Create a new map type as follows:
//
//      typedef std::pair<const int_fast32_t,WidgetDetails*> Widget;
//      typedef std::unordered_map<int_fast32_t,WidgetDetails*> Widgets;
//      typedef Widgets::iterator WidgetIt;
//
//  3) When containing a widget in another object, use a const Widget& reference.  Details:
//          I thought about storing a WidgetIt for maximum flexibility, but unordered_map iterators MAY be invalidated on insert if a rehash is required; see:
//              http://stackoverflow.com/questions/16781886/can-we-store-unordered-maptiterator
//          A const Widget& reference is guaranteed to remain valid, even if you got it through an iterator.
//              (VERIFY this - I am 90% sure they are referring to Widget and not WidgetDetails here, but use of the term "value" makes it a bit fuzzy, as map "values" are <key,value> pairs).
//
//  4) A second index into the set of objects can be maintained; it is best to use a datastore with [Add/Update/Find/Remove]Widget functions to coordinate them.
//
//-------------------------------------------------------------------//


class DPBase
{
public:
    void save();

protected:
    bool bChanged_;
};


// Here's a map of DPBase, but not really useful outside of this module.
typedef std::pair<int_fast32_t,DPBase*> DPPair;
typedef std::unordered_map<int_fast32_t,DPBase*> DBPointerMap;
typedef DBPointerMap::iterator DPIt;

// Obviously we compare the key to tell if they are equal, but adding this operator for your Widget can make the code a little prettier.
static bool operator==(const DPPair& l,const DPPair& r)
{
    return l.first == r.first;
}

