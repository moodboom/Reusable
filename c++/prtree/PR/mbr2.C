// Copyright (c) 1999 Octavian Procopiuc
//
// File: mbr.cc
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
// Created: 01/27/99
//
// $Id: mbr2.C,v 1.1 2008/07/28 17:08:01 m Exp $
//
// Performs a scan of a stream of rectangles to find their
// minimum bounding rectangle (MBR).
//

#include <float.h>
// Quick hack.
#define INFINITY DBL_MAX
#define MINUSINFINITY -(DBL_MAX-1)

#include "app_config.H"
#include "rectangle.H"
#include <ami_scan.H>
#include <ami_block.H>
#include <string.h>
#include <fstream>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include "ami_PRtree.H"
//#include <limits.h>

int side;
 PRtree<double, 2> * rtree;

template<class coord_t, class oid_t>
class MBRScanner: public AMI_scan_object {
protected:
  rectangle<coord_t, oid_t> mbr;
  ofstream *out;
public:
  MBRScanner(char *out_filename) {
    out = new std::ofstream(out_filename);
    mbr.xlo = INFINITY;
    mbr.ylo = INFINITY;
    mbr.xhi = MINUSINFINITY;
    mbr.yhi = MINUSINFINITY;
  }
    
  AMI_err initialize() {

  }

  AMI_err operate(const rectangle<coord_t, oid_t> &in, AMI_SCAN_FLAG *sfin) {

    if (*sfin) {
      if (in.xlo < mbr.xlo) mbr.xlo = in.xlo;
      if (in.ylo < mbr.ylo) mbr.ylo = in.ylo;
      if (in.xhi > mbr.xhi) mbr.xhi = in.xhi;
      if (in.yhi > mbr.yhi) mbr.yhi = in.yhi;
    } else {
      out->write((char *) &mbr, sizeof(mbr));
      cerr << " " << mbr.xlo << " " << mbr.ylo 
      	   << " " << mbr.xhi << " " << mbr.yhi << " ";
      ofstream outfile("q");
      int i;
      side = (int)sqrt((mbr.xhi - mbr.xlo)*(mbr.yhi - mbr.ylo) * side /10000);
      for (i=0; i<100; i++) {
      	int x = (rand() % (int)(mbr.xhi - mbr.xlo - side)) + (int)mbr.xlo;
      	int y = (rand() % (int)(mbr.yhi - mbr.ylo - side)) + (int)mbr.ylo;
	box_type<double, 2> t;
t.lo_[0] = x; t.lo_[1] = y; t.hi_[0] = x+side; t.hi_[1] = y+side;
		PRtree<double, 2>::stream_t *s = new PRtree<double, 2>::stream_t;	size_t re = rtree->query(t, s);
      if (s->stream_len() > 10000) outfile << x << " " << y << " " << x+side << " " << y+side << "\n";	
	else i--;
	cout << s->stream_len() << " " << i << "\n";
	delete s;
      }
      outfile.close();
      return AMI_SCAN_DONE;
    }
    return AMI_SCAN_CONTINUE; 
  }
};

int main(int argc, char **argv) {

  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <input_file>" << endl;
    exit(-1);
  }

	srand(time(NULL));

  std::cerr << "Working...";
  AMI_STREAM<rectangle<double, AMI_bid> > input_stream(argv[1]);
  input_stream.persist(PERSIST_PERSISTENT);
  side = atol(argv[2]);
 
 char basefilename[100];  strcpy(basefilename, argv[1]);  strcat(basefilename, ".pr");
  rtree = new PRtree<double, 2>(basefilename, 2);

  cerr << "Stream length : " << input_stream.stream_len() << endl;

  char *output_filename = new char[strlen(argv[1])+5];
  strcpy(output_filename, argv[1]);
  strcat(output_filename, ".mbr");
  MBRScanner<double, AMI_bid> scan(output_filename);
  AMI_err err = AMI_scan(&input_stream, &scan);
  std::cerr << "done." << endl;

}
