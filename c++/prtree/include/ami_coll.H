//
// File:   ami_coll.H
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_coll.H,v 1.1 2008/07/28 17:08:01 m Exp $
//
// Front end for the AMI_COLLECTION implementations.
//
#ifndef _AMI_COLL_H
#define _AMI_COLL_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#include <ami_coll_base.H>
#include <ami_coll_single.H>

// AMI_collection_single is the only implementation, so make it easy
// to get to.

#define AMI_collection AMI_collection_single

#ifdef BTE_COLLECTION
#  define AMI_COLLECTION AMI_collection_single< BTE_COLLECTION >
#endif

#endif // _AMI_COLL_H
