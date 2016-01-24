//
// File: bte_stream_base.C
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//         (using some code by Darren Erik Vengroff)
// Created: 01/08/02
//

#include "lib_config.H"
#include <versions.H>
VERSION(bte_stream_base_C,"$Id: bte_stream_base.C,v 1.1 2008/07/28 17:08:02 m Exp $");

#include <bte_stream_base.H>

static unsigned long get_remaining_streams() {
	TPIE_OS_SET_LIMITS_BODY;
}

tpie_stats_stream BTE_stream_base_generic::gstats_;

int BTE_stream_base_generic::remaining_streams = get_remaining_streams();

