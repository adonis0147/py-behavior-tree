#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef _DEBUG
#undef _DEBUG
#define _DEBUG_WAS_DEFINED
#endif // _DEBUG

#include "Python.h"

#define ERROR   -1
#define SUCCESS 0x1
#define FAILURE 0x2
#define RUNNING 0x4

#define DISABLE_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName &) = delete; \
TypeName &operator=(const TypeName &) = delete

#define PRINT_TRACE_INFO printf("%s %s - behavior_tree - %s\n", __DATE__, __TIME__, __func__)

#ifdef _DEBUG_WAS_DEFINED
#define _DEBUG
#endif // _DEBUG_WAS_DEFINED

#endif // !GLOBAL_H