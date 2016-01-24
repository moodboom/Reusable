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
#include "ami_PRtree.H"

#include <sys/types.h>
#include <time.h>

#define MEM_SIZE 8*1024*1024
#define USE_CACHE

int numleaves;

int main(int argc, char **argv) {
	if (argc < 3) {
		cout << "Usage: query_PRtree <PRtree base file name> <query file name>\n";
		return 0;
	}
  MM_manager.set_memory_limit(MEM_SIZE);
  MM_manager.enforce_memory_limit();
  box_type<double, 2> t, *t1;
  char basefilename[100];
  strcpy(basefilename, argv[1]);
  strcat(basefilename, ".pr");
  PRtree<double, 2> * rtree = new PRtree<double, 2>(basefilename, 1);
	cpu_timer ct;

	rtree->print_stats();

#ifdef USE_CACHE
	ct.reset(); ct.start();
	cout << "Initializing the static cache... " << flush;
	rtree->init_cache(MEM_SIZE - 1024*1024);
	ct.stop();
	cout << " Done\n";
#endif
	int k = 0;
	ifstream infile(argv[2]);
	int total = 0;
	numleaves = 0;
	ct.reset(); ct.start();
	for (int ite=0; ite<100; ite++) {
	infile >> t.lo_[0] >> t.lo_[1] >> t.hi_[0] >> t.hi_[1];
	tpie_stats_collection tsc1 = AMI_COLLECTION::gstats();
	PRtree<double, 2>::stream_t *s = new PRtree<double, 2>::stream_t;
	size_t re = rtree->query(t, s);
	tpie_stats_collection tsc2 = AMI_COLLECTION::gstats() - tsc1;

/*	cout << "Time : " << ct <<"\n";
	cout << "Block collection statistics (global):\n"
       << "\tGET BLOCK:    "
       << tsc2.get(BLOCK_GET) << "\n";
cout << "Stream statistics (global):\n"
//  		 << "\tChunk size:   " << s->chunk_size() << "\n"
       << "\tWRITE ITEM:   "
       << s->stats().get(ITEM_WRITE) << "\n";
*/
	total += s->stream_len();
  	delete s;
}
	cout << "Ave I/O = " << rtree->outcacheIO/100 << "\n";
	cout << "Outcache I/O = " << rtree->outcacheIO << "\n";
	cout << "Total I/O = " << rtree->totalIO << "\n";
	cout << "Ave output = " << total/100 << "\n";
	cout << "Total output = " << total << "\n";
	ct.stop();
 
  cout << "Time : " << ct << "\n";
	infile.close();
  /*
  s->seek(0);
  for (int i=0; i<10; i++) {
 		if (s->read_item(&t1) == AMI_ERROR_NO_ERROR) 
	  	cout << t1->id <<" " << t1->lo_[0] << " " << t1->lo_[1] << " " << t1->hi_[0] << " " << t1->hi_[1] << "\n";
  }*/
	cout <<"Block write" << AMI_COLLECTION::gstats().get(BLOCK_PUT) << "\n";
	delete rtree;
}

