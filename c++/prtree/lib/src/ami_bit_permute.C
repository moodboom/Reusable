// Copyright (c) 1995 Darren Vengroff
//
// File: ami_bit_permute.C
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/9/95
//

#include <iostream>

#include <versions.H>
VERSION(ami_bit_permute_C,"$Id: ami_bit_permute.C,v 1.1 2008/07/28 17:08:02 m Exp $");

#include <ami_bit_permute.H>

AMI_bit_perm_object::AMI_bit_perm_object(const bit_matrix &A,
                                         const bit_matrix &c) :
                                                 mA(A), mc(c)
{
}

AMI_bit_perm_object::~AMI_bit_perm_object(void)
{
}

bit_matrix AMI_bit_perm_object::A(void)
{
    return mA;
}

bit_matrix AMI_bit_perm_object::c(void)
{
    return mc;
}


