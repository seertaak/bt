#pragma once

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
#include <boost/preprocessor.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <range/v3/core.hpp>

#include <bullet/util.hpp>

namespace lexer {
    using namespace std;

    namespace hana = boost::hana;
    namespace r = ranges;

    using namespace hana::literals;

    using r::back;
    using r::front;

    namespace token {
        struct token_tag {};

        template <typename T>
        auto operator<<(ostream& os, T) -> enable_if_t<is_base_of_v<token_tag, T>, ostream&> {
            os << T::name;
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

#define def_token(tok_name, tok)                                                    \
    struct BOOST_PP_CAT(tok_name, _t) : token_tag {                                 \
        static constexpr const std::string_view name{BOOST_PP_STRINGIZE(tok_name)}; \
        static constexpr const std::string_view token{tok};                         \
    } tok_name;

#define def_token_helper(_, __, x) def_token x

#define def_tokens(...) \
    BOOST_PP_SEQ_FOR_EACH(def_token_helper, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

        def_tokens((eol, "EOL"),
                   (indent, "INDENT"),
                   (dedent, "DEDENT"),
                   (import, "import"),
                   (public_, "public"),
                   (private_, "private"),
                   (macro, "macro"),
                   (help, "help"),
                   (doc, "doc"),
                   (pre, "pre"),
                   (post, "post"),
                   (meta, "meta"),
                   (verbatim, "verbatim"),
                   (note, "note"),
                   (var, "var"),
                   (data, "data"),
                   (object, "object"),
                   (const_, "const"),
                   (type, "type"),
                   (fn, "fn"),
                   (def, "def"),
                   (in, "in"),
                   (for_, "for"),
                   (while_, "while"),
                   (repeat, "repeat"),
                   (until, "until"),
                   (break_, "break"),
                   (goto_, "goto"),
                   (throw_, "throw"),
                   (catch_, "catch"),
                   (if_, "if"),
                   (case_, "case"));

        def_tokens((plus_equal, "+="),
                   (minus_equal, "-="),
                   (star_equal, "*="),
                   (slash_equal, "/="),
                   (hat_equal, "^="),
                   (percentage_equal, "%="),
                   (leq, "<="),
                   (geq, ">="),
                   (equal, "=="),
                   (thick_arrow, "=>"),
                   (thin_arrow, "->"),
                   (question_mark, "?"),
                   (bar, "|"),
                   (tilde, "~"),
                   (ampersand, "!"),
                   (bang, "!"),
                   (dollar, "$"),
                   (colon, ":"),
                   (semicolon, ","),
                   (comma, ","),
                   (dot, "."),
                   (hash, "#"),
                   (atsign, "@"),
                   (backtick, "`"),
                   (backslash, "\\"),
                   (lt, "<"),
                   (gt, ">"),
                   (oparen, "("),
                   (cparen, ")"),
                   (obracket, "["),
                   (cbracket, "]"),
                   (obraces, "{"),
                   (cbraces, "}"),
                   (assign, "="),
                   (plus, "+"),
                   (minus, "-"),
                   (star, "*"),
                   (slash, "/"),
                   (hat, "^"),
                   (percentage, "%"));

    }  // namespace token

    struct identifier_t {
        string name;
        explicit identifier_t(string s) : name(s) {}
    };

    using token_t = variant<token::oparen_t,
                            token::cparen_t,
                            token::semicolon_t,
                            token::eol_t,
                            token::indent_t,
                            token::dedent_t,
                            string>;

    auto operator<<(ostream& os, const token_t& t) -> ostream& {
        visit([&](auto t) { os << t; }, t);
        return os;
    }

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(tokens) =
        boost::hof::pipable([](string_view input) -> vector<token_t> {
            using namespace boost::spirit;
            using x3::alnum;
            using x3::alpha;
            using x3::attr;
            using x3::double_;
            using x3::eoi;
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

            // Below, some helpers to handle indent/dedent via margin stack.  Very similar to python
            // implementation, but in addition to storing the current margin x-pos, we also record
            // whether a given indent was preceeded by a colon (':'). If so, the indent is "real",
            // and we emit open and close parentheses. Otherwise, the new lines are considered to be
            // an extension of the previous.

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
                    if (real_indent) _val(ctx).emplace_back(eol);
                } else if (n > margin) {
                    if (colon_indent) _val(ctx).emplace_back(indent);
                    margins.emplace_back(n, colon_indent);
                } else {
                    if (colon_indent) throw runtime_error("Indent expected");

                    while (!empty(margins) && n < get<int16_t>(back(margins))) {
                        if (get<bool>(back(margins))) _val(ctx).emplace_back(dedent);

                        _val(ctx).push_back(token_t(eol));

                        margins.pop_back();
                    }
                }

                colon_indent = false;
            };

            // Lexer grammar definition:

            auto margin = x3::rule<class margin_type, vector<token_t>>("margin") =
                no_skip[eps[on_margin_begin] >> (*lit(' '))[on_margin_end]];

            auto identifier = x3::rule<class identifier_type, string>("identifier") =
                lexeme[(alpha | char_('_')) >> *(alnum | char_('_'))];

            constexpr auto token = [](auto t) {
                return lit(std::data(decltype(t)::token)) >> attr(t);
            };

            const auto tokens = *(identifier | token(oparen) | token(cparen));
            const auto line = margin >> tokens;
            const auto lines = line % (-(lit(':')[on_colon]) >> x3::eol);

            // Now, we invoke the grammar on the input.

            auto result = vector<token_t>();
            auto i = begin(input);

            if (!phrase_parse(i, end(input), lines, lit(' '), result))
                throw runtime_error("Unable to parse.");

            return result;
        });
}  // namespace lexer

