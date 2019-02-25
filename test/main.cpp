#define CATCH_CONFIG_MAIN

#include <sstream>

#include <catch.hpp>

#include <bullet/token.hpp>
#include <bullet/token_tree.hpp>
#include <bullet/util.hpp>

using namespace std;
using namespace lexer;
using namespace lexer::token;

TEST_CASE("Basic token functionality", "[lexer]") {
    REQUIRE(token_name(VERBATIM) == "VERBATIM");
    REQUIRE(token_symbol(VERBATIM) == "verbatim");

    REQUIRE(token_name(BAR) == "BAR");
    REQUIRE(token_symbol(BAR) == "|");

    auto s = std::stringstream();
    s << CASE;

    REQUIRE(s.str() == "CASE");
}

TEST_CASE("Token tree functionality", "[lexer]") {
    const auto tt = token_tree_t(
        token_list_t(VERBATIM, BAR, BAR),
        
    );

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
