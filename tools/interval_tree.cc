#include <stdlib.h>
#include <stddef.h>

#include "interval_tree.h"
#include "interval_tree_generic.h"

#define START(node) ((node)->start)
#define LAST(node)  ((node)->last)

extern "C" {
INTERVAL_TREE_DEFINE(struct interval_tree_node, rb,
		     size_t, __subtree_last,
		     START, LAST,, interval_tree)
}
