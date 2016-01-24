// Copyright (c) 1994 Darren Erik Vengroff
//
// File: ami.H
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/19/94
//
// $Id: ami.H,v 1.1 2008/07/28 17:08:01 m Exp $
//
#ifndef _AMI_H
#define _AMI_H

// Get definitions for working with Unix and Windows
#include <portability.H>

// Get a stream implementation.
#include <ami_stream.H>

// Get templates for ami_scan().
#include <ami_scan.H>

// Get templates for ami_merge().
#include <ami_merge.H>

// Get templates for ami_sort().
#include <ami_sort.H>

// Get templates for general permutation.
#include <ami_gen_perm.H>

// Get templates for bit permuting.
#include <ami_bit_permute.H>

// Get a collection implementation.
#include <ami_coll.H>

// Get a block implementation.
#include <ami_block.H>

// Get templates for AMI_btree.
#include <ami_btree.H>

#endif // _AMI_H 
