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

    const auto output = input | tokens1;
    for (auto t : output) cout << t << endl;
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

    const auto output = input | tokens1;
    REQUIRE(expected == output);
}

TEST_CASE("Strict on colon/EOL without indent", "[lexer]") {
    const auto input = R"(
foo:
bang
)"sv;

    REQUIRE_THROWS(input | tokens1);
}

TEST_CASE("In-line colon", "[lexer]") {
    const auto input = R"(
foo: bar
   baz
)"sv;

    // clang-format off
    const auto expected = vector<token_t>{
        {eol},
        {eol},
        {"foo"},
        {colon},
        {"bar"},
        {"baz"},
        {eol},
    };
    // clang-format on

    const auto output = input | tokens1;
    REQUIRE(expected == output);
}

TEST_CASE("Token variant parser", "[lexer::op::value]") {
    const auto good_input = vector<token_t>{{eol}};
    auto i = begin(good_input);
    token_t output;

    auto success = parse(i, end(good_input), t(eol), output);

    REQUIRE(success);
    REQUIRE(output == back(good_input));

    const auto bad_input = vector<token_t>{{eol}};
    i = begin(bad_input);

    success = parse(i, end(bad_input), t(oparen), output);

    REQUIRE(!success);
}
