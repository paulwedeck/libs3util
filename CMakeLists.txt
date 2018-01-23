cmake_minimum_required(VERSION 3.2.2)
project(libs3util)

include(CPack)
include(TestBigEndian)

set(CMAKE_C_FLAGS "-Wall -Wno-pointer-sign")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")

TEST_BIG_ENDIAN(IS_BE)

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

include_directories(src ${PROJECT_BINARY_DIR})

#s3util
file(GLOB_RECURSE s3util_SOURCES "src/*.[ch]")

add_library(s3util SHARED ${s3util_SOURCES})

install(TARGETS s3util DESTINATION lib)
install(FILES src/s3util_ext.h src/s3util.h DESTINATION include/s3util)

