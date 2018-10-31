#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <bullet/lexer.hpp>
#include <bullet/util.hpp>

using namespace std;
using namespace lexer;
using namespace lexer::token;

TEST_CASE("Indented line", "[lexer]") {
    const auto input = R"(
foo:
   bar 
   baz
bang
)"sv;

    // clang-format off
    const auto expected = vector<token_t>{
        {eol},
        {eol},
        {"foo"},
        {indent},
        {"bar"},
        {eol},
        {"baz"},
        {dedent},
        {eol},
        {"bang"},
        {eol},
    };
    // clang-format on

    const auto output = input | tokens;
    REQUIRE(expected == output);
}

TEST_CASE("Extended line", "[lexer]") {
    const auto input = R"(
foo
   bar 
   baz
bang
)"sv;

    // clang-format off
    const auto expected = vector<token_t>{
        {eol},
        {eol},
        {"foo"},
        {"bar"},
        {"baz"},
        {eol},
        {"bang"},
        {eol},
    };
    // clang-format on

    const auto output = input | tokens;
    REQUIRE(expected == output);
}

TEST_CASE("Strict on colon/EOL without indent", "[lexer]") {
    const auto input = R"(
foo:
bang
)"sv;

    REQUIRE_THROWS(input | tokens);
}
