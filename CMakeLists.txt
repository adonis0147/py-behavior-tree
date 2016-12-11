cmake_minimum_required(VERSION 2.8)
project(behavior_tree)

find_package(PythonLibs)

set(PROJECT_PATH ./BehaviorTree)
file(GLOB HEADER_FILES "${PROJECT_PATH}/include/*.h")
file(GLOB SOURCE_FILES "${PROJECT_PATH}/src/*.cc")

include_directories(
	"${PROJECT_PATH}/include"
	"${PYTHON_INCLUDE_DIRS}"
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

if(DEBUG_BUILD)
	add_definitions(-DTRACE_TICK)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_d")
else(DEBUG_BUILD)
	set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
endif(DEBUG_BUILD)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".pyd")
	target_link_libraries(${PROJECT_NAME} "${PYTHON_LIBRARIES}")
else(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".so")
	add_definitions(--std=c++11 -Wno-write-strings)
	execute_process(COMMAND python-config --cflags OUTPUT_VARIABLE PYTHON_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND python-config --ldflags OUTPUT_VARIABLE PYTHON_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PYTHON_CFLAGS}")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${PYTHON_LDFLAGS}")
endif(MSVC)

