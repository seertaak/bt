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
target_compile_features(bt PRIVATE cxx_std_20)
target_compile_options(bt PRIVATE -Wno-logical-op-parentheses)
target_compile_definitions(bt PRIVATE BOOST_HANA_CONFIG_ENABLE_STRING_UDL)
target_include_directories(bt PRIVATE include)

add_executable(
    tests 
    test/main.cpp
    ${sources}
)

conan_target_link_libraries(tests)
target_compile_features(tests PRIVATE cxx_std_20)
target_compile_options(tests PRIVATE -Wno-logical-op-parentheses)
target_include_directories(tests PRIVATE include)
target_compile_definitions(tests PRIVATE BOOST_HANA_CONFIG_ENABLE_STRING_UDL)
enable_testing()

add_test(NAME Test COMMAND tests)
