#pragma once
#ifndef ROOT_H
#define ROOT_H

#include "global.h"
#include "structmember.h"
#include "node_manager.h"

typedef struct {
	PyObject_HEAD
	int node_id;
	Node *node;
	bool can_tick;
	int tick_result;
	ChildIndex *child_index;
} Root;

static void RootDealloc(Root *self) {
	self->node_id = 0;
	self->node = NULL;
	self->can_tick = false;
	self->tick_result = 0;
	delete self->child_index;
	self->child_index = NULL;
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *RootNew(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	Root *self = (Root *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->node_id = 0;
		self->node = NULL;
		self->can_tick = false;
		self->tick_result = 0;
		self->child_index = new ChildIndex();
	}
	return (PyObject *)self;
}

static int RootInit(Root *self, PyObject *args, PyObject *kwds) {
	static char *kwlist[] = { "node_id", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &self->node_id)) return -1;
	auto &node_manager = NodeManager::Instance();
	auto *nodes = node_manager.nodes();
	auto pointer = nodes->find(self->node_id);
	if (pointer != nodes->end()) {
		self->node = pointer->second;
		self->can_tick = true;
	}
	return 0;
}

static PyObject *RootTick(Root *self, PyObject *args) {
	if (self->can_tick) {
		self->tick_result = (self->node->*(self->node->Tick))(args, *self->child_index);
	}
	else self->tick_result = 0;
	Py_RETURN_NONE;
}

static PyMethodDef root_methods[] = {
	{"tick", (PyCFunction)RootTick, METH_VARARGS, "tick root"},
	{NULL, NULL, 0, NULL}
};

static PyObject *RootGetNodeId(Root *self, void *closure) {
	return PyInt_FromLong(self->node_id);
}

static int RootSetNodeId(Root *self, PyObject *value, void *closure) {
	int node_id = PyInt_AsLong(value);
	if (PyErr_Occurred()) return -1;

	auto &node_manager = NodeManager::Instance();
	self->node_id = node_id;
	auto *nodes = node_manager.nodes();
	auto pointer = nodes->find(self->node_id);
	if (pointer != nodes->end()) {
		self->node = pointer->second;
		self->can_tick = true;
	}
	else {
		self->node = NULL;
		self->can_tick = false;
	}
	return 0;
}

static PyObject *RootGetTickResult(Root *self, void *closure) {
	return PyInt_FromLong(self->tick_result);
}

static PyObject *RootGetCanTick(Root *self, void *closure) {
	return PyBool_FromLong(self->can_tick);
}

static PyGetSetDef root_getseters[] = {
	{"node_id", (getter)RootGetNodeId, (setter)RootSetNodeId, "node id", NULL},
	{"can_tick", (getter)RootGetCanTick, NULL, "can tick", NULL},
	{"tick_result", (getter)RootGetTickResult, NULL, "tick result", NULL},
	{NULL},
};

static PyTypeObject RootType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"behavior_tree.Root",      /*tp_name*/
	sizeof(Root),              /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)RootDealloc,   /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	"Root objects",            /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	root_methods,              /* tp_methods */
	0,                         /* tp_members */
	root_getseters,            /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)RootInit,        /* tp_init */
	0,                         /* tp_alloc */
	RootNew,                   /* tp_new */
};

#endif // !ROOT_H
