#pragma once
#ifndef NODE_H
#define NODE_H

#include "global.h"
#include <unordered_map>

class Node;
typedef std::unordered_map<int, size_t> ChildIndex;
typedef int(Node::*Function)(PyObject *, ChildIndex &);

class Node {
public:
	explicit Node(int id): id_(id), Tick(NULL), children_(NULL), size_(0), function_(NULL) {}
	DISABLE_COPY_AND_ASSIGN(Node);
	~Node() {
		Tick = NULL;
		delete[] children_;
		children_ = NULL;
		size_ = 0;
		Py_XDECREF(function_);
		function_ = NULL;
	}
	Function Tick;
	void SetFunction(PyObject *function);
	void SetChildren(Node **children, size_t size);
	void SetChild(size_t pos, Node *node);
	Node **children() const { return children_; }
	size_t size() const { return size_; }

	// tick methods
	int CallPythonFunction(PyObject *args, ChildIndex &child_index);
	int TickNode(PyObject *args, ChildIndex &child_index);
	int RunUntilSuccess(PyObject *args, ChildIndex &child_index);
	int RunUntilFail(PyObject *args, ChildIndex &child_index);
	int SequenceRun(PyObject *args, ChildIndex &child_index);
	int MemRunUntilSuccess(PyObject *args, ChildIndex &child_index);
	int MemRunUntilFail(PyObject *args, ChildIndex &child_index);
	int MemSequenceRun(PyObject *args, ChildIndex &child_index);
	int ReportSuccess(PyObject *args, ChildIndex &child_index);
	int ReportFailure(PyObject *args, ChildIndex &child_index);
	int RevertStatus(PyObject *args, ChildIndex &child_index);

private:
	int id_;
	Node **children_;
	size_t size_;
	PyObject *function_;
};

inline void Node::SetFunction(PyObject *function) {
	if (!PyCallable_Check(function)) return;
	Py_INCREF(function);
	Py_XDECREF(function_);
	function_ = function;
}

inline void Node::SetChildren(Node **children, size_t size) {
	delete[] children_;
	size_ = size;
	if (!size) {
		children_ = NULL;
		return;
	}

	children_ = new Node *[size];
	for (size_t i = 0; i < size; ++i)
		children_[i] = children[i];
}

inline void Node::SetChild(size_t pos, Node *node) {
	if (pos >= size_) return;
	children_[pos] = node;
}


// tick methods
inline int Node::CallPythonFunction(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PyObject *function_name = PyObject_GetAttrString(function_, "__name__");
	printf("%s %s - behavior_tree - %s : %s\n", __DATE__, __TIME__, __func__, PyString_AsString(function_name));
	Py_DECREF(function_name);
#endif // TRACE_TICK

	PyObject *result = PyObject_CallObject(function_, args);
	if (result == NULL) {

#ifdef _DEBUG
		PyErr_Print();
#endif // _DEBUG

		PyErr_Clear();
		return ERROR;
	}
	int status = PyInt_AsLong(result);

#ifdef _DEBUG
	PyErr_Print();
#endif // _DEBUG

	PyErr_Clear();
	Py_DECREF(result);
	return status;
}

inline int Node::TickNode(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	for (size_t i = 0; i < size_; ++i) {
		return (children_[i]->*(children_[i]->Tick))(args, child_index);
	}
	return ERROR;
}

inline int Node::RunUntilSuccess(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	int status = FAILURE;
	for (size_t i = 0; i < size_; ++i) {
		if ((status = (children_[i]->*(children_[i]->Tick))(args, child_index)) & SUCCESS)
			return status;
	}
	return status;
}

inline int Node::RunUntilFail(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	int status = SUCCESS;
	for (size_t i = 0; i < size_; ++i) {
		if ((status = (children_[i]->*(children_[i]->Tick))(args, child_index)) & FAILURE)
			return status;
	}
	return status;
}

inline int Node::SequenceRun(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	int status = SUCCESS;
	for (size_t i = 0; i < size_; ++i) {
		if ((status = (children_[i]->*(children_[i]->Tick))(args, child_index)) != SUCCESS)
			return status;
	}
	return status;
}

inline int Node::MemRunUntilSuccess(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	int status = FAILURE;
	size_t &index = child_index[id_];
	while (index < size_) {
		status = (children_[index]->*(children_[index]->Tick))(args, child_index);
		if (status & (SUCCESS | RUNNING)) {
			if (status != RUNNING) index = 0;
			return status;
		}
		++index;
	}
	index = 0;
	return status;
}

inline int Node::MemRunUntilFail(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	int status = SUCCESS;
	size_t &index = child_index[id_];
	while (index < size_) {
		status = (children_[index]->*(children_[index]->Tick))(args, child_index);
		if (status & (FAILURE | RUNNING)) {
			if (status != RUNNING) index = 0;
			return status;
		}
		++index;
	}
	index = 0;
	return status;
}

inline int Node::MemSequenceRun(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	int status = SUCCESS;
	size_t &index = child_index[id_];
	while (index < size_) {
		status = (children_[index]->*(children_[index]->Tick))(args, child_index);
		if (status & (FAILURE | RUNNING)) {
			if (status != RUNNING) index = 0;
			return status;
		}
		++index;
	}
	index = 0;
	return status;
}

inline int Node::ReportSuccess(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	for (size_t i = 0; i < size_; ++i) {
		(children_[i]->*(children_[i]->Tick))(args, child_index);
		return SUCCESS;
	}
	return ERROR;
}

inline int Node::ReportFailure(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	for (size_t i = 0; i < size_; ++i) {
		(children_[i]->*(children_[i]->Tick))(args, child_index);
		return FAILURE;
	}
	return ERROR;
}

inline int Node::RevertStatus(PyObject *args, ChildIndex &child_index) {
#ifdef TRACE_TICK
	PRINT_TRACE_INFO;
#endif // TRACE_TICK

	for (size_t i = 0; i < size_; ++i) {
		int status = (children_[i]->*(children_[i]->Tick))(args, child_index);
		if (status & RUNNING) return status;
		else return (status ^ (SUCCESS | FAILURE));
	}
	return ERROR;
}

#endif // !NODE_H