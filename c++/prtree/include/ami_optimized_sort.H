//
// File: ami_optimized_sort.H
// $Id: ami_optimized_sort.H,v 1.1 2008/07/28 17:08:01 m Exp $
//
// Optimized merge sorting.
//
#ifndef _AMI_SORT_OPTIMIZED_H
#define _AMI_SORT_OPTIMIZED_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#ifndef AMI_STREAM_IMP_SINGLE
#  warning Including __FILE__ when AMI_STREAM_IMP_SINGLE undefined.
#endif

#include <ami_merge.H>
#include <ami_optimized_merge.H>

//------------------------------------------------------------
template<class T>
AMI_err 
AMI_optimized_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream) {
  
  return AMI_partition_and_merge(instream, outstream);
}

//------------------------------------------------------------
template<class T, class KEY>
AMI_err 
AMI_optimized_sort(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
	 int keyoffset, KEY dummykey) {
  
  return AMI_partition_and_merge(instream, outstream, keyoffset, dummykey);
}
                          

#endif
