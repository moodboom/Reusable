// Copyright (C) 2001 Octavian Procopiuc
//
// File:    tpie_stats_coll.H
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats_coll.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
// Statistics for block collections.

#ifndef _TPIE_STATS_COLL_H
#define _TPIE_STATS_COLL_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#include <tpie_stats.H>

#define TPIE_STATS_COLLECTION_COUNT 9
enum TPIE_STATS_COLLECTION {
  BLOCK_GET = 0,
  BLOCK_PUT,
  BLOCK_NEW,
  BLOCK_DELETE,
  BLOCK_SYNC,
  COLLECTION_OPEN,
  COLLECTION_CLOSE,
  COLLECTION_CREATE,
  COLLECTION_DELETE
};

typedef tpie_stats<TPIE_STATS_COLLECTION_COUNT> tpie_stats_collection;

#endif //_TPIE_STATS_COLL_H
