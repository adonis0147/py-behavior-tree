#pragma once
#ifndef NODE_H
#define NODE_H

#include "global.h"
#include "node_data.h"
#include "root.h"
#include "stddef.h"

#define container_of(ptr, type, member) \
	( (type *)((char *)ptr - offsetof(type, member)) )

#define SHOULD_PRINT_TRACE_INFO (container_of(&tree_data, Root, tree_data)->debug)

#define PRINT_TRACE_INFO(func, format, info) \
	do { \
		char timestamp[64]; \
		get_timestamp(timestamp, sizeof(timestamp)); \
		char buffer[256]; \
		snprintf(buffer, sizeof(buffer), " - behavior_tree - %s : " format, __func__, info); \
		func("%s%s", timestamp, buffer); \
	} while(0)

#define PRINT_SIMPLE_TRACE_INFO \
	do { \
		if (SHOULD_PRINT_TRACE_INFO) \
			PRINT_TRACE_INFO(PySys_WriteStdout, "node %d\n", id_); \
	} while (0)

class Node {
public:
	typedef int(Node::*Function)(PyObject *args, TreeData *&tree_data);
	Function Tick;

	explicit Node(int id) : Tick(NULL), id_(id), children_(NULL), size_(0), function_(NULL) {}
	Node(const Node &node) :
			Tick(node.Tick),
			id_(node.id_),
			children_(new Node *[node.size_]),
			size_(node.size_),
			function_(node.function_) {
		memcpy(children_, node.children_, sizeof(Node *) * node.size_);
		Py_XINCREF(function_);
	}
	~Node() {
		Tick = NULL;
		id_ = 0;
		delete[] children_;
		children_ = NULL;
		size_ = 0;
		Py_XDECREF(function_);
		function_ = NULL;
	}
	Node &operator =(const Node &node) {
		Tick = node.Tick;
		id_ = node.id_;

		delete[] children_;
		children_ = new Node *[node.size_];
		memcpy(children_, node.children_, sizeof(Node *) * node.size_);
		size_ = node.size_;

		Py_XDECREF(function_);
		function_ = node.function_;
		Py_XINCREF(function_);

		return *this;
	}
	Node **children() { return children_; }
	void SetChildren(Node **children, size_t size);
	size_t size() { return size_; }
	void SetFunction(PyObject *function);

	// tick methods
	// common methods
	int CallPythonFunction(PyObject *args, TreeData *&tree_data);
	int TickNode(PyObject *args, TreeData *&tree_data);
	// composite node methods
	int RunUntilSuccess(PyObject *args, TreeData *&tree_data);
	int RunUntilFail(PyObject *args, TreeData *&tree_data);
	int MemRunUntilSuccess(PyObject *args, TreeData *&tree_data);
	int MemRunUntilFail(PyObject *args, TreeData *&tree_data);
	// decorator node methods
	int ReportSuccess(PyObject *args, TreeData *&tree_data);
	int ReportFailure(PyObject *args, TreeData *&tree_data);
	int RevertStatus(PyObject *args, TreeData *&tree_data);

private:
	int id_;
	Node **children_;
	size_t size_;
	PyObject *function_;
};

#define TICK_CHILDREN(i) ( (children_[i]->*(children_[i]->Tick))(args, tree_data) )

inline void Node::SetChildren(Node **children, size_t size) {
	delete[] children_;
	children_ = new Node *[size];
	memcpy(children_, children, sizeof(Node *) * size);
	size_ = size;
}

inline void Node::SetFunction(PyObject *function) {
	Py_XDECREF(function_);
	function_ = function;
	Py_XINCREF(function_);
}

inline int Node::CallPythonFunction(PyObject *args, TreeData *&tree_data) {
#ifdef TRACE_TICK
	PyObject *function_name = PyObject_GetAttrString(function_, "__name__");
	if (SHOULD_PRINT_TRACE_INFO)
		PRINT_TRACE_INFO(PySys_WriteStdout, "%s\n", PyString_AsString(function_name));
	Py_DECREF(function_name);
#endif // TRACE_TICK

	PyObject *result = PyObject_CallObject(function_, args);
	if (result == NULL) {

#if defined(_DEBUG) | defined(TRACE_TICK)
		PyErr_Print();
#endif

		PyErr_Clear();
		return ERROR;
	}
	int status = PyInt_AsLong(result);

#if defined(_DEBUG) | defined(TRACE_TICK)
	if (PyErr_Occurred()) {
		PyObject *function_name = PyObject_GetAttrString(function_, "__name__");
		PRINT_TRACE_INFO(PySys_WriteStderr, "%s - ", PyString_AsString(function_name));
		PyErr_Print();
		Py_DECREF(function_name);
	}
#endif

	PyErr_Clear();
	Py_DECREF(result);
	return status;
}

inline int Node::TickNode(PyObject *args, TreeData *&tree_data) {
#ifdef TRACE_TICK
	PRINT_SIMPLE_TRACE_INFO;
#endif // TRACE_TICK

	if (size_ > 0) {
		return TICK_CHILDREN(0);
	}
	return ERROR;
}

inline int Node::RunUntilSuccess(PyObject *args, TreeData *&tree_data) {
#ifdef TRACE_TICK
	PRINT_SIMPLE_TRACE_INFO;
#endif // TRACE_TICK

	int status = FAILURE;
	for (size_t i = 0; i < size_; ++i) {
		if ((status = TICK_CHILDREN(i)) & SUCCESS)
			return status;
	}
	return status;
}

inline int Node::RunUntilFail(PyObject *args, TreeData *&tree_data) {
#ifdef TRACE_TICK
	PRINT_SIMPLE_TRACE_INFO;
#endif // TRACE_TICK

	int status = SUCCESS;
	for (size_t i = 0; i < size_; ++i) {
		if ((status = TICK_CHILDREN(i)) & FAILURE)
			return status;
	}
	return status;
}

inline int Node::MemRunUntilSuccess(PyObject *args, TreeData *&tree_data) {
#ifdef TRACE_TICK
	PRINT_SIMPLE_TRACE_INFO;
#endif // TRACE_TICK

	int status = FAILURE;
	size_t &index = (*tree_data)[id_].child_index;
	while (index < size_) {
		status = TICK_CHILDREN(index);
		if (status & (SUCCESS | RUNNING)) {
			if (status != RUNNING) index = 0;
			return status;
		}
		++index;
	}
	index = 0;
	return status;
}

inline int Node::MemRunUntilFail(PyObject *args, TreeData *&tree_data) {
#ifdef TRACE_TICK
	PRINT_SIMPLE_TRACE_INFO;
#endif // TRACE_TICK

	int status = SUCCESS;
	size_t &index = (*tree_data)[id_].child_index;
	while (index < size_) {
		status = TICK_CHILDREN(index);
		if (status & (FAILURE | RUNNING)) {
			if (status != RUNNING) index = 0;
			return status;
		}
		++index;
	}
	index = 0;
	return status;
}

inline int Node::ReportSuccess(PyObject *args, TreeData *&tree_data) {
#ifdef TRACE_TICK
	PRINT_SIMPLE_TRACE_INFO;
#endif // TRACE_TICK

	if (size_ > 0) {
		TICK_CHILDREN(0);
		return SUCCESS;
	}
	return ERROR;
}

inline int Node::ReportFailure(PyObject *args, TreeData *&tree_data) {
#ifdef TRACE_TICK
	PRINT_SIMPLE_TRACE_INFO;
#endif // TRACE_TICK

	if (size_ > 0) {
		TICK_CHILDREN(0);
		return FAILURE;
	}
	return ERROR;
}

inline int Node::RevertStatus(PyObject *args, TreeData *&tree_data) {
#ifdef TRACE_TICK
	PRINT_SIMPLE_TRACE_INFO;
#endif // TRACE_TICK

	if (size_ > 0) {
		int status = TICK_CHILDREN(0);
		if (status & RUNNING) return status;
		else return (status ^ (SUCCESS | FAILURE));
	}
	return ERROR;
}

#endif // !NODE_H
