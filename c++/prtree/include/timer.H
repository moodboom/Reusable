// Copyright (c) 1995 Darren Vengroff
//
// File: timer.H
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//
// $Id: timer.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
// General definition of a virtual timer class.
//
#ifndef _TIMER_H
#define _TIMER_H

// Get definitions for working with Unix and Windows
#include <portability.H>

class timer {
public:
    virtual void start(void) = 0;
    virtual void stop(void) = 0;
    virtual void reset(void) = 0;
};

#endif // _TIMER_H 
