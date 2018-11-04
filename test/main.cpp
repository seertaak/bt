#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <bullet/lexer.hpp>
#include <bullet/util.hpp>

using namespace std;
using namespace lexer;
using namespace lexer::token;

/*
TEST_CASE("Indented line", "[lexer]") {
    const auto input = R"(
foo:
   bar
   baz
bang
)"sv;

    // clang-format off
    const auto expected = vector<token_t>{
        EOL,
        EOL,
        identifier_t("foo"),
        INDENT,
        identifier_t("bar"),
        EOL,
        identifier_t("baz"),
        DEDENT,
        EOL,
        identifier_t("bang"),
        EOL
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
        EOL,
        EOL,
        identifier_t("foo"),
        identifier_t("bar"),
        identifier_t("baz"),
        EOL,
        identifier_t("bang"),
        EOL,
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
        EOL,
        EOL,
        identifier_t("foo"),
        COLON,
        identifier_t("bar"),
        identifier_t("baz"),
        EOL,
    };
    // clang-format on

    const auto output = input | tokens1;
    REQUIRE(expected == output);
}

TEST_CASE("Token variant parser", "[lexer::op::value]") {
    const auto good_input = vector<token_t>{EOL};
    auto i = begin(good_input);
    token_t output;

    auto success = parse(i, end(good_input), tlit(EOL), output);

    REQUIRE(success);
    REQUIRE(output == back(good_input));

    const auto bad_input = vector<token_t>{EOL};
    i = begin(bad_input);

    success = parse(i, end(bad_input), tlit(OPAREN), output);

    REQUIRE(!success);
}
*/
