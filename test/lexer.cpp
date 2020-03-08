#define CATCH_CONFIG_MAIN

#include <sstream>

#include <range/v3/algorithm/copy.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/view/transform.hpp>

#include <catch2/catch.hpp>

#include <bullet/lexer/lexer.hpp>
#include <bullet/lexer/token.hpp>

using namespace std;
using namespace bt;
using namespace lexer;
using namespace lexer::token;

namespace {
auto tok_list(string_view input) -> source_token_list_t {
    auto output = input | tokenize;
    return output.tokens;
}
}  // namespace

TEST_CASE("Basic token functionality", "[token]") {
    REQUIRE(token_name(VERBATIM) == "VERBATIM");
    REQUIRE(token_symbol(VERBATIM) == "verbatim");

    REQUIRE(token_name(BAR) == "BAR");
    REQUIRE(token_symbol(BAR) == "|");

    auto s = std::stringstream();
    s << CASE;

    REQUIRE(s.str() == "token[CASE]");
}

TEST_CASE("Source token functionality", "[token]") {
    auto src_tok = source_token_t(PRIVATE);

    REQUIRE(src_tok.token == PRIVATE);
    REQUIRE(src_tok == PRIVATE);

    REQUIRE(src_tok.location == location_t{0, 0, 0});

    auto src_tok2 = source_token_t(PRIVATE);
    src_tok2.location.line = 5;

    REQUIRE(src_tok != src_tok2);
}

TEST_CASE("Token list functionality", "[token]") {
    const auto tt = token_list_t{VERBATIM, BAR, BAR};
    auto s = std::stringstream();
    s << tt;
    REQUIRE(s.str() == "[token[VERBATIM], token[BAR], token[BAR]]");
}

TEST_CASE("Tokenize: empty input", "[lexer/tokenize]") {
    REQUIRE(ranges::empty(tok_list(""sv)));
}

TEST_CASE("Tokenize: each basic token can be tokenized.", "[lexer/tokenize]") {
    // iterate over all non-special (INDENT, DEDENT, EOL, etc.) token types
    // and ensure they can be tokenized
    hana::for_each(token::types, [](auto token_type_c) {
        using token_type = typename decltype(token_type_c)::type;
        constexpr auto tok = token_type{};
        const auto tok_sym = token_symbol(tok);
        if (tok_sym.empty()) return;

        const auto ts = tok_list(tok_sym);

        REQUIRE(ranges::size(ts) == 1);
        REQUIRE(token_t(tok) == ranges::front(ts));
        REQUIRE(std::holds_alternative<token_type>(ranges::front(ts).token));
    });
}

TEST_CASE("Tokenize: identifiers.", "[lexer/tokenize]") {
    const auto inputs = {"foo"sv, "FOO"sv, "_foo82_34"sv, "BAR_23432"sv};
    for (auto input : inputs) {
        const auto ts = tok_list(input);
        REQUIRE(ranges::size(ts) == 1);
        const auto t = ranges::front(ts);
        REQUIRE(std::holds_alternative<identifier_t>(t.token));
        REQUIRE(std::get<identifier_t>(t.token).name == input);
    }
}

TEST_CASE("Tokenize: inline colon (no brackets generated)", "[lexer/tokenize]") {
    const auto expected = token_list_t{META, COLON, VERBATIM};
    REQUIRE(tok_list("meta:verbatim"sv) == expected);
    REQUIRE(tok_list("meta : verbatim"sv) == expected);
    REQUIRE(tok_list("meta: verbatim"sv) == expected);
}

TEST_CASE("Tokenize: end-of-line colon (brackets generated)", "[lexer/tokenize]") {
    const auto expected = token_list_t{META, OPAREN, VERBATIM, CPAREN};
    REQUIRE(tok_list("meta:\n    verbatim"sv) == expected);
}

TEST_CASE("Tokenize: line extension (no brackets generated)", "[lexer/tokenize]") {
    const auto expected = token_list_t{
        META,
        VERBATIM,
    };
    REQUIRE(tok_list("meta\n    verbatim"sv) == expected);
}

TEST_CASE("Tokenize: integers.", "[lexer/tokenize]") {
    using namespace literal::numeric;
    REQUIRE(tok_list("42"sv) == token_list_t{token_t(integral_t(42, 'i', 64))});

    REQUIRE(tok_list("42i64"sv) == token_list_t{token_t(integral_t(42, 'i', 64))});
    REQUIRE(tok_list("42u64"sv) == token_list_t{token_t(integral_t(42, 'u', 64))});
    REQUIRE(tok_list("42i32"sv) == token_list_t{token_t(integral_t(42, 'i', 32))});
    REQUIRE(tok_list("42u32"sv) == token_list_t{token_t(integral_t(42, 'u', 32))});
    REQUIRE(tok_list("42i16"sv) == token_list_t{token_t(integral_t(42, 'i', 16))});
    REQUIRE(tok_list("42u16"sv) == token_list_t{token_t(integral_t(42, 'u', 16))});
    REQUIRE(tok_list("42i8"sv) == token_list_t{token_t(integral_t(42, 'i', 8))});
    REQUIRE(tok_list("42u8"sv) == token_list_t{token_t(integral_t(42, 'u', 8))});
}

TEST_CASE("Tokenize: floats.", "[lexer/tokenize]") {
    using namespace literal::numeric;
    REQUIRE(tok_list("42.0"sv) == token_list_t{token_t(floating_point_t(42, 64))});
    REQUIRE(tok_list("42e0"sv) == token_list_t{token_t(floating_point_t(42, 64))});
    REQUIRE(tok_list("42e1"sv) == token_list_t{token_t(floating_point_t(420, 64))});

    REQUIRE(tok_list("42.0f64"sv) == token_list_t{token_t(floating_point_t(42, 64))});
    REQUIRE(tok_list("42e0f64"sv) == token_list_t{token_t(floating_point_t(42, 64))});
    REQUIRE(tok_list("42e1f64"sv) == token_list_t{token_t(floating_point_t(420, 64))});

    REQUIRE(tok_list("42.0f32"sv) == token_list_t{token_t(floating_point_t(42, 32))});
    REQUIRE(tok_list("42e0f32"sv) == token_list_t{token_t(floating_point_t(42, 32))});
    REQUIRE(tok_list("42e1f32"sv) == token_list_t{token_t(floating_point_t(420, 32))});
}

TEST_CASE("Tokenize: strings.", "[lexer/tokenize]") {
    REQUIRE(tok_list(R"bt("")bt"sv) == token_list_t{token_t(string_token_t(""))});
    REQUIRE(tok_list(R"bt("this is a test")bt"sv) ==
            token_list_t{token_t(string_token_t("this is a test"))});
    REQUIRE(tok_list(R"bt("this \"is\" a test")bt"sv) ==
            token_list_t{token_t(string_token_t(R"raw(this "is" a test)raw"))});

    REQUIRE(tok_list(R"bt("backslash? \\")bt"sv) ==
            token_list_t{token_t(string_token_t(R"raw(backslash? \)raw"))});

    REQUIRE(tok_list(R"bt("newline? \nfoo")bt"sv) ==
            token_list_t{token_t(string_token_t("newline? \nfoo"))});

    REQUIRE(tok_list(R"bt("tab? \tfoo")bt"sv) ==
            token_list_t{token_t(string_token_t("tab? \tfoo"))});
}

TEST_CASE("Tokenize: random shit.", "[lexer/tokenize]") {
    const auto input = R"bt(
        foo:
            print(bar)
            print:bar

            verbatim

        x = foo
        print(x + 
            x)

        meta
    )bt"sv;
    const auto ts = tok_list(input);
    auto u = std::stringstream();
    u << ts;
    const auto expected =
        "[ident[foo], token[OPAREN], ident[print], token[OPAREN], ident[bar], "
        "token[CPAREN], token[LINE_END], ident[print], token[COLON], ident[bar], "
        "token[LINE_END], token[VERBATIM], token[CPAREN], token[LINE_END], "
        "ident[x], token[ASSIGN], ident[foo], token[LINE_END], ident[print], "
        "token[OPAREN], ident[x], token[PLUS], ident[x], token[CPAREN], "
        "token[LINE_END], token[META]]";

    REQUIRE(u.str() == expected);
}

TEST_CASE("Tokenize: inline comments.", "[lexer/tokenize]") {
    const auto input = R"bt(
        verbatim -- this is a comment
    )bt"sv;
    const auto ts = tok_list(input);
    cout << "TEST WHITESPACE: " << ts << endl;
    REQUIRE(ranges::size(ts) == 1);
    REQUIRE(ranges::front(ts) == VERBATIM);
}

TEST_CASE("Tokenize: token positions.", "[lexer/tokenize]") {
    const auto input = R"bt(
        foo:
            verbatim
        23.5f64
        "hello"
    )bt"sv;
    const auto output = input | tokenize;
    for (const auto& t : output.tokens) cout << t.token << " at " << t.location << endl;
}
