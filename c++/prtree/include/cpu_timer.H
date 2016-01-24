//
// File: cpu_timer.H
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//
// $Id: cpu_timer.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
// A timer measuring user time, system time and wall clock time.  The
// timer can be start()'ed, stop()'ed, and queried. Querying can be
// done without stopping the timer, to report intermediate values.
//
#ifndef _CPU_TIMER_H
#define _CPU_TIMER_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#include <iostream>
#include <timer.H>

class cpu_timer : public timer {
private:
  long clock_tick;

  TPIE_OS_TIME last_sync;
  TPIE_OS_TIME elapsed;

  clock_t last_sync_real;
  clock_t elapsed_real;
  bool running;
public:
  cpu_timer();
  virtual ~cpu_timer();
  
  void start();
  void stop();
  void sync();
  void reset();

  double user_time();
  double system_time();
  double wall_time();
  
  friend std::ostream &operator<<(std::ostream &s, cpu_timer &ct);
};

std::ostream &operator<<(std::ostream &s, cpu_timer &ct);

#endif // _CPU_TIMER_H 
