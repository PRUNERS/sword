/*
  Interval Trees
  (C) 2012  Michel Lespinasse <walken@google.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  include/linux/interval_tree_generic.h
*/

#include <stdbool.h>

#include <algorithm>
#include <mutex>
#include <set>

extern "C" {
#include "rbtree_augmented.h"

/*
 * Template for implementing interval trees
 *
 * ITSTRUCT:   struct type of the interval tree nodes
 * ITRB:       name of struct rb_node field within ITSTRUCT
 * ITTYPE:     type of the interval endpoints
 * ITSUBTREE:  name of ITTYPE field within ITSTRUCT holding last-in-subtree
 * ITSTART(n): start endpoint of ITSTRUCT node n
 * ITLAST(n):  last endpoint of ITSTRUCT node n
 * ITSTATIC:   'static' or empty
 * ITPREFIX:   prefix to use for the inline tree definitions
 *
 * Note - before using this, please consider if non-generic version
 * (interval_tree.h) would work for you...
 */

 static inline bool overlap(const std::set<size_t>& s1, const std::set<size_t>& s2) {
   for(const auto& i : s1) {
     if(std::binary_search(s2.begin(), s2.end(), i))
       return true;
   }
   return false;
 }

enum AccessType {
  unsafe_read = 0,
  unsafe_write,
  atomic_read,
  atomic_write,
};

#define GET_ACCESS_TYPE(node) (node->size_type & 0x0F)

#define RACE(node1, node2)                                                    \
  ((GET_ACCESS_TYPE(node1) == unsafe_write) ||                                \
   (GET_ACCESS_TYPE(node2) == unsafe_write) ||                                \
   ((GET_ACCESS_TYPE(node1) == atomic_write) &&				      \
    (GET_ACCESS_TYPE(node2) == unsafe_read)) ||                               \
   ((GET_ACCESS_TYPE(node2) == atomic_write) &&                               \
    (GET_ACCESS_TYPE(node1) == unsafe_read)))

#define INTERVAL_TREE_DEFINE(ITSTRUCT, ITRB, ITTYPE, ITSUBTREE,		      \
			     ITSTART, ITLAST, ITSTATIC, ITPREFIX)	      \
									      \
/* Callbacks for augmented rbtree insert and remove */			      \
									      \
static inline ITTYPE ITPREFIX ## _compute_subtree_last(ITSTRUCT *node)	      \
{									      \
	ITTYPE max = ITLAST(node), subtree_last;			      \
	if (node->ITRB.rb_left) {					      \
		subtree_last = rb_entry(node->ITRB.rb_left,		      \
					ITSTRUCT, ITRB)->ITSUBTREE;	      \
		if (max < subtree_last)					      \
			max = subtree_last;				      \
	}								      \
	if (node->ITRB.rb_right) {					      \
		subtree_last = rb_entry(node->ITRB.rb_right,		      \
					ITSTRUCT, ITRB)->ITSUBTREE;	      \
		if (max < subtree_last)					      \
			max = subtree_last;				      \
	}								      \
	return max;							      \
}									      \
									      \
RB_DECLARE_CALLBACKS(static, ITPREFIX ## _augment, ITSTRUCT, ITRB,	      \
		     ITTYPE, ITSUBTREE, ITPREFIX ## _compute_subtree_last)    \
									      \
/* Insert / remove interval nodes from the tree */			      \
									      \
ITSTATIC void ITPREFIX ## _insert_data(ITSTRUCT node, struct rb_root *root)   \
{									      \
        struct rb_node **link = &root->rb_node, *rb_parent = NULL;            \
	ITTYPE start = node.start, last = node.last, end = 0;                 \
	ITSTRUCT *parent;						      \
									      \
	while (*link) {							      \
		rb_parent = *link;					      \
		parent = rb_entry(rb_parent, ITSTRUCT, ITRB);		      \
                                                                              \
                if((node.size_type == parent->size_type) &&                   \
                   (node.pc == parent->pc) && (node.mutex == parent->mutex)) {\
                  if(parent->diff != 0) {                                     \
                    end = END(parent);                                        \
                    if(node.start == (end + parent->diff)) {                  \
                      parent->count++;                                        \
                      parent->last = END(parent);                             \
                      return;                                                 \
                    }                                                         \
                    if((node.start >= parent->start) &&                       \
                       (node.start <= end))                                   \
                      return;                                                 \
                    if(node.start == (parent->start - parent->diff)) {        \
                      parent->start = node.start;                             \
                      parent->count++;                                        \
                      return;                                                 \
                    }                                                         \
                  } else {                                                    \
                    size_t diff = node.start - parent->start;                 \
                    if(diff != 0 && diff < 64) {                              \
                      end = END(parent);                                      \
                      parent->diff = node.start - parent->start;              \
                      if(node.start == (end + diff)) {                        \
                        parent->count++;                                      \
                        parent->last = END(parent);                           \
                        return;                                               \
                      }                                                       \
                      if((node.start >= parent->start) &&                     \
                         (node.start <= end))                                 \
                        return;                                               \
                      if(node.start == (parent->start - parent->diff)) {      \
                        parent->start = node.start;                           \
                        parent->count++;                                      \
                        return;                                               \
                      }                                                       \
                    } else {                                                  \
                      return;                                                 \
                    }                                                         \
                  }                                                           \
                }                                                             \
                                                                              \
		if (parent->ITSUBTREE < last)				      \
                  parent->ITSUBTREE = last;                                   \
		if (start < ITSTART(parent))				      \
                  link = &parent->ITRB.rb_left;                               \
		else                                                          \
                  link = &parent->ITRB.rb_right;                              \
	}								      \
									      \
        interval_tree_node *new_node = new interval_tree_node(                \
                 node.start, node.last, node.size_type, node.pc, node.mutex); \
	new_node->ITSUBTREE = last;					      \
	rb_link_node(&new_node->ITRB, rb_parent, link);			      \
	rb_insert_augmented(&new_node->ITRB, root, &ITPREFIX ## _augment);    \
}									      \
									      \
ITSTATIC void ITPREFIX ## _insert(ITSTRUCT *node, struct rb_root *root)	      \
{									      \
	struct rb_node **link = &root->rb_node, *rb_parent = NULL;	      \
	ITTYPE start = ITSTART(node), last = ITLAST(node);		      \
	ITSTRUCT *parent;						      \
									      \
	while (*link) {							      \
		rb_parent = *link;					      \
		parent = rb_entry(rb_parent, ITSTRUCT, ITRB);		      \
		if (parent->ITSUBTREE < last)				      \
			parent->ITSUBTREE = last;			      \
		if (start < ITSTART(parent))				      \
			link = &parent->ITRB.rb_left;			      \
		else							      \
			link = &parent->ITRB.rb_right;			      \
	}								      \
									      \
	node->ITSUBTREE = last;						      \
	rb_link_node(&node->ITRB, rb_parent, link);			      \
	rb_insert_augmented(&node->ITRB, root, &ITPREFIX ## _augment);	      \
}									      \
									      \
ITSTATIC void ITPREFIX ## _remove(ITSTRUCT *node, struct rb_root *root)	      \
{									      \
	rb_erase_augmented(&node->ITRB, root, &ITPREFIX ## _augment);	      \
}									      \
									      \
/*									      \
 * Iterate over intervals intersecting [start;last]			      \
 *									      \
 * Note that a node's interval intersects [start;last] iff:		      \
 *   Cond1: ITSTART(node) <= last					      \
 * and									      \
 *   Cond2: start <= ITLAST(node)					      \
 */									      \
									      \
static ITSTRUCT *							      \
ITPREFIX ## _subtree_search(ITSTRUCT *node, ITTYPE start, ITTYPE last)	      \
{									      \
	while (true) {							      \
		/*							      \
		 * Loop invariant: start <= node->ITSUBTREE		      \
		 * (Cond2 is satisfied by one of the subtree nodes)	      \
		 */							      \
		if (node->ITRB.rb_left) {				      \
			ITSTRUCT *left = rb_entry(node->ITRB.rb_left,	      \
						  ITSTRUCT, ITRB);	      \
			if (start <= left->ITSUBTREE) {			      \
				/*					      \
				 * Some nodes in left subtree satisfy Cond2.  \
				 * Iterate to find the leftmost such node N.  \
				 * If it also satisfies Cond1, that's the     \
				 * match we are looking for. Otherwise, there \
				 * is no matching interval as nodes to the    \
				 * right of N can't satisfy Cond1 either.     \
				 */					      \
				node = left;				      \
				continue;				      \
			}						      \
		}							      \
		if (ITSTART(node) <= last) {		/* Cond1 */	      \
			if (start <= ITLAST(node))	/* Cond2 */	      \
				return node;	/* node is leftmost match */  \
			if (node->ITRB.rb_right) {			      \
				node = rb_entry(node->ITRB.rb_right,	      \
						ITSTRUCT, ITRB);	      \
				if (start <= node->ITSUBTREE)		      \
					continue;			      \
			}						      \
		}							      \
		return NULL;	/* No match */				      \
	}								      \
}									      \
									      \
ITSTATIC void ITPREFIX ## _overlap(std::mutex &mtx, struct rb_root *tree1,    \
 struct rb_root *tree2, std::vector<std::pair<ITSTRUCT,ITSTRUCT>> &races)     \
{									      \
  struct rb_node **link, *rb_parent;                                          \
  ITSTRUCT *parent;                                                           \
  struct rb_node *node2;                                                      \
                                                                              \
  for (node2 = rb_first(tree2); node2; node2 = rb_next(node2)) {              \
    ITSTRUCT *node = rb_entry(node2, ITSTRUCT, ITRB);                         \
    ITTYPE start = node->start, last = node->last;                            \
    link = &tree1->rb_node;                                                   \
                                                                              \
    while (*link) {                                                           \
      rb_parent = *link;                                                      \
      parent = rb_entry(rb_parent, ITSTRUCT, ITRB);                           \
                                                                              \
      bool overlapping = false;                                               \
      if((node->mutex.size() != 0) && (parent->mutex.size() != 0))            \
        overlapping = overlap(node->mutex, parent->mutex);                    \
      if(RACE(node,parent) && !overlapping) {                                 \
        if((start <= parent->last) &&                                         \
           (parent->start <= last)) {                                         \
          bool has_overlapping = (parent->count == 1) && (node->count == 1);  \
          if(!has_overlapping) {                                              \
            if(parent->start > start)                                         \
              has_overlapping = solve_mip(parent, node);                      \
            else                                                              \
              has_overlapping = solve_mip(node, parent);                      \
          }                                                                   \
          if(has_overlapping) {                                               \
            mtx.lock();                                                       \
            races.emplace_back(*node, *parent);                               \
            mtx.unlock();                                                     \
          }                                                                   \
        }                                                                     \
      }                                                                       \
                                                                              \
      if (parent->ITSUBTREE < last)                                           \
        parent->ITSUBTREE = last;                                             \
      if (start < ITSTART(parent))                                            \
        link = &parent->ITRB.rb_left;                                         \
      else                                                                    \
        link = &parent->ITRB.rb_right;                                        \
    }                                                                         \
  }                                                                           \
}			      				                      \
			      				                      \
ITSTATIC void ITPREFIX ## _merge(struct rb_root *tree1, struct rb_root *tree2)\
{									      \
  struct rb_node **link, *rb_parent;                                          \
  ITSTRUCT *parent;                                                           \
  struct rb_node *node2;                                                      \
                                                                              \
  for (node2 = rb_last(tree2); node2; node2 = rb_prev(node2)) {               \
    ITSTRUCT *node = rb_entry(node2, ITSTRUCT, ITRB);                         \
    ITTYPE start = node->start, last = node->last;                            \
    bool merged = false;                                                      \
    link = &tree1->rb_node;                                                   \
                                                                              \
    while (*link) {                                                           \
      rb_parent = *link;                                                      \
      parent = rb_entry(rb_parent, ITSTRUCT, ITRB);                           \
                                                                              \
      if((node->size_type == parent->size_type) &&                            \
         (node->pc == parent->pc) && (node->mutex == parent->mutex) &&        \
         (node->diff == parent->diff)) {                                      \
        if(!((node->count == parent->count) && parent->count == 1)) {         \
          if(parent->start - last == parent->diff) {                          \
            merged = true;                                                    \
            parent->start = start;                                            \
            if(parent->diff != 0)                                             \
              parent->count = ((parent->last - parent->start) / parent->diff) + 1; \
            else                                                              \
              parent->count = 1;                                              \
            break;                                                            \
          } else if(start - parent->last == parent->diff) {                   \
            merged = true;                                                    \
            parent->last = last;                                              \
            if(parent->diff != 0)                                             \
              parent->count = ((parent->last - parent->start) / parent->diff) + 1; \
            else                                                              \
              parent->count = 1;                                              \
            break;                                                            \
          } else if((start <= parent->last) && (parent->start <= last)) {     \
            merged = true;                                                    \
            parent->start = parent->start < start ? parent->start : start;    \
            parent->last = parent->last < last ? last : parent->last;         \
            if(parent->diff != 0)                                             \
              parent->count = ((parent->last - parent->start) / parent->diff) + 1; \
            else                                                              \
              parent->count = 1;                                              \
            break;                                                            \
          }                                                                   \
        }                                                                     \
      }                                                                       \
                                                                              \
      if (parent->ITSUBTREE < last)                                           \
        parent->ITSUBTREE = last;                                             \
      if (start < ITSTART(parent))                                            \
        link = &parent->ITRB.rb_left;                                         \
      else                                                                    \
        link = &parent->ITRB.rb_right;                                        \
    }                                                                         \
                                                                              \
    if(!merged) {                                                             \
      interval_tree_node *new_node = new interval_tree_node(                  \
        node->start, node->last, node->size_type, node->pc, node->mutex);     \
      new_node->ITSUBTREE = last;                                             \
      rb_link_node(&new_node->ITRB, rb_parent, link);                         \
      rb_insert_augmented(&new_node->ITRB, tree1, &ITPREFIX ## _augment);     \
    }                                                                         \
    /*rb_erase(node2, tree2);                                           \
      free(node);*/                                                     \
  }                                                                           \
}			      				                      \
                                                                              \
ITSTATIC ITSTRUCT *							      \
ITPREFIX ## _iter_first(struct rb_root *root, ITTYPE start, ITTYPE last)      \
{									      \
	ITSTRUCT *node;							      \
									      \
	if (!root->rb_node)						      \
		return NULL;						      \
	node = rb_entry(root->rb_node, ITSTRUCT, ITRB);			      \
	if (node->ITSUBTREE < start)					      \
		return NULL;						      \
	return ITPREFIX ## _subtree_search(node, start, last);		      \
}									      \
									      \
ITSTATIC ITSTRUCT *							      \
ITPREFIX ## _iter_next(ITSTRUCT *node, ITTYPE start, ITTYPE last)	      \
{									      \
	struct rb_node *rb = node->ITRB.rb_right, *prev;		      \
									      \
	while (true) {							      \
		/*							      \
		 * Loop invariants:					      \
		 *   Cond1: ITSTART(node) <= last			      \
		 *   rb == node->ITRB.rb_right				      \
		 *							      \
		 * First, search right subtree if suitable		      \
		 */							      \
		if (rb) {						      \
			ITSTRUCT *right = rb_entry(rb, ITSTRUCT, ITRB);	      \
			if (start <= right->ITSUBTREE)			      \
				return ITPREFIX ## _subtree_search(right,     \
								start, last); \
		}							      \
									      \
		/* Move up the tree until we come from a node's left child */ \
		do {							      \
			rb = rb_parent(&node->ITRB);			      \
			if (!rb)					      \
				return NULL;				      \
			prev = &node->ITRB;				      \
			node = rb_entry(rb, ITSTRUCT, ITRB);		      \
			rb = node->ITRB.rb_right;			      \
		} while (prev == rb);					      \
									      \
		/* Check if the node intersects [start;last] */		      \
		if (last < ITSTART(node))		/* !Cond1 */	      \
			return NULL;					      \
		else if (start <= ITLAST(node))		/* Cond2 */	      \
			return node;					      \
	}								      \
}

  void print_dot_null(int key, int nullcount) {
    std::cout << "    null" << nullcount << " [shape=point];" << std::endl;
    std::cout << "    " << key <<"-> null" << nullcount << ";" << std::endl;
  }

  void print_dot_aux(struct interval_tree_node *node)
  {
    static int nullcount = 0;

    if (node->rb.rb_left) {
        interval_tree_node *left = rb_entry(node->rb.rb_left, struct interval_tree_node, rb);
        std::cout << "    " << node->key << " -> " << left->key << ";" << std::endl;
        std::cout << node->key << " [label=\"[" << node->start << "," << node->last << "]," << node->count << "\n" << TypeValue[(node->size_type & 0x0F)] << "," << 1 << (node->size_type >> 4)  << "," << node->pc << "\"]" << std::endl;
        std::cout << left->key << " [label=\"[" << left->start << "," << left->last << "]," << left->count << "\n" << TypeValue[(left->size_type & 0x0F)] << "," << 1 << (left->size_type >> 4) << "," << left->pc << "\"]" << std::endl;
        print_dot_aux(left);
      }
    else
      print_dot_null(node->key, nullcount++);

    if (node->rb.rb_right) {
        interval_tree_node *right = rb_entry(node->rb.rb_right, struct interval_tree_node, rb);
        std::cout << "    " << node->key << " -> " << right->key << ";" << std::endl;
        std::cout << node->key << " [label=\"[" << node->start << "," << node->last << "]," << node->count << "\n" << TypeValue[(node->size_type & 0x0F)] << "," << (node->size_type >> 4)  << "," << node->pc << "\"]" << std::endl;
        std::cout << right->key << " [label=\"[" << right->start << "," << right->last << "]," << right->count << "\n" << TypeValue[(right->size_type & 0x0F)] << "," << (right->size_type >> 4) << "," << right->pc << "\"]" << std::endl;
        print_dot_aux(right);
      }
    else
      print_dot_null(node->key, nullcount++);
  }

void interval_tree_print(struct rb_root *root) {
  std::cout << "digraph BST {\n" << std::endl;
  std::cout << "    node [fontname=\"Arial\"];\n" << std::endl;

  interval_tree_node *parent = rb_entry(root->rb_node, struct interval_tree_node, rb);

  if (!root->rb_node) {
    std::cout << "\n" << std::endl;
  } else if (!parent->rb.rb_right && !parent->rb.rb_left)
    std::cout << "    " << parent->key << ";" << std::endl;
  else
    print_dot_aux(parent);

  std::cout << "}" << std::endl;
}

/*
model = Model("IntervalsOvelap")

x1 = model.addVar(vtype=GRB.INTEGER, lb=0, ub=4, name="x1")
x2 = model.addVar(vtype=GRB.INTEGER, lb=0, ub=4, name="x2")
size1 = model.addVar(vtype=GRB.INTEGER, lb=0, ub=4, name="size1")
size2 = model.addVar(vtype=GRB.INTEGER, lb=0, ub=3, name="size2")

model.addConstr(8*x1 - 8*x2, GRB.EQUAL, 4 + size2 - size1, "c0")

# Set optimization objective
model.setObjective(x1 + x2, GRB.MINIMIZE)

# Save problem
model.write('intervals_overlap.lp')

# Optimize
model.optimize()
*/

bool solve_mip(struct interval_tree_node *node1, struct interval_tree_node *node2) {
  glp_prob *mip;
  int ia[1+4], ja[1+4];
  double ar[1+4], z, x1, x2, size1, size2;
  bool res = false;

  /* node1->print(); */
  /* node2->print(); */

  /* create problem */
  mip = glp_create_prob();
  glp_set_prob_name(mip, "overlap");
  glp_set_obj_dir(mip, GLP_MIN);
  /* fill problem */
  glp_add_rows(mip, 1);
  glp_set_row_name(mip, 1, "c0");
  long int diff = node1->start - node2->start;
  glp_set_row_bnds(mip, 1, GLP_FX, -1 * diff, -1 * diff);
  glp_add_cols(mip, 4);
  glp_set_col_name(mip, 1, "x1");
  glp_set_col_bnds(mip, 1, GLP_DB, 0.0, node1->count);
  glp_set_obj_coef(mip, 1, node1->diff);
  glp_set_col_kind(mip, 1, GLP_IV);
  glp_set_col_name(mip, 2, "x2");
  glp_set_col_bnds(mip, 2, GLP_DB, 0.0, node2->count);
  glp_set_obj_coef(mip, 2, node2->diff);
  glp_set_col_kind(mip, 2, GLP_IV);
  glp_set_col_name(mip, 3, "size1");
  glp_set_col_bnds(mip, 3, GLP_DB, 0.0, (1 << (node1->size_type >> 4)) - 1);
  glp_set_col_kind(mip, 3, GLP_IV);
  glp_set_col_name(mip, 4, "size2");
  glp_set_col_bnds(mip, 4, GLP_DB, 0.0, (1 << (node2->size_type >> 4)) - 1);
  glp_set_col_kind(mip, 4, GLP_IV);
  ia[1] = 1, ja[1] = 1, ar[1] = node1->diff;
  ia[2] = 1, ja[2] = 2, ar[2] = node2->diff; ar[2] *= -1;
  ia[3] = 1, ja[3] = 3, ar[3] = 1.0;
  ia[4] = 1, ja[4] = 4, ar[4] = -1.0;
  glp_load_matrix(mip, 4, ia, ja, ar);
  std::string filename = "ilp_problem" + std::to_string(rand()) + ".lp";
  /* if(glp_write_lp(mip, 0, filename.c_str()) == 0) */
  /*   printf("Problem written!\n"); */
  /* solve problem */
  /* int ret = glp_simplex(mip, NULL); */
  glp_iocp parm;
  glp_init_iocp(&parm);
  parm.presolve = GLP_ON;
  glp_term_out(GLP_OFF);
  glp_intopt(mip, &parm);
  int ret = glp_mip_status(mip);
  /* int ret = 0; */
  if(ret != GLP_NOFEAS) {
    /* recover and display results */
    /* z = glp_get_obj_val(mip); */
    /* x1 = glp_get_col_prim(mip, 1); */
    /* x2 = glp_get_col_prim(mip, 2); */
    /* size1 = glp_get_col_prim(mip, 3); */
    /* size2 = glp_get_col_prim(mip, 4); */
    /* printf("x1 = %g; x2 = %g, size1 = %g, size2 = %g\n", x1, x2, size1, size2); */
    res = true;
  }
  /* housekeeping */
  /* glp_delete_prob(mip); */
  /* glp_free_env(); */

  return res;
}

}
