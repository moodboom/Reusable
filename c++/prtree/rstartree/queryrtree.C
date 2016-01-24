#include <iostream>
#include <list>
#include "app_config.H"
#include <ami_block.H>
#include "rstartree.H"
#include <cpu_timer.H>

#define MEM_SIZE 8*1024*1024
#define USE_CACHE

int main(int argc, char** argv) {
	if (argc < 2) {
		cout << "Usage: query_semiRtree <semiRtree base file name> <query file name>\n";
		return 0;
	}
	MM_manager.set_memory_limit(MEM_SIZE);
	MM_manager.enforce_memory_limit();

	RStarTree<double>* r = new RStarTree<double>(argv[1], 100);
	rectangle<double, AMI_bid> t;
	cpu_timer ct;

#ifdef USE_CACHE
	ct.reset(); ct.start();
	cout << "Initializing the static cache... " << flush;
	r->init_cache(MEM_SIZE - 1024*1024);
	ct.stop();
	cout << " Done\n";
#endif

	r->show_stats();
	ct.reset(); ct.start();
	ifstream infile(argv[2]);
	int total = 0;
	for (int ite=0; ite<100; ite++){
		infile >> t.xlo >> t.ylo >> t.xhi >> t.yhi;
		AMI_STREAM<rectangle<double, AMI_bid> >* matches = new AMI_STREAM<rectangle<double, AMI_bid> >;
		matches->persist(PERSIST_DELETE);
		AMI_stack<AMI_bid>* candidates = new AMI_stack<AMI_bid>;
		candidates->persist(PERSIST_DELETE);
		tpie_stats_collection tsc1 = AMI_COLLECTION::gstats();
		size_t re = r->query(t,matches,candidates,false);
		tpie_stats_collection tsc2 = AMI_COLLECTION::gstats() - tsc1;

		/*	cout << "Time : " << ct <<"\n";
		cout << "Block collection statistics (global):\n"
		<< "\tGET BLOCK:    "
		<< tsc2.get(BLOCK_GET) << "\n";
		cout << "Stream statistics (global):\n"
		<< "\tWRITE ITEM:   "
		<< matches->stats().get(ITEM_WRITE) << "\n"
		<< "\tWRITE BLOCK:  "
		<< matches->stats().get(BLOCK_WRITE) << "\n";
		cout << "Stream statistics (global):\n"
			<< "\tWRITE ITEM:   "
			<< matches->stats().get(ITEM_WRITE) << "\n";
*/
		total += matches->stream_len();
                delete matches;
                delete candidates;
	}
	cout << "Ave I/O = " << r->outcacheIO/100 << "\n";
	cout << "Outcache I/O = " << r->outcacheIO << "\n";
	cout << "Total I/O = " << r->totalIO << "\n";
	cout << "Ave output = " << total/100 << "\n";
	cout << "Total output = " << total << "\n";
	ct.stop();
  cout << "Time : " << ct << "\n";
	delete r;
	infile.close();
	/*
	rectangle<double, AMI_bid> *p;
	matches->seek(0);
	for (int i=0; i<93; i++) {
	if (matches->read_item(&p) == AMI_ERROR_NO_ERROR) 
	cout << p->id << " " << p->xlo << " " << p->ylo << " " << p->xhi << " " << p->yhi << "\n";
	}*/
	return 0;
}
