// Copyright (c) 1995 Darren Erik Vengroff
//
// File: persist.H
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 4/7/95
//
// $Id: persist.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
// Persistence flags for TPIE streams.
//
#ifndef _PERSIST_H
#define _PERSIST_H

// Get definitions for working with Unix and Windows
#include <portability.H>

enum persistence {
    // Delete the stream from the disk when it is destructed.
    PERSIST_DELETE = 0,
    // Do not delete the stream from the disk when it is destructed.
    PERSIST_PERSISTENT,
    // Delete each block of data from the disk as it is read.
    PERSIST_READ_ONCE
};

#endif // _PERSIST_H 
