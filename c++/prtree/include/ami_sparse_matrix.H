// Copyright (c) 1995 Darren Vengroff
//
// File: ami_sparse_matrix.H
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/2/95
//
// $Id: ami_sparse_matrix.H,v 1.1 2008/07/28 17:08:02 m Exp $
//
#ifndef AMI_SPARSE_MATRIX_H
#define AMI_SPARSE_MATRIX_H

// Get definitions for working with Unix and Windows
#include <portability.H>

#include <iostream>

// We need dense matrices to support some sparse/dense interactions.
#include <ami_matrix.H>

// A spares matrix element is labeled with a row er and a column ec.
// A sparse matrix is simply represented by a colletion of these.
template <class T>
class AMI_sm_elem {
public:
    unsigned int er;
    unsigned int ec;
    T val;
};


template <class T>
std::ostream &operator<<(std::ostream& s, const AMI_sm_elem<T> &a)
{
    return s << a.er << ' ' << a.ec << ' ' << a.val;
};

template <class T>
std::istream &operator>>(std::istream& s, AMI_sm_elem<T> &a)
{
    return s >> a.er >> a.ec >> a.val;
};


template<class T>
class AMI_sparse_matrix : public AMI_STREAM< AMI_sm_elem<T> > {
private:
    // How many rows and columns.
    unsigned int r,c;
public:
    AMI_sparse_matrix(unsigned int row, unsigned int col);
    ~AMI_sparse_matrix(void);
    unsigned int rows();
    unsigned int cols();
};

template<class T>
AMI_sparse_matrix<T>::AMI_sparse_matrix(unsigned int row, unsigned int col) :
        r(row), c(col), AMI_STREAM< AMI_sm_elem<T> >()
{
}

template<class T>
AMI_sparse_matrix<T>::~AMI_sparse_matrix(void)
{
}

template<class T>
unsigned int AMI_sparse_matrix<T>::rows(void)
{
    return r;
}

template<class T>
unsigned int AMI_sparse_matrix<T>::cols(void)
{
    return c;
}


//
// A class of comparison object designed to facilitate sorting of
// elements of the spase matrix into bands.
//

template<class T>
class sm_band_comparator : public comparator< AMI_sm_elem<T> >
{
private:
    unsigned int rpb;
public:
    sm_band_comparator(unsigned int rows_per_band) :
            rpb(rows_per_band) {};
    virtual ~sm_band_comparator(void) {};
    // If they are in the same band, compare columns, otherwise,
    // compare rows.
    int compare(const AMI_sm_elem<T> &t1, const AMI_sm_elem<T> &t2) {
        if ((t1.er / rpb) == (t2.er / rpb)) {
            return int(t1.ec) - int(t2.ec);
        } else {
            return int(t1.er) - int(t2.er);
        }
    }
};


// A function to bandify a sparse matrix.

template<class T>
AMI_err AMI_sparse_bandify(AMI_sparse_matrix<T> &sm,
                           AMI_sparse_matrix<T> &bsm,
                           unsigned int rows_per_band)
{
    AMI_err ae;

    sm_band_comparator<T> cmp(rows_per_band);    

    ae = AMI_sort_V1((AMI_STREAM< AMI_sm_elem<T> > *)&sm,
                  (AMI_STREAM< AMI_sm_elem<T> > *)&bsm,
                  (sm_band_comparator<T> *)&cmp);

    return ae;
}

// Get all band information for the given matrix and the current
// runtime environment.

template<class T>
AMI_err AMI_sparse_band_info(AMI_sparse_matrix<T> &opm,
                             unsigned int &rows_per_band,
                             unsigned int &total_bands)
{
    size_t sz_avail, single_stream_usage;

    unsigned int rows = opm.rows();
    
    AMI_err ae;
    
    // Check available main memory.
    sz_avail = MM_manager.memory_available ();
    
    // How much memory does a single stream need in the worst case?
    
    if ((ae = opm.main_memory_usage(&single_stream_usage,
                                    MM_STREAM_USAGE_MAXIMUM)) !=
                                    AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    // Figure out how many elements of the output can fit in main
    // memory at a time.  This will determine the number of rows of
    // the sparse matrix that go into a band.

    rows_per_band = (sz_avail - single_stream_usage * 5) / sizeof(T);

    if (rows_per_band > rows) {
        rows_per_band = rows;
    }
    
    total_bands = (rows + rows_per_band - 1) / rows_per_band;
    
    return AMI_ERROR_NO_ERROR;
}


//
//
//
//

template<class T>
AMI_err AMI_sparse_mult_scan_banded(AMI_sparse_matrix<T> &banded_opm,
                                    AMI_matrix<T> &opv, AMI_matrix<T> &res,
                                    unsigned int rows, unsigned int /*cols*/,
                                    unsigned int rows_per_band)
{
    AMI_err ae;
    
    AMI_sm_elem<T> *sparse_current;
    T *vec_current;
    unsigned int vec_row;

    banded_opm.seek(0);
    ae = banded_opm.read_item(&sparse_current);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    opv.seek(0);
    ae = opv.read_item(&vec_current);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }
    vec_row = 0;

    res.seek(0);

    unsigned int next_band_start = rows_per_band;

    unsigned int ii;
    
    unsigned int curr_band_start = 0;
    unsigned int rows_in_current_band = rows_per_band;

    bool sparse_done = false;

    T *output_subvector = new T[rows_per_band];
    
    for (ii = rows_per_band; ii--; ) {
        output_subvector[ii] = 0;
    }

    while (1) {
        //
        // Each time we enter this loop, we have the following invariants:
        //
        // 	vec_current = an element of the vector.
        //
        // 	vec_row = row vec_current came from.
        //
        //	sparse_current = current element from the banded sparse mat.
        //
        //	curr_band_start = row beginning current band.
        //
        // 	next_band_start = row beginning next band.
        //
        // 	rows_in_current_band = as name implies.
        //
        
        if (sparse_done || (sparse_current->er >= next_band_start)) {

            // If we are out of sparse elements or the sparse element
            // row is in the next band then we have to write the
            // current results, reset the output buffer, and rewind
            // the vector.

            ae = res.write_array(output_subvector, rows_in_current_band);
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }

            if (sparse_done) {
                // Write more zeores if necessary before breaking out
                // of the loop.  We have to do this in some cases if
                // there are one or more empty bands at the bottom of
                // the sparse matrix.  It is unlikely this sort of
                // thing would ever really happen, but we should be
                // careful anyway.
                T tmp = 0;
                for (ii = rows - next_band_start; ii--; ) {
                    ae = res.write_item(tmp);
                    if (ae != AMI_ERROR_NO_ERROR) {
                        return ae;
                    }
                }
                break;
            }
            
            for (ii = rows_in_current_band; ii--; ) {
                output_subvector[ii] = 0;
            }
            
            opv.seek(0);

            curr_band_start = next_band_start;
            
            next_band_start += rows_per_band;
            
            if (next_band_start > rows) {
                // The final band may not have exactly rows_per_band
                // rows due to roundoff.  We make the appropropriate
                // adjustments here.
                rows_in_current_band = rows - curr_band_start; 
                next_band_start = rows;                
            } else {
                rows_in_current_band = rows_per_band;
            }
        } else if (sparse_current->ec == vec_row) {

            // If the column of the sparse matrix and the row of the
            // vector that the current inputs come from are the same,
            // then multiply them, add the result to the appropriate
            // output element, and advance past the sparse element.

            output_subvector[sparse_current->er - curr_band_start] +=
                sparse_current->val * *vec_current;

            ae = banded_opm.read_item(&sparse_current);
            if (ae == AMI_ERROR_END_OF_STREAM) {
                sparse_done = true;
            } else if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }
            
        } else {

            // If the sparse element column is past the current
            // row in the vector, then advance the vector.

            tp_assert(sparse_current->ec > vec_row,
                      "Sparse column fell behind current row.");
            
            ae = opv.read_item(&vec_current);
            if (ae != AMI_ERROR_NO_ERROR) {
                return ae;
            }
            vec_row++;
        }
        
    }

    delete [] output_subvector;

    return AMI_ERROR_NO_ERROR;
}    



// Multiply a sparse (n,m)-matrix by a dense m-vector to get a dense
// n-vector.

template<class T>
AMI_err AMI_sparse_mult(AMI_sparse_matrix<T> &opm, AMI_matrix<T> &opv,
                        AMI_matrix<T> &res)
{
    unsigned int rows_per_band;
    unsigned int total_bands;

    unsigned int rows;
    unsigned int cols;

    // size_t sz_avail, single_stream_usage;

    AMI_err ae;
    
    // Make sure the sizes of the matrix and vectors match up.

    rows = opm.rows();
    cols = opm.cols();
    
    if ((cols != opv.rows()) || (rows != res.rows()) ||
        (opv.cols() != 1) || (res.cols() != 1)) {
        return AMI_MATRIX_BOUNDS;
    }

    // Get band information.

    ae = AMI_sparse_band_info(opm, rows_per_band, total_bands);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }

    // Partition the sparse matrix into bands with the elements within
    // a band sorted by column.  This is all done by a single sort
    // operation.
    //
    // Note that if our goal is to multiply a large number of
    // different vectors by a single matrix we should seperate this
    // step out into a preprocessing phacse so that the sort is only
    // done once.

    AMI_sparse_matrix<T> banded_opm(rows, cols);
    
    ae = AMI_sparse_bandify(opm, banded_opm, rows_per_band);
    if (ae != AMI_ERROR_NO_ERROR) {
        return ae;
    }
    
    // Scan the contents of the bands and the vector to produce output.

    ae = AMI_sparse_mult_scan_banded(banded_opm, opv, res,
                                     rows, cols, rows_per_band);

    return ae;
}

#endif // _AMI_SPARSE_MATRIX_H 
