cmake_minimum_required(VERSION 3.2.2)
project(libs3util VERSION 1.0.0)

include(TestBigEndian)
include(CMakePackageConfigHelpers)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
set(CPACK_PACKAGE_NAME "libs3util")
set(CPACK_PACKAGE_VERSION ${libs3util_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${libs3util_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${libs3util_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${libs3util_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
include(CPack)

option(PRIVATE_FILENAME "stop leaking complete path to file (should be enabled on distribution)" ON)
option(FORCE_ICONV "Force iconv even if no iconv.h is found" OFF)
option(DISABLE_ICONV "Disables testing for iconv" OFF)

if(NOT DISABLE_ICONV AND NOT FORCE_ICONV)
	include(CheckIncludeFiles)
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
configure_file(libs3util.pc.in ${PROJECT_BINARY_DIR}/libs3util.pc @ONLY)

include_directories(src ${PROJECT_BINARY_DIR})

#s3util
file(GLOB_RECURSE s3util_SOURCES "src/*.[ch]")

add_library(s3util SHARED ${s3util_SOURCES})
target_include_directories(s3util PUBLIC $<INSTALL_INTERFACE:include>)

write_basic_package_version_file(${PROJECT_BINARY_DIR}/s3utilConfigVersion.cmake COMPATIBILITY ExactVersion)
configure_package_config_file(s3utilConfig.cmake.in s3utilConfig.cmake INSTALL_DESTINATION lib/cmake/s3util PATH_VARS CMAKE_INSTALL_PREFIX)

install(TARGETS s3util EXPORT s3utilTarget DESTINATION lib)
install(EXPORT s3utilTarget DESTINATION lib/cmake/s3util)

install(FILES src/s3util.h DESTINATION include)
install(FILES ${PROJECT_BINARY_DIR}/libs3util.pc DESTINATION lib/pkgconfig)
install(FILES ${PROJECT_BINARY_DIR}/s3utilConfigVersion.cmake ${PROJECT_BINARY_DIR}/s3utilConfig.cmake DESTINATION lib/cmake/s3util)
install(FILES LICENSE DESTINATION share/doc/libs3util)

