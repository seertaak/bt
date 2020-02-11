#define CATCH_CONFIG_MAIN

#include <sstream>

#include <range/v3/algorithm/copy.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/view/transform.hpp>

#include <catch2/catch.hpp>

#include <bullet/lexer/lexer.hpp>
#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/parser/parser.hpp>

using namespace std;
using namespace bt;
using namespace lexer;
using namespace lexer::token;
using namespace parser;
using namespace syntax;

namespace {
    auto ast(string_view input) -> syntax::tree_t { return input | tokenize | parse; }
}  // namespace

TEST_CASE("Integral literal parsing.", "[parser]") {
    REQUIRE(ast("42"sv) == tree_t(integral_literal_t(42, 'i', 64)));
}

TEST_CASE("Floating point parsing.", "[parser]") {
    REQUIRE(ast("42.0f32"sv) == tree_t(floating_point_literal_t(42.0, 32)));
}

TEST_CASE("String literal parsing.", "[parser]") {
    REQUIRE(ast(R"("a literal string")") == tree_t(string_literal_t(R"(a literal string)")));
}

TEST_CASE("Error message", "[parser]") {
    REQUIRE_THROWS(ast(R"(|)"));
}

TEST_CASE("Group", "[parser]") {
    // FIXME: there is a bug in integral token lexing (surprise, surprise)
    // which means spaces need to surround the `5' in order for it to be parsed
    // correctly.
    REQUIRE(ast("( 5 )") == tree_t(integral_literal_t(5, 'i', 64)));
}

TEST_CASE("Boolean literals", "[parser]") {
    REQUIRE(ast("true") == tree_t(token::true_t{}));
    REQUIRE(ast("false") == tree_t(token::false_t{}));
}

TEST_CASE("Boolean operations", "[parser]") {
    REQUIRE(ast("true or false") ==
            tree_t(bin_op_t{token::or_t{}, node_t(token::true_t{}), node_t(token::false_t{})}));
    REQUIRE(ast("false and true") ==
            tree_t(bin_op_t{token::and_t{}, node_t(token::false_t{}), node_t(token::true_t{})}));
}
