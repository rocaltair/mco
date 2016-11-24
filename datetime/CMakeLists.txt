#INCLUDE(/path/to/another/CMakeLists.txt)
cmake_minimum_required(VERSION 2.5)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_C_FLAGS "-Wall -O2")
set(CMAKE_C_FLAGS_DEBUG "-g3 -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall")

set(APPNAME datetime)

project(proj_datetime)


link_directories(
	../src
)


include_directories(
	../src
)

set(EXECUTE_FILE ${APPNAME})

set(SRC_LIST 
	main.c
	
)

set(LIB_LIST mco)

add_executable(${EXECUTE_FILE} ${SRC_LIST})
target_link_libraries(${EXECUTE_FILE} 
	${LIB_LIST}
)

message("== <CMAKE_SYSTEM_NAME> " ${CMAKE_SYSTEM_NAME})
foreach(item ${SRC_LIST})
	MESSAGE("--<SOURCE> " ${item})
endforeach(item)

foreach(item ${LIB_LIST})
	MESSAGE("--<LIBRARY> " ${item})
endforeach(item)
