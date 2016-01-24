// Copyright (c) 1994 Darren Erik Vengroff
//
// File: tpie_assert.H
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/12/94
//
// $Id: tpie_assert.H,v 1.1 2008/07/28 17:08:02 m Exp $
//

#ifndef _TPIE_ASSERT_H
#define _TPIE_ASSERT_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#include <tpie_log.H>
#include <assert.h>
#include <iostream>

#if DEBUG_ASSERTIONS

#define tp_assert(condition,message) { \
  if (!(condition)) { \
    LOG_FATAL_ID("Assertion failed:"); \
    LOG_FATAL_ID(message); \
    std::cerr << "Assertion failed: " << message << "\n"; \
    assert(0); \
  } \
}

#else
#define tp_assert(condition,message)
#endif

#endif // _TPIE_ASSERT_H

