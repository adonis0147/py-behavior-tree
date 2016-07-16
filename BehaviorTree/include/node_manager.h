#pragma once
#ifndef NODE_MANAGER_H
#define NODE_MANAGER_H

#include "global.h"
#include "node.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#define INIT_NODES_SIZE 1

class NodeManager {
public:
	DISABLE_COPY_AND_ASSIGN(NodeManager);
	~NodeManager() {
		delete[] nodes_;
		size_ = 0;
		current_index_ = 0;
	}
	static NodeManager &Instance() {
		static NodeManager instance;
		return instance;
	}
	void AddNode(int id, size_t index, const std::vector<int> &child_ids, PyObject *function);
	void AddRootNode(int id, Node **node) { root_nodes_[id].insert(node); }
	void DeleteRootNode(int id, Node **node) { root_nodes_[id].erase(node); }
	Node *GetNodeById(int id) {
		auto pointer = mapping_.find(id);
		if (pointer == mapping_.end()) return NULL;
		return &nodes_[pointer->second];
	}

private:
	NodeManager() : nodes_(new Node[INIT_NODES_SIZE]()), size_(INIT_NODES_SIZE), current_index_(0) {
		InitFunctions();
	}
	void InitFunctions();
	int GetNewNodesNum(int id, const std::vector<int> &child_ids);
	size_t CalNewSize(int new_nodes_num);
	void MapNewNodes(int id, const std::vector<int> &child_ids);
	void Resize(size_t size);
	void UpdateRootNodes();

private:
	Node *nodes_;
	size_t size_;
	size_t current_index_;
	std::unordered_map<int, size_t> mapping_;
	std::vector<Function> functions_;
	std::unordered_map<int, std::unordered_set<Node **>> root_nodes_;
};

inline void NodeManager::AddNode(int id, size_t index, const std::vector<int> &child_ids, PyObject *function) {
	if (index > functions_.size()) return;
	else if(index == 0 && !PyCallable_Check(function)) return;

	int new_nodes_num = GetNewNodesNum(id, child_ids);
	size_t new_size = CalNewSize(new_nodes_num);
	if (new_size != size_) Resize(new_size);

	MapNewNodes(id, child_ids);

	size_t mapped_node_id = mapping_[id];
	nodes_[mapped_node_id].Tick = functions_[index];
	nodes_[mapped_node_id].SetFunction(function);
	size_t children_size = child_ids.size();
	if (children_size) {
		Node **children = new Node *[children_size];
		for (size_t i = 0; i < child_ids.size(); ++i)
			children[i] = &nodes_[mapping_[child_ids[i]]];
		nodes_[mapped_node_id].SetChildren(children, children_size);
		delete[] children;
		children = NULL;
	}
	else nodes_[mapped_node_id].SetChildren(NULL, 0);
}

inline int NodeManager::GetNewNodesNum(int id, const std::vector<int> &child_ids) {
	int new_nodes_num = static_cast<int>(mapping_.find(id) == mapping_.end());
	for (size_t i = 0; i < child_ids.size(); ++i) {
		new_nodes_num += static_cast<int>(mapping_.find(child_ids[i]) == mapping_.end());
	}
	return new_nodes_num;
}

inline size_t NodeManager::CalNewSize(int new_nodes_num) {
	size_t size = size_;
	while (current_index_ + new_nodes_num > size) {
		size <<= 1;
	}
	return size;
}

inline void NodeManager::MapNewNodes(int id, const std::vector<int> &child_ids) {
	if (mapping_.find(id) == mapping_.end()) mapping_[id] = current_index_++;
	for (size_t i = 0; i < child_ids.size(); ++i) {
		if (mapping_.find(child_ids[i]) == mapping_.end())
			mapping_[child_ids[i]] = current_index_++;
	}
}

inline void NodeManager::Resize(size_t size) {
	Node *new_nodes = new Node[size]();
	for (size_t i = 0; i < size_; ++i) {
		new_nodes[i] = nodes_[i];
	}
	for (size_t i = 0; i < current_index_; ++i) {
		Node **children = nodes_[i].children();
		for (size_t j = 0; j < nodes_[i].size(); ++j) {
			int offset = children[j] - &nodes_[i];
			Node **new_node_children = new_nodes[i].children();
			new_node_children[j] = &new_nodes[i] + offset;
		}
	}
	delete[] nodes_;
	nodes_ = new_nodes;
	size_ = size;

	UpdateRootNodes();
}

inline void NodeManager::UpdateRootNodes() {
	for (auto it = root_nodes_.begin(); it != root_nodes_.end(); ++it) {
		int mapped_node_id = mapping_[it->first];
		auto set = it->second;
		for (auto other_it = set.begin(); other_it != set.end(); ++other_it) {
			**other_it = &nodes_[mapped_node_id];
		}
	}
}

inline void NodeManager::InitFunctions() {
	functions_ = {
		&Node::CallPythonFunction,
		&Node::TickNode,
		&Node::RunUntilSuccess,
		&Node::RunUntilFail,
	};
}

#endif // !NODE_MANAGER_H
