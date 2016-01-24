// Copyright (c) 1995 Darren Vengroff
//
// File: comparator.H
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: comparator.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
#ifndef _COMPARATOR_H
#define _COMPARATOR_H

// Get definitions for working with Unix and Windows
#include <portability.H>

// First we define a comparison object.
template<class T>
class comparator
{
public:
#if AMI_VIRTUAL_BASE
  virtual int compare(const T &t1, const T &t2) = 0;
#endif
};

#endif // _COMPARATOR_H 
