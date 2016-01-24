//
// File: lib_config.H
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/31/94
//
// $Id: lib_config.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
#ifndef _LIB_CONFIG_H
#define _LIB_CONFIG_H

#include <config.H>

// Use logs if requested.
#if TP_LOG_LIB
#define TPL_LOGGING 1
#endif
#include <tpie_log.H>

// Enable assertions if requested.
#if TP_ASSERT_LIB
#define DEBUG_ASSERTIONS 1
#endif
#include <tpie_assert.H>


#endif // _LIB_CONFIG_H 
 
