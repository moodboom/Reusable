// --------- date.cpp

// MDM Must always be included.
// Note that the include file is actually "..\stdafx.h", but because
// we PRECOMPILE stdafx.h, MS does not allow the correct path to be
// used.   Without the path, it works just fine.
#include "stdafx.h"

#include "date.h"

// ---- overloaded relational less-than
int Date::operator<(const Date&dt) const
{
	if (yr < dt.yr)
		return 1;
	if (yr == dt.yr)	{
		if (mo < dt.mo)
			return 1;
		if (mo == dt.mo)
			return da < dt.da;
	}
	return 0;
}

// ---- ostream insertion operator
ostream& operator<<(ostream&os, const Date& dt)
{
	os.setf(ios::right);
	os.fill(' ');
	os.width(2);
	os << static_cast<unsigned short>(dt.mo) << '/';
	os.fill('0');
	os.width(2);
	os << static_cast<unsigned short>(dt.da) << '/';
	os.width(2);
	os << static_cast<unsigned short>(dt.yr);
	os.fill(' ');
	return os;
}

