// Copyright (c) 1995 Darren Vengroff
//
// File: wall_timer.C
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//

#include <versions.H>
VERSION(wall_timer_C,"$Id: wall_timer.C,v 1.1 2008/07/28 17:08:02 m Exp $");

#include "wall_timer.H"

 
wall_timer::wall_timer() :
        elapsed(0),
        running(false)
{
}

wall_timer::~wall_timer()
{
}

void wall_timer::sync()
{
    time_t current = time(NULL);

    elapsed += current - last_sync;

    last_sync = current;
}


void wall_timer::start()
{
    if (!running) {
        time(&last_sync);
        running = true;
    }
}

void wall_timer::stop()
{
    if (running) {
        sync();
        running = false;
    }
}

void wall_timer::reset()
{
    if (running) {
        time(&last_sync);
    }
    elapsed = 0;
}

long wall_timer::seconds() {
  if (running) {
    sync();
  }

  return elapsed;
}

std::ostream &operator<<(std::ostream &s, wall_timer &wt)
{
    if (wt.running) {
        wt.sync();
    }

    return s << wt.elapsed;
}


