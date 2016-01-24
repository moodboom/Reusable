//
//  $RCSfile: buildtree.C,v $
//     $Date: 2008/07/28 17:08:02 $
// $Revision: 1.1 $
//    $State: Exp $
//   $Author: m $
//
//   $Source: /home/m/development/CVSROOT/Reusable/prtree/rstartree/buildtree.C,v $
//   $Locker:  $
// 
//  Description:     source code for program buildtree
//  Created:         09.02.1999
//  Author:          Jan Vahrenhold
//  mail:            jan.vahrenhold@math.uni-muenster.de
//
//  Copyright (C) 1999-2001 by  
// 
//  Jan Vahrenhold
//  Westfaelische Wilhelms-Universitaet Muenster
//  Institut fuer Informatik
//  Einsteinstr. 62
//  D-48149 Muenster
//  GERMANY
//
#include <float.h>
// Quick hack.
#define INFINITY DBL_MAX
#define MINUSINFINITY -(DBL_MAX-1)

#include "app_config.H"
#include "bulkloader.H"
#include <iostream>
#include <stdlib.h>
#include <cpu_timer.H>

int main(int argc, char** argv) {

	MM_manager.ignore_memory_limit();	
	//  i.e., TPIE is not in control of memory allocation and does not
	//  complain if more than  test_mm_size is allocated.

	// Set the main memory size. 
	MM_manager.set_memory_limit(64*1024*1024);    

	if (argc < 4) {
		cerr << "Missing command parameter." << "\n";
		cerr << "Usage: buildtree <input_stream> <fanout> <R|H>" << "\n";
	}
	else {

		RStarTree<double>* tree = NULL;

		std::cerr << "\n";
		std::cerr << "----------------------------------------------------------------------" << "\n";
		std::cerr << "\n" << "Creating ";
		if (!strcmp(argv[3], "H")) {
			std::cerr << "Hilbert";
		}
		else {
			std::cerr << "R*";
		}
		std::cerr << "-Tree (fanout=" << atol(argv[2]) << ") for " << argv[1] << "...";

		BulkLoader<double> bl(argv[1], atol(argv[2]));
		AMI_err result = AMI_ERROR_NO_ERROR;

		cpu_timer ct;
		ct.reset(); ct.start(); 	ct.stop();
		ct.reset(); ct.start();

		if (!strcmp(argv[3], "H")) {
			result = bl.createHilbertRTree(&tree);
		} else if (!strcmp(argv[3], "H4")) {
			result = bl.createHilbert4RTree(&tree);
		}	else {
			result = bl.createRStarTree(&tree);
		}

		if (result != AMI_ERROR_NO_ERROR) {
			std::cerr << "Error " << hex << result;
		}

		std::cerr << "...done (" << tree->totalObjects() << " objects)." << "\n";
		tree->show_stats();

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
       << AMI_STREAM<std::pair<rectangle<double, AMI_bid>, long long> >::gstats().get(ITEM_READ) << "\n"
       << "\tWRITE ITEM:   "
       << AMI_STREAM<std::pair<rectangle<double, AMI_bid>, long long> >::gstats().get(ITEM_WRITE) << "\n"
       << "\tREAD BLOCK:   "
       << AMI_STREAM<std::pair<rectangle<double, AMI_bid>, long long> >::gstats().get(BLOCK_READ) << "\n"
       << "\tWRITE BLOCK:  "
       << AMI_STREAM<std::pair<rectangle<double, AMI_bid>, long long> >::gstats().get(BLOCK_WRITE) << "\n"
    ;

		//
		//  Uncomment the next few lines to count all nodes/objects in the tree.
		//

		// 	RStarNode<double>* n=NULL;
		// 	std::list<AMI_bid> l;
		// 	int nodes = 0;
		// 	int objects = 0;

		// 	l.push_back(tree->rootPosition());

		// 	while (!l.empty()) {
		// 	    AMI_bid next = l.front();
		// 	    l.pop_front();
		// 	    n = tree->readNode(next);
		// 	    ++nodes;
		// 	    if (!n->isLeaf()) {
		// 		for(int i=0; i<n->numberOfChildren(); ++i) {
		// 		    l.push_back(n->getChild(i).getID());		    
		// 		}
		// 	    }
		// 	    else {
		// 		objects += n->numberOfChildren();
		// 	    }
		// 	    delete n;
		// 	}
		// 	std::cout << nodes << " nodes" << "\n";
		// 	std::cout << objects << " objects" << "\n";

		delete tree;
	}

	return 0;
}
