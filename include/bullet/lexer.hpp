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

#include <bullet/token.hpp>
#include <bullet/util.hpp>

namespace lexer {
    using namespace std;

    namespace hana = boost::hana;
    namespace r = ranges;

    using namespace hana::literals;

    using r::back;
    using r::front;

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(tokens1) =
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
                    if (real_indent) _val(ctx).push_back(EOL);
                } else if (n > margin) {
                    if (colon_indent) _val(ctx).push_back(INDENT);
                    margins.emplace_back(n, colon_indent);
                } else {
                    if (colon_indent) throw runtime_error("Indent expected");

                    while (!empty(margins) && n < get<int16_t>(back(margins))) {
                        if (get<bool>(back(margins))) _val(ctx).push_back(DEDENT);

                        _val(ctx).push_back(EOL);

                        margins.pop_back();
                    }
                }

                colon_indent = false;
            };

            // Lexer grammar definition:

            const auto symbol = hana::fold(token::types, !eps, [](auto l, auto r) {
                using T = typename decltype(r)::type;
                const auto t = T{};

                return l | (lit(std::data(T::token)) >> attr(t));
            });

            // clang-format off

            auto margin = x3::rule<class margin_type, vector<token_t>>("margin") =
                no_skip[
                    eps [on_margin_begin] >> (*lit(' ')) [on_margin_end]
                ];

            auto identifier = x3::rule<class identifier_type, identifier_t>("identifier") =
                lexeme[
                    (alpha | char_('_')) >> *(alnum | char_('_'))
                ];

            const auto tokens = *(identifier | symbol);

            const auto line = margin >> tokens;
            const auto lines = line % ( -(lit(':') [on_colon]) >> x3::eol );

            // clang-format on

            // Now, we invoke the grammar on the input.

            auto result = vector<token_t>();
            auto i = begin(input);

            if (!phrase_parse(i, end(input), lines, lit(' '), result))
                throw runtime_error("Unable to parse.");

            return result;
        });

    namespace op {
        using namespace boost::spirit;

        template <typename TokenVariant>
        struct token_literal : x3::parser<token_literal<TokenVariant>> {
            using base_type = x3::parser<token_literal<TokenVariant>>;
            using attribute_type = TokenVariant;
            static bool const has_attribute = true;

            explicit token_literal(TokenVariant t) : literal(t) {}

            template <typename Iterator, typename Context, typename RContext, typename Attribute>
            bool parse(Iterator& first,
                       Iterator const& last,
                       Context const& context,
                       RContext& rcontext,
                       Attribute& attr) const {
                Iterator i = first;

                if (*i == literal) {
                    ++i;
                    attr = *first;
                    return true;
                }

                return false;
            }

            TokenVariant literal;
        };

    }  // namespace op

    constexpr auto tlit = [](auto tok) { return op::token_literal<decltype(tok)>(tok); };

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(parse_tokens) =
        boost::hof::pipable([](const auto& input, auto&& grammar) -> vector<token_t> {
            auto i = begin(input);
            auto output = vector<token_t>();
            output.reserve(size(input));

            if (!parse(i, end(input), grammar, output)) throw runtime_error("Unable to parse.");

            return output;
        });

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(tokens2) =
        boost::hof::pipable([](vector<token_t> input) -> vector<token_t> {
            using namespace boost::spirit;

            auto atom = x3::rule<class margin_type, vector<token_t>>("atom");

            /*
            const auto group = indent_group | brace_group | bracket_group | paren_group;

            const auto indent_group = t(indent) >> lines_group >> t(dedent);
            const auto lines_group = inline_group % t(eol);

            const auto brace_group = t(obrace) >> -inline_group >> t(cbrace);
            const auto bracket_group = t(obracket) >> -inline_group >> t(cbracket);
            const auto paren_group = t(oparen) >> -inline_group >> t(cparen);

            const auto inline_group = semicolon_group >> -(t(colon) >> +inline_group);
            const auto semicolon_group = comma_group % t(semicolon);
            const auto comma_group = atom % t(comma);

            const auto basic_token = tokens - delimiters - separators;

            const auto delimiters = t(oparen) | t(cparen) | t(obracket) | t(cbracket) |
            t(obrace) | t(cbrace) | t(indent) | t(dedent) | t(colon);

            const auto separators = (t(eol) | t(semicolon) | t(comma));

            atom = basic_token;  // | group;

            using namespace token;

            auto output = vector<token_t>();
            auto i = begin(input);

            if (!parse(i, end(input), atom, output)) throw runtime_error("Unable to parse.");
            */

            auto output = vector<token_t>();

            return output;
        });
    }  // namespace lexer

