#define CATCH_CONFIG_MAIN

#include <sstream>

#include <range/v3/core.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/algorithm/copy.hpp>

#include <catch2/catch.hpp>

#include <bullet/token.hpp>
#include <bullet/lexer.hpp>

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

TEST_CASE("Token list functionality", "[token]") {
    const auto tt = token_list_t{VERBATIM, BAR, BAR};
    auto s = std::stringstream();
    s << tt;
    REQUIRE(s.str() == "[token[VERBATIM], token[BAR], token[BAR]]");
}

TEST_CASE("Tokenize: empty input", "[lexer/tokenize]") {
    REQUIRE(ranges::empty(""sv | tokenize | tokens));
}

TEST_CASE("Tokenize: each basic token can be tokenized.", "[lexer/tokenize]") {
    // iterate over all non-special (INDENT, DEDENT, EOL, etc.) token types 
    // and ensure they can be tokenized
    hana::for_each(token::types, [] (auto token_type_c) {
        using token_type = typename decltype(token_type_c)::type;
        constexpr auto tok = token_type{};
        const auto tok_sym = token_symbol(tok);
        if (tok_sym.empty()) return;

        const auto ts = tok_sym | tokenize | tokens;
        REQUIRE(ranges::size(ts) == 1);
        REQUIRE(token_t(tok) == ranges::front(ts));
        REQUIRE(std::holds_alternative<token_type>(ranges::front(ts)));
    });
}

TEST_CASE("Tokenize: identifiers.", "[lexer/tokenize]") {
    const auto inputs = { 
        "foo"sv, 
        "FOO"sv, 
        "_foo82_34"sv, 
        "BAR_23432"sv 
    };
    for (auto input: inputs) {
        const auto ts = input | tokenize | tokens;
        REQUIRE(ranges::size(ts) == 1);
        const auto t = ranges::front(ts);
        REQUIRE(std::holds_alternative<identifier_t>(t));
        REQUIRE(std::get<identifier_t>(t).name == input);
    }
}


TEST_CASE("Tokenize: inline colon (no brackets generated)", "[lexer/tokenize]") {
    const auto expected = token_list_t{META, COLON, VERBATIM};
    REQUIRE(("meta:verbatim"sv | tokenize | tokens) == expected);
    REQUIRE(("meta : verbatim"sv | tokenize | tokens) == expected);
    REQUIRE(("meta: verbatim"sv | tokenize | tokens) == expected);
}

TEST_CASE("Tokenize: end-of-line colon (brackets generated)", "[lexer/tokenize]") {
    const auto expected = token_list_t{
        META, 
        OPAREN, 
        VERBATIM,
        CPAREN
    };
    REQUIRE(("meta:\n    verbatim"sv | tokenize | tokens) == expected);
}

TEST_CASE("Tokenize: line extension (no brackets generated)", "[lexer/tokenize]") {
    const auto expected = token_list_t{
        META, 
        VERBATIM,
    };
    REQUIRE(("meta\n    verbatim"sv | tokenize | tokens) == expected);
}

TEST_CASE("Tokenize: integers.", "[lexer/tokenize]") {
    using namespace literal::numeric;
    REQUIRE(("42"sv | tokenize | tokens) == token_list_t{token_t(integral_t(42, 'i', 64))});

    REQUIRE(("42i64"sv | tokenize | tokens) == token_list_t{token_t(integral_t(42, 'i', 64))});
    REQUIRE(("42u64"sv | tokenize | tokens) == token_list_t{token_t(integral_t(42, 'u', 64))});
    REQUIRE(("42i32"sv | tokenize | tokens) == token_list_t{token_t(integral_t(42, 'i', 32))});
    REQUIRE(("42u32"sv | tokenize | tokens) == token_list_t{token_t(integral_t(42, 'u', 32))});
    REQUIRE(("42i16"sv | tokenize | tokens) == token_list_t{token_t(integral_t(42, 'i', 16))});
    REQUIRE(("42u16"sv | tokenize | tokens) == token_list_t{token_t(integral_t(42, 'u', 16))});
    REQUIRE(("42i8"sv | tokenize | tokens) == token_list_t{token_t(integral_t(42, 'i', 8))});
    REQUIRE(("42u8"sv | tokenize | tokens) == token_list_t{token_t(integral_t(42, 'u', 8))});
}

TEST_CASE("Tokenize: floats.", "[lexer/tokenize]") {
    using namespace literal::numeric;
    REQUIRE(("42.0"sv | tokenize | tokens) == token_list_t{token_t(floating_point_t(42, 64))});
    REQUIRE(("42e0"sv | tokenize | tokens) == token_list_t{token_t(floating_point_t(42, 64))});
    REQUIRE(("42e1"sv | tokenize | tokens) == token_list_t{token_t(floating_point_t(420, 64))});

    REQUIRE(("42.0f64"sv | tokenize | tokens) == token_list_t{token_t(floating_point_t(42, 64))});
    REQUIRE(("42e0f64"sv | tokenize | tokens) == token_list_t{token_t(floating_point_t(42, 64))});
    REQUIRE(("42e1f64"sv | tokenize | tokens) == token_list_t{token_t(floating_point_t(420, 64))});

    REQUIRE(("42.0f32"sv | tokenize | tokens) == token_list_t{token_t(floating_point_t(42, 32))});
    REQUIRE(("42e0f32"sv | tokenize | tokens) == token_list_t{token_t(floating_point_t(42, 32))});
    REQUIRE(("42e1f32"sv | tokenize | tokens) == token_list_t{token_t(floating_point_t(420, 32))});
}

TEST_CASE("Tokenize: strings.", "[lexer/tokenize]") {
    REQUIRE(
        (R"bt("")bt"sv | tokenize | tokens) == 
            token_list_t{token_t(string_token_t(""))});
    REQUIRE(
        (R"bt("this is a test")bt"sv | tokenize | tokens) == 
            token_list_t{token_t(string_token_t("this is a test"))});
    REQUIRE(
        (R"bt("this \"is\" a test")bt"sv | tokenize | tokens) == 
            token_list_t{token_t(string_token_t(R"raw(this "is" a test)raw"))});

    REQUIRE(
        (R"bt("backslash? \\")bt"sv | tokenize | tokens) == 
            token_list_t{token_t(string_token_t(R"raw(backslash? \)raw"))});

    REQUIRE(
        (R"bt("newline? \nfoo")bt"sv | tokenize | tokens) == 
            token_list_t{token_t(string_token_t("newline? \nfoo"))});

    REQUIRE(
        (R"bt("tab? \tfoo")bt"sv | tokenize | tokens) == 
            token_list_t{token_t(string_token_t("tab? \tfoo"))});
}

TEST_CASE("Tokenize: random shit.", "[lexer/tokenize]") {
    const auto input = R"bt(
        foo:
            print(bar)
            print:bar

            verbatim

        x = if
        print(x + 
            x)

        meta
    )bt"sv;
    const auto ts = input | tokenize | tokens;
    auto u = std::stringstream();
    u << ts;
    const auto expected = 
        "[ident[foo], token[OPAREN], ident[print], token[OPAREN], ident[bar], "
        "token[CPAREN], token[LINE_END], ident[print], token[COLON], ident[bar], "
        "token[LINE_END], token[VERBATIM], token[CPAREN], token[LINE_END], "
        "ident[x], token[ASSIGN], token[IF], token[LINE_END], ident[print], "
        "token[OPAREN], ident[x], token[PLUS], ident[x], token[CPAREN], "
        "token[LINE_END], token[META], token[LINE_END]]";

    REQUIRE(u.str() == expected);
}

/*
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
