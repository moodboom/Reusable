//
// File: ami_sort.H
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/10/94
//
// $Id: ami_sort.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
#ifndef _AMI_SORT_H
#define _AMI_SORT_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#define CONST const

#ifdef AMI_STREAM_IMP_SINGLE
#include <ami_sort_single.H>
#include <ami_optimized_sort.H>
#include <ami_sort_single_dh.H>
#endif

#endif // _AMI_SORT_H 
