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
using id = with_loc<identifier_t>;

template <template <typename> class T>
using noattr = T<empty_attribute_t>;

using node_t = noattr<attr_node_t>;
auto ast(string_view input) -> syntax::tree_t {
    return input | tokenize | parse;
}
}  // namespace

TEST_CASE("Integral literal parsing.", "[parser]") {
    REQUIRE(ast("42"sv) == tree_t(integral_literal_t(42, '?', 0)));
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
    REQUIRE(ast("( 5 )") == tree_t(integral_literal_t(5, '?', 0)));
}

TEST_CASE("Boolean literals", "[parser]") {
    REQUIRE(ast("true") == tree_t(token::true_t{}));

    REQUIRE(ast("false") == tree_t(token::false_t{}));
}

TEST_CASE("Boolean operations", "[parser]") {
    REQUIRE(ast("true or false") ==
            tree_t(noattr<bin_op_t>{OR, node_t(token::true_t{}), node_t(token::false_t{})}));

    REQUIRE(ast("false and true") ==
            tree_t(noattr<bin_op_t>{AND, node_t(token::false_t{}), node_t(token::true_t{})}));
}

TEST_CASE("Boolean comparisons", "[parser]") {
    const auto x = node_t(id("x"));
    REQUIRE(ast("x > 5") ==
            tree_t(noattr<bin_op_t>{GT, x, node_t(integral_literal_t(5, '?', 0))}));

    REQUIRE(ast("x<5.0") ==
            tree_t(noattr<bin_op_t>{LT, x, node_t(floating_point_literal_t(5, 0))}));

    REQUIRE(ast("10.0f32 >= 5") ==
            tree_t(noattr<bin_op_t>{GEQ, node_t(floating_point_literal_t(10, 32)),
                                    node_t(integral_literal_t(5, '?', 0))}));

    REQUIRE(ast("x == y") == tree_t(noattr<bin_op_t>{EQUAL, node_t(id("x")), node_t(id("y"))}));

    REQUIRE(ast("x!=y") == tree_t(noattr<bin_op_t>{NOT_EQUAL, x, node_t(id("y"))}));

    REQUIRE(ast("x in xs") == tree_t(noattr<bin_op_t>{IN, x, node_t(id("xs"))}));

    REQUIRE(ast("x not in xs") ==
            tree_t(noattr<unary_op_t>{NOT, node_t(noattr<bin_op_t>{IN, x, node_t(id("xs"))})}));

    REQUIRE(ast("x is y") == tree_t(noattr<bin_op_t>{IS, x, node_t(id("y"))}));

    REQUIRE(ast("x is not y") ==
            tree_t(noattr<unary_op_t>{
                NOT, node_t(noattr<bin_op_t>{IS, node_t(id("x")), node_t(id("y"))})}));
}

TEST_CASE("Arithmetic expressions", "[parser]") {
    const auto x = node_t(id("x"));
    const auto y = node_t(id("y"));
    const auto z = node_t(id("z"));

    REQUIRE(ast("x | y") == tree_t(noattr<bin_op_t>{BAR, x, y}));

    REQUIRE(ast("x | y | z") ==
            tree_t(noattr<bin_op_t>{BAR, tree_t(noattr<bin_op_t>{BAR, x, y}), z}));
}

TEST_CASE("Statements", "[parser]") {
    const auto x = node_t(id("x"));
    const auto y = node_t(id("y"));
    const auto z = node_t(id("z"));

    const auto two = node_t(integral_literal_t(2, '?', 0));
    const auto three = node_t(integral_literal_t(3, '?', 0));
    const auto four = node_t(integral_literal_t(4, '?', 0));

    REQUIRE(ast("x = y + 2") ==
            tree_t(noattr<syntax::assign_t>{x, node_t(noattr<bin_op_t>{PLUS, y, two})}));

    REQUIRE(ast("var x = y + 2") ==
            tree_t(noattr<syntax::var_def_t>{id("x"), 0, node_t(),
                                             node_t(noattr<bin_op_t>{PLUS, y, two})}));

    REQUIRE(ast(R"(some_fn(x))") ==
            tree_t(noattr<syntax::invoc_t>{node_t(id("some_fn")), noattr<syntax::data_t>{x}}));
}
