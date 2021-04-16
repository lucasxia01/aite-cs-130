cmake_minimum_required(VERSION 3.10.0)
project(aite)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system)
message(STATUS "Boost version: ${Boost_VERSION}")

# Including custom cmake rules, code from https://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html
include(cmake/clang-cxx-dev-tools.cmake)

include_directories(include)

# adds libraries for server.cc and session.cc
add_library(server_lib src/server.cc)
add_library(session_lib src/session.cc)
add_library(config_parser_lib src/config_parser.cc)

# add server_lib dependency on session_lib
target_link_libraries(server_lib session_lib config_parser_lib)

# Creates executable
add_executable(server src/server_main.cc)

# Link relevant libraries
target_link_libraries(server Boost::system pthread server_lib)

# Create test executable name, srcs, and deps
add_executable(config_parser_lib_test tests/config_parser/config_parser_test.cc)
target_link_libraries(config_parser_lib_test config_parser_lib gtest_main)
add_executable(session_lib_test tests/session/session_test.cc)
target_link_libraries(session_lib_test session_lib gtest_main)
add_executable(server_lib_test tests/server/server_test.cc)
target_link_libraries(server_lib_test server_lib gtest_main)

# Automatically add tests with CTest by querying the compiled test executable for available tests
gtest_discover_tests(config_parser_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/config_parser)
gtest_discover_tests(session_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/session)
gtest_discover_tests(server_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/server)

# generate coverage report
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS server config_parser_lib server_lib session_lib TESTS config_parser_lib_test server_lib_test session_lib_test)
