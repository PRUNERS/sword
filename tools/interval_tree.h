#ifndef _LINUX_INTERVAL_TREE_H
#define _LINUX_INTERVAL_TREE_H

#include <cstdint>
#include <glpk.h>
#include <stdio.h>

#include <iostream>
#include <mutex>
#include <set>
#include <vector>

#ifdef PRINT
#include <atomic>
static std::atomic<int> global_key(0);
#endif // PRINT

extern "C" {
#include "rbtree.h"

static const char * TypeValue[] = { "R", "W", "AR", "AW" };

#define END(node) ((node)->start + ((node)->diff * ((node)->count - 1)))

struct interval_tree_node {
#ifdef PRINT
  int key;
#endif
  struct rb_node rb;
  size_t start;
  size_t last;
  size_t __subtree_last;
  unsigned count;
  uint8_t size_type; // size in first 4 bits, type in last 4 bits
  unsigned diff;
  size_t pc;
  std::set<size_t> mutex;

  interval_tree_node(size_t s, size_t l, uint8_t st, size_t p, const std::set<size_t> mtx) {
#ifdef PRINT
    key = ++global_key;
#endif // PRINT
    start = s;
    last = l;
    count = 1;
    diff = 0;
    size_type = st;
    pc = p;
    mutex.insert(mtx.begin(), mtx.end());
  }

  void print() {
    std::cout << "Start: " << start << std::endl
              << " Last: " << last << std::endl
              << " Type: " << TypeValue[(size_type & 0x0F)] << std::endl
              << " Size: " << (1 << (size_type >> 4)) << std::endl
              << "Count: " << count << std::endl
              << " Diff: " << diff << std::endl
              << "   PC: " << pc << std::endl;
  }
};

extern void
interval_tree_insert(struct interval_tree_node *node, struct rb_root *root);

extern void
interval_tree_insert_data(struct interval_tree_node node, struct rb_root *root, int t);

extern void
interval_tree_merge(struct rb_root *tree1, struct rb_root *tree2);

extern void
interval_tree_overlap(std::mutex &mtx, unsigned t1, struct rb_root *tree1,
                      unsigned t2, struct rb_root *tree2,
                      std::vector<std::pair<struct interval_tree_node, struct interval_tree_node>> &races);

extern void
interval_tree_remove(struct interval_tree_node *node, struct rb_root *root);

extern struct interval_tree_node *
interval_tree_iter_first(struct rb_root *root,
			 size_t start, size_t last);

extern struct interval_tree_node *
interval_tree_iter_next(struct interval_tree_node *node,
			size_t start, size_t last);

#ifdef PRINT
extern void interval_tree_print(struct rb_root *root);
extern void print_dot_aux(struct interval_tree_node *node, std::stringstream &ss);
extern void print_dot_null(int key, int nullcount, std::stringstream &ss);
#endif // PRINT
}

#endif	/* _LINUX_INTERVAL_TREE_H */
