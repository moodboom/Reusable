// Copyright (c) 1994 Darren Erik Vengroff
//
// File: mm.H (plus contents from mm_imps.H, now deprecated)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/30/94
//
// $Id: mm.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
#ifndef _MM_H
#define _MM_H

// Get definitions for working with Unix and Windows
#include <portability.H>

// Get the base class, enums, etc...
#include <mm_base.H>

// Get an implementation definition

// For now only single address space memory management is supported.
#ifdef MM_IMP_REGISTER
#include <mm_register.H>
#else
#error No MM implementation selected.
#endif

#endif // _MM_H 
