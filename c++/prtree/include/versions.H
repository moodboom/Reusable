// File: versions.H
// Created: 99/11/15

// This file defines a macro VERSION that creates a static variable
// __name whose contents contain the given string __id. This is
// intended to be used for creating RCS version identifiers as static
// data in object files and executables.

// The "compiler_fooler stuff creates a (small) self-referential
// structure that prevents the compiler from warning that __name is
// never referenced.

// $Id: versions.H,v 1.1 2008/07/28 17:08:02 m Exp $

#ifndef _VERSIONS_H
#define _VERSIONS_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#endif // _VERSIONS_H 
