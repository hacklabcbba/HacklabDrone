cmake_minimum_required(VERSION 2.6)
project(HLDrone) 

set(BASE_DIR ../../)
set(CMAKE_C_FLAGS "-Wall -g -std=gnu99")

include(${BASE_DIR}/libs/rules.cmake)

include_directories(../)

add_executable(crcTest ../crcTest.c)
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
	include_directories( ${ZLIB_INCLUDE_DIRS})
	target_link_libraries( crcTest crc32 ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

find_library(LTTNG_UST_LIBRARY REQUIRED)
if (LTTNG_UST_FOUND)
	include_directories(${LTTNG_UST_INCLUDE_DIRS})
endif(LTTNG_UST_FOUND)

add_library(debugHelper STATIC ../debugHelper.c)
target_link_libraries(debugHelper slink iscom)

add_library(tpoint STATIC ../tpoint.c)
target_link_libraries(tpoint)

add_executable(demo ../udpdron.c)
target_link_libraries(demo sbuffer iscom slink debugHelper tpoint lttng-ust dl)

add_executable(serialBridge ../serialBridge.c)
target_link_libraries(serialBridge slink)

