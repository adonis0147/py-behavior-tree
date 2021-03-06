#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef _DEBUG
#undef _DEBUG
#define DEBUG_WAS_DEFINED
#endif // _DEBUG

#include "Python.h"

#define ERROR   -1
#define SUCCESS 0x1
#define FAILURE 0x2
#define RUNNING 0x4

#define DISABLE_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName &) = delete; \
TypeName &operator=(const TypeName &) = delete

void get_timestamp(char *buffer, size_t size);

#ifdef DEBUG_WAS_DEFINED
#define _DEBUG
#endif // DEBUG_WAS_DEFINED

#endif // !GLOBAL_H
