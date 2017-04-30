#include "rtl/sword_common.h"
#include <cstdio>
#include <iostream>
#include <list>
#include <string>
#include <vector>

static int global_key = 0;

class Interval {
	friend class IntervalTree;
private:
	int key;
	size_t address;
	unsigned count;
	uint8_t size_type; // size in first 4 bits, type in last 4 bits
	size_t diff;
	Int48 pc;
	size_t max;
	Interval *left;
	Interval *right;

public:
	Interval(size_t addr, uint8_t st, size_t p) {
		key = ++global_key;
		address = addr;
		count = 1;
		diff = 0;
		size_type = st;
		pc.num = p;
		max = addr;
		left = NULL;
		right = NULL;
	}

	Interval(const Access &item) {
		key = ++global_key;
		address = item.getAddress();
		count = 1;
		diff = 0;
		size_type = item.getAccessSizeType();
		pc.num = item.getPC();
		max = item.getAddress();;
		left = NULL;
		right = NULL;
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
		// return "[" + std::to_string(getAddress()) + ", " + std::to_string(getEnd()) + ", " + std::to_string(getMax()) + "]";
		std::stringstream ss;
		// ss << "[" << std::hex << address << "," << getEnd() << "," << getMax() << "," << std::dec << count << "," << diff << "]";
		ss << "[" << address << "," << getEnd() << "," << getMax() << "," << std::dec << count << "," << diff << "]";
//		for(int i = 0; i < count + 1; i++)
//			ss << address + (diff * i) << ",";
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
public:
	Interval *root;

	Interval *insertNode(Interval *tmp, const Access &item) {
		size_t end;

		if (tmp == NULL) {
			tmp = new Interval(item);
			return tmp;
		}

		if (item.getAddress() > tmp->getMax()) {
			tmp->setMax(item.getAddress());
		}

		if((item.getAccessSizeType() == tmp->size_type) && (item.getPC() == tmp->getPC())) {
			if(tmp->diff != 0) {
				end = tmp->getEnd();
				if(item.getAddress() == (end + tmp->diff)) {
					tmp->count++;
					if(tmp->getEnd() > tmp->max)
						tmp->max = tmp->getEnd();
					return tmp;
				}
				if((item.getAddress() >= tmp->address) && (item.getAddress() <= end))
					return tmp;
				if(item.getAddress() == (tmp->address - tmp->diff)) {
					tmp->address = item.getAddress();
					tmp->count++;
					if(tmp->getEnd() > tmp->max)
						tmp->max = tmp->getEnd();
					return tmp;
				}
			} else {
				tmp->diff = item.getAddress() - tmp->address;
				if(tmp->diff != 0) {
					end = tmp->getEnd();
					if(item.getAddress() == (end + tmp->diff)) {
						tmp->count++;
						if(tmp->getEnd() > tmp->max)
							tmp->max = tmp->getEnd();
						return tmp;
					}
					if((item.getAddress() >= tmp->address) && (item.getAddress() <= end))
						return tmp;
					if(item.getAddress() == (tmp->address - tmp->diff)) {
						tmp->address = item.getAddress();
						tmp->count++;
						if(tmp->getEnd() > tmp->max)
							tmp->max = tmp->getEnd();
						return tmp;
					}
				} else {
					return tmp;
				}
			}
		}

		if (tmp->compareTo(item) <= 0) {
			if (tmp->getRight() == NULL) {
				tmp->setRight(new Interval(item));
			} else {
				insertNode(tmp->getRight(), item);
			}
		} else {
			if (tmp->getLeft() == NULL) {
				tmp->setLeft(new Interval(item));
			} else {
				insertNode(tmp->getLeft(), item);
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

	void intersectInterval(Interval *tmp, Interval *i, std::vector<std::pair<Interval,Interval>> &res) {

		if (tmp == NULL) {
			return;
		}

		// std::cout << "Compare: " << tmp->tostring() << " to " << i->tostring() << std::endl;
	    // std::cout << "Compare: " << i->tostring() << std::endl;

		if (!((tmp->getAddress() > i->getEnd()) || (tmp->getEnd() < i->getAddress()))) {
			res.emplace_back(*tmp, *i);
		}

		if ((tmp->getLeft() != NULL) && (tmp->getLeft()->getMax() >= i->getAddress())) {
			intersectInterval(tmp->getLeft(), i, res);
		}

		intersectInterval(tmp->getRight(), i, res);
	}

	void intersectIntervals(Interval *tree1, Interval *tree2, std::vector<std::pair<Interval,Interval>> &res) {

		if (tree1 == NULL || tree2 == NULL) {
			return;
		}

		// Search current interval of tree2 in tree1
		//std::cout << "To check: " << tree2->tostring() << std::endl;
		intersectInterval(tree1, tree2, res);

		// Call recursively on left and right tree
		if (tree2->getLeft() != NULL) {
			intersectIntervals(tree1, tree2->getLeft(), res);
		}

		if (tree2->getRight() != NULL) {
			intersectIntervals(tree1, tree2->getRight(), res);
		}
	}

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
};
