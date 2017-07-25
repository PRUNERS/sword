#include "rtl/sword_common.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <list>
#include <string>
#include <set>
#include <stack>
#include <vector>

#define GET_ACCESS_TYPE(node) ((AccessType) (node->size_type & 0x0F))

#define RACE(node1, node2)                                              \
  ((GET_ACCESS_TYPE(node1) == unsafe_write) ||                          \
   (GET_ACCESS_TYPE(node2) == unsafe_write) ||                          \
   ((GET_ACCESS_TYPE(node1) == atomic_write) &&				\
    (GET_ACCESS_TYPE(node2) == unsafe_read)) ||                         \
   ((GET_ACCESS_TYPE(node2) == atomic_write) &&                         \
    (GET_ACCESS_TYPE(node1) == unsafe_read)))

#define GET_END(ptr) ptr->address + (ptr->diff * (ptr->count - 1))

#if PRINT
static int global_key = 0;
#endif

class Interval {
  friend class IntervalTree;
 public:
#if PRINT
  int key;
#endif
  size_t address;
  unsigned count;
  uint8_t size_type; // size in first 4 bits, type in last 4 bits
  size_t diff;
  Int48 pc;
  size_t max;
  Interval *left;
  Interval *right;
  std::set<size_t> mutex;

  Interval(size_t addr, uint8_t st, size_t p) {
#if PRINT
    key = ++global_key;
#endif
    address = addr;
    count = 1;
    diff = 0;
    size_type = st;
    pc.num = p;
    max = addr;
    left = NULL;
    right = NULL;
  }

  Interval(const Access &item, const std::set<size_t> mtx) {
#if PRINT
    key = ++global_key;
#endif
    address = item.address;
    count = 1;
    diff = 0;
    size_type = item.size_type;
    pc.num = item.pc.num;
    max = item.address;;
    left = NULL;
    right = NULL;
    mutex.insert(mtx.begin(), mtx.end());
  }

  AccessSize getAccessSize() const {
    return (AccessSize) (size_type >> 4);
  }

  std::string tostring() {
    std::stringstream ss;
    ss << "[" << address << "," << GET_END(this) << "," << max << "," << std::dec << count << "," << diff << "]";
    ss << std::endl;
    return ss.str();
  }
};

class IntervalTree {
 private:
  static inline bool overlap(const std::set<size_t>& s1, const std::set<size_t>& s2) {
    for(const auto& i : s1) {
      if(std::binary_search(s2.begin(), s2.end(), i))
        return true;
    }
    return false;
  }

 public:
  Interval *root;

 IntervalTree() : root(NULL) {}

  Interval *insertNode(Interval *tmp, const Access &item, const std::set<size_t> &mutex) {
    if (tmp == NULL) {
      tmp = new Interval(item, mutex);
      return tmp;
    }

    size_t end;
    Interval *ptr = tmp;

    while(ptr != NULL) {
      if (item.address > ptr->max) {
        ptr->max = item.address;
      }

      if((item.size_type == ptr->size_type) && (item.pc.num == ptr->pc.num) && (mutex == ptr->mutex)) {
        if(ptr->diff != 0) {
          end = GET_END(ptr);
          if(item.address == (end + ptr->diff)) {
            ptr->count++;
            if(GET_END(ptr) > ptr->max)
              ptr->max = GET_END(ptr);
            return tmp;
          }
          if((item.address >= ptr->address) && (item.address <= end))
            return tmp;
          if(item.address == (ptr->address - ptr->diff)) {
            ptr->address = item.address;
            ptr->count++;
            if(GET_END(ptr) > ptr->max)
              ptr->max = GET_END(ptr);
            return tmp;
          }
        } else {
          size_t diff = item.address - ptr->address;
          // ptr->diff = item.address - ptr->address;
          // if(ptr->diff != 0) {
          if(diff != 0 && diff < 64) {
            end = GET_END(ptr);
            ptr->diff = item.address - ptr->address;
            if(item.address == (end + diff)) {
              ptr->count++;
              if(GET_END(ptr) > ptr->max)
                ptr->max = GET_END(ptr);
              return tmp;
            }
            if((item.address >= ptr->address) && (item.address <= end))
              return tmp;
            if(item.address == (ptr->address - ptr->diff)) {
              ptr->address = item.address;
              ptr->count++;
              if(GET_END(ptr) > ptr->max)
                ptr->max = GET_END(ptr);
              return tmp;
            }
          } else {
            return tmp;
          }
        }
      }

      if (ptr->address < item.address) {
        if (ptr->right == NULL) {
          ptr->right = new Interval(item, mutex);
          return tmp;
        } else {
          ptr = ptr->right;
        }
      } else {
        if (ptr->left == NULL) {
          ptr->left = new Interval(item, mutex);
          return tmp;
        } else {
          ptr = ptr->left;
        }
      }
    }

    return tmp;
  }

  void printTree(Interval *tmp) {
    if (tmp == NULL) {
      return;
    }

    if (tmp->left != NULL) {
      printTree(tmp->left);
    }

    std::cout << tmp->tostring();

    if (tmp->right != NULL) {
      printTree(tmp->right);
    }
  }

  static void intersectInterval(Interval *node1, Interval *node2, std::vector<std::pair<Interval,Interval>> &res) {

    if (node1 == NULL || node2 == NULL) {
      return;
    }

    std::stack<Interval *> stack;
    stack.push(node1);

    while (stack.empty() == false) {
      Interval *node = stack.top();
      bool overlapping = false;
      if((node->mutex.size() != 0) && (node2->mutex.size() != 0))
        overlapping = overlap(node->mutex, node2->mutex);
      if(RACE(node,node2) && !overlapping) {
        if((node->address <= GET_END(node2)) && (node2->address <= GET_END(node))) {
          // INFO(std::cout, std::hex << "[" << node->address << "," << GET_END(node) << "][" << node2->address << "," << GET_END(node2) << "]");
          res.emplace_back(*node, *node2);
        }
      }
      stack.pop();

      if ((node->left != NULL) && (node->left->max >= node2->address)) {
        stack.push(node->left);
      }

      if (node->right)
        stack.push(node->right);
    }
  }

  static void intersectIntervals(Interval *tree1, Interval *tree2, std::vector<std::pair<Interval,Interval>> &res) {

    if (tree1 == NULL || tree2 == NULL) {
      return;
    }

    std::stack<Interval *> stack;
    stack.push(tree2);

    while (stack.empty() == false) {
      Interval *node = stack.top();
      intersectInterval(tree1, node, res);
      stack.pop();

      if (node->right)
        stack.push(node->right);
      if (node->left)
        stack.push(node->left);
    }
  }

#if PRINT
  void bst_print_dot_null(int key, int nullcount)
  {
    printf("    null%d [shape=point];\n", nullcount);
    printf("    %d -> null%d;\n", key, nullcount);
  }

  void bst_print_dot_aux(Interval *node)
  {
    static int nullcount = 0;

    if (node->left)
      {
        printf("    %d -> %d;\n", node->key, node->left->key);
        printf("%d [label=\"%zu,%zu\n%zu,%u\"]", node->key, node->address, GET_END(node), node->max, node->count);
        printf("%d [label=\"%zu,%zu\n%zu,%u\"]", node->left->key, node->left->address, GET_END(node), node->left->max, node->count);
        bst_print_dot_aux(node->left);
      }
    else
      bst_print_dot_null(node->key, nullcount++);

    if (node->right)
      {
        printf("    %d -> %d;\n", node->key, node->right->key);
        printf("%d [label=\"%zu,%zu\n%zu,%u\"]", node->key, node->address, GET_END(node), node->max, node->count);
        printf("%d [label=\"%zu,%zu\n%zu,%u\"]", node->right->key, node->right->address, GET_END(node->right), node->right->max, node->count);
        bst_print_dot_aux(node->right);
      }
    else
      bst_print_dot_null(node->key, nullcount++);
  }

  void bst_print_dot(Interval *tree)
  {
    printf("digraph BST {\n");
    printf("    node [fontname=\"Arial\"];\n");

    if (!tree)
      printf("\n");
    else if (!tree->right && !tree->left)
      printf("    %d;\n", tree->key);
    else
      bst_print_dot_aux(tree);

    printf("}\n");
  }
#endif
};
