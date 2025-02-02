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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log_setup log)
message(STATUS "Boost version: ${Boost_VERSION}")

# Including custom cmake rules, code from https://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html
include(cmake/clang-cxx-dev-tools.cmake)

include_directories(include)

# adds libraries for server.cc and session.cc
add_library(session_lib src/session.cc)
add_library(server_lib src/server.cc)
add_library(config_parser_lib src/config_parser.cc)
add_library(request_parser_lib src/request_parser.cc)
add_library(utils_lib src/utils.cc)
add_library(logger_lib src/logger.cc)
add_library(dummy_handler_lib src/handlers/dummy_handler.cc)
add_library(echo_handler_lib src/handlers/echo_handler.cc)
add_library(not_found_handler_lib src/handlers/not_found_handler.cc)
add_library(reverse_proxy_handler_lib src/handlers/reverse_proxy_handler.cc)
add_library(static_file_handler_lib src/handlers/static_file_handler.cc)
add_library(status_handler_lib src/handlers/status_handler.cc)
add_library(meme_handler_lib src/handlers/meme_handler.cc)
add_library(health_handler_lib src/handlers/health_handler.cc)
add_library(sleep_handler_lib src/handlers/sleep_handler.cc)
add_library(http_client_lib src/http_client.cc)

# Add logger dependencies
target_link_libraries(config_parser_lib logger_lib)

# add session_lib dependency on request_parser, request, and request_handler libs (indirectly linked with logger)
target_link_libraries(session_lib request_parser_lib echo_handler_lib not_found_handler_lib reverse_proxy_handler_lib static_file_handler_lib status_handler_lib health_handler_lib sleep_handler_lib meme_handler_lib server_lib)

target_link_libraries(logger_lib Boost::log_setup Boost::log)

target_link_libraries(server_lib session_lib utils_lib)

# add config_parser dependency on utils
target_link_libraries(utils_lib config_parser_lib)

# add config_parser dependency on request_handler libs
target_link_libraries(reverse_proxy_handler_lib http_client_lib utils_lib) 
target_link_libraries(static_file_handler_lib utils_lib) 
target_link_libraries(status_handler_lib server_lib)
target_link_libraries(meme_handler_lib server_lib)
target_link_libraries(health_handler_lib server_lib)
target_link_libraries(sleep_handler_lib server_lib)

# Creates executable
add_executable(server src/server_main.cc)

# Link relevant libraries
target_link_libraries(server Boost::system pthread server_lib logger_lib)

# Create test executable name, srcs, and deps
add_executable(config_parser_lib_test tests/config_parser/config_parser_test.cc)
add_executable(utils_lib_test tests/utils/utils_test.cc)
add_executable(session_lib_test tests/session/session_test.cc)
add_executable(server_lib_test tests/server/server_test.cc)
add_executable(request_parser_lib_test tests/request_parser/request_parser_test.cc)
add_executable(request_handler_lib_test tests/request_handler/request_handler_test.cc)

target_link_libraries(config_parser_lib_test config_parser_lib gtest_main)
target_link_libraries(utils_lib_test utils_lib gtest_main)
target_link_libraries(session_lib_test server_lib session_lib gtest_main)
target_link_libraries(server_lib_test server_lib dummy_handler_lib gtest_main)
target_link_libraries(request_parser_lib_test request_parser_lib gtest_main)
target_link_libraries(request_handler_lib_test echo_handler_lib not_found_handler_lib reverse_proxy_handler_lib static_file_handler_lib status_handler_lib gtest_main)

add_test(NAME integration_tests COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/integration_tests.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration)

# Automatically add tests with CTest by querying the compiled test executable for available tests
gtest_discover_tests(config_parser_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/config_parser)
gtest_discover_tests(utils_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/config_parser)
gtest_discover_tests(session_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/session)
gtest_discover_tests(server_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/server)
gtest_discover_tests(request_parser_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/request_parser)
gtest_discover_tests(request_handler_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/request_handler)

# generate coverage report
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(
    TARGETS server config_parser_lib server_lib session_lib request_parser_lib utils_lib dummy_handler_lib echo_handler_lib not_found_handler_lib reverse_proxy_handler_lib static_file_handler_lib status_handler_lib health_handler_lib
    TESTS config_parser_lib_test request_parser_lib_test server_lib_test utils_lib_test session_lib_test request_handler_lib_test
)
