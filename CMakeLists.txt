cmake_minimum_required (VERSION 3.5.1)

project(bullet  VERSION 0.1
                DESCRIPTION "Bullet"
                LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

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
    "-Wno-#warnings"
)
target_compile_definitions(bt PUBLIC BOOST_HANA_CONFIG_ENABLE_STRING_UDL)
target_include_directories(bt PUBLIC include third_party)

###################################################################
# Compiler
###################################################################

add_executable(
    btc 
    src/main.cpp
)
conan_target_link_libraries(btc)
target_link_libraries(btc bt)

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
