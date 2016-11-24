#INCLUDE(/path/to/another/CMakeLists.txt)
cmake_minimum_required(VERSION 2.5)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_FLAGS "-Wall -O2")
set(CMAKE_C_FLAGS_DEBUG "-g3 -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall")

project(libmco)

set(LIBNAME mco)

include_directories(
	.
)

set(SRC_LIST 
	mco.c
	htimer.c
	mpoll.c
	fd.c
	net.c
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	add_definitions(-D_XOPEN_SOURCE)
endif()

add_library(${LIBNAME} STATIC
	${SRC_LIST}
)

message("== <CMAKE_SYSTEM_NAME> " ${CMAKE_SYSTEM_NAME})
foreach(item ${SRC_LIST})
	MESSAGE("--<SOURCE> " ${item})
endforeach(item)

