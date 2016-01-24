// ----- date.h

#ifndef DATE_H
#define DATE_H

#include <iostream>
using namespace std;

typedef unsigned char DtEl;

class Date {
public:
	
	// Made public for access in operator<< in Date.cpp.
	DtEl mo, da, yr;

	Date(DtEl m=0,DtEl d=0,DtEl y=0) : mo(m),da(d),yr(y)
		{ /* ... */ }
	DtEl Month() const
		{ return mo; }
	DtEl Day() const
		{ return da; }
	DtEl Year() const
		{ return yr; }
	void SetMonth(DtEl m)
		{ mo = m; }
	void SetDay(DtEl d)
		{ da = d; }
	void SetYear(DtEl y)
		{ yr = y; }
	int operator<(const Date& dt) const;
	int operator==(const Date& dt) const
		{ return mo == dt.mo && da == dt.da && yr == dt.yr; }
	int operator!=(const Date& dt) const
		{ return !(*this == dt); }
	int operator>(const Date& dt) const
		{ return !(*this == dt || *this < dt); }
	int operator<=(const Date& dt) const
		{ return (*this == dt || *this < dt); }
	int operator>=(const Date& dt) const
		{ return (*this == dt || *this > dt); }

	friend ostream& operator<<(ostream&os, const Date& dt);

};


#endif

