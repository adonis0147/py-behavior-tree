#!/usr/bin/env python
# -*- coding: utf-8 -*-

from distutils.core import setup, Extension

module = Extension('behavior_tree',
			sources = ['./BehaviorTree/src/behavior_tree.cc'],
			extra_compile_args = ['--std=c++11', '-I./BehaviorTree/include'])

setup(name = 'behavior_tree', ext_modules = [module])

