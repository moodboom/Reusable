// ------ money.h

#ifndef MONEY_H
#define MONEY_H

// #include <iostream.h>
#include <iostream>
using namespace std;

class Money	{
public:

	// Made public for access in operator<< in Money.cpp.
	float value;
	unsigned char wd; // display width

	Money(float v=0, unsigned char w = 6);
	float& Value()
		{ return value; }
	int operator<(const Money& m) const
		{ return value < m.value; }
	int operator==(const Money& m) const
		{ return value == m.value; }
	int operator!=(const Money& m) const
		{ return value != m.value; }
	int operator>(const Money& m) const
		{ return value > m.value; }
	int operator<=(const Money& m) const
		{ return value <= m.value; }
	int operator>=(const Money& m) const
		{ return value >= m.value; }
	friend ostream& operator<<(ostream&os,const Money& m);
};

#endif

