// File:    ami_PRtree.H
// Author:  Ke Yi  <yike@cs.duke.edu>
//
// KD-tree declaration and implementation.
//

#ifndef _AMI_KDRTREE_H
#define _AMI_KDRTREE_H

#include <quicksort.H>
#include <ami_stream.H>
#include <ami_sort.H>
#include <ami_coll.H>
#include <ami_block.H>
#include <cpu_timer.H>
#include <ami_cache.H>

#include <hash_set>
#include <queue>

using namespace std;

#define KDRTREE_HEADER_MAGIC_NUMBER 0x41236

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
class KDRtree_node_info {
public:
	size_t fanout;
};
#define KDRtree_node_info_t KDRtree_node_info<coord_t,dim>

template<class coord_t, size_t dim, class BTECOLL>
class KDRtree_node: public AMI_block<box_t, KDRtree_node_info<coord_t, dim>, BTECOLL> {
public:
	KDRtree_node(AMI_collection_single<BTECOLL>* pcoll, AMI_bid bid=0):
			AMI_block<box_t, KDRtree_node_info_t, BTECOLL>(pcoll, 0, bid) {}
};
#define KDRTREE_NODE KDRtree_node<coord_t, dim, BTECOLL>

template<class coord_t, size_t dim, class BTECOLL = BTE_COLLECTION > 
class KDRtree {
	typedef AMI_STREAM<box_t> stream_t;
private:
	class header_type {
	public:
		unsigned int magic_number;
		AMI_bid root_bid;

		header_type():
		magic_number(KDRTREE_HEADER_MAGIC_NUMBER), root_bid(0) {}
	};

	// Collection of all the nodes
	AMI_collection_single<BTECOLL> * pcoll_nodes_;

	// A static cache for upper levels of the tree, only used for query
	class remove_node {
	public:
		void operator()(KDRTREE_NODE* p) { delete p; }
	};
	typedef AMI_cache_manager_static<AMI_bid, KDRTREE_NODE*, remove_node> cache_manager_t;
	cache_manager_t* cache_;

	cpu_timer ct;
	stream_t *result;
	// max fanout
	size_t B;
	// size of the subtrees, except for one (possibly)
	size_t subtree;
	size_t grid_size;
	// stores the id's of the priority boxes
	hash_set<size_t> pbox;
	// current streams the binary tree is operating on
	stream_t* curstreams[2*dim];

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

		BinNode() {
			lchild = rchild = NULL;
			for (int i=0; i<2*dim; i++)
				s[i] = NULL;
		}
	};

	class BinNode_mm {
	public:
		size_t split_dim;
		// the coordinate and id of the first point in the right half
		coord_t l;
		size_t id;
		BinNode_mm *lchild, *rchild;
		box_t **s[2*dim];
		size_t sz;
		box_t bounding_box;

		BinNode_mm() {
			lchild = rchild = NULL;
			for (int i=0; i<2*dim; i++)
				s[i] = NULL;
		}
	};

public:
	KDRtree(char* base_file_name, size_t lbf = 1) {
		pcoll_nodes_ = new AMI_collection_single<BTECOLL>(base_file_name, AMI_READ_WRITE_COLLECTION, lbf);
		pcoll_nodes_->persist(PERSIST_PERSISTENT);
		memcpy(&header_, pcoll_nodes_->user_data(), sizeof(header_));
		cache_ = NULL;
		B = (pcoll_nodes_->block_size() - sizeof(KDRtree_node_info_t)) / sizeof(box_t);
		size_t mem = MM_manager.memory_available();
		grid_size = 1;
		size_t memrequired = sizeof(size_t);
		while (memrequired < mem/3) {
			grid_size *= 2;
			memrequired <<= (2*dim);
		}
		grid_size /= 2;
	}

	~KDRtree() {
		delete pcoll_nodes_;
		if (cache_) delete cache_;
	}

	void persist(persistence per) {
		pcoll_nodes_->persist(per);
	}

	bool IsValid() {
		return header_.magic_number == KDRTREE_HEADER_MAGIC_NUMBER;
	}

	// Bulk-load the KDRtree using unsorted stream in_stream
	AMI_err load(stream_t* in_stream) {
		size_t i;
		AMI_err err;
		cmp_hi<coord_t, dim>* comp_hi_obj_[dim];
		cmp_lo<coord_t, dim>* comp_lo_obj_[dim];
		for (i = 0; i < dim; i++) {
			comp_hi_obj_[i] = new cmp_hi<coord_t, dim>(i);
			comp_lo_obj_[i] = new cmp_lo<coord_t, dim>(i);
		}

		// Sort the input streams in 2d dimensions
		ct.reset(); ct.start();
		cout << "Sorting ...\n";
		stream_t *streams[2*dim];
		for (i = 0; i < dim; i++) {
			streams[i] = new stream_t;
			streams[i]->persist(PERSIST_DELETE);
			streams[i+dim] = new stream_t;
			streams[i+dim]->persist(PERSIST_DELETE);
			err = AMI_sort(in_stream, streams[i], comp_lo_obj_[i]);
			err = AMI_sort(in_stream, streams[i+dim], comp_hi_obj_[i]);
			if (err != AMI_ERROR_NO_ERROR)
				break;
		}
		if (err != AMI_ERROR_NO_ERROR) return err;
		for (i = 0; i < dim; i++) {
			delete comp_hi_obj_[i];
			delete comp_lo_obj_[i];
		}
		cout << "Time : " << ct << "\n";

		// Actual loading
		cout << "Loading ...\n";
		ct.reset(); ct.start();
		header_.root_bid = create_node(streams, 0);
		header_.magic_number = KDRTREE_HEADER_MAGIC_NUMBER;
		memcpy(pcoll_nodes_->user_data(), &header_, sizeof(header_));
		cout << "Time : " << ct << "\n";

		return AMI_ERROR_NO_ERROR;
	}

	void delete_bintree_mm(BinNode_mm* p) {
		if (p->lchild == NULL) {
			delete p;
			return;
		}
		delete_bintree_mm(p->lchild);
		delete_bintree_mm(p->rchild);
		delete p;
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

	// create the subtree using binary method, smb's are destroyed upon return, except at leaves
	BinNode_mm* create_bintree_mm(box_t **smb[2*dim], size_t N, size_t split_dim, BinNode_mm** binleaves, size_t &numleaves) {
		assert(N>0);
		if (split_dim == 2*dim) split_dim = 0;
		BinNode_mm* p;
		size_t i, j, tl, tr;
		p = new BinNode_mm;
		if (N<=subtree) {
			p->bounding_box = *smb[0][0];
			p->sz = N;
			p->split_dim = split_dim;
			for (i=1; i<N; i++) {
				for (j=0; j<dim; j++) {
					if (smb[0][i]->lo_[j] < p->bounding_box.lo_[j]) p->bounding_box.lo_[j] = smb[0][i]->lo_[j];
					if (smb[0][i]->hi_[j] > p->bounding_box.hi_[j]) p->bounding_box.hi_[j] = smb[0][i]->hi_[j];
				}
			}
			for (i=0; i<2*dim; i++)
				p->s[i] = smb[i];
			binleaves[numleaves] = p;
			numleaves++;
			return p;
		}
		size_t lefttree = subtree;
		while (lefttree < N / 2) lefttree += subtree;
		box_t *it;
		// compute dividing line
		it = smb[split_dim][lefttree];
		p->id = it->id;
		if (split_dim < dim) p->l = it->lo_[split_dim];
		else p->l = it->hi_[split_dim - dim];
		p->split_dim = split_dim;
		box_t **sml[2*dim], **smr[2*dim];
		// partition split_dim
		sml[split_dim] = new box_t*[lefttree];
		smr[split_dim] = new box_t*[N-lefttree];
		memcpy(sml[split_dim], smb[split_dim], sizeof(box_t*)*lefttree);
		memcpy(smr[split_dim], &smb[split_dim][lefttree], sizeof(box_t*)*(N-lefttree));
		delete[] smb[split_dim];
		// partition streams except split_dim
		for (i=0; i<2*dim; i++) {
			if (i==split_dim) continue;
			sml[i] = new box_t*[lefttree];
			smr[i] = new box_t*[N-lefttree];
			tl = tr = 0;
			if (split_dim<dim) {
				for (j=0; j<N; j++) {
					it = smb[i][j];
					if (it->lo_[split_dim] < p->l || it->lo_[split_dim] == p->l && it->id < p->id) {
						sml[i][tl] = it;
						tl++;
					} else {
						smr[i][tr] = it;
						tr++;
					}
				}
			} else {
				for (j=0; j<N; j++) {
					it = smb[i][j];
					if (it->hi_[split_dim-dim] > p->l || it->hi_[split_dim-dim] == p->l && it->id < p->id) {
						sml[i][tl] = it;
						tl++;
					} else {
						smr[i][tr] = it;
						tr++;
					}
				}
			}
			delete[] smb[i];
			assert(tl==lefttree);
			assert(tr==N-lefttree);
		}
		p->lchild = create_bintree_mm(sml, lefttree, split_dim+1, binleaves, numleaves);
		p->rchild = create_bintree_mm(smr, N-lefttree, split_dim+1, binleaves, numleaves);
		return p;
	}

	// create the subtree using grid g, g is destroyed upon return
	BinNode* create_bintree(grid *g, size_t split_dim) {
		if (split_dim == 2*dim) split_dim = 0;
		BinNode* p;
		if (g->total_count <= subtree) {
			delete g;
			p = new BinNode;
			p->split_dim = split_dim;
			return p;
		}
		size_t lefttree = subtree;
		while (lefttree < g->total_count / 2) lefttree += subtree;
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
		p = new BinNode;
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
		size_t N = sz;
		assert(N>0);
		size_t i, j, t;
		AMI_err err;
		box_t *it;

		KDRTREE_NODE* p = new KDRTREE_NODE(pcoll_nodes_);
		KDRtree_node_info_t *info = p->info();
		AMI_bid b = p->bid();
		if (N <= B) { // leaf node
			info->fanout = B+N;
			for (i=0; i<N; i++)	
				p->el[i] = *sm[0][i];
			for (i=0; i<2*dim; i++)
				delete[] sm[i];
			delete p;
			return b;
		}
		subtree = B;
		size_t last_subtree;
		while (subtree < N) {
			last_subtree = subtree;
			subtree = subtree*B;
		}
		subtree = last_subtree;
		size_t fanout = N / subtree;
		if (N > fanout*subtree) fanout++;
		assert(fanout <= B);
		info->fanout = fanout;

		// build the binary partition
		size_t numleaves = 0;
		BinNode_mm **binleaves = new BinNode_mm*[fanout];
		BinNode_mm *binroot = create_bintree_mm(sm, N, split_dim, binleaves, numleaves);
		assert(numleaves == fanout);

		for (i=0; i<fanout; i++) {
			p->el[i] = binleaves[i]->bounding_box;
			p->el[i].id = create_node_mm(binleaves[i]->s, binleaves[i]->sz, binleaves[i]->split_dim);
		}
		delete[] binleaves;
		delete_bintree_mm(binroot);
		b = p->bid();
		delete p;
		return b;
	}
	
	// create a node and its subtrees using sorted streams
	// delete the streams when finish
	// return the bid of the node
	AMI_bid create_node(stream_t* streams[2*dim], size_t split_dim) {
		size_t N = streams[0]->stream_len();
		assert(N>0);
		size_t i, j;
		AMI_err err;
		box_t *it;

		if (MM_manager.memory_available() > N*sizeof(box_t) + N*sizeof(void*)*4*dim + pcoll_nodes_->block_size()*4) {
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
			AMI_bid b = create_node_mm(sm, N, split_dim);
			delete[] stream_mm;
			return b;
		}
		if (split_dim == 2*dim) split_dim = 0;
		KDRTREE_NODE* p = new KDRTREE_NODE(pcoll_nodes_);
		KDRtree_node_info_t *info = p->info();
		AMI_bid b = p->bid();

		// # boxes in a subtree
		subtree = B;
		size_t last_subtree;
		while (subtree < N) {
			last_subtree = subtree;
			subtree = subtree*B;
		}
		subtree = last_subtree;
		size_t fanout = N / subtree;
		if (N > fanout*subtree) fanout++;
		assert(fanout <= B);
		info->fanout = fanout;

		// compute grid lines
		grid * g = new grid(grid_size);
		size_t n = N / grid_size;  
		size_t k;
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
		
		// distribute the streams and compute bounding boxes
		size_t num_leaves = 0;
		BinNode **binleaves = new BinNode*[fanout];
		BinNode *bp;
		for (i=0; i<2*dim; i++) {
			streams[i]->seek(0);
			while (1) {
				err = streams[i]->read_item(&it);
				if (err == AMI_ERROR_END_OF_STREAM) break;
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
					bp->bounding_box = *it;
				}
				if (bp->s[i] == NULL) {
					bp->s[i] = new stream_t;
					bp->s[i]->persist(PERSIST_PERSISTENT);
				}
				bp->s[i]->write_item(*it);
				if (i==0) {  // update bounding box
					for (j=0; j<dim; j++) {
						if (it->lo_[j] < bp->bounding_box.lo_[j]) bp->bounding_box.lo_[j] = it->lo_[j];
						if (it->hi_[j] > bp->bounding_box.hi_[j]) bp->bounding_box.hi_[j] = it->hi_[j];
					}
				}
			}
			for (j=0; j<num_leaves; j++) {
				binleaves[j]->s[i]->name(&(binleaves[j]->names[i]));
				delete binleaves[j]->s[i];
			}
		}
		assert(num_leaves == fanout);

		for (i=0; i<2*dim; i++)
			delete streams[i];
		size_t underfull = 0;
		size_t subtree1 = subtree;
		for (i=0; i<num_leaves; i++) {
			p->el[i] = binleaves[i]->bounding_box;
			for (j=0; j<2*dim; j++) {
				binleaves[i]->s[j] = new stream_t(binleaves[i]->names[j]);
				binleaves[i]->s[j]->persist(PERSIST_DELETE);
			}
			assert(binleaves[i]->s[0]->stream_len() <= subtree1);
			assert(binleaves[i]->s[0]->stream_len() == binleaves[i]->s[1]->stream_len());
			if (binleaves[i]->s[0]->stream_len() < subtree1) underfull++;
			p->el[i].id = create_node(binleaves[i]->s, binleaves[i]->split_dim);
			for (j=0; j<2*dim; j++)
				delete[] binleaves[i]->names[j];
		}
		assert(underfull<=1);
		delete[] binleaves;
		delete_bintree(binroot);
		b = p->bid();
		delete p;
		return b;
	}


	// query the tree with q, write result to stream s
	// return # boxes reported
	bool query(box_t& q, stream_t *s) {
		if (header_.magic_number != KDRTREE_HEADER_MAGIC_NUMBER) {
			cerr << "Not a valid KDR-tree!\n";
			return false;
		}

		result = s;
		query_node(q, header_.root_bid);
		return true;
	}

	void query_node(box_t& q, AMI_bid bid) {
		KDRTREE_NODE *p;
		bool hit = fetch_node(bid, p);
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

	// Initialize a static cache of size capacity, in bytes
	void init_cache(size_t capacity) {
		// broad-first-search to fill in the cache
		queue<KDRTREE_NODE*> q;
		KDRTREE_NODE *p = new KDRTREE_NODE(pcoll_nodes_, header_.root_bid);
		KDRTREE_NODE *p1;
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
				p1 = new KDRTREE_NODE(pcoll_nodes_, (AMI_bid)(p->el[i].id));
				if (!cache_->insert((AMI_bid)(p->el[i].id), p1)) {
					delete p1;
					return;
				}
				q.push(p1);
			}
		}
	}

	inline bool fetch_node(AMI_bid bid, KDRTREE_NODE* &p) {
		if (cache_ && cache_->read(bid, p)) return true;
		p = new KDRTREE_NODE(pcoll_nodes_, bid);
		return false;
	}

	void print_stats() {
		KDRTREE_NODE *p = new KDRTREE_NODE(pcoll_nodes_, header_.root_bid);
		AMI_bid b;
		cout << "Block size = " << p->block_size() << "\n";
		cout << "Root fanout = " << p->info()->fanout << "\n";
		if (p->info()->fanout > 1) {
			KDRTREE_NODE *p1 = new KDRTREE_NODE(pcoll_nodes_, p->el[0].id);
			KDRTREE_NODE *p2 = new KDRTREE_NODE(pcoll_nodes_, p->el[1].id);
			KDRTREE_NODE *p3;
			if (p1->info()->fanout > p2->info()->fanout) p3 = p1;
			else p3 = p2;
			cout << "Node fanout = " << p3->info()->fanout << "\n";
			delete p1;
			delete p2;
		}
		size_t d = 0;
		while (p->info()->fanout <= B) {
			KDRTREE_NODE *p1 = new KDRTREE_NODE(pcoll_nodes_, p->el[0].id);
			delete p;
			p = p1;
			d++;
		}
		delete p;
		cout << "Depth = " << d << "\n";
	}

};


#endif
