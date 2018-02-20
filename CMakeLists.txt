cmake_minimum_required(VERSION 3.2.2)
project(libs3util)

include(CPack)
include(TestBigEndian)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")

option(PRIVATE_FILENAME "stop leaking complete path to file (should be enabled on distribution)" ON)
option(FORCE_ICONV "Force iconv even if no iconv.h is found" OFF)
option(DISABLE_ICONV "Disables testing for iconv" OFF)

if(NOT DISABLE_ICONV AND NOT FORCE_ICONV)
	INCLUDE (CheckIncludeFiles)
	CHECK_INCLUDE_FILES(iconv.h ICONV_H_FOUND)
endif()

if(ICONV_H_FOUND)
	set(USE_ICONV 1)
endif()

if(FORCE_ICONV)
	set(USE_ICONV 1)
endif()

if(DISABLE_ICONV)
	unset(USE_ICONV)
endif()

if(NOT USE_ICONV)
	message(WARNING "libs3util needs iconv for good iso8859-2 (Polish) to utf8 conversion and EUC-KR (Korean) or SHIFT_JIS (Japanese) to utf8 conversion.\nISO8859-1 (western Europa) to utf8 conversion is builtin")
endif()

TEST_BIG_ENDIAN(IS_BE)

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.s3util.h)

include_directories(src ${PROJECT_BINARY_DIR})

#s3util
file(GLOB_RECURSE s3util_SOURCES "src/*.[ch]")

add_library(s3util SHARED ${s3util_SOURCES})

install(TARGETS s3util DESTINATION lib)
install(FILES src/s3util_ext.h src/s3util.h DESTINATION include/s3util)

