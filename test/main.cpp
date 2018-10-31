#define CATCH_CONFIG_MAIN

#include <exception>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/hana.hpp>
#include <boost/hof.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <boost/variant.hpp>
#include <catch.hpp>
#include <range/v3/core.hpp>

#include <bullet/util.hpp>

using namespace std;

namespace hana = boost::hana;
namespace r = ranges;

using namespace hana::literals;

using r::back;
using r::front;

TEST_CASE("Hello", "[hello]") {
    REQUIRE(1 == 1);	// yup
}

namespace lexer {
    namespace token {
        struct token_tag {};

        struct semicolon_t : token_tag {
            static constexpr auto name = "semicolon"_s;
            static constexpr auto token = ";"_s;
        } semicolon;

        struct oparen_t : token_tag {
            static constexpr auto name = "oparen"_s;
            static constexpr auto token = "("_s;
        } oparen;

        struct cparen_t : token_tag {
            static constexpr auto name = "cparen"_s;
            static constexpr auto token = ")"_s;
        } cparen;

        template <typename T>
        auto operator<<(ostream& os, T) -> enable_if_t<is_base_of_v<token_tag, T>, ostream&> {
            os << T::name.c_str();
            return os;
        }

        template <typename T>
        auto operator==(T, T) -> enable_if_t<is_base_of_v<token_tag, T>, bool> {
            return true;
        }

        template <typename T>
        auto operator!=(T, T) -> enable_if_t<is_base_of_v<token_tag, T>, bool> {
            return false;
        }

        template <typename T, typename U>
        auto operator==(T, U)
            -> enable_if_t<is_base_of_v<token_tag, T> || is_base_of_v<token_tag, U>, bool> {
            return false;
        }

        template <typename T, typename U>
        auto operator!=(T, U)
            -> enable_if_t<is_base_of_v<token_tag, T> || is_base_of_v<token_tag, U>, bool> {
            return true;
        }
    }  // namespace token

    struct identifier_t {
        string name;
        identifier_t(string s) : name(s) {}
    };

    using token_t = variant<token::oparen_t, token::cparen_t, token::semicolon_t, string>;
    auto operator<<(ostream& os, const token_t& t) -> ostream& {
        visit([&](auto t) { os << t; }, t);
        return os;
    }

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(
        tokenize) = boost::hof::pipable([](string_view input) -> vector<token_t> {
        using namespace boost::spirit;
        using x3::alnum;
        using x3::alpha;
        using x3::attr;
        using x3::double_;
        using x3::eoi;
        using x3::eol;
        using x3::eps;
        using x3::int_;
        using x3::lexeme;
        using x3::lit;
        using x3::no_skip;
        using x3::omit;
        using x3::raw;
        using x3::ascii::char_;
        using x3::ascii::space;

        using namespace token;

        auto margins = vector<pair<int16_t, bool>>{{0, true}};
        auto colon_indent = false;
        auto b_ws = begin(input);

        const auto on_margin_begin = [&](auto& ctx) {
            _val(ctx) = {};
            b_ws = begin(_where(ctx));
        };

        const auto on_colon = [&](auto& ctx) { colon_indent = true; };

        const auto on_margin_end = [&margins, &colon_indent, &b_ws, input](auto& ctx) {
            const auto e_ws = begin(_where(ctx));

            const int n = e_ws - b_ws;
            const auto [margin, real_indent] = back(margins);

            if (n == margin) {
                if (colon_indent) throw runtime_error("Indent expected");
                if (real_indent) _val(ctx).push_back(semicolon);
            } else if (n > margin) {
                if (colon_indent) _val(ctx).push_back(oparen);
                margins.emplace_back(n, colon_indent);
            } else {
                if (colon_indent) throw runtime_error("Indent expected");

                while (!empty(margins) && n < get<int16_t>(back(margins))) {
                    if (get<bool>(back(margins))) _val(ctx).push_back(token_t(cparen));

                    _val(ctx).push_back(token_t(semicolon));

                    margins.pop_back();
                }
            }

            colon_indent = false;
        };

        auto margin = x3::rule<class margin_type, vector<token_t>>("margin") =
            no_skip[eps[on_margin_begin] >> (*lit(' '))[on_margin_end]];

        auto identifier = x3::rule<class identifier_type, string>("identifier") =
            lexeme[(alpha | char_('_')) >> *(alnum | char_('_'))];

        constexpr auto token = [](auto t) {
        return lit(decltype(t)::token.c_str()) >> attr(t); };

        const auto tokens = *(identifier | token(oparen) | token(cparen));
        const auto line = margin >> tokens;
        const auto lines = line % (-(lit(':')[on_colon]) >> eol);

        auto result = vector<token_t>();
        auto i = begin(input);

        if (!phrase_parse(i, end(input), lines, lit(' '), result))
            throw runtime_error("Unable to parse.");

        return result;
    });
}  // namespace lexer

TEST_CASE("Indented line", "[parser]") {
    const auto input = R"(
foo:
   bar 
   baz
bang
)"sv;

    using namespace lexer;
    using namespace lexer::token;

    // clang-format off
    const auto expected = vector<token_t>{
        {semicolon},
        {semicolon},
        {"foo"},
        {oparen},
        {"bar"},
        {semicolon},
        {"baz"},
        {cparen},
        {semicolon},
        {"bang"},
        {semicolon},
    };
    // clang-format on

    const auto output = input | tokenize;
    REQUIRE(expected == output);
}

TEST_CASE("Extended line", "[parser]") {
    const auto input = R"(
foo
   bar 
   baz
bang
)"sv;

    using namespace lexer;
    using namespace lexer::token;

    // clang-format off
    const auto expected = vector<token_t>{
        {semicolon},
        {semicolon},
        {"foo"},
        {"bar"},
        {"baz"},
        {semicolon},
        {"bang"},
        {semicolon},
    };
    // clang-format on

    const auto output = input | tokenize;
    REQUIRE(expected == output);
}

TEST_CASE("Extended line", "[parser]") {
    const auto input = R"(
foo
   bar 
   baz
bang
)"sv;

    using namespace lexer;
    using namespace lexer::token;

    // clang-format off
    const auto expected = vector<token_t>{
        {semicolon},
        {semicolon},
        {"foo"},
        {"bar"},
        {"baz"},
        {semicolon},
        {"bang"},
        {semicolon},
    };
    // clang-format on

    const auto output = input | tokenize;
    REQUIRE(expected == output);
}
