#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {
/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
	struct Node {
		T data;
		Node *left;
		Node *right;
		int npl;  // null path length
		
		Node(const T &d) : data(d), left(nullptr), right(nullptr), npl(0) {}
	};
	
	Node *root;
	size_t count;
	Compare cmp;
	
	// Helper function to get npl of a node
	int getNpl(Node *node) const {
		return node ? node->npl : -1;
	}
	
	// Deep copy a tree
	Node* copyTree(Node *node) {
		if (!node) return nullptr;
		Node *newNode = new Node(node->data);
		newNode->npl = node->npl;
		try {
			newNode->left = copyTree(node->left);
			try {
				newNode->right = copyTree(node->right);
			} catch (...) {
				deleteTree(newNode->left);
				delete newNode;
				throw;
			}
		} catch (...) {
			delete newNode;
			throw;
		}
		return newNode;
	}
	
	// Delete entire tree
	void deleteTree(Node *node) {
		if (!node) return;
		deleteTree(node->left);
		deleteTree(node->right);
		delete node;
	}
	
	// Merge two leftist heaps
	Node* mergeNodes(Node *h1, Node *h2) {
		if (!h1) return h2;
		if (!h2) return h1;
		
		// Make sure h1 has the larger priority
		if (cmp(h1->data, h2->data)) {
			Node *temp = h1;
			h1 = h2;
			h2 = temp;
		}
		
		// Merge h2 with right subtree of h1
		h1->right = mergeNodes(h1->right, h2);
		
		// Maintain leftist property
		if (getNpl(h1->left) < getNpl(h1->right)) {
			Node *temp = h1->left;
			h1->left = h1->right;
			h1->right = temp;
		}
		
		// Update npl
		h1->npl = getNpl(h1->right) + 1;
		
		return h1;
	}
	
public:
	/**
	 * @brief default constructor
	 */
	priority_queue() : root(nullptr), count(0) {}

	/**
	 * @brief copy constructor
	 * @param other the priority_queue to be copied
	 */
	priority_queue(const priority_queue &other) : root(nullptr), count(other.count), cmp(other.cmp) {
		if (other.root) {
			root = copyTree(other.root);
		}
	}

	/**
	 * @brief deconstructor
	 */
	~priority_queue() {
		deleteTree(root);
	}

	/**
	 * @brief Assignment operator
	 * @param other the priority_queue to be assigned from
	 * @return a reference to this priority_queue after assignment
	 */
	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		
		Node *newRoot = nullptr;
		if (other.root) {
			newRoot = copyTree(other.root);
		}
		
		deleteTree(root);
		root = newRoot;
		count = other.count;
		cmp = other.cmp;
		
		return *this;
	}

	/**
	 * @brief get the top element of the priority queue.
	 * @return a reference of the top element.
	 * @throws container_is_empty if empty() returns true
	 */
	const T & top() const {
		if (!root) throw container_is_empty();
		return root->data;
	}

	/**
	 * @brief push new element to the priority queue.
	 * @param e the element to be pushed
	 */
	void push(const T &e) {
		Node *newNode = new Node(e);
		Node *oldRoot = root;
		size_t oldCount = count;
		
		try {
			root = mergeNodes(root, newNode);
			count++;
		} catch (...) {
			// Restore original state
			delete newNode;
			root = oldRoot;
			count = oldCount;
			throw runtime_error();
		}
	}

	/**
	 * @brief delete the top element from the priority queue.
	 * @throws container_is_empty if empty() returns true
	 */
	void pop() {
		if (!root) throw container_is_empty();
		
		Node *oldRoot = root;
		Node *left = root->left;
		Node *right = root->right;
		size_t oldCount = count;
		
		try {
			root = mergeNodes(left, right);
			delete oldRoot;
			count--;
		} catch (...) {
			// Restore original state
			root = oldRoot;
			count = oldCount;
			throw runtime_error();
		}
	}

	/**
	 * @brief return the number of elements in the priority queue.
	 * @return the number of elements.
	 */
	size_t size() const {
		return count;
	}

	/**
	 * @brief check if the container is empty.
	 * @return true if it is empty, false otherwise.
	 */
	bool empty() const {
		return count == 0;
	}

	/**
	 * @brief merge another priority_queue into this one.
	 * The other priority_queue will be cleared after merging.
	 * The complexity is at most O(logn).
	 * @param other the priority_queue to be merged.
	 */
	void merge(priority_queue &other) {
		if (this == &other) return;
		
		Node *oldRoot = root;
		Node *otherOldRoot = other.root;
		size_t oldCount = count;
		size_t otherOldCount = other.count;
		
		try {
			root = mergeNodes(root, other.root);
			count += other.count;
			other.root = nullptr;
			other.count = 0;
		} catch (...) {
			// Restore both queues to original state
			root = oldRoot;
			other.root = otherOldRoot;
			count = oldCount;
			other.count = otherOldCount;
			throw runtime_error();
		}
	}
};

}

#endif