# py-behavior-tree [![travis-ci status](https://travis-ci.org/adonis0147/py-behavior-tree.svg?branch=master)](https://travis-ci.org/adonis0147/py-behavior-tree)
A Python extension with C++ of Behavior Tree

## Prerequisite
  - Python v2.7+
  - cmake v3.0+

## Build
```
git clone https://github.com/adonis0147/py-behavior-tree
mkdir build
cd build
cmake ..
make
```

## Quick Start

### Tick Functions
The module implements several tick functions for a node in behavior tree.
  
  - tick_leaf: CallPythonFunction
  - tick_node: TickNode
  - run_until_success: RunUntilSuccess
  - run_until_fail: RunUntilFail
  - mem_run_until_success: MemRunUntilSuccess
  - mem_run_until_fail: MemRunUntilFail
  - report_success: ReportSuccess
  - report_failure: ReportFailure
  - revert_status: RevertStatus

More functions can be found in `behavior_tree.FUNCTIONS_INDEX`.

### Add Nodes
You can call the `behavior_tree.add_node` method to add a node. However, there is a subtle difference between adding leaf nodes and non-leaf nodes.

For a leaf node which is used to call Python function, you should pass Python function to the method:
``` Python
import behavior_tree

def foo():
  print 'Hello, world!'
  return behavior_tree.SUCCESS
  
behavior_tree.add_node(1, behavior_tree.FUNCTIONS_INDEX['tick_leaf'], function=foo)
```

For a non-leaf node, you should pass tick function and children nodes to the method:
``` Python
behavior_tree.add_node(2, behavior_tree.FUNCTIONS_INDEX['tick_node'], children=[1])
```

### Tick A Tree
  1. create the root of the tree
``` Python
  root = behavior_tree.Root(2)
```
  2. tick the root
``` Python
  root.tick()
```
  3. the followings are the result in terminal
``` bash
  Hello, world!
```

## About Hotfix
Nodes are identified by `id` and you can change the tick function, the children nodes and the Python function of a node by calling the `behavior_tree.add_node`.
``` Python
def bar():
  print 'Hello, hotfix!'
  return behavior_tree.SUCCESS

behavior_tree.add_node(1, behavior_tree.FUNCTIONS_INDEX['tick_leaf'], function=bar)

root.tick()
```
The result will be changed to `Hello, hotfix!`.

The nodes increment by `id` and will not be freed until the process terminates. Therefore, if you change the children nodes of a node, the memory of the old useless children nodes will not be released.
``` Python
def baz():
  print 'Hello, new child!'
  return behavior_tree.SUCCESS

behavior_tree.add_node(3, behavior_tree.FUNCTIONS_INDEX['tick_leaf'], function=baz)

behavior_tree.add_node(2, behavior_tree.FUNCTIONS_INDEX['tick_node'], children=[3])
```
`Node 1`, `Node 2`, and `Node 3` are all in the memory.

## Related Project
  - [profile-viewer](https://github.com/adonis0147/profile-viewer) - Profile viewer for behavior tree
