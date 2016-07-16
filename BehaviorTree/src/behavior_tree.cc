#include "global.h"
#include "node_manager.h"
#include "root.h"
#include <vector>

static NodeManager &node_manager = NodeManager::Instance();

static PyObject *AddNode(PyObject *self, PyObject *args, PyObject *keywds) {
	int id, index;
	PyObject *children = NULL, *function = NULL;
	static char *kwlist[] = { "id", "index", "children", "function", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, keywds, "ii|OO", kwlist, &id, &index, &children, &function))
		return NULL;

	if (children && !PyList_Check(children)) {
		PyErr_SetString(PyExc_TypeError, "The argument children must be a list");
		return NULL;
	}

	if (function && !PyCallable_Check(function)) {
		PyErr_SetString(PyExc_TypeError, "The argument function must be callable");
		return NULL;
	}
	
	if (!children && !function) {
		PyErr_SetString(PyExc_TypeError, "Must pass children or function");
		return NULL;
	}

	std::vector<int> children_ids;
	for (Py_ssize_t i = 0; children && i < PyList_Size(children); ++i) {
		PyObject *item = PyList_GetItem(children, i);
		int children_id = PyInt_AsLong(item);
		if (PyErr_Occurred()) {
			PyErr_SetString(PyExc_RuntimeError, "The element of children must be a integer");
			return NULL;
		}
		children_ids.push_back(children_id);
	}

	node_manager.AddNode(id, index, children_ids, function);

	auto *nodes = node_manager.nodes();
	if (nodes->find(id) == nodes->end()) Py_RETURN_FALSE;
	else Py_RETURN_TRUE;
}

static PyMethodDef behavior_tree_methods[] = {
	{"add_node", (PyCFunction)AddNode, METH_VARARGS | METH_KEYWORDS, "add_node(id, index, children, function)"},
	{NULL, NULL, 0, NULL},
};

PyMODINIT_FUNC
initbehavior_tree() {
	if (PyType_Ready(&RootType) < 0) return;
	
	PyObject *module = Py_InitModule("behavior_tree", behavior_tree_methods);
	if (module == NULL) return;

	Py_INCREF(&RootType);
	PyModule_AddObject(module, "Root", (PyObject *)&RootType);
}