#ifndef _LINUX_INTERVAL_TREE_H
#define _LINUX_INTERVAL_TREE_H

static const char * TypeValue[] = { "R", "W", "AR", "AW" };

#include <stdio.h>

#include <cstdint>
#include <iostream>
#include <set>
#include <vector>

extern "C" {
#include "rbtree.h"

#define END(node) ((node)->start + ((node)->diff * ((node)->count - 1)))

static int global_key = 0;

struct interval_tree_node {
  int key;
  struct rb_node rb;
  size_t start;
  size_t last;
  size_t __subtree_last;
  unsigned count;
  uint8_t size_type; // size in first 4 bits, type in last 4 bits
  size_t diff;
  size_t pc;
  size_t max;
  std::set<size_t> mutex;

  interval_tree_node(unsigned int s, unsigned int l, uint8_t st, size_t p, const std::set<size_t> mtx) {
    key = ++global_key;
    start = s;
    last = l;
    count = 1;
    diff = 0;
    size_type = st;
    pc = p;
    mutex.insert(mtx.begin(), mtx.end());
  }
};

extern void
interval_tree_insert(struct interval_tree_node *node, struct rb_root *root);

extern void
interval_tree_insert_data(struct interval_tree_node node, struct rb_root *root);

extern void
interval_tree_merge(struct rb_root *tree1, struct rb_root *tree2,
std::vector<std::pair<struct interval_tree_node,struct interval_tree_node>> &races);

extern void
interval_tree_remove(struct interval_tree_node *node, struct rb_root *root);

extern struct interval_tree_node *
interval_tree_iter_first(struct rb_root *root,
			 unsigned long start, unsigned long last);

extern struct interval_tree_node *
interval_tree_iter_next(struct interval_tree_node *node,
			unsigned long start, unsigned long last);

extern void interval_tree_print(struct rb_root *root);
extern void print_dot_aux(struct interval_tree_node *node);
extern void print_dot_null(int key, int nullcount);
}

#endif	/* _LINUX_INTERVAL_TREE_H */
