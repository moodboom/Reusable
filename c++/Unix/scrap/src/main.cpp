#include <iostream>         // For std::out

#include "main.h"


/*
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
// =======================


// Testing xml now...
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>


#include <sstream>											// For stringstream


// ------------- boost serialize archive tests ----------	
// This is based on boost_cvs/boost/libs/serialization/example/demo_pimpl.cpp	
class A 
{
public:
	
    A():m_n(1)
	{}
		
    ~A()
	{}

	template<class Archive>
    void serialize(Archive & ar, const unsigned int file_version)
	 {
		 ar & m_n;
	 }
		
protected:
	int m_n;
};

// This is based on boost_cvs/boost/libs/serialization/example/demo_xml.cpp	
class gps_position
{
    friend class boost::serialization::access;
    friend std::ostream & operator<<(std::ostream &os, const gps_position &gp);

    int degrees;
    int minutes;
    float seconds;

    template<class Archive>
    void serialize(
        Archive & ar, 
        const unsigned int // file_version
    ){
        ar  & BOOST_SERIALIZATION_NVP(degrees)
            & BOOST_SERIALIZATION_NVP(minutes)
            & BOOST_SERIALIZATION_NVP(seconds);
    }

public:
    // every serializable class needs a constructor
    gps_position(){};
    gps_position(int _d, int _m, float _s) : 
        degrees(_d), minutes(_m), seconds(_s)
    {}
};
std::ostream & operator<<(std::ostream &os, const gps_position &gp)
{
    return os << ' ' << gp.degrees << (unsigned char)186 << gp.minutes << '\'' << gp.seconds << '"';
}

// ------------- boost serialize archive tests end ------
*/



int Scrap::Go()
{
  /*
	// ------------- boost serialize archive tests ----------	

	// -----------------
	// XML archive test
	
    // create a position
    gps_position gtest1( 2, 3, 4 );
	
	// display
	std::cout << "orig object: ";
    std::cout << gtest1;
	std::cout << "\n";
	
    std::string filename(boost::archive::tmpdir());
    filename += "/gtest_save.xml";
	
	std::cout << "filename = ";
	std::cout << filename.c_str();
	std::cout << "\n";

    // save the schedule
	// need this stuff to go out of scope to finish
	// that's the key!!  it all works now!  whoop!
	{
		// make an archive
		std::ofstream ofs(filename.c_str());
		assert(ofs.good());
		boost::archive::xml_oarchive oa(ofs);
	
		std::cout << "Serializing...\n";
		oa << BOOST_SERIALIZATION_NVP(gtest1);
	}
	
	std::cout << "second object...\n";
    // make  a new position
    gps_position gtest2;

	// open the archive
    std::ifstream ifs(filename.c_str());
    assert(ifs.good());
    boost::archive::xml_iarchive ia(ifs);

    // restore the schedule from the archive
    ia >> BOOST_SERIALIZATION_NVP(gtest2);
	
    std::cout << "restored object: ";
    std::cout << gtest2;
	std::cout << "\n";

	// -----------------
	
	// -----------------
	// Now try with a text archive.	
	// We got it, this works!!  :>

	// This string will hold a transferrable representation 
	// of the object, cool!
	std::string str;

	// make an archive
	std::stringstream ss;
	boost::archive::text_oarchive ota(ss);	
	std::cout << "serializing to a string...\n";
	ota << BOOST_SERIALIZATION_NVP(gtest1);
	str = ss.str();
	
	std::cout << "serialized string: ";
	std::cout << str;
	std::cout << "\n";

	std::cout << "restoring to a second object...\n";
    // make  a new position
    gps_position gtest4;

	// open the archive
	std::stringstream iss(str);
	boost::archive::text_iarchive ita(iss);
	
	// restore the schedule from the archive
	ita >> BOOST_SERIALIZATION_NVP(gtest4);
	
    std::cout << "restored object: ";
    std::cout << gtest4;
	std::cout << "\n";
	// -----------------
	// -------------------------------------------
    */

	// ------------- struct initialization ------
	typedef struct
	{
	    int x;
	    int y;
	
	} Doh;
	
	// Standard init.
	Doh doh = { 3, 4 };
    std::cout << doh.x << " " << doh.y;
	std::cout << "\n";
	
    // array
	Doh doh_array[3] =
	{
	    { 3, 4 },
	    { 3, 4 },
	    { 3, 4 },
	};

    // You can't assign in one step.  :<
    // doh = { 4, 5 };
    Doh d2 = {4,5};
    doh = d2;
    std::cout << doh.x << " " << doh.y;
	std::cout << "\n";
    
    // You can do a partial initialization.
    Doh d3 = {6};
    doh = d3;
    std::cout << doh.x << " " << doh.y;
	std::cout << "\n";
    
    // using "set notation" 
    // THIS ONLY WORKS IN C!  WTF!!
    // It's nice, too - you can change the order, or initialize only the END of the struct if you want.
	/*
	Doh doh_set = { 
	    .x = 4,
	    .y = 3
	};
	Doh doh_set2 = { 
	    .y = 3, 
	    .x = 4 
	};
	Doh doh_set3 = { 
	    .y = 3
	};
    Doh st4[]=
    {
        {
            .x  = 23,
            .y  = 1039      
        },
        {
            .x  = 23,
            .y  = 1040
        }
    };
	*/

	// ------------------------------------------


	// ------------- ready for next test... ------
	// -------------------------------------------

	
  return 0;
}


int main ( int argc, char ** argv )
{
  Scrap scrap;
  return scrap.Go();
}
