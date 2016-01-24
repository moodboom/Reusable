// Copyright (c) 1994 Darren Erik Vengroff
//
// File: pqueue.H
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 9/28/94
//
// This code is based on a priority queue class given to me by
// Owen  L. Astrachan <ola@cs.duke.edu>
//
// $Id: pqueue.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
#ifndef _PQUEUE_H
#define _PQUEUE_H

// K is the kind of object stored in the queue, P is it's priority.  

template <class K, class P> class pqueue
{
private:
    struct Qelt{
	K thing;
	P priority;
    } * elements;                        // array of prioritized stuff
    int numelts;                         // # elements in priority queue
    int size;                            // capacity of priority queue
    int (*cmp_f)(const P&, const P&);
public:
    pqueue(int,int (*)(const P&, const P&));  // initialize to given size
    ~pqueue();
    void insert(const K &,P);       // insert K with int priority
    int num_elts();
    int IsFull();
    bool extract_min(T& elt, P& prio);
    void DeleteMin();                    // remove min elt from pqueue
    void MinElt(K &,P &);           // return min elt and priority
};

template <class K, class P> pqueue<K,P>::pqueue(int n,
                                                int (*cmp)(const P&, const P&))
{
    elements = new Qelt[n+1];
    numelts = 0;
    size = n;
    cmp_f = cmp;
}

template <class K, class P> pqueue<K,P>::~pqueue()
{
    delete [] elements;
    size = 0;
    numelts = 0;
}

template <class K, class P> int pqueue<K,P>::num_elts()
{
    return numelts;
}
    
template <class K, class P> int pqueue<K,P>::IsFull()
{
    return numelts == size ? 1 : 0;
}

template <class K, class P> void pqueue<K,P>::insert(const K & elt, P prio)
{
    if (!IsFull()){
	++numelts;
	elements[numelts].thing  = elt;
	elements[numelts].priority = prio;
    }
}

template <class T, class P>
bool pqueue_heap<T,P>::extract_min(T& elt, P& prio) {
{
    if (!cur_elts) {
        return false;
    }

    int min=1;
    int k;
    for(k=2; k <= numelts; k++){
        if (cmp_f(elements[k].priority,elements[min].priority) < 0){
            min = k;
        }
    }
    elt = elements[min].priority;
    prio = elements[min].thing;
    for(k=min; k < numelts; k++){
        elements[k] = elements[k+1];
    }
    numelts--;
    
    return true;
}

#if 0        
template <class K, class P> void pqueue<K,P>::DeleteMin()
{
    if (numelts > 0){
	int min=1;
	int k;
	for(k=2; k <= numelts; k++){
	    if (cmp_f(elements[k].priority,elements[min].priority) < 0){
		min = k;
	    }
	}
	for(k=min; k < numelts; k++){
	    elements[k] = elements[k+1];
	}
	numelts--;
    }
}

template <class K, class P> void pqueue<K,P>::MinElt(K & ref,P & prio)
{
    int min=1;
    int k;
    for(k=2; k <= numelts; k++){
	if (cmp_f(elements[k].priority, elements[min].priority) < 0){
	    min = k;
	}
    }
    ref = elements[min].thing;
    prio = elements[min].priority;
}
#endif

#endif // _PQUEUE_H 
