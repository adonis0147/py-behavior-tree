#pragma once
#ifndef BEHAVIOR_TREE_H
#define BEHAVIOR_TREE_H

#include "global.h"

PyObject *AddNode(PyObject *self, PyObject *args, PyObject *keywds);

static PyMethodDef behavior_tree_methods[] = {
	{"add_node", (PyCFunction)AddNode, METH_VARARGS | METH_KEYWORDS, "add_node(id, index, children, function)"},
	{NULL, NULL, 0, NULL},
};

void InitModule(PyObject *module);

#endif // !BEHAVIOR_TREE_H
