#pragma once
#ifndef ROOT_H
#define ROOT_H

#include "global.h"
#include <unordered_map>

class Node;
struct NodeData;
typedef std::unordered_map<int, NodeData> TreeData;

struct Root {
	Root() : node_id(0), node(NULL), tree_data(new TreeData()), debug(false) {}
	~Root() {
		node_id = 0;
		node = NULL;
		delete tree_data;
		tree_data = NULL;
		debug = false;
	}

	int node_id;
	Node *node;
	TreeData *tree_data;
	bool debug;
};

#endif // !ROOT_H
