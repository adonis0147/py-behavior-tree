#!/usr/bin/env python
# -*- coding: utf-8 -*-

from distutils.core import setup, Extension

module = Extension(
    'behavior_tree',
    sources=[
        './BehaviorTree/src/global.cc',
        './BehaviorTree/src/behavior_tree.cc',
        './BehaviorTree/src/main.cc',
    ],
    extra_compile_args=[
        '--std=c++11',
        '-I./BehaviorTree/include',
        '-Wno-write-strings',
        '-Wno-error=format-security',
        '-Wno-format-security',
    ]
)

setup(name='behavior_tree', ext_modules=[module])

