#define CATCH_CONFIG_MAIN

#include <sstream>

#include <range/v3/core.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/algorithm/copy.hpp>

#include <catch2/catch.hpp>

#include <bullet/token.hpp>
#include <bullet/token_tree.hpp>
#include <bullet/tokenizer.hpp>

using namespace std;
using namespace lexer;
using namespace lexer::token;

TEST_CASE("Basic token functionality", "[token]") {
    REQUIRE(token_name(VERBATIM) == "VERBATIM");
    REQUIRE(token_symbol(VERBATIM) == "verbatim");

    REQUIRE(token_name(BAR) == "BAR");
    REQUIRE(token_symbol(BAR) == "|");

    auto s = std::stringstream();
    s << CASE;

    REQUIRE(s.str() == "token[CASE]");
}

TEST_CASE("Token tree functionality", "[token_tree]") {
    const auto tt = token_tree_t(token_list_t{VERBATIM, BAR, BAR});
    auto s = std::stringstream();
    s << tt;
    REQUIRE(s.str() == "[token[VERBATIM], token[BAR], token[BAR]]");
}

namespace {
    auto token_list_to_tree(const std::vector<token_t>& tokens) -> token_tree_t {
        auto result = token_list_t{};
        for (auto t: tokens) 
            result.push_back(t);
        return token_tree_t(result);
    }
}

TEST_CASE("Parse token tree", "[tokenizer/tokens1]") {
    const auto input = 
R"bt(foo:
    print(bar)

    verbatim

x = if

meta
)bt"sv;
    const auto tokens = token_list_to_tree(lexer::tokens1(input));
    auto u = std::stringstream();
    u << tokens;
    REQUIRE(u.str() == "[token[META], token[META]]");
}

/*
TEST_CASE("Indented line", "[lexer]") {
    const auto input = R"(foo )"sv;

    // clang-format off
    const auto expected = vector<token_t>{
        identifier_t("foo"),
    };
    // clang-format on

    const auto output = input | tokens1;
    for (auto t : output) cout << t << endl;
    REQUIRE(expected == output);
}

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

    std::cout << "GOT HERE" << std::endl;
    const auto output = input | tokens1;
    std::cout << "GOT HERE" << std::endl;
    for (auto t : output) cout << t << endl;
    std::cout << "GOT HERE" << std::endl;
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
