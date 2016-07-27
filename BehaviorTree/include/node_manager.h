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

class NodeManager {
public:
	DISABLE_COPY_AND_ASSIGN(NodeManager);
	~NodeManager() {
		for (auto it = nodes_.begin(); it != nodes_.end(); ++ it) {
			delete it->second;
			it->second = NULL;
		}
		nodes_.clear();
		functions_.clear();
	}
	static NodeManager &Instance() {
		static NodeManager instance;
		return instance;
	}
	const std::unordered_map<int, Node *> *nodes() const { return &nodes_; }

	void AddNode(int id, size_t index, std::vector<int> children_ids, PyObject *function);
	void AddRootNode(int id, Node **node);
	void RemoveRootNode(int id, Node **node);

private:
	NodeManager() {
		InitFunctions();
	}
	Node *CreateNode(int id, size_t index, std::vector<int> children_ids, PyObject *function);
	bool IsNodeDataValid(size_t index, std::vector<int> children_ids, PyObject *function);
	void InitFunctions();
	void UpdateFathers(int id, Node *child, Node *new_child);
	void AddFather(int id, int father_id);

private:
	std::unordered_map<int, Node *> nodes_;
	std::unordered_map<int, std::vector<int>> fathers_;
	std::unordered_map<int, std::unordered_set<Node **>> root_nodes_;
	std::vector<Function> functions_;
};

inline void NodeManager::AddNode(int id, size_t index, std::vector<int> children_ids, PyObject *function) {
	if (std::find(children_ids.begin(), children_ids.end(), id) != children_ids.end())
		return;

	Node *node = CreateNode(id, index, children_ids, function);
	if (!node) return;

	auto pointer = nodes_.find(id);
	if (pointer != nodes_.end()) {
		UpdateFathers(id, pointer->second, node);
		delete nodes_[id];
	}
	nodes_[id] = node;
	for (size_t i = 0; i < children_ids.size(); ++i) {
		AddFather(children_ids[i], id);
	}
	
	if (root_nodes_.find(id) != root_nodes_.end()) {
		for (auto it = root_nodes_[id].begin(); it != root_nodes_[id].end(); ++it)
			**it = node;
	}
}

inline void NodeManager::UpdateFathers(int id, Node *child, Node *new_child) {
	auto pointer = fathers_.find(id);
	if (pointer == fathers_.end()) return;
	std::vector<int> &fathers = pointer->second;
	for (size_t i = 0; i < fathers.size(); ++i) {
		auto node_pointer = nodes_.find(fathers[i]);
		if (node_pointer != nodes_.end()) {
			Node *father = node_pointer->second;
			Node **children = father->children();
			size_t size = father->size();
			for (size_t j = 0; j < size; ++j) {
				if (children[j] == child) father->SetChild(j, new_child);
			}
		}
	}
}

inline void NodeManager::AddFather(int id, int father_id) {
	fathers_[id].push_back(father_id);
}

inline Node *NodeManager::CreateNode(int id, size_t index, std::vector<int> children_ids, PyObject *function) {
	if (!IsNodeDataValid(index, children_ids, function)) return NULL;

	Node *node = new Node(id);
	node->Tick = functions_[index];
	node->SetFunction(function);
	size_t size = children_ids.size();
	if (size) {
		Node **children = new Node *[size];
		for (size_t i = 0; i < size; ++i) {
			children[i] = nodes_[children_ids[i]];
		}
		node->SetChildren(children, size);
		delete[] children;
	}
	else node->SetChildren(NULL, size);
	return node;
}

inline bool NodeManager::IsNodeDataValid(size_t index, std::vector<int> children_ids, PyObject *function) {
	if (index >= functions_.size()) return false;
	if (index == 0 && !PyCallable_Check(function)) return false;
	for (size_t i = 0; i < children_ids.size(); ++i) {
		auto pointer = nodes_.find(children_ids[i]);
		if (pointer == nodes_.end() || !pointer->second) return false;
	}
	return true;
}

inline void NodeManager::AddRootNode(int id, Node **node) {
	root_nodes_[id].insert(node);
}

inline void NodeManager::RemoveRootNode(int id, Node **node) {
	root_nodes_[id].erase(node);
	if (root_nodes_[id].size() == 0) root_nodes_.erase(id);
}

inline void NodeManager::InitFunctions() {
	functions_ = {
		&Node::CallPythonFunction,
		&Node::TickNode,
		&Node::RunUntilSuccess,
		&Node::RunUntilFail,
		&Node::SequenceRun,
		&Node::MemRunUntilSuccess,
		&Node::MemRunUntilFail,
		&Node::MemSequenceRun,
		&Node::ReportSuccess,
		&Node::ReportFailure,
		&Node::RevertStatus,
	};
}

#endif // !NODE_MANAGER_H