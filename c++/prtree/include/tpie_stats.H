//
// File:    tpie_stats.H
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
// The tpie_stats class for recording statistics. The parameter C is
// the number of statistics to be recorded.
//
#ifndef _TPIE_STATS_H
#define _TPIE_STATS_H

// Get definitions for working with Unix and Windows
#include <portability.H>

template<int C>
class tpie_stats {
private:

  // The array storing the C statistics.
  unsigned long stats_[C];

public:

  // Reset all counts to 0.
  void reset() {
    for (int i = 0; i < C; i++)
      stats_[i] = 0;
  }
  // Default constructor. Set all counts to 0.
  tpie_stats() {
    reset();
  }
  // Copy constructor.
  tpie_stats(const tpie_stats<C>& ts) {
    for (int i = 0; i < C; i++)
      stats_[i] = ts.stats_[i];
  }
  // Record ONE event of type t.
  void record(int t) {
    stats_[t]++;
  }
  // Record k events of type t.
  void record(int t, unsigned long k) {
    stats_[t] += k;
  }
  // Record the events stored in s.
  void record(const tpie_stats<C>& s) {
    for (int i = 0; i < C; i++)
      stats_[i] += s.stats_[i];
  }
  // Set the number of type t events to k.
  void set(int t, unsigned long k) {
    stats_[t] = k;
  }
  // Inquire the number of type t events.
  unsigned long get(int t) const {
    return stats_[t];
  }
  // Destructor.
  ~tpie_stats() {}
};

template<int C>
const tpie_stats<C> operator-(const tpie_stats<C> & lhs, 
			      const tpie_stats<C> & rhs) {
  tpie_stats<C> res;
  for (int i = 0; i < C; i++)
    res.set(i, lhs.get(i) - rhs.get(i));
  return res;
}

#endif //_TPIE_STATS_H
