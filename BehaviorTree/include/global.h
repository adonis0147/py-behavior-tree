#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef _DEBUG
#undef _DEBUG
#define _DEBUG_WAS_DEFINED
#endif // _DEBUG

#include "Python.h"
#include <ctime>

#define ERROR   -1
#define SUCCESS 0x1
#define FAILURE 0x2
#define RUNNING 0x4

#define DISABLE_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName &) = delete; \
TypeName &operator=(const TypeName &) = delete

#define PRINT_TRACE_INFO print_timestamp(); printf(" - behavior_tree - %s : node %d\n", __func__, id_)

void print_timestamp() {
	time_t timestamp = time(NULL);
	struct tm *time_info = localtime(&timestamp);
	char buffer[80] = {};
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);
	printf("%s", buffer);
}

#ifdef _DEBUG_WAS_DEFINED
#define _DEBUG
#endif // _DEBUG_WAS_DEFINED

#endif // !GLOBAL_H