cmake_minimum_required (VERSION 3.5.1)

project(bullet  VERSION 0.1
                DESCRIPTION "Bullet"
                LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# We generate the kernel.json file, given the installation prefix and the executable name
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/bullet/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/bullet/kernel.json"
)

###################################################################
# Source Files
###################################################################

set(sources
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    src/lexer/numeric_token.cpp
    src/lexer/string_token.cpp
    src/lexer/identifier.cpp
    src/lexer/location.cpp
    src/parser/parser.cpp
    src/parser/location.cpp
    src/analysis/type.cpp
    src/analysis/error.cpp
)

###################################################################
# Bullet Library
###################################################################

add_library(
    bt
    ${sources}
)
conan_target_link_libraries(bt)
target_compile_features(bt PUBLIC cxx_std_20)
target_compile_options(bt PUBLIC 
    -Wno-logical-op-parentheses 
    -Wno-ambiguous-reversed-operator
    -Wunused-command-line-argument
    "-Wno-#warnings"   # remove this when Boost 1.72 comes out (Hana doesn't like Clang 11!).
    -Wl,--export-dynamic
)
target_compile_definitions(bt PUBLIC 
    BOOST_HANA_CONFIG_ENABLE_STRING_UDL
    BOOST_STACKTRACE_LINK
)
target_include_directories(bt PUBLIC include third_party)

###################################################################
# Compiler
###################################################################

add_executable(
    btc 
    src/main.cpp
)
conan_target_link_libraries(btc)
target_link_libraries(btc bt dl)

###################################################################
# Jupyter Plugin
###################################################################

add_executable(
    bt_kernel
    src/jupyter/interpreter.cpp
    src/jupyter/main.cpp
)
conan_target_link_libraries(bt_kernel)
target_link_libraries(bt_kernel bt dl)

###################################################################
# Tests
###################################################################

enable_testing()
foreach(test_case lexer parser)
    add_executable(
        test_${test_case}
        test/${test_case}.cpp
    )

    set(target test_${test_case})

    conan_target_link_libraries(${target})
    target_link_libraries(${target} bt)
    add_test(NAME ${test_case} COMMAND ${target})
endforeach()

# Installation
# ============

install(TARGETS bt_kernel
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Configuration and data directories for jupyter and my_kernel
set(XJUPYTER_DATA_DIR "share/jupyter" CACHE STRING "Jupyter data directory")

# Install Jupyter kernelspecs
set(KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
install(DIRECTORY ${KERNELSPEC_DIR}
        DESTINATION ${XJUPYTER_DATA_DIR}
        PATTERN "*.in" EXCLUDE)

