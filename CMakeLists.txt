cmake_minimum_required(VERSION 3.20.0)

project(compiler VERSION 1.0)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -g") # needed by binaryen

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(Parser ${PROJECT_SOURCE_DIR}/src/parser.yy ${PROJECT_SOURCE_DIR}/build/parser.tab.cc)
FLEX_TARGET(Lexer ${PROJECT_SOURCE_DIR}/src/lexer.l ${PROJECT_SOURCE_DIR}/build/lexer.yy.cc)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)
set_property(DIRECTORY build APPEND PROPERTY ADDITIONAL_CLEAN_FILES location.hh)

include_directories("/usr/include/") # needed by binaryen, includes binaryen-c.h
link_directories("/usr/lib/x86_64-linux-gnu/") # needed by binaryen, includes libbinaryen.so

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/tests)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(compiler main.cpp ${SOURCES} ${FLEX_Lexer_OUTPUTS} ${BISON_Parser_OUTPUTS})

option(DEBUG "Build with debug flags" OFF)

if (DEBUG)
  target_compile_definitions(compiler PRIVATE DEBUG)
endif()

set(RUN_WASM_FILE_LOCATION ${PROJECT_SOURCE_DIR}/tests/helpers/run2.js)
target_compile_definitions(compiler PRIVATE RUN_WASM_FILE_LOCATION)

target_link_libraries(compiler binaryen)

option(TESTS "Build the tests" OFF)

if (TESTS) 
  message("Building tests")
  enable_testing()

  find_package(GTest REQUIRED)

  file(GLOB_RECURSE TEST_SOURCES tests/*_test.cpp)

  add_library(parser STATIC ${PROJECT_SOURCE_DIR}/build/parser.tab.cc ${PROJECT_SOURCE_DIR}/build/parser.tab.hh)
  add_library(compiler_lib STATIC ${SOURCES} ${FLEX_Lexer_OUTPUTS})
  add_library(test_helper_lib STATIC ${PROJECT_SOURCE_DIR}/tests/helpers/compiler_helper.cpp ${SOURCES} ${FLEX_Lexer_OUTPUTS} ${BISON_Parser_OUTPUTS})
  target_compile_definitions(test_helper_lib PRIVATE RUN_WASM_FILE_LOCATION="${RUN_WASM_FILE_LOCATION}")
  if (DEBUG) 
    target_compile_definitions(compiler_lib PRIVATE DEBUG)
    target_compile_definitions(test_helper_lib PRIVATE DEBUG)
  endif()

  # For each test file, create a separate test executable
  foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} GTest::gtest GTest::gtest_main binaryen parser compiler_lib test_helper_lib)
    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()

endif()
