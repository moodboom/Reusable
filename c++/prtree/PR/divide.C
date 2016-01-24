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

int numleaves;

int main(int argc, char **argv) {
  box_type<double, 2> *t;
  char basefilename[100];
	cpu_timer ct;
	PRtree<double, 2>::stream_t *s = new PRtree<double, 2>::stream_t("east");
	PRtree<double, 2>::stream_t *s1 = new PRtree<double, 2>::stream_t(argv[1]);
	s1->persist(PERSIST_PERSISTENT);
	s->persist(PERSIST_PERSISTENT);
	for (int i = 0; i< s->stream_len() / 5 * atoi(argv[2]); i++) {
		s->read_item(&t);
		s1->write_item(*t);
	}
	delete s1;
	delete s;
}

