include_directories(${BASE_DIR}/libs/inc)
add_library(sbuffer STATIC ${BASE_DIR}/libs/src/sbuffer.c)
add_library(iscom STATIC ${BASE_DIR}/libs/src/iscom.c)
add_library(crc32 STATIC ${BASE_DIR}/libs/src/crc32.c)
add_library(slink STATIC ${BASE_DIR}/libs/src/slink.c)

target_link_libraries(slink crc32)
