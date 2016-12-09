#include "global.h"
#include "behavior_tree.h"

#if defined(_DEBUG) | defined(TRACE_TICK)
#define MODULE_NAME "behavior_tree_d"
#else
#define MODULE_NAME "behavior_tree"
#endif

PyMODINIT_FUNC
#if defined(_DEBUG) | defined(TRACE_TICK)
initbehavior_tree_d() {
	PyErr_Warn(PyExc_RuntimeWarning, "This is the debug build of behavior_tree module!");
#else
initbehavior_tree() {
#endif

	PyObject *module = Py_InitModule(MODULE_NAME, behavior_tree_methods);
	if (module == NULL) return;

	InitModule(module);
}
