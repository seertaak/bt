cmake_minimum_required (VERSION 3.5.1)

project(bullet  VERSION 0.1
                DESCRIPTION "Bullet"
                LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(sources "")

add_executable(
    bt 
    src/main.cpp
    ${sources}
)

conan_target_link_libraries(bt)
target_compile_features(bt PRIVATE cxx_std_17)
target_compile_options(bt PRIVATE -Wno-logical-op-parentheses)
target_compile_definitions(bt PRIVATE BOOST_HANA_CONFIG_ENABLE_STRING_UDL)
target_include_directories(bt PRIVATE include)

# TESTS
enable_testing()

add_executable(test_lexer test/lexer.cpp)
conan_target_link_libraries(test_lexer)
target_compile_features(test_lexer PRIVATE cxx_std_17)
target_compile_options(test_lexer PRIVATE -Wno-logical-op-parentheses)
target_include_directories(test_lexer PRIVATE include)
target_compile_definitions(test_lexer PRIVATE BOOST_HANA_CONFIG_ENABLE_STRING_UDL)
add_test(NAME Lexer COMMAND test_lexer)
