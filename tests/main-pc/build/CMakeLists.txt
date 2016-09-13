cmake_minimum_required(VERSION 2.6)
project(HLDrone) 

# Configurations
set(BASE_DIR ../../)
set(CMAKE_C_FLAGS "-Wall -g -std=gnu99")

include(${BASE_DIR}/libs/rules.cmake)

include_directories(../)

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
endif(ZLIB_FOUND)

find_library(LTTNG_UST_LIBRARY REQUIRED)
if (LTTNG_UST_FOUND)
	include_directories(${LTTNG_UST_INCLUDE_DIRS})
endif(LTTNG_UST_FOUND)

# Libraries
add_library(tpoint STATIC ../tpoint.c)

add_library(debugHelper STATIC ../debugHelper.c)
target_link_libraries(debugHelper slink iscom)

# Executables
add_executable(demo ../udpdron.c ../slink_user.c)
target_link_libraries(demo sbuffer iscom slink debugHelper tpoint lttng-ust dl ${ZLIB_LIBRARIES})

add_executable(serialBridge ../serialBridge.c ../slink_user.c)
target_link_libraries(serialBridge slink ${ZLIB_LIBRARIES})

add_executable(slinkBench ../slinkBench.c ../slink_user.c)
target_link_libraries(slinkBench slink ${ZLIB_LIBRARIES})

add_executable(crcTest ../crcTest.c)
target_link_libraries(crcTest crc32 ${ZLIB_LIBRARIES})

