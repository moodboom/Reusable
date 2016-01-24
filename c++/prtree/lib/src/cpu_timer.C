// Copyright (c) 1995 Darren Vengroff
//
// File: cpu_timer.C
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/11/95
//

#include <versions.H>
VERSION(cpu_timer_C,"$Id: cpu_timer.C,v 1.1 2008/07/28 17:08:02 m Exp $");

#include <cpu_timer.H>

cpu_timer::cpu_timer() :
        running(false)
{
    TPIE_OS_SET_CLOCK_TICK;

    elapsed_real = 0;
}

cpu_timer::~cpu_timer()
{
}

void cpu_timer::sync()
{
    clock_t current_real;

    TPIE_OS_TIME current;
    TPIE_OS_SET_CURRENT_TIME(current);
    TPIE_OS_UNIX_ONLY_SET_ELAPSED_TIME(current);

    elapsed_real += current_real - last_sync_real;

    last_sync = current;
    last_sync_real = current_real;
}


void cpu_timer::start()
{
    if (!running) {
	TPIE_OS_LAST_SYNC_REAL_DECLARATION;
	running = true;
    }
}

void cpu_timer::stop()
{
    if (running) {
        sync();
        running = false;
    }
}

void cpu_timer::reset()
{
    if (running) {		
	TPIE_OS_LAST_SYNC_REAL_DECLARATION;
    }
    
    TPIE_OS_SET_CLOCK_TICK;	
    elapsed_real = 0;
}

double cpu_timer::user_time() {
  if (running) sync();
  TPIE_OS_USER_TIME_BODY;
}

double cpu_timer::system_time() {
  if (running) sync();
  TPIE_OS_USER_TIME_BODY;
}

double cpu_timer::wall_time() {
  if (running) sync();
  return double(elapsed_real) / double(clock_tick);
}

std::ostream &operator<<(std::ostream &s, cpu_timer &wt)
{
    if (wt.running) {
        wt.sync();
    }
    
    TPIE_OS_OPERATOR_OVERLOAD;
}


