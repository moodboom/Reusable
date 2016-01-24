// Copyright (c) 1995 Darren Erik Vengroff
//
// File: ami_key.C
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/12/95
//

#include <versions.H>
#include <ami_key.H>

VERSION(ami_key_C,"$Id: ami_key.C,v 1.1 2008/07/28 17:08:02 m Exp $");


key_range::key_range(kb_key min_key, kb_key max_key) {
    this->min = min_key;
    this->max = max_key;
}

key_range::key_range(void) {
    this->min = 0;
    this->max = 0;
}

