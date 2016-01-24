// Copyright (c) 1994 Darren Vengroff
//
// File: bit.C
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//

#include <versions.H>
VERSION(bit_C,"$Id: bit.C,v 1.1 2008/07/28 17:08:02 m Exp $");

#include <bit.H>

bit::bit(void)
{
}

bit::bit(bool b)
{
    data = (b == true);
}

bit::bit(int i)
{
    data = (i != 0);
}

bit::bit(long int i)
{
    data = (i != 0);
}

bit::operator bool(void)
{
    return (data != 0);
}
        
bit::operator int(void)
{
    return data;
}
        
bit::operator long int(void)
{
    return data;
}
        
bit::~bit(void)
{
}

bit bit::operator+=(bit rhs)
{
    return *this = *this + rhs;
}
        
bit bit::operator*=(bit rhs)
{
    return *this = *this + rhs;
}

bit operator+(bit op1, bit op2)
{
    return bit(op1.data ^ op2.data);
}


bit operator*(bit op1, bit op2)
{
    return bit(op1.data & op2.data);
}


std::ostream &operator<<(std::ostream &s, bit b)
{
    return s << int(b.data);
}




