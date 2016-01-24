// Copyright (c) 1995 Darren Vengroff
//
// File: wall_timer.H
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//
// $Id: wall_timer.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
#ifndef _WALL_TIMER_H
#define _WALL_TIMER_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#include <time.h>

#include <iostream>

#include <timer.H>

class wall_timer : public timer
{
private:
    time_t last_sync;
    time_t elapsed;
    bool running;
public:
    wall_timer();
    virtual ~wall_timer();
    
    void start();
    void stop();
    void sync();
    void reset();
  long seconds();

    friend std::ostream &operator<<(std::ostream &s, wall_timer &wt);

};

std::ostream &operator<<(std::ostream &s, wall_timer &wt);

#endif // _WALL_TIMER_H 
