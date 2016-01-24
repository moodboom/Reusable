//
// File:    test_semiRtree.C
// Author:  Ke Yi <yike@cs.duke.edu>
// 
// Test file for class AMI_semiRtree.
//

#include <iostream>
#include <fstream>
#include "app_config.H"
#include <cpu_timer.H>
#include <ami_PRtree.H>

#include <sys/types.h>
#include <time.h>
#include "../rstartree/rectangle.H"

int main(int argc, char **argv) {
	if (argc<2) {
		cout << "Usage: build_PRtree <input file name>\n";
		return 0;
	}
	MM_manager.set_memory_limit(64*1024*1024);
  MM_manager.enforce_memory_limit();

  PRtree<double, 2>::stream_t *s = new PRtree<double, 2>::stream_t(argv[1]);
  box_type<double, 2> t;
  char basefilename[100], tmp[100];
  strcpy(basefilename, argv[1]);
  strcat(basefilename, ".pr");
	strcpy(tmp, basefilename);
	strcat(tmp, ".blk");
	remove(tmp);

	PRtree<double, 2> * rtree = new PRtree<double, 2>(basefilename, 1);

	cpu_timer ct;
	ct.reset(); ct.start(); 	ct.stop();
	ct.reset(); ct.start();
  rtree->load(s);
  ct.stop();
  cout << "Time : " << ct << "\n";
    cout << "Block collection statistics (global):\n"
       << "\tGET BLOCK:    "
       << AMI_COLLECTION::gstats().get(BLOCK_GET) << "\n"
       << "\tPUT BLOCK:    "
       << AMI_COLLECTION::gstats().get(BLOCK_PUT) << "\n"
    ;
  cout << "Stream statistics (global):\n"
       << "\tREAD ITEM:    "
       << PRtree<double, 2>::stream_t::gstats().get(ITEM_READ) << "\n"
       << "\tWRITE ITEM:   "
       << PRtree<double, 2>::stream_t::gstats().get(ITEM_WRITE) << "\n"
       << "\tREAD BLOCK:   "
       << PRtree<double, 2>::stream_t::gstats().get(BLOCK_READ) << "\n"
       << "\tWRITE BLOCK:  "
       << PRtree<double, 2>::stream_t::gstats().get(BLOCK_WRITE) << "\n"
    ;
  s->persist(PERSIST_PERSISTENT);
  delete s;
	rtree->print_stats();
  delete rtree;
	cout << "Memory available " << MM_manager.memory_available() << "\n";
}

