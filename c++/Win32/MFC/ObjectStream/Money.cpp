// ------ money.cpp

// MDM Must always be included.
// Note that the include file is actually "..\stdafx.h", but because
// we PRECOMPILE stdafx.h, MS does not allow the correct path to be
// used.   Without the path, it works just fine.
#include "stdafx.h"

#include "money.h"

#include <math.h>

// --- construct Money from float
//     adjust to even cents
Money::Money(float v, unsigned char w)
{
	float vl = fabs(v);
	long dol = vl;
	unsigned short ct = (vl - dol) * 100;
	value = ct;
	value /= 100;
	value += dol;
	if (v < 0)
		value *= -1;
	wd = w;
}

// ---- ostream Money insertion operator
ostream& operator<<(ostream&os,const Money& m)
{
	os << '$';
	os.width(m.wd);
	os.precision(2);
	os.setf(ios::fixed | ios::right);
	os << m.value;
	return os;
}


