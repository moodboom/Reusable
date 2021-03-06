// Copyright (C) 2001 Octavian Procopiuc
//
// File:    ami_kdtree_base.H
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// Supporting types for AMI_kdtree and AMI_kdbtree: 
// AMI_kdtree_status, link_type_t,
// AMI_kdtree_params, Bin_node_default,
// AMI_kdbtree_status, AMI_kdbtree_params, 
// region_t, kdb_item_t, path_stack_item_t.
//
// $Id: ami_kd_base.H,v 1.1 2008/07/28 17:08:01 m Exp $
//

#ifndef _AMI_KD_BASE_H
#define _AMI_KD_BASE_H

#include <iostream>
#include <ami_block_base.H>
#include <ami_point.H>

// AMI_KDTREE_STORE_WEIGHTS determines whether weights are stored in all
// binary kd-tree nodes (when set to 1), or just in block nodes (when set
// to 0). Setting to 1 results in bigger binary nodes and, consequently,
// smaller fanout. The weights are used to dramatically improve the
// performance of range *counting* queries (not range reporting
// queries). Caveat emptor: avoid often change of this parameter. Trying to
// open an existing kd-tree with the wrong value for this parameter will
// generate an invalid kd-tree.
#ifndef AMI_KDTREE_STORE_WEIGHTS
#  define AMI_KDTREE_STORE_WEIGHTS 0
#endif

// AMI_KDTREE_USE_EXACT_SPLIT determines how points on the median line are
// distributed. If set to 1, some of the points go into the left child,
// some into the right child; the search procedure should look into both
// children. If set to 0, only the left child contains those points. In
// theory, this is a tradeoff between space utilization and query
// performance. However, query performance is rarely affected, so a value
// of 1 is appropriate for most instances. Trying to open an existing
// kd-tree with the wrong value for this parameter will generate an invalid
// kd-tree.
#ifndef AMI_KDTREE_USE_EXACT_SPLIT
#  define AMI_KDTREE_USE_EXACT_SPLIT 1
#endif

// AMI_KDTREE_USE_KDBTREE_LEAF determines what the info field of a leaf
// contains. Setting to 1 gives a three-element info field, similar to
// the one used by the K-D-B-tree. This allows a kd-tree to be
// transformed into a K-D-B-tree without touching the leaves, but it
// wastes 4 bytes in every leaf. If set to 0, the info field of a leaf
// contains only two 4-byte elements. Caveat emptor: avoid often
// change of this parameter. Trying to open an existing kd-tree with
// the wrong value for this parameter will generate an invalid kd-tree.
#ifndef AMI_KDTREE_USE_KDBTREE_LEAF
#  define AMI_KDTREE_USE_KDBTREE_LEAF 1
#endif

// AMI_KDTREE_USE_REAL_MEDIAN determines the kd-tree splitting method.  If
// set to 1, medians are used. If set to 0, the weight of the left branch
// is always a power of 2. This allows kd-trees to be very compact in terms
// of storage utilization. The only place this value is checked is the
// median() method of the kd-tree.
#ifndef AMI_KDTREE_USE_REAL_MEDIAN
#  define AMI_KDTREE_USE_REAL_MEDIAN 0
#endif

// The default grid size (on each dimension) for the grid bulk loading.
#ifndef AMI_KDTREE_GRID_SIZE
#  define AMI_KDTREE_GRID_SIZE  256
#endif

// Loading methods. These bits can be combined, but not all
// combinations are valid.
#define AMI_KDTREE_LOAD_SORT    0x1
#define AMI_KDTREE_LOAD_SAMPLE  0x2
#define AMI_KDTREE_LOAD_BINARY  0x4
#define AMI_KDTREE_LOAD_GRID    0x8


// AMI_kdtree status type.
enum AMI_kdtree_status {
  AMI_KDTREE_STATUS_VALID = 0,
  AMI_KDTREE_STATUS_INVALID = 1,
};

// Node type type.
enum link_type_t {
  BLOCK_NODE = 0,
  BLOCK_LEAF,
  BIN_NODE,

  GRID_INDEX
};


// AMI_kdtree run-time parameters.
class AMI_kdtree_params {
public:

  // Max number of Value's in a leaf. 0 means use all available capacity.
  size_t leaf_size_max;
  // Max number of Key's in a node. 0 means use all available capacity.
  size_t node_size_max;
   // How much bigger is the leaf logical block than the system block.
  size_t leaf_block_factor;
  // How much bigger is the node logical block than the system block.
  size_t node_block_factor; 
  // The max number of leaves cached.
  size_t leaf_cache_size;
  // The max number of nodes cached.
  size_t node_cache_size;
  // Max height of a binary node inside a block node (other than
  // root). The root binary node has height 0. A default value, based
  // on node capacity, is used if set to 0.
  size_t max_intranode_height;
  // Max height of a binary node inside the root block node. The root
  // binary node has height 0. A default value, based on node
  // capacity, is used if set to 0.
  size_t max_intraroot_height;
  // The grid size on each dimension, for grid bulk loading.
  size_t grid_size;

  // The default parameter values.
  AMI_kdtree_params(): 
    leaf_size_max(0), node_size_max(0),
    leaf_block_factor(1), node_block_factor(1), 
    leaf_cache_size(8), node_cache_size(8),
    max_intranode_height(0), max_intraroot_height(0),
    grid_size(AMI_KDTREE_GRID_SIZE) {}
};


// A base class for all binary node implementations. This is not a complete
// implementation of a kd-tree binary node!
template<class coord_t, size_t dim>
class AMI_kdtree_bin_node_base {
public:

  void initialize(const Point<coord_t, dim> &p, size_t d) {
    assert(d < dim);
    discr_val_ = p[d];
    discr_dim_ = d;
  }
  size_t get_discriminator_dim() {
    return discr_dim_;
  }
  coord_t get_discriminator_val() {
    return discr_val_;
  }
  int discriminate(const Point<coord_t, dim> &p) const {
    return (p[discr_dim_] < discr_val_) ? -1: (p[discr_dim_] > discr_val_) ? 1: 0;
  }
  //  int discriminate(const Point<coord_t, dim> &p) const {
  //    return (p[discr_dim_] <= discr_val_) ? -1: 1;
  //  }

#if AMI_KDTREE_STORE_WEIGHTS
  size_t &low_weight() {
    return lo_weight_;
  }
  size_t &high_weight() {
    return hi_weight_;
  }
  size_t low_weight() const {
    return lo_weight_;
  }
  size_t high_weight() const {
    return hi_weight_;
  }
private:
  size_t lo_weight_;
  size_t hi_weight_;
#endif

private:
  // The split coordinate (the split hyperplane crosses the orthogonal
  // axis in this value).
  coord_t discr_val_; 
  // The dimension orthogonal to the split hyperplane. Should be less than dim.
  size_t discr_dim_;
};


// The default binary node implementation. 
// (All binary node implementations should have the same public interface).
template<class coord_t, size_t dim>
class AMI_kdtree_bin_node_default: public AMI_kdtree_bin_node_base<coord_t, dim> {
public:

  void set_low_child(size_t idx, link_type_t idx_type) {
    lo_child_ = idx;
    lo_type_ = idx_type;
  }
  void set_high_child(size_t idx, link_type_t idx_type) {
    hi_child_ = idx;
    hi_type_ = idx_type;
  }
  void get_low_child(size_t &idx, link_type_t &idx_type) const {
    idx = lo_child_;
    idx_type = lo_type_;
  }
  void get_high_child(size_t &idx, link_type_t &idx_type) const {
    idx = hi_child_;
    idx_type = hi_type_;
  }

private:
  // The low child (i.e., its position in the block node).
  size_t lo_child_;
  link_type_t lo_type_;
  // The high child (i.e., its position in the block node).
  size_t hi_child_;
  link_type_t hi_type_;  
};


// Another binary node implementation, smaller than the default (uses short
// int instead of size_t and link_type_t).
template<class coord_t, size_t dim>
class AMI_kdtree_bin_node_short: public AMI_kdtree_bin_node_base<coord_t, dim> {
public:

  void set_low_child(size_t idx, link_type_t idx_type) {
    lo_child_ = (unsigned short) idx;
    lo_type_ = (unsigned short) idx_type;
  }
  void set_high_child(size_t idx, link_type_t idx_type) {
    hi_child_ = (unsigned short) idx;
    hi_type_ = (unsigned short) idx_type;
  }
  void get_low_child(size_t &idx, link_type_t &idx_type) const {
    idx = (size_t) lo_child_;
    idx_type = (link_type_t) lo_type_;
  }
  void get_high_child(size_t &idx, link_type_t &idx_type) const {
    idx = (size_t) hi_child_;
    idx_type = (link_type_t) hi_type_;
  }

private:
  // The low child (i.e., its position in the block node).
  unsigned short lo_child_;
  unsigned short lo_type_;
  // The high child (i.e., its position in the block node).
  unsigned short hi_child_;
  unsigned short hi_type_;
};


// Yet another binary node type. Same functionality as the default
// type, but much more compact. 
template<class coord_t, size_t dim>
class AMI_kdtree_bin_node_small: public AMI_kdtree_bin_node_base<coord_t, dim> {
public:

  void set_low_child(size_t idx, link_type_t idx_type) {
    lo_child_ = ((unsigned short) idx << 2)  | ((unsigned short) idx_type & 0x3);
  }
  void set_high_child(size_t idx, link_type_t idx_type) {
    hi_child_ = ((unsigned short) idx << 2) | ((unsigned short) idx_type & 0x3);
  }  

  void get_low_child(size_t &idx, link_type_t &idx_type) const {
    idx = lo_child_ >> 2;
    idx_type = (link_type_t) (lo_child_ & 0x3);
  }
  void get_high_child(size_t &idx, link_type_t &idx_type) const {
    idx = hi_child_ >> 2;
    idx_type = (link_type_t) (hi_child_ & 0x3);
  }

private:
  // The low child and type together.
  unsigned short lo_child_;
  // The high child and type together.
  unsigned short hi_child_;
};


// A binary node larger than the default. Stores an entire point as a
// discriminator, instead of just one value. Does not inherit from the base
// class.
template<class coord_t, size_t dim>
class AMI_kdtree_bin_node_large {
public:
  void initialize(const Point<coord_t, dim> &p, size_t d) {
    discr_val_ = p;
    discr_dim_ = d;
  }
  void set_low_child(size_t idx, link_type_t idx_type) {
    lo_child_ = idx;
    lo_type_ = idx_type;
  }
  void set_high_child(size_t idx, link_type_t idx_type) {
    hi_child_ = idx;
    hi_type_ = idx_type;
  }
  int discriminate(const Point<coord_t, dim> &p) const {
    return (p[discr_dim_] < discr_val_[discr_dim_]) ? -1: 
      (p[discr_dim_] > discr_val_[discr_dim_]) ? 1: 
      (p[(discr_dim_+1)%dim] < discr_val_[(discr_dim_+1)%dim]) ? -1 : 
      (p[(discr_dim_+1)%dim] == discr_val_[(discr_dim_+1)%dim]) ? 0: 1;
  }
  void get_low_child(size_t &idx, link_type_t &idx_type) const {
    idx = lo_child_;
    idx_type = lo_type_;
  }
  void get_high_child(size_t &idx, link_type_t &idx_type) const {
    idx = hi_child_;
    idx_type = hi_type_;
  }
#if AMI_KDTREE_STORE_WEIGHTS
  size_t &low_weight() {
    return lo_weight_;
  }
  size_t &high_weight() {
    return hi_weight_;
  }
  size_t low_weight() const {
    return lo_weight_;
  }
  size_t high_weight() const {
    return hi_weight_;
  }
private:
  size_t lo_weight_;
  size_t hi_weight_;
#endif


private:
  // The split point.
  Point<coord_t, dim> discr_val_; 
  // The dimension orthogonal to the split hyperplane. Should be less than dim.
  size_t discr_dim_;
  // The low child (i.e., its position in the block node).
  size_t lo_child_;
  link_type_t lo_type_;
  // The high child (i.e., its position in the block node).
  size_t hi_child_;
  link_type_t hi_type_;   
};


///// AMI_kdbtree stuff /////

template<class coord_t, size_t dim>
class region_t {
protected:
  // The low and high coordinates. The boolean bit is true iff the
  // box is bounded on that dimension.
  //  pair<coord_t, bool> lo_[dim];
  //  pair<coord_t, bool> hi_[dim];
  coord_t lo_[dim];
  coord_t hi_[dim];
  unsigned char bd_[dim]; // Contains the bounded bits: least significant bit is for low
	    // boundary, and second least significant bit is for upper
	    // boundary.
#define LO_BD_MASK ((unsigned char) 1)
#define HI_BD_MASK ((unsigned char) 2)

  //  char lo_bd_[dim];
  //  char hi_bd_[dim];
public:
  region_t() { 
    for (int i = 0; i < dim; i++)
      bd_[i] = 0;
      //      lo_bd_[i] = hi_bd_[i] = 0; //false;
  }

  // Initialize this box with the values stored in points p1 and p2.
  region_t(const Point<coord_t, dim>& p1, const Point<coord_t, dim>& p2) {
    for (size_t i = 0; i < dim; i++) {
      lo_[i] = std::min(p1[i], p2[i]);
      //      lo_bd_[i] = 1;//true;
      bd_[i] |= LO_BD_MASK; // true on low bd.
      hi_[i] = std::max(p1[i], p2[i]);
      //      hi_bd_[i] = 1;//true;
      bd_[i] |= HI_BD_MASK; // true on high bd.
      if (p1[i] == p2[i])
	LOG_WARNING_ID("  region_t: points have one identical coordinate.");
    }
  }

  coord_t lo(size_t d) const { return lo_[d]; }
  coord_t& lo(size_t d) { return lo_[d];  }

  coord_t hi(size_t d) const { return hi_[d]; }
  coord_t& hi(size_t d) { return hi_[d];  }

  bool is_bounded_lo(size_t d) const { return bool (bd_[d] & LO_BD_MASK); }
  bool is_bounded_hi(size_t d) const { return bool (bd_[d] & HI_BD_MASK); }
  bool is_bounded(size_t d) const 
    { return is_bounded_lo(d) && is_bounded_hi(d); }
  bool is_bounded() const {
    size_t i;
    for (i = 0; i < dim; i++)
      if (!is_bounded(i))
	break;
    return (i == dim);
  }

  void set_bounded_lo(size_t d, bool b) { bd_[d] |= (b ? LO_BD_MASK: 0); }
  void set_bounded_hi(size_t d, bool b) { bd_[d] |= (b ? HI_BD_MASK: 0); }

  coord_t span(size_t d) const { return hi(d) - lo(d); }

  Point<coord_t, dim> point_lo() const {
    Point<coord_t, dim> p;
    for (size_t i = 0; i < dim; i++)
      p[i] = lo_[i];
    return p;
  }

  Point<coord_t, dim> point_hi() const {
    Point<coord_t, dim> p;
    for (size_t i = 0; i < dim; i++)
      p[i] = hi_[i];
    return p;
  }

  // Cutout the portion of the region that's higher than the given
  // coordinate.
  void cutout_hi(coord_t v, size_t d) {
    hi_[d] = v;
    //    hi_bd_[d] = 1;//true;
    bd_[d] |= HI_BD_MASK;
  }

  // Cutout the portion of the region that's lower than the given
  // coordinate.
  void cutout_lo(coord_t v, size_t d) {
    lo_[d] = v;
    //    lo_bd_[d] = 1;//true;
    bd_[d] |= LO_BD_MASK;
  }

  // Return true if this box contains point p.
  bool contains(const Point<coord_t, dim>& p) const {
    size_t i;
    for (i = 0; i < dim; i++) {
      if ((is_bounded_lo(i) && p[i] <  lo_[i]) || 
	  (is_bounded_hi(i) && p[i] >  hi_[i]))
	break;
    }
    return (i == dim);
  }

  // Return true if this box intersects box r.
  bool intersects(const region_t<coord_t, dim>& r) const {
    size_t i;
    for (i = 0; i < dim; i++) {
      if ((r.is_bounded_lo(i) && relative_to_plane(r.lo_[i], i) == -1) || 
	  (r.is_bounded_hi(i) && relative_to_plane(r.hi_[i], i) == 1))
	break;
    }
    return (i == dim); 
  }

  // Return the position of this box relative to the hyperplane
  // orthogonal to dimension d and passing through sp: -1 if left of
  // the hyperplane, 1 if right of the hyperplane, and 0 if it
  // intersects the hyperplane.
  int relative_to_plane(coord_t sp, size_t d) const {
    if (is_bounded_hi(d) && !(sp < hi_[d]))
      return -1;
    if (is_bounded_lo(d) && !(lo_[d] < sp))
      return 1;
    return 0;
  }
#undef LO_BD_MASK
#undef HI_BD_MASK
};

template<class coord_t, size_t dim>
class kdb_item_t {
public:
  // For this purpose, every interval in region is considered open on
  // the left and closed on the right.
  region_t<coord_t, dim> region;
  AMI_bid bid;
  link_type_t type;
  kdb_item_t(const region_t<coord_t, dim>& r, AMI_bid b, link_type_t t): 
    region(r), bid(b), type(t) {}
  kdb_item_t() {}
};

template<class coord_t, size_t dim>
ostream &operator<<(ostream& s, const kdb_item_t<coord_t, dim>& ki) {
  s << "[";
  for (size_t i = 0; i < dim; i++) {
    if (ki.region.is_bounded_lo(i))
      s << ki.region.lo(i);
    else
      s << "-INF";
    s << " ";
  }
  for (size_t i = 0; i < dim; i++) {
    if (ki.region.is_bounded_hi(i))
      s << ki.region.hi(i);
    else
      s << "INF";
    s << " ";
  }
  s << (ki.type == BLOCK_NODE ? 'N': 'L') << ki.bid;
  s << "]";
  return s;
}

template<class coord_t, size_t dim>
struct path_stack_item_t {
  kdb_item_t<coord_t, dim> item;
  size_t d;
  size_t el_idx; // 
  path_stack_item_t(const kdb_item_t<coord_t, dim>& ki, size_t di, 
		    size_t idx = 0): item(ki), d(di), el_idx(idx) {}
  path_stack_item_t() {}
};

// Kdtree status type.
enum AMI_kdbtree_status {
  AMI_KDBTREE_STATUS_VALID = 0,
  AMI_KDBTREE_STATUS_INVALID = 1,
  AMI_KDBTREE_STATUS_KDTREE = 2, // For opening the kdb-tree as a kd-tree.
};

// Split heuristics
enum split_heuristic_t {
  CYCLICAL,
  LONGEST_SPAN,
  RANDOM,
};

class AMI_kdbtree_params: public AMI_kdtree_params {
public:
  AMI_kdbtree_params(): AMI_kdtree_params(), split_heuristic(LONGEST_SPAN) {}
  split_heuristic_t split_heuristic;
};

#endif // _AMI_KD_BASE_H
