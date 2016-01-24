// Copyright (c) 1999 Octavian Procopiuc
//
// File: ascii2stream.C
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/26/99
// 
// Transforms an ASCII file containing rectangles (as in rectangle.H)
// into a TPIE stream.
//
#include "app_config.H"
#include <fstream>
#include "rectangle.H"
#include <ami_block.H>
#include <ami_scan_utils.H>

#define MIN(a,b) (a <= b ? a : b)
#define MAX(a,b) (a >= b ? a : b)

int main(int argc, char **argv) {

  AMI_err err;
  std::istream *file_stream;
  char *out_filename;
  if (argc < 2) {
    std::cerr << "Transforms ASCII rectangles file into TPIE stream.\n" 
	 << "Usage: " << argv[0] << " [ <input_file> ] <output_file>\n";
    exit(-1);
  } else if (argc == 2) {
    file_stream = &cin;
    out_filename = argv[1];
  } else {
    file_stream = new std::ifstream(argv[1]);
    out_filename = argv[2];
  }
  AMI_STREAM<rectangle<double, AMI_bid> > *out_stream;
  rectangle<double, AMI_bid> t;
  out_stream = new AMI_STREAM<rectangle<double, AMI_bid> >(out_filename);
  out_stream->persist(PERSIST_PERSISTENT);
  
  cerr << "Working...";
	int id;
	double x1, y1, x2, y2;
	int k = 0;
	while (*file_stream >>x1 >> y1 >> x2 >> y2 >> id) {
		t.id = k;
		t.xlo = MIN(x1, x2);
		t.xhi = MAX(x1, x2);
		t.ylo = MIN(y1, y2);
		t.yhi = MAX(y1, y2);
		k++;
		out_stream->write_item(t);
	}

	delete out_stream;
}
