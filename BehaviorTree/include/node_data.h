#pragma once
#ifndef NODE_DATA_H
#define NODE_DATA_H

struct NodeData {
	NodeData() : child_index(0) {}

	size_t child_index;
};

#endif // !NODE_DATA_H
