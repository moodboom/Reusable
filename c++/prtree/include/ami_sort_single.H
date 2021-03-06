//
// File: ami_sort_single.H
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/28/94
//
// $Id: ami_sort_single.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
// Merge sorting for the AMI_STREAM_IMP_SINGLE implementation.
// 
#ifndef _AMI_SORT_SINGLE_H
#define _AMI_SORT_SINGLE_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#ifndef AMI_STREAM_IMP_SINGLE
#  warning Including __FILE__ when AMI_STREAM_IMP_SINGLE undefined.
#endif

// For use in core by main_mem_operate().
#include <quicksort.H>

#include <pqueue_heap.H>

#include <ami_merge.H>
#include <ami_optimized_merge.H>


// A class of merge objects for merge sorting objects of type T.  We
// will actually use one of three subclasses of this class which use
// either a comparison function, a comparison object,  or the binary 
// comparison operator <.

template <class T, class Q>
class merge_sort_manager /*: public AMI_merge_base<T> */{
private:
    bool use_operator;
protected:
    Q  *pq;
    arity_t input_arity;
#if DEBUG_ASSERTIONS
    unsigned int input_count, output_count;
#endif
public:
    merge_sort_manager(void);
    virtual ~merge_sort_manager(void);
    inline AMI_err operate(CONST T * CONST *in, AMI_merge_flag *taken_flags,
                    int &taken_index, T *out);
    size_t space_usage_per_stream(void);
};


template<class T, class Q>
merge_sort_manager<T,Q>::merge_sort_manager(void)
{
}

template<class T, class Q>
merge_sort_manager<T,Q>::~merge_sort_manager(void)
{
    if (pq != NULL) {
        delete pq;
    }
}



template<class T, class Q>
size_t merge_sort_manager<T,Q>::space_usage_per_stream(void)
{
    return sizeof(arity_t) + sizeof(T);
}

template<class T, class Q>
AMI_err merge_sort_manager<T,Q>::operate(CONST T * CONST *in,
                                       AMI_merge_flag */*taken_flags*/,
                                       int &taken_index,
                                       T *out)
{
    bool pqret;
    
    // If the queue is empty, we are done.  There should be no more
    // inputs.
    if (!pq->num_elts()) {

#if DEBUG_ASSERTIONS
        arity_t ii;
        
        for (ii = input_arity; ii--; ) {
            tp_assert(in[ii] == NULL, "Empty queue but more input.");
        }

        tp_assert(input_count == output_count,
                  "Merge done, input_count = " << input_count <<
                  ", output_count = " << output_count << '.');
#endif        

        // Delete the queue, which may take up a lot of main memory.
        tp_assert(pq != NULL, "pq == NULL");
        delete pq;
        pq = NULL;
        
        return AMI_MERGE_DONE;

    } else {
        arity_t min_source;
        T min_t;

        pqret = pq->extract_min(min_source,min_t);
        tp_assert(pqret, "pq->extract_min() failed.");
        *out = min_t;
	if (in[min_source] != NULL) {
            pqret = pq->insert(min_source,*in[min_source]);
            tp_assert(pqret, "pq->insert() failed.");
            taken_index = min_source;
            //taken_flags[min_source] = 1;
#if DEBUG_ASSERTIONS
            input_count++;
#endif            
        } else {
            taken_index = -1;
        }
#if DEBUG_ASSERTIONS
        output_count++;
#endif        
        return AMI_MERGE_OUTPUT;
    }
}


// Operator based merge sort manager.

template <class T, class Q>
class merge_sort_manager_op : public merge_sort_manager<T,Q> {
private:
    Q *new_pqueue(arity_t arity);
public:
    merge_sort_manager_op(void);    
    virtual ~merge_sort_manager_op(void);    
    AMI_err main_mem_operate(T* mm_stream, size_t len);
    size_t space_usage_overhead(void);
    AMI_err initialize(arity_t arity, CONST T * CONST *in,
                       AMI_merge_flag *taken_flags,
                       int &taken_index);
};    

template<class T,class Q>
merge_sort_manager_op<T,Q>::merge_sort_manager_op(void)
{
    pq = NULL;
}

template<class T, class Q>
Q *merge_sort_manager_op<T,Q>::new_pqueue(arity_t arity)
{
    return pq = new Q (arity);
}

template<class T,class Q>
merge_sort_manager_op<T,Q>::~merge_sort_manager_op(void)
{
}

template<class T,class Q>
AMI_err merge_sort_manager_op<T,Q>::main_mem_operate(T* mm_stream, size_t len)
{
    quick_sort_op(mm_stream, len);
    return AMI_ERROR_NO_ERROR;
}

template<class T,class Q>
size_t merge_sort_manager_op<T,Q>::space_usage_overhead(void)
{
    return sizeof(Q);
}

template<class T, class Q>
AMI_err merge_sort_manager_op<T,Q>::initialize(arity_t arity, CONST T * CONST *in,
                                          AMI_merge_flag *taken_flags,
                                          int &taken_index)
{
    arity_t ii;

    input_arity = arity;

    bool pqret;
    
    tp_assert(arity > 0, "Input arity is 0.");
    
    if (pq != NULL) {
        delete pq;
        pq = NULL;
    }
    new_pqueue(arity);
    
#if DEBUG_ASSERTIONS
    input_count = output_count = 0;
#endif    
    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            taken_flags[ii] = 1;
            pqret = pq->insert(ii,*in[ii]);
            tp_assert(pqret, "pq->insert() failed.");
#if DEBUG_ASSERTIONS
            input_count++;
#endif                  
        } else {
            taken_flags[ii] = 0;
        }
    }

    taken_index = -1;
    return AMI_MERGE_READ_MULTIPLE;
}

// Comparison object based merge sort manager.

template <class T, class Q, class CMPR>
class merge_sort_manager_obj : public merge_sort_manager<T,Q> {
private:
    CMPR *cmp_o;
    Q *new_pqueue(arity_t arity);
public:
    merge_sort_manager_obj(CMPR *cmp);
    virtual ~merge_sort_manager_obj(void);    
    AMI_err main_mem_operate(T* mm_stream, size_t len);
    size_t space_usage_overhead(void);
    AMI_err initialize(arity_t arity, CONST T * CONST *in,
                       AMI_merge_flag *taken_flags,
                       int &taken_index);
};   

template<class T, class Q, class CMPR>
merge_sort_manager_obj<T,Q,CMPR>::merge_sort_manager_obj(CMPR *cmp)
{
    cmp_o = cmp;
    pq = NULL;
}

template<class T, class Q, class CMPR>
Q *merge_sort_manager_obj<T,Q,CMPR>::new_pqueue(arity_t arity)
{
    return pq = new Q (arity,cmp_o);
}

template<class T, class Q, class CMPR>
merge_sort_manager_obj<T,Q,CMPR>::~merge_sort_manager_obj(void)
{
}


template<class T, class Q, class CMPR>
AMI_err merge_sort_manager_obj<T,Q,CMPR>::main_mem_operate(T* mm_stream, size_t len)
{
    quick_sort_obj(mm_stream, len, cmp_o);
    return AMI_ERROR_NO_ERROR;
}

template<class T, class Q, class CMPR>
size_t merge_sort_manager_obj<T,Q,CMPR>::space_usage_overhead(void)
{
    return sizeof(Q);
}

template<class T, class Q, class CMPR>
AMI_err merge_sort_manager_obj<T,Q,CMPR>::initialize(arity_t arity, CONST T * CONST *in,
                                          AMI_merge_flag *taken_flags,
                                          int &taken_index)
{
    arity_t ii;

    input_arity = arity;

    bool pqret;
    
    tp_assert(arity > 0, "Input arity is 0.");
    
    if (pq != NULL) {
        delete pq;
        pq = NULL;
    }
    new_pqueue(arity);
    
#if DEBUG_ASSERTIONS
    input_count = output_count = 0;
#endif    
    for (ii = arity; ii--; ) {
        if (in[ii] != NULL) {
            taken_flags[ii] = 1;
            pqret = pq->insert(ii,*in[ii]);
            tp_assert(pqret, "pq->insert() failed.");
#if DEBUG_ASSERTIONS
            input_count++;
#endif                  
        } else {
            taken_flags[ii] = 0;
        }
    }

    taken_index = -1;
    return AMI_MERGE_READ_MULTIPLE;
}


// Comment: (jan) You must not you this version anymore

// // Comparison function based merge sort manager.

// End Comment


// *******************************************************************
// *                                                                 *
// *           The actual AMI_sort calls                             *
// *                                                                 *
// ******************************************************************* 

// Comment: (jan) You must not you this version anymore

// // A version of AMI_sort that takes an input stream of elements of
// // type T, an output stream, and a user-specified comparison function

// End Comment

// A version of AMI_sort that takes an input streamof elements of type
// T, and an output stream, and and uses the < operator to sort

template<class T>
AMI_err AMI_sort_V1(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream)
{
    merge_sort_manager_op<T,pqueue_heap_op<arity_t,T> > msm;

    return AMI_generalized_partition_and_merge(instream, outstream,
           (merge_sort_manager_op<T,pqueue_heap_op<arity_t,T> > *)&msm);
}

// A version of AMI_sort that takes an input stream of elements of
// type T, an output stream, and a user-specified comparison
// object. The comparison object "cmp", of (user-defined) class
// represented by CMPR, must have a member function called "compare"
// which is used for sorting the input stream.

template<class T, class CMPR>
AMI_err AMI_sort_V1(AMI_STREAM<T> *instream, AMI_STREAM<T> *outstream,
                 CMPR *cmp)
{
    merge_sort_manager_obj<T,pqueue_heap_obj<arity_t,T,CMPR>,CMPR > msm(cmp);

    return AMI_generalized_partition_and_merge (instream, outstream,
     (merge_sort_manager_obj<T,pqueue_heap_obj<arity_t,T,CMPR>,CMPR> *)&msm);
}
                          
#endif // _AMI_SORT_SINGLE_H 
