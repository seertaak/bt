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

TEST_CASE("Integral literal parsing.", "[parser]") {
    const auto input = "42"sv;
    const auto ast = input | tokenize | parse;
    REQUIRE(ast == tree_t(integral_literal_t(42, 'i', 64)));
}

TEST_CASE("Floating point parsing.", "[parser]") {
    const auto input = "42.0f32"sv;
    const auto ast = input | tokenize | parse;
    REQUIRE(ast == tree_t(floating_point_literal_t(42.0, 32)));
}

TEST_CASE("String literal parsing.", "[parser]") {
    const auto input = R"("a literal string")";
    const auto ast = input | tokenize | parse;
    REQUIRE(ast == tree_t(string_literal_t(R"(a literal string)")));
}

TEST_CASE("Error message", "[parser]") {
    const auto input = R"(|)";
    try {
        const auto ast = input | tokenize | parse;
    } catch (std::runtime_error& e) {
        cout << e.what() << endl;
    }
    REQUIRE_THROWS(input | tokenize | parse);
}
