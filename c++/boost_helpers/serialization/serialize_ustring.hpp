// NOTE that it is assumed that you have included boost serialization headers before this point...


// MDM In order to stream our ustring class we need a serialize() function.
// Let's try putting this one outside the class.  That's called
// the "non-intrusive formulation".  The only problem is that you
// obviously need access to all the internal data.
namespace boost {
namespace serialization {

template<class Archive>
inline void serialize(Archive & ar, ustring & str, const unsigned int version)
{
   // See here for how to serialize a base class in a derived class...
   // http://www.boost.org/libs/serialization/doc/tutorial.html#derivedclasses
   
	#ifdef UNICODE
		ar & boost::serialization::base_object<std::wstring>(str);
	#else
		ar & boost::serialization::base_object<std::string>(str);
	#endif
}


} // namespace serialization
} // namespace boost
