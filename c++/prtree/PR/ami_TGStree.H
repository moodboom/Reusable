// File:    ami_PRtree.H
// Author:  Ke Yi  <yike@cs.duke.edu>
//
// TGS-tree declaration and implementation.
//

#ifndef _AMI_TGSTREE_H
#define _AMI_TGSTREE_H

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

#define TGSTREE_HEADER_MAGIC_NUMBER 0x41237

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
		if (p1.hi_[d_] < p2.hi_[d_]
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
		if (p1->hi_[d_] < p2->hi_[d_]
		|| p1->hi_[d_] == p2->hi_[d_] && p1->id < p2->id)
			return -1;
		else return 1;
	}
};

template<class coord_t, size_t dim>
class TGStree_node_info {
public:
	size_t fanout;
};
#define TGStree_node_info_t TGStree_node_info<coord_t,dim>

template<class coord_t, size_t dim, class BTECOLL>
class TGStree_node: public AMI_block<box_t, TGStree_node_info<coord_t, dim>, BTECOLL> {
public:
	TGStree_node(AMI_collection_single<BTECOLL>* pcoll, AMI_bid bid=0):
			AMI_block<box_t, TGStree_node_info_t, BTECOLL>(pcoll, 0, bid) {}
};
#define TGSTREE_NODE TGStree_node<coord_t, dim, BTECOLL>

template<class coord_t, size_t dim, class BTECOLL = BTE_COLLECTION > 
class TGStree {
	typedef AMI_STREAM<box_t> stream_t;
private:
	class header_type {
	public:
		unsigned int magic_number;
		AMI_bid root_bid;

		header_type():
		magic_number(TGSTREE_HEADER_MAGIC_NUMBER), root_bid(0) {}
	};

	// Collection of all the nodes
	AMI_collection_single<BTECOLL> * pcoll_nodes_;

	// A static cache for upper levels of the tree, only used for query
	class remove_node {
	public:
		void operator()(TGSTREE_NODE* p) { delete p; }
	};
	typedef AMI_cache_manager_static<AMI_bid, TGSTREE_NODE*, remove_node> cache_manager_t;
	cache_manager_t* cache_;

	cpu_timer ct;
	stream_t *result;
	// max fanout
	size_t B;
	// size of the subtrees, except for one (possibly)
	size_t subtree;

	header_type header_;

	class BinNode {
	public:
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
	int totalIO, outcacheIO;
	
	TGStree(char* base_file_name, size_t lbf = 1) {
		totalIO = outcacheIO = 0;
		pcoll_nodes_ = new AMI_collection_single<BTECOLL>(base_file_name, AMI_READ_WRITE_COLLECTION, lbf);
		pcoll_nodes_->persist(PERSIST_PERSISTENT);
		memcpy(&header_, pcoll_nodes_->user_data(), sizeof(header_));
		cache_ = NULL;
		B = (pcoll_nodes_->block_size() - sizeof(TGStree_node_info_t)) / sizeof(box_t);
	}

	~TGStree() {
		delete pcoll_nodes_;
		if (cache_) delete cache_;
	}

	void persist(persistence per) {
		pcoll_nodes_->persist(per);
	}

	bool IsValid() {
		return header_.magic_number == TGSTREE_HEADER_MAGIC_NUMBER;
	}

	// Bulk-load the TGStree using unsorted stream in_stream
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
		header_.root_bid = create_node(streams);
		header_.magic_number = TGSTREE_HEADER_MAGIC_NUMBER;
		memcpy(pcoll_nodes_->user_data(), &header_, sizeof(header_));
		cout << "Time : " << ct << "\n";

		return AMI_ERROR_NO_ERROR;
	}

	// create the subtree using binary method, smb's are destroyed upon return, except at leaves
	// Bs is always destroyed
	void create_bintree_mm(box_t **smb[2*dim], size_t N, BinNode_mm** binleaves, size_t &numleaves, box_t *Bs[2*dim]) {
		assert(N>0);
		BinNode_mm* p;
		size_t i, j, k, l, tl, tr;
		p = new BinNode_mm;
		if (N<=subtree) {
			p->bounding_box = *smb[0][0];
			p->sz = N;
			for (i=1; i<N; i++) {
				for (j=0; j<dim; j++) {
					if (smb[0][i]->lo_[j] < p->bounding_box.lo_[j]) p->bounding_box.lo_[j] = smb[0][i]->lo_[j];
					if (smb[0][i]->hi_[j] > p->bounding_box.hi_[j]) p->bounding_box.hi_[j] = smb[0][i]->hi_[j];
				}
			}
			for (i=0; i<2*dim; i++) {
				p->s[i] = smb[i];
				delete[] Bs[i];
			}
			binleaves[numleaves] = p;
			numleaves++;
			return;
		}
		size_t split_dim, split_loc;
		coord_t min_area, area, a;
		size_t num_subtrees = (N-1)/subtree+1;
		bool first = true;
		for (i=0;i<2*dim;i++) {
			for (j=1; j<num_subtrees; j++) {
				area = 0;
				box_t b0 = Bs[i][0];
				for (k=1; k<j; k++) {
					for (l=0; l<dim; l++) {
						if (b0.lo_[l] > Bs[i][k].lo_[l]) b0.lo_[l] = Bs[i][k].lo_[l];
						if (b0.hi_[l] < Bs[i][k].hi_[l]) b0.hi_[l] = Bs[i][k].hi_[l];
					}
				}
				a = 1;
				for (l=0; l<dim; l++)
					a *= (b0.hi_[l] - b0.lo_[l]);
				area += a;
				box_t b1 = Bs[i][j];
				for (k=j+1; k<num_subtrees; k++) {
					for (l=0; l<dim; l++) {
						if (b1.lo_[l] > Bs[i][k].lo_[l]) b1.lo_[l] = Bs[i][k].lo_[l];
						if (b1.hi_[l] < Bs[i][k].hi_[l]) b1.hi_[l] = Bs[i][k].hi_[l];
					}
				}
				a = 1;
				for (l=0; l<dim; l++)
					a *= (b1.hi_[l] - b1.lo_[l]);
				area += a;
				if (first || area < min_area) {
					first = false;
					min_area = area;
					split_dim = i; split_loc = j;
				}
			}
		}
		size_t lefttree = subtree*split_loc;
		box_t *it;
		// compute dividing line
		it = smb[split_dim][lefttree];
		p->id = it->id;
		if (split_dim < dim) p->l = it->lo_[split_dim];
		else p->l = it->hi_[split_dim - dim];
		box_t **sml[2*dim], **smr[2*dim], *Bsl[2*dim], *Bsr[2*dim];
		// partition split_dim
		sml[split_dim] = new box_t*[lefttree];
		Bsl[split_dim] = new box_t[split_loc];
		smr[split_dim] = new box_t*[N-lefttree];
		Bsr[split_dim] = new box_t[num_subtrees-split_loc];
		memcpy(sml[split_dim], smb[split_dim], sizeof(box_t*)*lefttree);
		memcpy(smr[split_dim], &smb[split_dim][lefttree], sizeof(box_t*)*(N-lefttree));
		delete[] smb[split_dim];
		for (k=0; k<split_loc; k++) Bsl[split_dim][k]=Bs[split_dim][k];
		for (k=0; k<num_subtrees-split_loc; k++) Bsr[split_dim][k]=Bs[split_dim][k+split_loc];
		delete[] Bs[split_dim];
		// partition streams except split_dim
		for (i=0; i<2*dim; i++) {
			if (i==split_dim) continue;
			sml[i] = new box_t*[lefttree];
			smr[i] = new box_t*[N-lefttree];
			Bsl[i] = new box_t[split_loc];
			Bsr[i] = new box_t[num_subtrees-split_loc];
			tl = tr = 0;
			for (j=0; j<N; j++) {
				it = smb[i][j];
				if (split_dim<dim) a = it->lo_[split_dim];
				else a = it->hi_[split_dim-dim];
				if (a < p->l || a == p->l && it->id < p->id) {
					sml[i][tl] = it;
					k = tl/subtree;
					if (tl % subtree == 0) Bsl[i][k] = *it;
					else {
						for (l=0; l<dim; l++) {
							if (it->lo_[l] < Bsl[i][k].lo_[l]) Bsl[i][k].lo_[l] = it->lo_[l];
							if (it->hi_[l] > Bsl[i][k].hi_[l]) Bsl[i][k].hi_[l] = it->hi_[l];
						}
					}
					tl++;
				} else {
					smr[i][tr] = it;
					k = tr/subtree;
					if (tr % subtree == 0) Bsr[i][k] = *it;
					else {
						for (l=0; l<dim; l++) {
							if (it->lo_[l] < Bsr[i][k].lo_[l]) Bsr[i][k].lo_[l] = it->lo_[l];
							if (it->hi_[l] > Bsr[i][k].hi_[l]) Bsr[i][k].hi_[l] = it->hi_[l];
						}
					}
					tr++;
				}
			}
			delete[] smb[i];
			delete[] Bs[i];
			assert(tl==lefttree);
			assert(tr==N-lefttree);
		}
		delete p;
		create_bintree_mm(sml, lefttree, binleaves, numleaves, Bsl);
		create_bintree_mm(smr, N-lefttree, binleaves, numleaves, Bsr);
		return;
	}

	// create the subtree using binary method, streams are destroyed upon return, except at leaves
	// Bs is always destroyed
	void create_bintree(stream_t *streams[2*dim], BinNode** binleaves, size_t &numleaves, box_t *Bs[2*dim]) {
		size_t N = streams[0]->stream_len();
		assert(N>0);
		BinNode* p;
		size_t i, j, k, l, tl, tr;
		box_t *it;
		p = new BinNode;
		if (N<=subtree) {
			streams[0]->seek(0);
			streams[0]->read_item(&it);
			p->bounding_box = *it;
			for (i=1; i<N; i++) {
				for (j=0; j<dim; j++) {
					streams[0]->read_item(&it);
					if (it->lo_[j] < p->bounding_box.lo_[j]) p->bounding_box.lo_[j] = it->lo_[j];
					if (it->hi_[j] > p->bounding_box.hi_[j]) p->bounding_box.hi_[j] = it->hi_[j];
				}
			}
			for (i=0; i<2*dim; i++) {
				p->s[i] = streams[i];
				p->s[i]->name(&(p->names[i]));
				p->s[i]->persist(PERSIST_PERSISTENT);
				delete p->s[i];
				delete[] Bs[i];
			}
			binleaves[numleaves] = p;
			numleaves++;
			return;
		}
		size_t split_dim, split_loc;
		coord_t min_area, area, a;
		size_t num_subtrees = (N-1)/subtree+1;
		bool first = true;
		for (i=0;i<2*dim;i++) {
			for (j=1; j<num_subtrees; j++) {
				area = 0;
				box_t b0 = Bs[i][0];
				for (k=1; k<j; k++) {
					for (l=0; l<dim; l++) {
						if (b0.lo_[l] > Bs[i][k].lo_[l]) b0.lo_[l] = Bs[i][k].lo_[l];
						if (b0.hi_[l] < Bs[i][k].hi_[l]) b0.hi_[l] = Bs[i][k].hi_[l];
					}
				}
				a = 1;
				for (l=0; l<dim; l++)
					a *= (b0.hi_[l] - b0.lo_[l]);
				area += a;
				box_t b1 = Bs[i][j];
				for (k=j+1; k<num_subtrees; k++) {
					for (l=0; l<dim; l++) {
						if (b1.lo_[l] > Bs[i][k].lo_[l]) b1.lo_[l] = Bs[i][k].lo_[l];
						if (b1.hi_[l] < Bs[i][k].hi_[l]) b1.hi_[l] = Bs[i][k].hi_[l];
					}
				}
				a = 1;
				for (l=0; l<dim; l++)
					a *= (b1.hi_[l] - b1.lo_[l]);
				area += a;
				if (first || area < min_area) {
					first = false;
					min_area = area;
					split_dim = i; split_loc = j;
				}
			}
		}
		size_t lefttree = subtree*split_loc;
		// compute dividing line
		streams[split_dim]->seek(lefttree);
		streams[split_dim]->read_item(&it);
		p->id = it->id;
		if (split_dim < dim) p->l = it->lo_[split_dim];
		else p->l = it->hi_[split_dim - dim];
		stream_t *sml[2*dim], *smr[2*dim];
		box_t *Bsl[2*dim], *Bsr[2*dim];
		// partition streams
		for (i=0; i<2*dim; i++) {
			sml[i] = new stream_t;
			sml[i]->persist(PERSIST_DELETE);
			smr[i] = new stream_t;
			smr[i]->persist(PERSIST_DELETE);
			Bsl[i] = new box_t[split_loc];
			Bsr[i] = new box_t[num_subtrees-split_loc];
			tl = tr = 0;
			streams[i]->seek(0);
			for (j=0; j<N; j++) {
				streams[i]->read_item(&it);
				if (split_dim<dim) a = it->lo_[split_dim];
				else a = it->hi_[split_dim-dim];
				if (a < p->l || a == p->l && it->id < p->id) {
					sml[i]->write_item(*it);
					k = tl/subtree;
					if (tl % subtree == 0) Bsl[i][k] = *it;
					else {
						for (l=0; l<dim; l++) {
							if (it->lo_[l] < Bsl[i][k].lo_[l]) Bsl[i][k].lo_[l] = it->lo_[l];
							if (it->hi_[l] > Bsl[i][k].hi_[l]) Bsl[i][k].hi_[l] = it->hi_[l];
						}
					}
					tl++;
				} else {
					smr[i]->write_item(*it);
					k = tr/subtree;
					if (tr % subtree == 0) Bsr[i][k] = *it;
					else {
						for (l=0; l<dim; l++) {
							if (it->lo_[l] < Bsr[i][k].lo_[l]) Bsr[i][k].lo_[l] = it->lo_[l];
							if (it->hi_[l] > Bsr[i][k].hi_[l]) Bsr[i][k].hi_[l] = it->hi_[l];
						}
					}
					tr++;
				}
			}
			delete streams[i];
			delete[] Bs[i];
			assert(tl==lefttree);
			assert(tr==N-lefttree);
		}
		delete p;
		char* names[2*dim];
		for (i=0; i<2*dim; i++) {
			smr[i]->name(&names[i]);
			smr[i]->persist(PERSIST_PERSISTENT);
			delete smr[i];
		}
		create_bintree(sml, binleaves, numleaves, Bsl);
		for (i=0; i<2*dim; i++) {
			smr[i] = new stream_t(names[i]);
			delete[] names[i];
			smr[i]->persist(PERSIST_DELETE);
		}
		create_bintree(smr, binleaves, numleaves, Bsr);
	}

	AMI_bid create_node_mm(box_t** sm[2*dim], size_t N) {
		assert(N>0);
		size_t i, j, k, l, t;
		AMI_err err;
		box_t *it;

		TGSTREE_NODE* p = new TGSTREE_NODE(pcoll_nodes_);
		TGStree_node_info_t *info = p->info();
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

		// compute Bs
		box_t *Bs[2*dim];
		for (i=0; i<2*dim; i++) {
			Bs[i] = new box_t[fanout];
			for (j=0; j<N; j++) {
				k = j/subtree;
				if (j % subtree == 0) {
					Bs[i][k] = *sm[i][j];
				} else {
					for (l=0; l<dim; l++) {
						if (sm[i][j]->lo_[l] < Bs[i][k].lo_[l]) Bs[i][k].lo_[l] = sm[i][j]->lo_[l];
						if (sm[i][j]->hi_[l] > Bs[i][k].hi_[l]) Bs[i][k].hi_[l] = sm[i][j]->hi_[l];
					}
				}
			}
		}
		// build the binary partition
		size_t numleaves = 0;
		BinNode_mm **binleaves = new BinNode_mm*[fanout];
		create_bintree_mm(sm, N, binleaves, numleaves, Bs);
		assert(numleaves == fanout);

		for (i=0; i<fanout; i++) {
			p->el[i] = binleaves[i]->bounding_box;
			p->el[i].id = create_node_mm(binleaves[i]->s, binleaves[i]->sz);
			delete binleaves[i];
		}
		delete[] binleaves;
		b = p->bid();
		delete p;
		return b;
	}
	
	// create a node and its subtrees using sorted streams
	// delete the streams when finish
	// return the bid of the node
	AMI_bid create_node(stream_t* streams[2*dim]) {
		size_t N = streams[0]->stream_len();
		assert(N>0);
		size_t i, j, k, l;
		AMI_err err;
		box_t *it;

		if (MM_manager.memory_available() > N*sizeof(box_t) + N*sizeof(void*)*4*dim + pcoll_nodes_->block_size()*B*2) {
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
			AMI_bid b = create_node_mm(sm, N);
			delete[] stream_mm;
			return b;
		}
		TGSTREE_NODE* p = new TGSTREE_NODE(pcoll_nodes_);
		TGStree_node_info_t *info = p->info();
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

		// compute Bs
		box_t *Bs[2*dim];
		for (i=0; i<2*dim; i++) {
			Bs[i] = new box_t[fanout];
			streams[i]->seek(0);
			for (j=0; j<N; j++) {
				k = j/subtree;
				streams[i]->read_item(&it);
				if (j % subtree == 0) {
					Bs[i][k] = *it;
				} else {
					for (l=0; l<dim; l++) {
						if (it->lo_[l] < Bs[i][k].lo_[l]) Bs[i][k].lo_[l] = it->lo_[l];
						if (it->hi_[l] > Bs[i][k].hi_[l]) Bs[i][k].hi_[l] = it->hi_[l];
					}
				}
			}
		}
		// build the binary partition
		size_t numleaves = 0;
		BinNode **binleaves = new BinNode*[fanout];
		create_bintree(streams, binleaves, numleaves, Bs);
		assert(numleaves == fanout);

		size_t underfull = 0;
		size_t subtree1 = subtree;
		for (i=0; i<numleaves; i++) {
			p->el[i] = binleaves[i]->bounding_box;
			for (j=0; j<2*dim; j++) {
				binleaves[i]->s[j] = new stream_t(binleaves[i]->names[j]);
				binleaves[i]->s[j]->persist(PERSIST_DELETE);
			}
			assert(binleaves[i]->s[0]->stream_len() <= subtree1);
			assert(binleaves[i]->s[0]->stream_len() == binleaves[i]->s[1]->stream_len());
			if (binleaves[i]->s[0]->stream_len() < subtree1) underfull++;
			p->el[i].id = create_node(binleaves[i]->s);
			for (j=0; j<2*dim; j++)
				delete[] binleaves[i]->names[j];
			delete binleaves[i];
		}
		assert(underfull<=1);
		delete[] binleaves;
		b = p->bid();
		delete p;
		return b;
	}

	// query the tree with q, write result to stream s
	// return # boxes reported
	bool query(box_t& q, stream_t *s) {
		if (header_.magic_number != TGSTREE_HEADER_MAGIC_NUMBER) {
			cerr << "Not a valid TGS-tree!\n";
			return false;
		}

		result = s;
		query_node(q, header_.root_bid);
		return true;
	}

	void query_node(box_t& q, AMI_bid bid) {
		TGSTREE_NODE *p;
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

	// Initialize a static cache of size capacity, in bytes
	void init_cache(size_t capacity) {
		// broad-first-search to fill in the cache
		queue<TGSTREE_NODE*> q;
		TGSTREE_NODE *p = new TGSTREE_NODE(pcoll_nodes_, header_.root_bid);
		TGSTREE_NODE *p1;
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
				p1 = new TGSTREE_NODE(pcoll_nodes_, (AMI_bid)(p->el[i].id));
				if (p1->info()->fanout > B ||
						!cache_->insert((AMI_bid)(p->el[i].id), p1)) {
					delete p1;
					return;
				}
				q.push(p1);
			}
		}
	}

	inline bool fetch_node(AMI_bid bid, TGSTREE_NODE* &p) {
		if (cache_ && cache_->read(bid, p)) return true;
		p = new TGSTREE_NODE(pcoll_nodes_, bid);
		return false;
	}

	void print_stats() {
		TGSTREE_NODE *p = new TGSTREE_NODE(pcoll_nodes_, header_.root_bid);
		AMI_bid b;
		cout << "Block size = " << p->block_size() << "\n";
		cout << "Root fanout = " << p->info()->fanout << "\n";
		if (p->info()->fanout > 1) {
			TGSTREE_NODE *p1 = new TGSTREE_NODE(pcoll_nodes_, p->el[0].id);
			TGSTREE_NODE *p2 = new TGSTREE_NODE(pcoll_nodes_, p->el[1].id);
			TGSTREE_NODE *p3;
			if (p1->info()->fanout > p2->info()->fanout) p3 = p1;
			else p3 = p2;
			cout << "Node fanout = " << p3->info()->fanout << "\n";
			delete p1;
			delete p2;
		}
		size_t d = 0;
		while (p->info()->fanout <= B) {
			TGSTREE_NODE *p1 = new TGSTREE_NODE(pcoll_nodes_, p->el[0].id);
			delete p;
			p = p1;
			d++;
		}
		delete p;
		cout << "Depth = " << d << "\n";
	}

};


#endif
