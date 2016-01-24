// File:    ami_PRtree.H
// Author:  Ke Yi  <yike@cs.duke.edu>
//
// Priority R-tree declaration and implementation.
//

#ifndef _AMI_PRTREE_H
#define _AMI_PRTREE_H

// at least priority_ratio of the block capacity is used to store prioirty boxes
const double priority_ratio = 0.5;

#include <quicksort.H>
#include <ami_stream.H>
#include <ami_sort.H>
#include <ami_coll.H>
#include <ami_block.H>
#include <cpu_timer.H>
#include <ami_cache.H>

#include <hash_set>
#include <queue>
#include <list>

using namespace std;

#define PRTREE_HEADER_MAGIC_NUMBER 0x41235

#ifdef NDEBUG
#define DBG(msg)
#else
#define DBG(msg)      std::cerr << msg << flush
#endif

#define box_t box_type<coord_t,dim>
template<class coord_t, size_t dim>
class box_type {
public:
	size_t id;  // used as bid of child when act as bounding boxes
	coord_t lo_[dim];
	coord_t hi_[dim];
	// each box has a unique id, assigned by the application.
	inline bool intersect(box_t& t) {
		for (size_t i=0; i<dim; i++) {
			if (t.lo_[i] > hi_[i] || t.hi_[i] < lo_[i]) return false;
		}
		return true;
	}
	inline bool contain(box_t& t) {
		for (size_t i=0; i<dim; i++) {
			if (t.lo_[i] < lo_[i] || t.hi_[i] > hi_[i]) return false;
		}
		return true;
	}
};

template<class coord_t, size_t dim>
class cmp_lo {
	// The dimension on which to compare. It should be less than dim.
	size_t d_;
public:
	cmp_lo(size_t d): d_(d) {}
	inline int compare(const box_t& p1, const box_t& p2) const {
		if (p1.id == p2.id) return 0;
		if (p1.lo_[d_] < p2.lo_[d_] 
		|| p1.lo_[d_] == p2.lo_[d_] && p1.id < p2.id)
			return -1;
		else return 1;
	}
};

template<class coord_t, size_t dim>
class cmp_hi {
	// The dimension on which to compare. It should be less than dim.
	size_t d_;
public:
	cmp_hi(size_t d): d_(d) {}
	inline int compare(const box_t& p1, const box_t& p2) const {
		if (p1.id == p2.id) return 0;
		if (p1.hi_[d_] > p2.hi_[d_]
		|| p1.hi_[d_] == p2.hi_[d_] && p1.id < p2.id)
			return -1;
		else return 1;
	}
};

template<class coord_t, size_t dim>
class cmp_lo_mm {
	// The dimension on which to compare. It should be less than dim.
	size_t d_;
public:
	cmp_lo_mm(size_t d): d_(d) {}
	inline int compare(const box_t* p1, const box_t* p2) const {
		if (p1->id == p2->id) return 0;
		if (p1->lo_[d_] < p2->lo_[d_] 
		|| p1->lo_[d_] == p2->lo_[d_] && p1->id < p2->id)
			return -1;
		else return 1;
	}
};

template<class coord_t, size_t dim>
class cmp_hi_mm {
	// The dimension on which to compare. It should be less than dim.
	size_t d_;
public:
	cmp_hi_mm(size_t d): d_(d) {}
	inline int compare(const box_t* p1, const box_t* p2) const {
		if (p1->id == p2->id) return 0;
		if (p1->hi_[d_] > p2->hi_[d_]
		|| p1->hi_[d_] == p2->hi_[d_] && p1->id < p2->id)
			return -1;
		else return 1;
	}
};

template<class coord_t, size_t dim>
class PRtree_node_info {
public:
	size_t fanout;  // Being greater than B means it is a leaf, storing fanout-B data boxes
};
#define PRtree_node_info_t PRtree_node_info<coord_t,dim>

template<class coord_t, size_t dim, class BTECOLL>
class PRtree_node: public AMI_block<box_t, PRtree_node_info<coord_t, dim>, BTECOLL> {
public:
	PRtree_node(AMI_collection_single<BTECOLL>* pcoll, AMI_bid bid=0):
			AMI_block<box_t, PRtree_node_info_t, BTECOLL>(pcoll, 0, bid) {}
};
#define PRTREE_NODE PRtree_node<coord_t, dim, BTECOLL>

template<class coord_t, size_t dim, class BTECOLL = BTE_COLLECTION > 
class PRtree {
	typedef AMI_STREAM<box_t> stream_t;
private:
	class header_type {
	public:
		unsigned int magic_number;
		AMI_bid root_bid;

		header_type():
		magic_number(PRTREE_HEADER_MAGIC_NUMBER), root_bid(0) {}
	};

	// Collection of all the nodes
	AMI_collection_single<BTECOLL> *pcoll_nodes_, *scratch_;

	// A static cache for upper levels of the tree, only used for query
	class remove_node {
	public:
		void operator()(PRTREE_NODE* p) { delete p; }
	};
	typedef AMI_cache_manager_static<AMI_bid, PRTREE_NODE*, remove_node> cache_manager_t;
	cache_manager_t* cache_;

	cpu_timer ct;
	stream_t *result;
	// # boxes stored in a leaf
	size_t B;
	size_t lbf_;
	char base_file_name_[100];
	// number of boxes fit in memory
	size_t M;
	size_t subtree;
	static const size_t grid_size = 32;
	// fanout of the binary tree
	static const size_t max_fanout = 64;
	// current streams the binary tree is operating on
	stream_t* curstreams[2*dim];
	// stream of the level of R-tree bounding boxes
	stream_t* level;
	// true if current level is the leaves level
	bool bottomlevel;
	hash_set<size_t> pbox;

	header_type header_;

	class grid {
	public:
		// The number of strips on each dimension.
		size_t t[2*dim];
		// The coord of the grid lines. l[i] is an array of length
		// t[i]+1.  l[i][0] is always the smallest point, l[i][t[i]] is the right boundary.
		coord_t* l[2*dim]; 
		// The id of the box on the grid lines.
		size_t* id[2*dim];
		// o[i][j] is the offset in streams[i] of box l[i][j]. 
		// o[i] is an array of length t[i].
		size_t* o[2*dim];
		// Total number of points represented by this sub-grid.
		size_t total_count;
		// The grid counts. It's an array of length sz (the number of cells).
		size_t* c;
		// Total number of cells: t[0] * t[1] *...* t[dim-1].
		size_t sz;

		void init(size_t tt[]) {
			sz = 1;
			for (int i=0; i<2*dim; i++) {
				t[i] = tt[i];
				sz *= t[i];
				l[i] = new coord_t[t[i]+1];
				id[i] = new size_t[t[i]+1];
				o[i] = new size_t[t[i]];
			}
			c = new size_t[sz];
			memset(c, 0, sz*sizeof(size_t));
			total_count = 0;
		}

		grid(size_t tt[]) {
			init(tt);
		}

		// construct the initial grid
		grid(size_t gs) {
			size_t tt[2*dim];
			for (int i=0; i<2*dim; i++) tt[i] = gs;
			init(tt);
		}

		~grid() {
			for (int i=0; i<2*dim; i++) {
				delete[] l[i];
				delete[] o[i];
				delete[] id[i];
			}
			delete[] c;
		}

		// hard code!
		inline size_t pos(size_t gl[]) {
			return ((gl[3]*t[2]+gl[2])*t[1]+gl[1])*t[0]+gl[0];
		}

		// return the sum of all entries at stripe x on dimension d
		size_t sum(size_t d, size_t x) {
			size_t s = 0;
			size_t gl[2*dim];
			for (gl[0]=(d==0?x:0); gl[0]<(d==0?(x+1):t[0]); gl[0]++)
				for (gl[1]=(d==1?x:0); gl[1]<(d==1?(x+1):t[1]); gl[1]++)
					for (gl[2]=(d==2?x:0); gl[2]<(d==2?(x+1):t[2]); gl[2]++)
						for (gl[3]=(d==3?x:0); gl[3]<(d==3?(x+1):t[3]); gl[3]++)
							s += c[pos(gl)];
			return s;
		}

		bool check() {
			size_t s = 0;
			for (size_t i=0; i<sz; i++)
				s+=c[i];
			return s == total_count;
		}
	};

	class BinNode {
	public:
		size_t split_dim;
		// the coordinate and id of the first point in the right half
		coord_t l;
		size_t id;
		BinNode *lchild, *rchild;
		stream_t *s[2*dim];
		char *names[2*dim];
		box_t bounding_box;
		box_t *boxes[2*dim];
		size_t num_boxes[2*dim];

		BinNode(size_t B) {
			lchild = rchild = NULL;
			for (int i=0; i<2*dim; i++) {
				s[i] = NULL;
				boxes[i] = new box_t[B];
				num_boxes[i] = 0;
			}
		}
		~BinNode() {
			for (int i=0; i<2*dim; i++)
				delete[] boxes[i];
		}
	};

public:
	int totalIO, outcacheIO;
	PRtree(char* base_file_name, size_t lbf = 1) {
		totalIO = outcacheIO = 0;
		lbf_ = lbf;
		strcpy(base_file_name_, base_file_name);
		pcoll_nodes_ = new AMI_collection_single<BTECOLL>(base_file_name, AMI_READ_WRITE_COLLECTION, lbf);
		pcoll_nodes_->persist(PERSIST_PERSISTENT);
		memcpy(&header_, pcoll_nodes_->user_data(), sizeof(header_));
		cache_ = NULL;
		B = (pcoll_nodes_->block_size() - sizeof(PRtree_node_info_t)) / sizeof(box_t);
		size_t mem = MM_manager.memory_available();
		M = (mem - 5*150*1024 - pcoll_nodes_->block_size()*4 - max_fanout*(B*sizeof(box_t)+4*B+sizeof(BinNode)+100))
			/ (sizeof(box_t) + sizeof(void*)*4*dim);
	}

	~PRtree() {
		delete pcoll_nodes_;
		if (cache_) delete cache_;
	}

	void persist(persistence per) {
		pcoll_nodes_->persist(per);
	}

	bool IsValid() {
		return header_.magic_number == PRTREE_HEADER_MAGIC_NUMBER;
	}

	// Bulk-load the PRtree using unsorted stream in_stream
	AMI_err load(stream_t* in_stream) {
		if (in_stream->stream_len() <= 1) {
			cout << "Stream too short.\n";
			return AMI_ERROR_NO_ERROR;
		}
		pcoll_nodes_->persist(PERSIST_DELETE);
		delete pcoll_nodes_;
		pcoll_nodes_ = new AMI_collection_single<BTECOLL>(lbf_);
		size_t i;
		AMI_err err;
		box_t *it;
		AMI_bid b;
		cmp_hi<coord_t, dim>* comp_hi_obj_[dim];
		cmp_lo<coord_t, dim>* comp_lo_obj_[dim];
		for (i = 0; i < dim; i++) {
			comp_hi_obj_[i] = new cmp_hi<coord_t, dim>(i);
			comp_lo_obj_[i] = new cmp_lo<coord_t, dim>(i);
		}
		stream_t *streams[2*dim];
		bottomlevel = true;
		level = in_stream;
		while (level->stream_len() > 1) {
			ct.reset(); ct.start();
			cout << "Sorting ...\n";
			for (i = 0; i < dim; i++) {
				streams[i] = new stream_t;
				streams[i]->persist(PERSIST_DELETE);
				streams[i+dim] = new stream_t;
				streams[i+dim]->persist(PERSIST_DELETE);
				err = AMI_sort(level, streams[i], comp_lo_obj_[i]);
				err = AMI_sort(level, streams[i+dim], comp_hi_obj_[i]);
				assert(err == AMI_ERROR_NO_ERROR);
			}
			ct.stop();
			cout << "Time : " << ct << "\n";
			// Actual loading
			cout << "Loading ...\n";
			ct.reset(); ct.start();
			if (level != in_stream) delete level;
			level = new stream_t;
			level->persist(PERSIST_DELETE);
			b = create_node(streams, 0);
			ct.stop();
			cout << "Time : " << ct << "\n";
			bottomlevel = false;
		}
		if (level!=in_stream) delete level;
		for (i = 0; i < dim; i++) {
			delete comp_hi_obj_[i];
			delete comp_lo_obj_[i];
		}
		// Restructuring
		cout << "Restructuring ...\n";
		ct.reset(); ct.start();
		scratch_ = new AMI_collection_single<BTECOLL>(base_file_name_, AMI_READ_WRITE_COLLECTION, lbf_);
		header_.root_bid = Restructure(b);
		delete pcoll_nodes_;
		pcoll_nodes_ = scratch_;
		ct.stop();
		header_.magic_number = PRTREE_HEADER_MAGIC_NUMBER;
		memcpy(pcoll_nodes_->user_data(), &header_, sizeof(header_));
		cout << "Time : " << ct << "\n";
		return AMI_ERROR_NO_ERROR;
	}

	AMI_bid Restructure(AMI_bid b) {
		PRTREE_NODE *p, *p1;
		p = new PRTREE_NODE(pcoll_nodes_, b);
		p1 = new PRTREE_NODE(scratch_);
		p1->info()->fanout = p->info()->fanout;
		p1->el.copy(0, p1->info()->fanout, p->el);
		if (p1->info()->fanout <= B)
			for (size_t i=0; i<p->info()->fanout; i++) 
				p1->el[i].id = Restructure(p->el[i].id);
		delete p;
		AMI_bid b1 = p1->bid();
		delete p1;
		return b1;
	}

	void delete_bintree(BinNode* p) {
		if (p->lchild == NULL) {
			delete p;
			return;
		}
		delete_bintree(p->lchild);
		delete_bintree(p->rchild);
		delete p;
	}

	// create the subtree using grid g, g is destroyed upon return
	BinNode* create_bintree(grid *g, size_t split_dim) {
		if (split_dim == 2*dim) split_dim = 0;
		BinNode* p;
		if (g->total_count <= subtree) {
			delete g;
			p = new BinNode(B);
			p->split_dim = split_dim;
			return p;
		}
		size_t lefttree = subtree;
		while (lefttree < g->total_count / 2) lefttree+=subtree;
		size_t s = 0, s1, i, j, k;
		for (i=0; i<g->t[split_dim]; i++) {
			s1 = s;
			s += g->sum(split_dim, i);
			if (s >= lefttree) break;
		}
		AMI_err err;
		box_t *it;
		// two sub grids
		grid *gl, *gr;
		size_t tt[2*dim], t[2*dim];
		memcpy(tt, g->t, sizeof(tt));
		tt[split_dim] = i+1;
		gl = new grid(tt);
		size_t d;
		for (d=0; d<2*dim; d++) 
			for (j=0; j<=tt[d]; j++) {
				gl->l[d][j] = g->l[d][j];
				gl->id[d][j] = g->id[d][j];
				if (j<tt[d]) gl->o[d][j] = g->o[d][j];
			}
		tt[split_dim] = i;
		for (t[0]=0; t[0]<tt[0]; t[0]++)
			for (t[1]=0; t[1]<tt[1]; t[1]++)
				for (t[2]=0; t[2]<tt[2]; t[2]++)
					for (t[3]=0; t[3]<tt[3]; t[3]++)
						gl->c[gl->pos(t)] = g->c[g->pos(t)];
		tt[split_dim] = g->t[split_dim] - i;
		gr = new grid(tt);
		for (d=0; d<2*dim; d++) 
			for (j=0; j<=tt[d]; j++) {
				if (d==split_dim) k = j+i;
				else k = j;
				gr->l[d][j] = g->l[d][k];
				gr->id[d][j] = g->id[d][k];
				if (j<tt[d]) gr->o[d][j] = g->o[d][k];
			}
		for (t[0]=0; t[0]<tt[0]; t[0]++)
			for (t[1]=0; t[1]<tt[1]; t[1]++)
				for (t[2]=0; t[2]<tt[2]; t[2]++)
					for (t[3]=0; t[3]<tt[3]; t[3]++) {
						size_t grpos = gr->pos(t);
						t[split_dim] += i;
						gr->c[grpos] = g->c[g->pos(t)];
						t[split_dim] -= i;
					}
		
		curstreams[split_dim]->seek(g->o[split_dim][i]);
		size_t new_off = g->o[split_dim][i];
		coord_t co;
		while (s1 < lefttree) {
			err = curstreams[split_dim]->read_item(&it);
			assert(err==AMI_ERROR_NO_ERROR);
			assert(i==g->t[split_dim]-1 || new_off<g->o[split_dim][i+1]);
			new_off++;
			if (it->lo_[0] < g->l[0][0] || it->lo_[0] == g->l[0][0] && it->id < g->id[0][0]) continue;
			if (it->lo_[0] > g->l[0][g->t[0]] || it->lo_[0] == g->l[0][g->t[0]] && it->id > g->id[0][g->t[0]]) continue;
			if (it->lo_[1] < g->l[1][0] || it->lo_[1] == g->l[1][0] && it->id < g->id[1][0]) continue;
			if (it->lo_[1] > g->l[1][g->t[1]] || it->lo_[1] == g->l[1][g->t[1]] && it->id > g->id[1][g->t[1]]) continue;
			if (it->hi_[0] > g->l[2][0] || it->hi_[0] == g->l[2][0] && it->id < g->id[2][0]) continue;
			if (it->hi_[0] < g->l[2][g->t[2]] || it->hi_[0] == g->l[2][g->t[2]] && it->id > g->id[2][g->t[2]]) continue;
			if (it->hi_[1] > g->l[3][0] || it->hi_[1] == g->l[3][0] && it->id < g->id[3][0]) continue;
			if (it->hi_[1] < g->l[3][g->t[3]] || it->hi_[1] == g->l[3][g->t[3]] && it->id > g->id[3][g->t[3]]) continue;
			s1++;
			for (d=0; d<2*dim; d++) {
				if (d==split_dim) continue;
				t[d] = g->t[d] - 1;
				if (d<dim) {
					co = it->lo_[d];
					while (co < g->l[d][t[d]] || co == g->l[d][t[d]] && it->id < g->id[d][t[d]])
						t[d]--; 
				}	else {
					co = it->hi_[d-dim];
					while (co > g->l[d][t[d]] || co == g->l[d][t[d]] && it->id < g->id[d][t[d]])
						t[d]--; 
				}
				assert(t[d]>=0 && t[d]<g->t[d]);
			}
			t[split_dim] = gl->t[split_dim] - 1;
			gl->c[gl->pos(t)]++;
			t[split_dim] = 0;
			assert(gr->c[gr->pos(t)] > 0);
			gr->c[gr->pos(t)]--;
		}
		if (split_dim < dim) gl->l[split_dim][gl->t[split_dim]] = it->lo_[split_dim];
		else gl->l[split_dim][gl->t[split_dim]] = it->hi_[split_dim-dim];
		gl->id[split_dim][gl->t[split_dim]] = it->id;
		err = curstreams[split_dim]->read_item(&it);
		assert(err==AMI_ERROR_NO_ERROR);
		if (split_dim < dim) gr->l[split_dim][0] = it->lo_[split_dim];
		else gr->l[split_dim][0] = it->hi_[split_dim-dim];
		gr->id[split_dim][0] = it->id;
		gr->o[split_dim][0] = new_off;
		gl->total_count = lefttree;
		gr->total_count = g->total_count - lefttree;
		assert(gl->check());
		assert(gr->check());
		p = new BinNode(B);
		p->id = it->id;
		if (split_dim < dim) p->l = it->lo_[split_dim];
		else p->l = it->hi_[split_dim - dim];
		p->split_dim = split_dim;
		delete g;
		p->lchild = create_bintree(gl, split_dim+1);
		p->rchild = create_bintree(gr, split_dim+1);
		return p;
	}

	AMI_bid create_node_mm(box_t** sm[2*dim], size_t sz, size_t split_dim) {
		if (sz == 0) {
			for (size_t i=0; i<2*dim; i++)
				delete[] sm[i];
			return 0;
		}
		size_t i, j, l;
		PRTREE_NODE* p;
		AMI_err err;
		box_t *it;
		AMI_bid b;

		if (split_dim == 2*dim) split_dim = 0;

		size_t k;
		// Take m boxes from each dimension
		size_t m = B / (2*dim);
		// head and tail of each mm stream
		size_t smh[2*dim], lsmt[2*dim], rsmt[2*dim]; 
		box_t bounding_box;

		// for each dimension, extract the first B boxes
		pbox.clear();
		for (i=0; i<2*dim; i++) {
			if (pbox.size() == sz) break;
			p = new PRTREE_NODE(pcoll_nodes_);
			bounding_box = *sm[i][0];
			j = 0;
			smh[i] = 0;
			do {
				it = sm[i][smh[i]];
				smh[i]++;
				if (pbox.find(it->id) == pbox.end()) {
					p->el[j] = *it;
					j++;
					pbox.insert(it->id);
					for (l=0; l<dim; l++) {
						if (it->hi_[l] > bounding_box.hi_[l]) bounding_box.hi_[l] = it->hi_[l];
						if (it->lo_[l] < bounding_box.lo_[l]) bounding_box.lo_[l] = it->lo_[l];
					}
				}
			} while (j<B && smh[i]<sz);
			b = p->bid();
			bounding_box.id = b;
			level->write_item(bounding_box);
			if (bottomlevel) p->info()->fanout = B + j;
			else p->info()->fanout = j;
			delete p;
			if (j < B) break;
		}
		if (sz <= 4*B) {
			for (size_t i=0; i<2*dim; i++)
				delete[] sm[i];
			return b;
		}

		// distribute streams[split_num] into two new left & right streams
		size_t leftnum;
		size_t temp = (sz - 4*B) / 2;
		leftnum = 4*B;
		while (leftnum < temp) leftnum = leftnum*2 + 4*B;
		if (leftnum > sz-4*B) 
			leftnum = sz-4*B;
		j = 0;
		box_t **lsm[2*dim], **rsm[2*dim];
		coord_t split_coord;
		size_t split_id;
		for (i=0; i<2*dim; i++) {
			lsm[i] = new box_t*[leftnum];
			rsm[i] = new box_t*[sz-4*B-leftnum];
			lsmt[i] = rsmt[i] = 0;
		}
		// left stream
		do {
			it = sm[split_dim][smh[split_dim]];
			smh[split_dim]++;
			if (pbox.find(it->id) != pbox.end()) continue;
			j++;
			lsm[split_dim][lsmt[split_dim]] = it;
			lsmt[split_dim]++;
		} while (j<leftnum);
		if (split_dim<dim) split_coord = it->lo_[split_dim];
		else split_coord = it->hi_[split_dim - dim];
		split_id = it->id;
		// right stream
		while (1) {
			if (smh[split_dim]==sz) break;
			it = sm[split_dim][smh[split_dim]];
			smh[split_dim]++;
			if (pbox.find(it->id) != pbox.end()) continue;
			rsm[split_dim][rsmt[split_dim]] = it;
			rsmt[split_dim]++;
		}

		// distribute other streams
		coord_t t;
		for (i=0; i<2*dim; i++) {
			if (i == split_dim) continue;
			while (1) {
				if (smh[i]==sz) break;
				it = sm[i][smh[i]];
				smh[i]++;
				if (pbox.find(it->id) != pbox.end()) continue;
				if (split_dim<dim) t = it->lo_[split_dim];
				else t = it->hi_[split_dim - dim];
				if (split_dim < dim && t < split_coord
					|| split_dim >= dim && t > split_coord 
					|| t == split_coord && it->id <= split_id) {
						lsm[i][lsmt[i]] = it; lsmt[i]++; 
					}	else {
						rsm[i][rsmt[i]] = it; rsmt[i]++; 
					}
			}
		}
		for (i=0; i<2*dim; i++)
			delete[] sm[i];

		// recursive calls
		create_node_mm(lsm, leftnum, split_dim+1);
		create_node_mm(rsm, sz-4*B-leftnum, split_dim+1);

		return b;
	}
	
	// create a node and its subtrees using sorted streams
	// delete the streams when finish
	// return the bid of the node
	AMI_bid create_node(stream_t* streams[2*dim], size_t split_dim) {
		size_t N = streams[0]->stream_len();
		assert(N>0);
		size_t i, j, k;
		AMI_err err;
		box_t *it;
		AMI_bid b;

		if (N <= M) {
			for (i=1; i<2*dim; i++)
				delete streams[i];
			streams[0]->seek(0);
			box_t* stream_mm = new box_t[N];
			box_t** sm[2*dim];
			sm[0] = new box_t*[N];
			for (i=0; i<N; i++) {
				streams[0]->read_item(&it);
				stream_mm[i] = *it;
				sm[0][i] = &stream_mm[i];
			}
			delete streams[0];
			cmp_hi_mm<coord_t, dim>* comp_hi_mm_obj[dim];
			cmp_lo_mm<coord_t, dim>* comp_lo_mm_obj[dim];
			for (i = 0; i < dim; i++) {
				comp_hi_mm_obj[i] = new cmp_hi_mm<coord_t, dim>(i);
				comp_lo_mm_obj[i] = new cmp_lo_mm<coord_t, dim>(i);
			}
			for (i=1; i<2*dim; i++) {
				sm[i] = new box_t*[N];
				memcpy(sm[i], sm[0], sizeof(box_t*)*N);
				if (i<dim) quick_sort_obj(sm[i], N, comp_lo_mm_obj[i]);
				else quick_sort_obj(sm[i], N, comp_hi_mm_obj[i-dim]);
			}
			for (i = 0; i < dim; i++) {
				delete comp_hi_mm_obj[i];
				delete comp_lo_mm_obj[i];
			}			
			b = create_node_mm(sm, N, split_dim);
			delete[] stream_mm;
			return b;
		}
		if (split_dim == 2*dim) split_dim = 0;

		// # boxes in a subtree
		subtree = M;
		size_t last_subtree;
		while (subtree < N) {
			last_subtree = subtree;
			subtree = subtree*max_fanout;
		}
		subtree = last_subtree;
		size_t fanout = (N-1)/ subtree + 1;
		assert(fanout <= max_fanout);

		// compute grid lines
		grid * g = new grid(grid_size);
		size_t n = N / grid_size;  
		for (i=0; i<2*dim; i++) {
			for (j=0; j<grid_size; j++) {
				k = n*j;
				streams[i]->seek(k);
				err = streams[i]->read_item(&it);
				assert(err==AMI_ERROR_NO_ERROR);
				g->id[i][j] = it->id;
				if (i<dim) g->l[i][j] = it->lo_[i];
				else g->l[i][j] = it->hi_[i-dim];
				g->o[i][j] = k;
			}
			streams[i]->seek(streams[i]->stream_len()-1);
			err = streams[i]->read_item(&it);
			assert(err==AMI_ERROR_NO_ERROR);
			if (i<dim) g->l[i][g->t[i]] = it->lo_[i];
			else g->l[i][g->t[i]] = it->hi_[i-dim];
			g->id[i][g->t[i]] = it->id;
		}
		// compute grid counts
		size_t gl[2*dim];
		coord_t co;
		streams[0]->seek(0);
		while (1) {
			err = streams[0]->read_item(&it);
			if (err == AMI_ERROR_END_OF_STREAM) break;
			for (i=0; i<2*dim; i++) {
				gl[i] = grid_size-1;
				if (i<dim) {
					co = it->lo_[i];
					while (co < g->l[i][gl[i]] || co == g->l[i][gl[i]] && it->id < g->id[i][gl[i]])
						gl[i]--;
				} else {
					co = it->hi_[i-dim];
					while (co > g->l[i][gl[i]] || co == g->l[i][gl[i]] && it->id < g->id[i][gl[i]])
						gl[i]--;
				}
				assert(gl[i]>=0 && gl[i]<grid_size);
			}
			g->c[g->pos(gl)]++;
		}
		g->total_count = N;
		assert(g->check());
		// build the binary partition
		for (i=0; i<2*dim; i++)
			curstreams[i] = streams[i];
		BinNode *binroot = create_bintree(g, split_dim);
		
		// fill in priority boxes
		PRTREE_NODE *p;
		BinNode *bp;
		size_t m;
		size_t num_bnodes = 0;
		box_t bounding_box, box, box1;
		BinNode **bnodes = new BinNode*[fanout-1];

		streams[0]->seek(0);
		while (1) {
next1:
			err = streams[0]->read_item(&it);
			if (err == AMI_ERROR_END_OF_STREAM) break;
			bp = binroot; box = *it;
			while (bp->lchild != NULL) {  // could have priority boxes
				if (bp->num_boxes[0] == 0) {
					bnodes[num_bnodes] = bp;
					num_bnodes++;
				}
				for (i=0; i<2*dim; i++) {
					if (i<dim) {
						if (bp->num_boxes[i]<B) {
							j = bp->num_boxes[i];
							bp->num_boxes[i]++;
							while (j>0 && box.lo_[i] > bp->boxes[i][(j-1)/2].lo_[i]) {
								bp->boxes[i][j] = bp->boxes[i][(j-1)/2];
								j = (j-1)/2; 
							}
							bp->boxes[i][j] = box;
							goto next1;
						} else {
							box1 = box;
							box = bp->boxes[i][0];
							j = 0;
							while (j*2+1<B) {
								if (j*2+1==B-1 || bp->boxes[i][j*2+1].lo_[i] > bp->boxes[i][j*2+2].lo_[i]) k = j*2+1;
								else k = j*2+2;
								if (bp->boxes[i][k].lo_[i] > box1.lo_[i]) {
									bp->boxes[i][j] = bp->boxes[i][k];
									j=k;
								}	else break;
							}
							bp->boxes[i][j] = box1;
						}
					} else {
						if (bp->num_boxes[i]<B) {
							j = bp->num_boxes[i];
							bp->num_boxes[i]++;
							while (j>0 && box.hi_[i-dim] < bp->boxes[i][(j-1)/2].hi_[i-dim]) {
								bp->boxes[i][j] = bp->boxes[i][(j-1)/2];
								j = (j-1)/2; 
							}
							bp->boxes[i][j] = box;
							goto next1;
						} else {
							box1 = box;
							box = bp->boxes[i][0];
							j = 0;
							while (j*2+1<B) {
								if (j*2+1==B-1 || bp->boxes[i][j*2+1].lo_[i] < bp->boxes[i][j*2+2].lo_[i]) k = j*2+1;
								else k = j*2+2;
								if (bp->boxes[i][k].hi_[i-dim] < box1.hi_[i-dim]) {
									bp->boxes[i][j] = bp->boxes[i][k];
									j=k;
								}	else break;
							}
							bp->boxes[i][j] = box1;
						}
					}
				}
				if (bp->split_dim < dim) { 
					co = box.lo_[bp->split_dim]; 
					if (co < bp->l || co == bp->l && box.id < bp->id) bp = bp->lchild;
					else bp = bp->rchild;
				}	else {
					co = box.hi_[bp->split_dim - dim];
					if (co > bp->l || co == bp->l && box.id < bp->id) bp = bp->lchild;
					else bp = bp->rchild;
				}
			}
		}
		assert(num_bnodes == fanout-1);
		pbox.clear();
		for (i=0; i<num_bnodes; i++) {
			for (j=0; j<2*dim; j++) {
				bounding_box = bnodes[i]->boxes[j][0];
				p = new PRTREE_NODE(pcoll_nodes_);
				for (k=0; k<B; k++) {
					p->el[k] = bnodes[i]->boxes[j][k];
					pbox.insert(p->el[k].id);
					for (size_t l=0; l<dim; l++) {
						if (p->el[k].hi_[l] > bounding_box.hi_[l]) bounding_box.hi_[l] = p->el[k].hi_[l];
						if (p->el[k].lo_[l] < bounding_box.lo_[l]) bounding_box.lo_[l] = p->el[k].lo_[l];
					}
				}
				if (bottomlevel) p->info()->fanout = B+B;
				else p->info()->fanout = B;
				b = p->bid();
				bounding_box.id = b;
				level->write_item(bounding_box);
				delete p;
			}
		}
		delete[] bnodes;

		// distribute the streams
		size_t num_leaves = 0;
		BinNode **binleaves = new BinNode*[fanout];
		for (i=0; i<2*dim; i++) {
			streams[i]->seek(0);
			while (1) {
next:
				err = streams[i]->read_item(&it);
				if (err == AMI_ERROR_END_OF_STREAM) break;
				if (pbox.find(it->id) != pbox.end()) goto next;
				bp = binroot;
				while (bp->lchild != NULL) {
					if (bp->split_dim < dim) { 
						co = it->lo_[bp->split_dim]; 
						if (co < bp->l || co == bp->l && it->id < bp->id) bp = bp->lchild;
						else bp = bp->rchild;
					}	else {
						co = it->hi_[bp->split_dim - dim];
						if (co > bp->l || co == bp->l && it->id < bp->id) bp = bp->lchild;
						else bp = bp->rchild;
					}
				}
				if (bp->s[0] == NULL) { // a new bin leaf
					binleaves[num_leaves] = bp;
					num_leaves++;
				}
				if (bp->s[i] == NULL) {
					bp->s[i] = new stream_t;
					bp->s[i]->persist(PERSIST_PERSISTENT);
				}
				bp->s[i]->write_item(*it);
			}
			for (j=0; j<num_leaves; j++) {
				binleaves[j]->s[i]->name(&(binleaves[j]->names[i]));
				delete binleaves[j]->s[i];
			}
		}
		assert(num_leaves == fanout);

		for (i=0; i<2*dim; i++)
			delete streams[i];
		size_t subtree1 = subtree;
		for (i=0; i<num_leaves; i++) {
			for (j=0; j<2*dim; j++) {
				binleaves[i]->s[j] = new stream_t(binleaves[i]->names[j]);
				binleaves[i]->s[j]->persist(PERSIST_DELETE);
			}
			assert(binleaves[i]->s[0]->stream_len() <= subtree1);
			assert(binleaves[i]->s[0]->stream_len() == binleaves[i]->s[1]->stream_len());
			b = create_node(binleaves[i]->s, binleaves[i]->split_dim);
			for (j=0; j<2*dim; j++)
				delete[] binleaves[i]->names[j];
		}
		delete[] binleaves;
		delete_bintree(binroot);
		return b;
	}


	// query the tree with q, write result to stream s
	// return # boxes reported
	bool query(box_t& q, stream_t *s) {
		if (header_.magic_number != PRTREE_HEADER_MAGIC_NUMBER) {
			cerr << "Not a valid PR-tree!\n";
			return false;
		}

		result = s;
		query_node(q, header_.root_bid);
		return true;
	}

	void query_node(box_t& q, AMI_bid bid) {
		PRTREE_NODE *p;
		bool hit = fetch_node(bid, p);
		totalIO++;
		if (!hit) outcacheIO++;
		size_t i;
		if (p->info()->fanout <= B) {
			for (i=0; i<p->info()->fanout; i++) {
				if (q.intersect(p->el[i])) query_node(q, (AMI_bid)(p->el[i].id));
			}
		} else {
			for (i=0; i<p->info()->fanout - B; i++) {
				if (q.intersect(p->el[i])) 
					result->write_item(p->el[i]);;
			}
		}
		p->dirty() = 0;
		if (!hit) delete p;
	}

	// load the top levels of the R-tree into cache
	void init_cache(size_t capacity) {
		// broad-first-search to fill in the cache
		queue<PRTREE_NODE*> q;
		PRTREE_NODE *p = new PRTREE_NODE(pcoll_nodes_, header_.root_bid);
		PRTREE_NODE *p1;
		cache_ = new cache_manager_t(capacity/p->block_size());
		if (!(cache_->insert(header_.root_bid, p))) {
			delete p;
			return;
		}
		q.push(p);
		while (!q.empty()) {
			p = q.front();
			q.pop();
			if (p->info()->fanout > B) return;
			for (size_t i=0; i<p->info()->fanout; i++) {
				p1 = new PRTREE_NODE(pcoll_nodes_, (AMI_bid)(p->el[i].id));
				if (p1->info()->fanout > B || 
						!cache_->insert((AMI_bid)(p->el[i].id), p1)) {
					delete p1;
					return;
				}
				q.push(p1);
			}
		}
	}

	inline bool fetch_node(AMI_bid bid, PRTREE_NODE* &p) {
		if (cache_ && cache_->read(bid, p)) return true;
		p = new PRTREE_NODE(pcoll_nodes_, bid);
		return false;
	}

	void print_stats() {
		PRTREE_NODE *p = new PRTREE_NODE(pcoll_nodes_, header_.root_bid);
		AMI_bid b;
		cout << "Block size = " << p->block_size() << "\n";
		cout << "Root fanout = " << p->info()->fanout << "\n";
		if (p->info()->fanout > 1) {
			PRTREE_NODE *p1 = new PRTREE_NODE(pcoll_nodes_, p->el[0].id);
			PRTREE_NODE *p2 = new PRTREE_NODE(pcoll_nodes_, p->el[1].id);
			PRTREE_NODE *p3;
			if (p1->info()->fanout > p2->info()->fanout) p3 = p1;
			else p3 = p2;
			cout << "Node fanout = " << p3->info()->fanout << "\n";
			delete p1;
			delete p2;
		}
		size_t d = 0;
		while (p->info()->fanout <= B) {
			PRTREE_NODE *p1 = new PRTREE_NODE(pcoll_nodes_, p->el[0].id);
			delete p;
			p = p1;
			d++;
		}
		delete p;
		cout << "Depth = " << d << "\n";
	}

};


#endif
