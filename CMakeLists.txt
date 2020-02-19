cmake_minimum_required (VERSION 3.5.1)

project(bullet  VERSION 0.1
                DESCRIPTION "Bullet"
                LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(sources
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    src/lexer/numeric_token.cpp
    src/lexer/string_token.cpp
    src/lexer/identifier.cpp
    src/parser/ast.cpp
    src/parser/parser.cpp
)

add_library(
    bt
    ${sources}
)
conan_target_link_libraries(bt)
target_compile_features(bt PRIVATE cxx_std_17)
target_compile_options(bt PRIVATE -Wno-logical-op-parentheses)
target_compile_definitions(bt PRIVATE BOOST_HANA_CONFIG_ENABLE_STRING_UDL)
target_include_directories(bt PRIVATE include)

add_executable(
    btc 
    src/main.cpp
)

conan_target_link_libraries(btc)
target_link_libraries(btc bt)
target_compile_features(btc PRIVATE cxx_std_17)
target_compile_options(btc PRIVATE -Wno-logical-op-parentheses)
target_compile_definitions(btc PRIVATE BOOST_HANA_CONFIG_ENABLE_STRING_UDL)
target_include_directories(btc PRIVATE include)

# TESTS
enable_testing()


foreach(test_case lexer parser)
    add_executable(
        test_${test_case}
        test/${test_case}.cpp
    )

    set(target test_${test_case})

    conan_target_link_libraries(${target})
    target_link_libraries(${target} bt)
    target_compile_features(${target} PRIVATE cxx_std_17)
    target_compile_options(${target} PRIVATE -Wno-logical-op-parentheses)
    target_include_directories(${target} PRIVATE include)
    target_compile_definitions(${target} PRIVATE BOOST_HANA_CONFIG_ENABLE_STRING_UDL)
    add_test(NAME ${test_case} COMMAND ${target})
endforeach()

#add_executable(
#    test_parser 
#    test/parser.cpp
#)
#conan_target_link_libraries(test_parser)
#target_link_libraries(test_parser bt)
#target_compile_features(test_parser PRIVATE cxx_std_17)
#target_compile_options(test_parser PRIVATE -Wno-logical-op-parentheses)
#target_include_directories(test_parser PRIVATE include)
#target_compile_definitions(test_parser PRIVATE BOOST_HANA_CONFIG_ENABLE_STRING_UDL)
#add_test(NAME Parser COMMAND test_parser)
