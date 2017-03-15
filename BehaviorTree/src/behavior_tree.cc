#include "behavior_tree.h"
#include "node_manager.h"
#include "pyroot.h"
#include "profile/profiler.h"
#include <sstream>

static PyObject *AddNode(PyObject *self, PyObject *args, PyObject *keywds);
static PyObject *IsProfilerEnable(PyObject *self, PyObject *args);
static PyObject *EnableProfiler(PyObject *self, PyObject *args);
static PyObject *ResetProfiler(PyObject *self, PyObject *args);
static PyObject *DumpProfile(PyObject *self, PyObject *args, PyObject *keywds);
static PyObject *DumpProfileInPyDictObject();
static PyObject *DumpProfileInBinaryFormat();

static PyObject *AddNode(PyObject *self, PyObject *args, PyObject *keywds) {
	int id, index;
	PyObject *children = NULL, *function = NULL;
	static char *kwlist[] = {"id", "index", "children", "function", NULL};

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

	auto &node_manager = NodeManager::Instance();
	node_manager.AddNode(id, index, children_ids, function);

	auto *nodes = node_manager.nodes();
	if (nodes->find(id) == nodes->end()) Py_RETURN_FALSE;
	else Py_RETURN_TRUE;
}

static PyObject *IsProfilerEnable(PyObject *self, PyObject *args) {
	return PyBool_FromLong(Profiler::Instance().enable());
}

static PyObject *EnableProfiler(PyObject *self, PyObject *args) {
#ifndef PROFILE_TICK
	PySys_WriteStderr("This build doesn't support profile. Recompile the source code with PROFILE_TICK flag to enable profile.\n");
	Py_RETURN_NONE;
#endif // !PROFILE_TICK

	int value;
	if (!PyArg_ParseTuple(args, "i", &value)) return NULL;
	Profiler::Instance().SetEnable((value != 0));
	Py_RETURN_NONE;
}

static PyObject *ResetProfiler(PyObject *self, PyObject *args) {
	Profiler::Instance().Reset();
	Py_RETURN_NONE;
}

PyDoc_STRVAR(
	DumpProfile__doc__,
	"DumpProfile(binary=False) -- dump profile data\n\n"
	"binary: False -- dump profile data in python dictionary\n\n"
	"binary: True -- dump profile data in binary fomat\n"
	"format: [total_size][root_id1][collection_size][node_id][profile_data][node_id][profile_data]..."
	"[root_id2][collection_size][node_id][profile_data][node_id][profile_data]..."
);
static PyObject *DumpProfile(PyObject *self, PyObject *args, PyObject *keywds) {
	int binary = 0;
	static char *kwlist[] = { "binary", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|i", kwlist, &binary))
		return NULL;
	if (!binary) return DumpProfileInPyDictObject();
	else return DumpProfileInBinaryFormat();
}

static PyObject *DumpProfileInPyDictObject() {
	PyObject *py_profile = PyDict_New();
	auto &collection = *(Profiler::Instance().collections());
	for (auto &pair : collection) {
		PyObject *py_root_id = Py_BuildValue("i", pair.first);
		PyObject *py_collection = PyDict_New();
		for (auto &inner_pair : pair.second) {
			PyObject *py_node_id = Py_BuildValue("i", inner_pair.first);
			PyObject *py_data = PyDict_New();

			PyObject *py_calls = Py_BuildValue("I", inner_pair.second.calls);
			PyDict_SetItemString(py_data, "calls", py_calls);
			Py_DECREF(py_calls);

			PyObject *py_clocks = Py_BuildValue("k", inner_pair.second.clocks);
			PyDict_SetItemString(py_data, "clocks", py_clocks);
			Py_DECREF(py_clocks);

			PyDict_SetItem(py_collection, py_node_id, py_data);
			Py_DECREF(py_data);
			Py_DECREF(py_node_id);
		}
		PyDict_SetItem(py_profile, py_root_id, py_collection);
		Py_DECREF(py_collection);
		Py_DECREF(py_root_id);
	}
	return py_profile;
}

static PyObject *DumpProfileInBinaryFormat() {
	std::ostringstream out(std::ios::binary | std::ios::out);
	auto &collections = *(Profiler::Instance().collections());
	auto total_size = collections.size();
	out.write(reinterpret_cast<const char *>(&total_size), sizeof(total_size));
	for (auto &pair : collections) {
		auto root_id = pair.first;
		auto &collection = pair.second;
		auto size = collection.size();
		out.write(reinterpret_cast<const char *>(&root_id), sizeof(root_id));
		out.write(reinterpret_cast<char *>(&size), sizeof(size));
		for (auto &inner_pair : collection) {
			auto node_id = inner_pair.first;
			auto profile_data = inner_pair.second;
			out.write(reinterpret_cast<char *>(&node_id), sizeof(node_id));
			out.write(reinterpret_cast<char *>(&profile_data), sizeof(profile_data));
		}
	}
	return PyString_FromStringAndSize(out.str().c_str(), out.str().length());
}

static PyMethodDef behavior_tree_methods[] = {
	{ "add_node", (PyCFunction)AddNode, METH_VARARGS | METH_KEYWORDS, "add_node(id, index, children, function)" },
	{ "is_profiler_enable", IsProfilerEnable, METH_VARARGS, "is_profiler_enable()" },
	{ "enable_profiler", EnableProfiler, METH_VARARGS, "enable_profiler(value)" },
	{ "reset_profiler", ResetProfiler, METH_VARARGS, "reset_profiler()" },
	{ "dump_profile", (PyCFunction)DumpProfile, METH_VARARGS | METH_KEYWORDS, DumpProfile__doc__ },
	{ NULL, NULL, 0, NULL },
};

void InitModule(const char *module_name) {
	if (PyType_Ready(&RootType) < 0) return;

	PyObject *module = Py_InitModule(module_name, behavior_tree_methods);
	if (module == NULL) return;

	Py_INCREF(&RootType);
	PyModule_AddObject(module, "Root", (PyObject *)&RootType);

	// tick functions index
	PyObject *index = PyDict_New();
	const char *keys[] = {
		"tick_leaf",
		"tick_node",
		"run_until_success",
		"run_until_fail",
		"mem_run_until_success",
		"mem_run_until_fail",
		"report_success",
		"report_failure",
		"revert_status",
	};

	size_t size = sizeof(keys) / sizeof(const char *);
	for (size_t i = 0; i < size; ++i) {
		PyObject *key = PyString_FromString(keys[i]);
		PyObject *value = PyInt_FromLong(i);
		PyDict_SetItem(index, key, value);
		Py_DECREF(key);
		Py_DECREF(value);
	}
	PyModule_AddObject(module, "FUNCTIONS_INDEX", index);
}
