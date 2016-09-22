project(behavior_tree)
cmake_minimum_required(VERSION 2.8)

find_package(PythonLibs)

file(GLOB HEADER_FILES BehaviorTree/include/*)
file(GLOB SOURCE_FILES BehaviorTree/src/*)

include_directories(
	BehaviorTree/include
	"${PYTHON_INCLUDE_DIRS}"
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".pyd")
	target_link_libraries(${PROJECT_NAME} "${PYTHON_LIBRARIES}")
else(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".so")
	add_definitions(-Wno-write-strings -std=c++11)
	execute_process(COMMAND python-config --cflags OUTPUT_VARIABLE PYTHON_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND python-config --ldflags OUTPUT_VARIABLE PYTHON_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PYTHON_CFLAGS}")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${PYTHON_LDFLAGS}")
endif(MSVC)

