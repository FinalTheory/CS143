project(cool)

# TODO: Fix code to remove all warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wno-write-strings -Wno-attributes")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-string-compare -Wno-deprecated-register -Wno-reserved-user-defined-literal")
endif()

set(SOURCE_FILES
        cool.cc
        semant.cc
        cgen.cc
        cgen_supp.cc
        cool-tree.cc
        dumptype.cc
        handle_flags.cc
        stringtab.cc
        tree.cc
        utilities.cc
        classtable.cc
        globals.cc
        emit.cc)

include_directories(.)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(FLEX)
FLEX_TARGET(Lexer cool.flex
        ${CMAKE_CURRENT_BINARY_DIR}/cool-lex.cc
        COMPILE_FLAGS "-d")

set(BISON_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/cool-parse.cc)
set(BISON_HEADER ${CMAKE_CURRENT_BINARY_DIR}/cool-parse.h)
find_package(BISON)
BISON_TARGET(Parser cool.y
        ${BISON_SOURCE}
        COMPILE_FLAGS "-d -v -y -b cool --debug -p cool_yy --defines=${BISON_HEADER}")

add_executable(
        coolc
        ${SOURCE_FILES}
        ${FLEX_Lexer_OUTPUTS}
        ${BISON_Parser_OUTPUTS}
)

set(COOLC_TEST_DIRECTORY ${CMAKE_SOURCE_DIR}/tests/compiler)

add_custom_target(
        cooltest
        sh ${COOLC_TEST_DIRECTORY}/run_all_tests.sh ${CMAKE_CURRENT_BINARY_DIR}
        WORKING_DIRECTORY ${COOLC_TEST_DIRECTORY}
        DEPENDS coolc
        COMMENT "Run tests for coolc compiler"
        VERBATIM
)

# Note that on Linux or cygwin you should link to libfl.so, not libl.so
# This is just for Mac OS
target_link_libraries(coolc l)
