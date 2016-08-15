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
	
private:
	NodeManager() {
		InitFunctions();
	}
	Node *CreateNode(int id, size_t index, std::vector<int> children_ids, PyObject *function);
	bool IsNodeDataValid(size_t index, std::vector<int> children_ids, PyObject *function);
	void InitFunctions();

private:
	std::unordered_map<int, Node *> nodes_;
	std::vector<Function> functions_;
};

inline void NodeManager::AddNode(int id, size_t index, std::vector<int> children_ids, PyObject *function) {
	if (std::find(children_ids.begin(), children_ids.end(), id) != children_ids.end())
		return;

	Node *node = CreateNode(id, index, children_ids, function);
	if (!node) return;

	auto pointer = nodes_.find(id);
	if (pointer != nodes_.end()) {
		*nodes_[id] = *node;
		delete node;
	} else nodes_[id] = node;
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
