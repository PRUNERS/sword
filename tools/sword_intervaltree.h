#include "rtl/sword_common.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <list>
#include <string>
#include <set>
#include <stack>
#include <vector>

#define RACE(node1, node2)                                              \
  ((node1->getAccessType() == unsafe_write) ||                          \
   (node2->getAccessType() == unsafe_write) ||                          \
   ((node1->getAccessType() == atomic_write) &&				\
    (node2->getAccessType() == unsafe_read)) ||                         \
   ((node2->getAccessType() == atomic_write) &&                         \
    (node1->getAccessType() == unsafe_read)))

#if PRINT
static int global_key = 0;
#endif

class Interval {
  friend class IntervalTree;
 private:
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

 public:
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
    address = item.getAddress();
    count = 1;
    diff = 0;
    size_type = item.getAccessSizeType();
    pc.num = item.getPC();
    max = item.getAddress();;
    left = NULL;
    right = NULL;
    mutex.insert(mtx.begin(), mtx.end());
  }

  uint8_t getAccessSizeType() const {
    return size_type;
  }

  AccessSize getAccessSize() const {
    return (AccessSize) (size_type >> 4);
  }

  AccessType getAccessType() const {
    return (AccessType) (size_type & 0x0F);
  }

  size_t getAddress() const {
    return address;
  }

  size_t getPC() const {
    return pc.num;
  }

  size_t getEnd() {
    return address + (diff * (count - 1));
  }

  size_t getMax() {
    return max;
  }

  size_t getDiff() {
    return diff;
  }

  unsigned getCount() {
    return count;
  }

  void setMax(size_t max) {
    this->max = max;
  }

  Interval *getLeft() {
    return left;
  }

  void setLeft(Interval *left) {
    this->left = left;
  }

  Interval *getRight() {
    return right;
  }

  void setRight(Interval *right) {
    this->right = right;
  }

  std::string tostring() {
    std::stringstream ss;
    ss << "[" << address << "," << getEnd() << "," << getMax() << "," << std::dec << count << "," << diff << "]";
    ss << std::endl;
    return ss.str();
  }

  int compareTo(const Access &item) {
    if (address < item.getAddress()) {
      return -1;
    } else {
      return 1;
    }
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
    size_t end;

    if (tmp == NULL) {
      tmp = new Interval(item, mutex);
      return tmp;
    }

    Interval *ptr = tmp;

    while(ptr != NULL) {
      if (item.getAddress() > ptr->getMax()) {
        ptr->setMax(item.getAddress());
      }

      if((item.getAccessSizeType() == ptr->size_type) && (item.getPC() == ptr->getPC()) && (mutex == ptr->mutex)) {
        if(ptr->diff != 0) {
          end = ptr->getEnd();
          if(item.getAddress() == (end + ptr->diff)) {
            ptr->count++;
            if(ptr->getEnd() > ptr->max)
              ptr->max = ptr->getEnd();
            return tmp;
          }
          if((item.getAddress() >= ptr->address) && (item.getAddress() <= end))
            return tmp;
          if(item.getAddress() == (ptr->address - ptr->diff)) {
            ptr->address = item.getAddress();
            ptr->count++;
            if(ptr->getEnd() > ptr->max)
              ptr->max = ptr->getEnd();
            return tmp;
          }
        } else {
          size_t diff = item.getAddress() - ptr->address;
          // ptr->diff = item.getAddress() - ptr->address;
          // if(ptr->diff != 0) {
          if(diff != 0 && diff < 64) {
            end = ptr->getEnd();
            ptr->diff = item.getAddress() - ptr->address;
            if(item.getAddress() == (end + diff)) {
              ptr->count++;
              if(ptr->getEnd() > ptr->max)
                ptr->max = ptr->getEnd();
              return tmp;
            }
            if((item.getAddress() >= ptr->address) && (item.getAddress() <= end))
              return tmp;
            if(item.getAddress() == (ptr->address - ptr->diff)) {
              ptr->address = item.getAddress();
              ptr->count++;
              if(ptr->getEnd() > ptr->max)
                ptr->max = ptr->getEnd();
              return tmp;
            }
          } else {
            return tmp;
          }
        }
      }

      if (ptr->compareTo(item) <= 0) {
        if (ptr->getRight() == NULL) {
          ptr->setRight(new Interval(item, mutex));
          return tmp;
        } else {
          ptr = ptr->getRight();
        }
      } else {
        if (ptr->getLeft() == NULL) {
          ptr->setLeft(new Interval(item, mutex));
          return tmp;
        } else {
          ptr = ptr->getLeft();
        }
      }
    }

    return tmp;
  }

  void printTree(Interval *tmp) {
    if (tmp == NULL) {
      return;
    }

    if (tmp->getLeft() != NULL) {
      printTree(tmp->getLeft());
    }

    std::cout << tmp->tostring();

    if (tmp->getRight() != NULL) {
      printTree(tmp->getRight());
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
        if((node->getAddress() <= node2->getEnd()) && (node2->getAddress() <= node->getEnd())) {
          // INFO(std::cout, std::hex << "[" << node->getAddress() << "," << node->getEnd() << "][" << node2->getAddress() << "," << node2->getEnd() << "]");
          res.emplace_back(*node, *node2);
        }
      }
      stack.pop();

      if ((node->getLeft() != NULL) && (node->getLeft()->getMax() >= node2->getAddress())) {
        stack.push(node->getLeft());
      }

      if (node->getRight())
        stack.push(node->getRight());
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

      if (node->getRight())
        stack.push(node->getRight());
      if (node->getLeft())
        stack.push(node->getLeft());
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

    if (node->getLeft())
      {
        printf("    %d -> %d;\n", node->key, node->left->key);
        printf("%d [label=\"%zu,%zu\n%zu,%u\"]", node->key, node->address, node->getEnd(), node->getMax(), node->count);
        printf("%d [label=\"%zu,%zu\n%zu,%u\"]", node->left->key, node->left->address, node->left->getEnd(), node->left->getMax(), node->count);
        bst_print_dot_aux(node->getLeft());
      }
    else
      bst_print_dot_null(node->key, nullcount++);

    if (node->getRight())
      {
        printf("    %d -> %d;\n", node->key, node->right->key);
        printf("%d [label=\"%zu,%zu\n%zu,%u\"]", node->key, node->address, node->getEnd(), node->getMax(), node->count);
        printf("%d [label=\"%zu,%zu\n%zu,%u\"]", node->right->key, node->right->address, node->right->getEnd(), node->right->getMax(), node->count);
        bst_print_dot_aux(node->getRight());
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
