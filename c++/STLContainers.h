//-------------------------------------------------------------------//
//	Copyright � 2001-2016 A better Software, Inc.
//-------------------------------------------------------------------//

#if !defined(STL_CONTAINERS_H)
#define STL_CONTAINERS_H

#include <map>
#include <list>
#include <vector>
#include <algorithm>			// For lower_bound
#include <sstream>              // For stringstream - without this Qt will get confused with its stream classes
#include <boost/lexical_cast.hpp>
#ifdef QT_CORE_LIB
    #include "ustring.h"
#endif


using namespace std;


//-------------------------------------------------------------------//
// TraceMap()																			//
//-------------------------------------------------------------------//
// This function allows you to dump a trace of a std::map object
// to the debug window.
//
// NOTE: this requires operator<<() for the first and second class types
// of your map.  Make sure you provide one.  e.g.:
//
//		void operator<<( std::ostringstream strm, const MyInfo& info )
//		{
//			strm << _T("( ");
//			strm << info.m_nX;
//			strm << _T(", ");
//			strm << info.m_nY;
//			strm << _T(" )\n");
//		}
//
//-------------------------------------------------------------------//
template <typename T1, typename T2>
void TraceMap( std::map< T1, T2 >& mapTarget, int nMaxObjects = 100 )
{
	#ifdef _DEBUG
		ustring strMsg;
		TRACE( _T("\n===========\n") );
		TRACE( _T(  " MAP TRACE\n") );
		TRACE( _T(  "===========\n") );
		strMsg.printf(
			2000,
			_T(  "Size: %d\n"),
			mapTarget.size()
		);
		TRACE( strMsg.c_str() );
		TRACE( _T(  "===========\n") );
		std::map< T1, T2 >::iterator it;
		int nCounter = 0;
		for ( it = mapTarget.begin(); it != mapTarget.end(); it++, nCounter++ )
		{
			strMsg.printf(
				2000,
				_T("\tItem %d: %s: %s\n"),
				nCounter,
				to_string(
					it->first
				).c_str(),
				to_string(
					it->second
				).c_str()
			);
			TRACE( strMsg.c_str() );

			if ( nCounter >= nMaxObjects )
			{
				TRACE( _T("...\n") );
				break;
			}
		}
		TRACE( _T(  "===========\n\n") );
	#endif
}


//-------------------------------------------------------------------//
// TraceVector()																		//
//-------------------------------------------------------------------//
// This function allows you to dump a trace of a std::vector object
// to the debug window.
//
// NOTE: this requires operator<<() for the class type of your vector.
// Make sure you provide one.  e.g.:
//
//		std::stringstream& operator<<( std::stringstream strm, const MyInfo& info )
//		{
//			strm << _T("( ");
//			strm << info.m_nX;
//			strm << _T(", ");
//			strm << info.m_nY;
//			strm << _T(" )\n");
//          return strm;
//		}
//
// NOTE that I had a hard time getting this to work using a member function
// for operator<<(), for some reason.
//-------------------------------------------------------------------//
template <typename T>
void TraceVector( std::stringstream& strm, std::vector< T >& vectorTarget, int nMaxObjects = 100 )
{
    // Header
    strm << "\n==============\n";
    strm << " VECTOR TRACE\n";
    strm << "==============\n";
    strm << " Size: " << vectorTarget.size() << "\n";
    strm << "==============\n";

    typename std::vector< T >::iterator it;
    int nCounter = 0;
    for ( it = vectorTarget.begin(); it != vectorTarget.end(); it++, nCounter++ )
    {
        strm << "Item " << nCounter << ": ";

        // What us up, g++?  It croaks when the T stream is on the same line as another stream.
        // That compiler bug just cost me an afternoon.  Bugger off.
        strm << (*it);

        strm << "\n";
        if ( nCounter >= nMaxObjects )
        {
            strm << "...\n";
            break;
        }
    }
    strm << "==============\n\n";
}


//-------------------------------------------------------------------//
// TraceList()																		//
//-------------------------------------------------------------------//
// This function allows you to dump a trace of a std::list object
// to the debug window.
//
// NOTE: this requires operator<<() for the class type of your list.
// Make sure you provide one.  e.g.:
//
//		std::stringstream& operator<<( std::ostringstream strm, const MyInfo& info )
//		{
//			strm << _T("( ");
//			strm << info.m_nX;
//			strm << _T(", ");
//			strm << info.m_nY;
//			strm << _T(" )\n");
//          return strm;
//		}
//
//-------------------------------------------------------------------//
template <typename T>
void TraceList( std::stringstream& strm, std::list< T >& listTarget, int nMaxObjects = 100 )
{
    // Header
    strm << "\n==============\n";
    strm << " LIST TRACE\n";
    strm << "==============\n";
    strm << " Size: " << listTarget.size() << "\n";
    strm << "==============\n";

    typename std::list< T >::iterator it;
    int nCounter = 0;
    for ( it = listTarget.begin(); it != listTarget.end(); it++, nCounter++ )
    {
        strm << "Item " << nCounter << ": ";

        // What us up, g++?  It croaks when the T stream is on the same line as another stream.
        // That compiler bug just cost me an afternoon.  Bugger off.
        strm << (*it);

        strm << "\n";
        if ( nCounter >= nMaxObjects )
        {
            strm << "...\n";
            break;
        }
    }
    strm << "==============\n\n";
}


//=========================================================
// PersistentObject
//=========================================================
// Basic object that tracks dirty status, for the sake of delayed writes.
// All containers should consider using a delayed-write pattern.
//=========================================================
class PersistentObject
{
public:

    PersistentObject()
    :
        // init vars
        bSaved_(false)
    {}

    // We provide diff functions for dirty vs saved.
    // This makes the code more readable.
    // Also, we may actually do different things
    // in derived classes when we go dirty vs saved.
    bool bSaved() const { return bSaved_; }
    virtual bool bDirty() const { return !bSaved_; }
    virtual void setDirty() { bSaved_ = false; }
    virtual void setSaved() { bSaved_ = true; }

protected:
    bool bSaved_;
};


//=========================================================
// PersistentIDObject
//=========================================================
// This object expects that an AUTOINCREMENTED INTEGER PRIMARY KEY is used for persistence.
// We provide functions to handle -1 as a request to get a new db id.
// We use a static counter to do so, allowing delayed-write of new objects.
//=========================================================
// History
// -------
// v1.00 2015/08/06     First added.  In heavy use by AbetterTrader server app.
// v1.01 2015/08/11     Updated documentation to move work to derived classes,
//                      since we have no virtual inheritance in constructor.
// v1.02 2016/03/23     Rolled some functionality back into base, by using an int reference constructor param
//                      of a static member in derived class
//=========================================================
class PersistentIDObject : public PersistentObject
{
typedef PersistentObject inherited;

public:

    typedef enum {
        DBID_UNSAVED = -1,
        DBID_DO_NOT_SAVE = -2
    } DB_ID_CONSTANTS;

    PersistentIDObject(int64_t& static_max_db_id, const int64_t& db_id = -1)
    :
        // Call base class
        inherited(),

        // init vars
        static_max_db_id_(static_max_db_id),
        db_id_(db_id)
    {
        assignNewDbIdAsNeeded();
    }

    // IN-MEMORY ID HANDLING
    // We track our own "max db_id" per persisted class,
    // so we can create new objects without need to hit the db until later.
    // This allows for delayed writes, very important.
    //
    // Special db_id values provide the functionality:
    //
    //      Value               Meaning
    //      -----               -------
    //      DBID_UNSAVED        Used for newly created objects; in initialize(), a new "largest-yet" unique db_id
    //                          will be generated and the -1 value will be replaced
    //      DBID_DO_NOT_SAVE    Used for a temporary object; we will NEVER save it or change the value
    //
    // In derived classes:
    //
    //     1) In each derived class header, set up a static max db_id tracker, and provide it to the base class.
    //     -----------------------------------------------------------------------------
    //
    //          class Widget : public PersistentIDObject
    //          {
    //          typedef PersistentIDObject inherited;
    //          public:
    //              Widget(const int64_t& db_id = -1)
    //              :
    //                  // Call base class
    //                  inherited(widget_max_db_id_,db_id)
    //              {}
    //
    //             static int64_t widget_max_db_id_;
    //         }
    //
    //     2) On startup, capture the starting max_db_id for each persistent class
    //     -----------------------------------------------------------------------
    //
    //         { SQLite::Statement query(db, string("SELECT MAX(id) FROM ") + c_str_Tablename_Widgets ); query.executeStep(); Widget::widget_max_db_id_ = query.getColumn(0).getInt64(); }
    //
    void assignNewDbIdAsNeeded()
    {
        if (db_id_ == DBID_UNSAVED)
        {
            assignNewDbId();
            setDirty();
        }
    }

    // Call this to force a new ID when appropriate.
    // This works great for our newer more-agile default-constructor pattern.
    // We default db_id_ to DB_DO_NOT_SAVE, then call this when we want to persist a new object.
    void assignNewDbId()
    {
        ++static_max_db_id_;
        db_id_ = static_max_db_id_;
    }

    // These will persist, and require room for growth.  Use an exact and generous bit size.
    int64_t& static_max_db_id_;
    int64_t db_id_;
};


// ===================
// CONTAINER FUNCTORS
// ===================
// You can use these directly as functors for your containers of pointers of objects derived from PersistentIDObject.  Sweet!
// ===================
// for [unordered_]set|map
// ===================
struct PersistentIDObjects_equal
{
    bool operator()(const PersistentIDObject* left, const PersistentIDObject* right) const
    {
        return (left->db_id_ == right->db_id_);
    }
};
struct PersistentIDObject_hash
{
    std::size_t operator()(const PersistentIDObject* ppo) const
    {
        return std::hash<int64_t>()(ppo->db_id_);
    }
};
// ===================
// for sorted_vector
// ===================
struct PersistentIDObjects_lessthan
{
    bool operator()(const PersistentIDObject* left, const PersistentIDObject* right)
    {
        return left->db_id_ < right->db_id_;
    }
};
// ===================


//-------------------------------------------------------------------//
// sorted_vector																		//
//-------------------------------------------------------------------//
// History
// -------
// v0.1 1999? A long time ago in a compiler far far away...
// .
// v0.9 12/1/2010   Added predicate function support
// v1.0 1/23/2011   Class first archived and documented on wiki
//                  https://www.thedigitalmachine.com/wiki/index.php/C%2B%2B_sorted_vector
// v1.1 1/24/2011   Fixed predicate version of find_it_or_fail to actually use predicate
// v1.2 2/04/2011   Added find_ptr_or_fail
//-------------------------------------------------------------------//
// This class is derived from std::vector, and adds sort tracking.
// There are two basic ways to use a sorted vector:
//
//		METHOD 1
//			Always maintain sort order by inserting with push_sorted() -
//			the location of new items is determined before inserting;
//			since the vector remains sorted, this doesn't take too long
//			(although for large batch insertions METHOD 2 is definitely
//			faster);
//
//		METHOD 2
//			Allow batch insertion without sorting with push_unsorted(); then
//			provide an additional call to sort the vector;  before
//			searching for an item, the vector is always sorted if needed;
//
//  Of course you need to provide an operator()< for the type of object
//  you're sorting, if it doesn't have one.  Example:
    /*
        class MyClass
        {
        public:
            bool operator< (const MyClass left) const
            {
                if ( left.m_nMostImportant == m_nMostImportant )
                    return left.m_nLeastImportant < m_nLeastImportant;

                return left.m_nMostImportant < m_nMostImportant;
            }

            int m_nMostImportant;
            int m_nLeastImportant;
        }
    */
//
//  NOTE: C++ doesn't let you use an operator()< for POINTERS.  This
//  breaks down when creating the template code, as you end up with
//  a ref to a ref which is not allowed (or something :P).
//  So if you have a vector of pointers, here's what you have to do:
//
//      FOR NOW, with straight C++, create a less-than functor, then
//      pass that in to the functor versions of the class methods below.
//      Create a functor, aka function object, as follows:
//
//          struct my_class_lessthan
//          {
//              bool operator()(const MyClass* left, const MyClass* right)
//              {
//                  return left->get_timestamp() < right->get_timestamp();
//              }
//          };
//
//      Usage example:
//
//          sorted_vector<MyClass*> svpMC;
//          svpMC.push_unsorted(new MyClass(blah, blah);
//          svpMC.push_unsorted(new MyClass(blah, blah);
//          vpMC.sort( my_class_lessthan() );
//
//      Once C++0x is available, I need to update this class to use
//      a function object wrapper, and allow the user to set it
//      in the constructor, then always use it automatically.
//      http://en.wikipedia.org/wiki/C%2B%2B0x#Polymorphic_wrappers_for_function_objects
//
// WARNING: if you change the key value of any object in the vector,
// you have unsorted the array without marking it as such.  Make sure
// you call SetSorted(false) where appropriate.
//
//-------------------------------------------------------------------//
template<class T, class _A = std::allocator<T> >
class sorted_vector : public std::vector<T, _A>
{
    typedef std::vector<T> inherited;

public:

    //-------------------------------------------------------------------//
    // SetSorted()																			//
    //-------------------------------------------------------------------//
    // I didn't want to override every constructor to set this
    // member variable, so this function is publicly accessible.
    // You should call SetSorted( true ) right after construction.
    // TO DO: if you feel like it... derive all constructors to avoid
    // the need for this.  There are 4 last time I checked.
    //-------------------------------------------------------------------//
    void SetSorted( bool bSorted = true ) { m_bSorted = bSorted; }
    bool bSorted() const { return m_bSorted; }

    //-------------------------------------------------------------------//
    // sort()                                                            //
    //-------------------------------------------------------------------//
    // This function sorts the data as needed.  Call it after repeated calls to
    // push_unsorted(), or just let other members call it for you on next access.
    // It calls std::sort(), which defaults to using operator<() for
    // comparisons.
    //-------------------------------------------------------------------//
    void sort()
    {
        if ( !m_bSorted )
        {
            std::sort( inherited::begin(), inherited::end() );
            SetSorted();
        }
    }

    // This function is stupid.  binary_search() was a dumb design, STL peeps.
    bool bContains( const T& t )
    {
        sort();
        return std::binary_search( inherited::begin(), inherited::end(), t );
    }

    typename std::vector<T>::iterator lower_bound_it( const T& key )
    {
        sort();
        typename std::vector<T>::iterator it = std::lower_bound( inherited::begin(), inherited::end(), key );
        return it;
    }

    T* lower_bound_ptr( const T& key )
    {
        typename std::vector<T>::iterator it = lower_bound_it( key );

        if (it==inherited::end())
            return 0;

        T* t = &(*it);
        return t;
    }

    //-------------------------------------------------------------------//
    // find_it_or_fail()                                                 //
    //-------------------------------------------------------------------//
    // This function takes the given object and determines if there is
    // a match in the vector.  It returns an iterator to the actual
    // object in the vector, if found.  Otherwise returns std::vector::end().
    //
    // This is the function you want to use most of the time
    // (or the predicate version if you are using object pointers).
    // With lower_bound(), you have to check the result to see if
    // there was a hit.  Here, that work is done for you.
    //
    // USAGE: it makes most sense to use this function if you have
    // an object with a key, other member variables, and operator<()
    // that uses the key to test for equality.  You then set up a dummy
    // "search" object with the key set to the search value, call the
    // function, and use the result to extract the additional information
    // from the object.
    //-------------------------------------------------------------------//
     typename std::vector<T>::iterator find_it_or_fail( const T& key )
     {
         sort();
         typename std::vector<T>::iterator it = lower_bound_it( key );
         if ( it != inherited::end() )

         // lower_bound() does not necessarily indicate a successful search.
         // The iterator points to the object where an insertion
         // should take place.  We check that result to see if we actually
         // had an exact match.

         // NOTE: This is how the STL determines equality using only operator()<.
         // Two comparisons, ugg, but it is a nice little trick.
         if( !((*it)<key) && !(key<(*it)) )
             return it;

         return inherited::end();
     }

     //-------------------------------------------------------------------//
     // find_ptr_or_fail()                                                 //
     //-------------------------------------------------------------------//
    // A variation of find_it_or_fail() that provides a pointer to result.
     //-------------------------------------------------------------------//
      T* find_ptr_or_fail( const T& key )
      {
          typename std::vector<T>::iterator it = find_it_or_fail( key );
          if ( it != inherited::end() )
              return &(*it);

          return 0;
      }

    //-------------------------------------------------------------------//
    // push_sorted()																		//
    //-------------------------------------------------------------------//
    // This is used to insert into a vector that always remains sorted.
    // Because we have a sorted vector, finding the insertion location
    // with std::lower_bound() is relatively cheap.
    //
    // If you have multiple insertions, consider
    // using push_unsorted() for each, then calling sort().
    // In fact, I can't think of a situation where this approach
    // would be preferred.  Prolly stick with push_unsorted()...
    //-------------------------------------------------------------------//
    void push_sorted( const T& t )
    {
        sort();

        // Insert at "lower_bound" (the proper sorted location).
        insert( std::lower_bound( inherited::begin(), inherited::end(), t ), t );
    }

    //-------------------------------------------------------------------//
    // push_unsorted()																	//
    //-------------------------------------------------------------------//
    // This is similar to push_back(), but in addition, it sets the
    // unsorted flag.  Always use this instead!
    //-------------------------------------------------------------------//
    void push_unsorted( const T& t )
    {
        SetSorted( false );
        this->push_back(t);
    }

    //-------------------------------------------------------------------//
    // insert_or_update()
    //-------------------------------------------------------------------//
    // Given an object, we search for it, and if found, update it.
    // If not found, we add it.  Nice!
    //
    // We return true if inserted, false if updated.
    //
    // (Should we return the iterator instead?  Nah, it would be invalidated
    //  on next access anyway on an insert... altho update could benefit...)
    //
    // We're treading close to unsorted_map functionality here, and I might
    // consider switching once C++11 is stable everywhere.
    //
    // NOTE that we will be unsorted after insertion.  This allows the user
    // to carry on (possibly making push_unsorted() calls), or calling
    // sort() to maintain sort.
    //-------------------------------------------------------------------//
    bool insert_or_update(const T& t)
    {
        typename std::vector<T>::iterator it = find_it_or_fail(t);

        if (it == inherited::end())
        {
            push_unsorted(t);
            return true;

        } else
        {
            *it = t;
            return false;
        }
    }

    //-------------------------------------------------------------------//
    // operator=()																	//
    //-------------------------------------------------------------------//
    // This allows us to set the sorted_vector from a std::vector.
    //-------------------------------------------------------------------//
    sorted_vector<T>& operator=(std::vector<T>& v)
    {
        typename std::vector<T>::iterator it;
        for (it= v.begin(); it != v.end(); ++it)
            push_unsorted((*it));
        return this;
    }

    // CALLS WHERE YOU PROVIDE THE FUNCTOR OR FUNCTION POINTER
    // If you need to use a predicate sort function, ALWAYS use these methods
    // instead of the non-functor versions.
    // NOTE: UPDATE THIS when C++0x polymorphic function wrappers are available.
    template<class _Pr> inline
    void sort( _Pr pr )
    {
        if ( !m_bSorted )
        {
            std::sort( inherited::begin(), inherited::end(), pr );
            SetSorted();
        }
    }
    template<class _Pr> inline
    typename std::vector<T>::iterator lower_bound_it( const T& key, _Pr pr )
    {
         sort(pr);
         typename std::vector<T>::iterator it = std::lower_bound( inherited::begin(), inherited::end(), key, pr );
         return it;
    }
    template<class _Pr> inline
    T* lower_bound_ptr( const T& key, _Pr pr )
    {
        typename std::vector<T>::iterator it = lower_bound_it( key, pr );
        if (it==inherited::end())
            return 0;

        T* t = &(*it);
        return t;
    }
    template<class _Pr> inline
    void push_sorted( const T& t, _Pr pr )
    {
         sort(pr);
         insert( std::lower_bound( inherited::begin(), inherited::end(), t, pr ), t );
    }
    template<class _Pr> inline
    typename std::vector<T>::iterator find_it_or_fail( const T& key, _Pr pr )
    {
        typename std::vector<T>::iterator it = lower_bound_it( key, pr );
        if ( it != inherited::end() )

            // NOTE: We have to apply this using the predicate function, be careful...
            if (!(pr((*it), key)) && !(pr(key,(*it))))

                  return it;

        return inherited::end();
    }
    template<class _Pr> inline
    T* find_ptr_or_fail( const T& key, _Pr pr )
    {
        typename std::vector<T>::iterator it = find_it_or_fail( key, pr );
        if ( it != inherited::end() )
            return &(*it);

        return 0;
    }
    template<class _Pr> inline
    bool insert_or_update(const T& t, _Pr pr)
    {
        typename std::vector<T>::iterator it = find_it_or_fail(t,pr);
        if (it == inherited::end())
        {
            push_unsorted(t);
            return true;

        } else
        {
            *it = t;
            return false;
        }
    }

    // TODO get this working, then consolidate these - or rework to use a functor??
    //      non-const non-predicate
    //      non-const predicate
    //      const non-predicate
    //      const predicate
    /*
    // MDM We want to be able to search a ---->const<---- container.
    // That means we CANNOT sort in these functions; the container must be sorted already.
    template<class _Pr> inline
    typename std::vector<T>::iterator lower_bound_it_presorted( const T& key, _Pr pr ) const
    {
         assert(bSorted());
         typename std::vector<T>::iterator it = std::lower_bound( inherited::begin(), inherited::end(), key, pr );
         return it;
    }
    template<class _Pr> inline
    T* lower_bound_ptr_presorted( const T& key, _Pr pr ) const
    {
        typename std::vector<T>::iterator it = lower_bound_it_presorted( key, pr );
        if (it==inherited::end())
            return 0;

        T* t = &(*it);
        return t;
    }
    template<class _Pr> inline
    typename std::vector<T>::iterator find_it_or_fail_presorted( const T& key, _Pr pr ) const
    {
        typename std::vector<T>::iterator it = lower_bound_it_presorted( key, pr );
        if ( it != inherited::end() )

            // NOTE: We have to apply this using the predicate function, be careful...
            if (!(pr((*it), key)) && !(pr(key,(*it))))

                  return it;

        return inherited::end();
    }
    template<class _Pr> inline
    T* find_ptr_or_fail_presorted( const T& key, _Pr pr ) const
    {
        typename std::vector<T>::iterator it = find_it_or_fail_presorted( key, pr );
        if ( it != inherited::end() )
            return &(*it);

        return 0;
    }
    */


protected:
    bool m_bSorted;

};


#endif // !defined(STL_CONTAINERS_H)
