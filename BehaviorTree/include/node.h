#pragma once
#ifndef NODE_H
#define NODE_H

#include "global.h"

class Node;
typedef int(Node::*Function)(PyObject *);

class Node {
public:
	Node(): Tick(NULL), children_(NULL), size_(0), index_(0), function_(NULL) {}
	DISABLE_COPY_AND_ASSIGN(Node);
	~Node() {
		Tick = NULL;
		delete[] children_;
		children_ = NULL;
		size_ = index_ = 0;
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
	int CallPythonFunction(PyObject *args);
	int TickNode(PyObject *args);
	int RunUntilSuccess(PyObject *args);
	int RunUntilFail(PyObject *args);
	int SequenceRun(PyObject *args);
	int MemRunUntilSuccess(PyObject *args);
	int MemRunUntilFail(PyObject *args);
	int MemSequenceRun(PyObject *args);
	int ReportSuccess(PyObject *args);
	int ReportFailure(PyObject *args);
	int RevertStatus(PyObject *args);

private:
	Node **children_;
	size_t size_;
	size_t index_;
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
	index_ = 0;
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
inline int Node::CallPythonFunction(PyObject *args) {
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

inline int Node::TickNode(PyObject *args) {
	for (size_t i = 0; i < size_; ++i) {
		return (children_[i]->*(children_[i]->Tick))(args);
	}
	return ERROR;
}

inline int Node::RunUntilSuccess(PyObject *args) {
	int status = FAILURE;
	for (size_t i = 0; i < size_; ++i) {
		if ((status = (children_[i]->*(children_[i]->Tick))(args)) & SUCCESS)
			return status;
	}
	return status;
}

inline int Node::RunUntilFail(PyObject *args) {
	int status = SUCCESS;
	for (size_t i = 0; i < size_; ++i) {
		if ((status = (children_[i]->*(children_[i]->Tick))(args)) & FAILURE)
			return status;
	}
	return status;
}

inline int Node::SequenceRun(PyObject *args) {
	int status = SUCCESS;
	for (size_t i = 0; i < size_; ++i) {
		if ((status = (children_[i]->*(children_[i]->Tick))(args)) != SUCCESS)
			return status;
	}
	return status;
}

inline int Node::MemRunUntilSuccess(PyObject *args) {
	int status = FAILURE;
	while (index_ < size_) {
		status = (children_[index_]->*(children_[index_]->Tick))(args);
		if (status & (SUCCESS | RUNNING)) {
			if (status != RUNNING) index_ = 0;
			return status;
		}
		++index_;
	}
	index_ = 0;
	return status;
}

inline int Node::MemRunUntilFail(PyObject *args) {
	int status = SUCCESS;
	while (index_ < size_) {
		status = (children_[index_]->*(children_[index_]->Tick))(args);
		if (status & (FAILURE | RUNNING)) {
			if (status != RUNNING) index_ = 0;
			return status;
		}
		++index_;
	}
	index_ = 0;
	return status;
}

inline int Node::MemSequenceRun(PyObject *args) {
	int status = SUCCESS;
	while (index_ < size_) {
		status = (children_[index_]->*(children_[index_]->Tick))(args);
		if (status & (FAILURE | RUNNING)) {
			if (status != RUNNING) index_ = 0;
			return status;
		}
		++index_;
	}
	index_ = 0;
	return status;
}

inline int Node::ReportSuccess(PyObject *args) {
	for (size_t i = 0; i < size_; ++i) {
		(children_[i]->*(children_[i]->Tick))(args);
		return SUCCESS;
	}
	return ERROR;
}

inline int Node::ReportFailure(PyObject *args) {
	for (size_t i = 0; i < size_; ++i) {
		(children_[i]->*(children_[i]->Tick))(args);
		return FAILURE;
	}
	return ERROR;
}

inline int Node::RevertStatus(PyObject *args) {
	for (size_t i = 0; i < size_; ++i) {
		int status = (children_[i]->*(children_[i]->Tick))(args);
		if (status & RUNNING) return status;
		else return (status ^ (SUCCESS | FAILURE));
	}
	return ERROR;
}

#endif // !NODE_H