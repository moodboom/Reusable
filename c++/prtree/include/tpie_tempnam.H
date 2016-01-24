//
// File: tpie_tempnam.H
// Author: 
// Created: 02/02/02
//
// $Id: tpie_tempnam.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
//
#ifndef _TPIE_TEMPNAM_H
#define _TPIE_TEMPNAM_H

// Get definitions for working with Unix and Windows
#include <portability.H>

// The name of the environment variable pointing to a tmp directory.
#define TMPDIR_ENV "TMPDIR"

// The name of the environment variable to consult for default device
// descriptions.
#define AMI_SINGLE_DEVICE_ENV "AMI_SINGLE_DEVICE"

char *tpie_tempnam(char *base, const char *dir = NULL);

#endif // _TPIE_TEMPNAM_H 
