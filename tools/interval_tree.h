#include "rtl/sword_common.h"
#include <cstdio>
#include <iostream>
#include <list>
#include <string>
#include <vector>

class Interval {
	friend class IntervalTree;
private:
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
		ss << "[" << address << "," << count << "]" << std::endl;
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
					return tmp;
				}
				if((item.getAddress() >= tmp->address) && (item.getAddress() <= end))
					return tmp;
				if(item.getAddress() == (tmp->address - tmp->diff)) {
					tmp->address = item.getAddress();
					tmp->count++;
					return tmp;
				}
			} else {
				tmp->diff = item.getAddress() - tmp->address;
				end = tmp->getEnd();
				if(item.getAddress() == (end + tmp->diff)) {
					tmp->count++;
					return tmp;
				}
				if((item.getAddress() >= tmp->address) && (item.getAddress() <= end))
					return tmp;
				if(item.getAddress() == (tmp->address - tmp->diff)) {
					tmp->address = item.getAddress();
					tmp->count++;
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

	void intersectInterval(Interval *tmp, Interval i, std::vector<Interval> &res) {

		if (tmp == NULL) {
			return;
		}

		std::cout << "Compare: " << tmp << " to " << i.tostring() << std::endl;

		if (!((tmp->getAddress() > i.getEnd()) || (tmp->getEnd() < i.getAddress()))) {
			res.push_back(*tmp);
		}

		if ((tmp->getLeft() != NULL) && (tmp->getLeft()->getMax() >= i.getAddress())) {
			intersectInterval(tmp->getLeft(), i, res);
		}

		intersectInterval(tmp->getRight(), i, res);
	}
};
