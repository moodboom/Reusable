// Copyright (C) 2001 Octavian Procopiuc
//
// File:   ami_coll_base.H
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_coll_base.H,v 1.1 2008/07/28 17:08:01 m Exp $
//
// Basic definitions for all AMI_COLLECTION implementations.
//
#ifndef _AMI_COLL_BASE_H
#define _AMI_COLL_BASE_H

// Get definitions for working with Unix and Windows
#include <portability.H>

// AMI collection types passed to constructors
enum AMI_collection_type {
    AMI_READ_COLLECTION = 1,	// Open existing collection for reading
    AMI_WRITE_COLLECTION,      	// Open for writing.  Create if non-existent
    AMI_READ_WRITE_COLLECTION	// Open to read and write.
};

// AMI collection status.
enum AMI_collection_status {
  AMI_COLLECTION_STATUS_VALID = 0,
  AMI_COLLECTION_STATUS_INVALID = 1,
};

#endif // _AMI_COLL_BASE_H
