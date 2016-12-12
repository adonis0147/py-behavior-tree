#pragma once
#ifndef PYROOT_H
#define PYROOT_H

#include "root.h"
#include "structmember.h"
#include "node_manager.h"

typedef struct {
	PyObject_HEAD
	bool can_tick;
	int tick_result;
	Root *root;
} PyRoot;

static void RootDealloc(PyRoot *self) {
	self->can_tick = false;
	self->tick_result = 0;
	delete self->root;
	self->root = NULL;
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *RootNew(PyTypeObject *type, PyObject *args, PyObject *kwds) {
	PyRoot *self = (PyRoot *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->can_tick = false;
		self->tick_result = 0;
		self->root = new Root();
	}
	return (PyObject *)self;
}

static int RootInit(PyRoot *self, PyObject *args, PyObject *kwds) {
	static char *kwlist[] = {"node_id", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &self->root->node_id)) return -1;
	auto &node_manager = NodeManager::Instance();
	auto *nodes = node_manager.nodes();
	if (nodes->find(self->root->node_id) != nodes->end()) {
		self->root->node = nodes->at(self->root->node_id);
		self->can_tick = true;
	}
	return 0;
}

static PyObject *RootTick(PyRoot *self, PyObject *args) {
	if (self->can_tick) {
		Root *root = self->root;
		self->tick_result = (root->node->*(root->node->Tick))(args, root->tree_data);
	}
	else self->tick_result = 0;
	Py_RETURN_NONE;
}

static PyMethodDef root_methods[] = {
	{"tick", (PyCFunction)RootTick, METH_VARARGS, "tick root"},
	{NULL, NULL, 0, NULL},
};

static PyObject *RootGetNodeId(PyRoot *self, void *closure) {
	return PyInt_FromLong(self->root->node_id);
}

static int RootSetNodeId(PyRoot *self, PyObject *value, void *closure) {
	int node_id = PyInt_AsLong(value);
	if (PyErr_Occurred()) return -1;

	self->root->node_id = node_id;

	auto &node_manager = NodeManager::Instance();
	auto *nodes = node_manager.nodes();
	if (nodes->find(self->root->node_id) != nodes->end()) {
		self->root->node = nodes->at(self->root->node_id);
		self->can_tick = true;
		self->tick_result = 0;
	}
	else {
		self->root->node = NULL;
		self->can_tick = false;
		self->tick_result = 0;
	}
	return 0;
}

static PyObject *RootGetCanTick(PyRoot *self, void *closure) {
	return PyBool_FromLong(self->can_tick);
}

static PyObject *RootGetTickResult(PyRoot *self, void *closure) {
	return PyInt_FromLong(self->tick_result);
}

static PyObject *RootGetDebug(PyRoot *self, void *closure) {
	return PyBool_FromLong(self->root->debug);
}

static int RootSetDebug(PyRoot *self, PyObject *value, void *closure) {
	int debug = PyObject_IsTrue(value);
	if (debug < 0) return -1;
	self->root->debug = (debug != 0);
	return 0;
}

static PyGetSetDef root_getseters[] = {
	{"node_id", (getter)RootGetNodeId, (setter)RootSetNodeId, "node id", NULL},
	{"can_tick", (getter)RootGetCanTick, NULL, "can tick", NULL},
	{"tick_result", (getter)RootGetTickResult, NULL, "tick result", NULL},
	{"debug", (getter)RootGetDebug, (setter)RootSetDebug, "debug", NULL},
	{NULL },
};

static PyTypeObject RootType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"behavior_tree.Root",      /*tp_name*/
	sizeof(PyRoot),            /*tp_basicsize*/
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

#endif // !PYROOT_H
