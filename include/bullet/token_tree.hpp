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
    } tok_name

        def_token(eol, "EOL");
        def_token(indent, "INDENT");
        def_token(dedent, "DEDENT");
        def_token(import, "import");
        def_token(_public, "public");
        def_token(_private, "private");
        def_token(macro, "macro");
        def_token(help, "help");
        def_token(doc, "doc");
        def_token(pre, "pre");
        def_token(post, "post");
        def_token(meta, "meta");
        def_token(verbatim, "verbatim");
        def_token(note, "note");
        def_token(var, "var");
        def_token(data, "data");
        def_token(object, "object");
        def_token(_const, "const");
        def_token(type, "type");
        def_token(fn, "fn");
        def_token(def, "def");
        def_token(in, "in");
        def_token(_for, "for");
        def_token(_while, "while");
        def_token(repeat, "repeat");
        def_token(until, "until");
        def_token(_break, "break");
        def_token(_goto, "goto");
        def_token(_throw, "throw");
        def_token(_catch, "catch");
        def_token(_if, "if");
        def_token(_case, "case");
        def_token(plus_equal, "+=");
        def_token(minus_equal, "-=");
        def_token(star_equal, "*=");
        def_token(slash_equal, "/=");
        def_token(hat_equal, "^=");
        def_token(percentage_equal, "%=");
        def_token(leq, "<=");
        def_token(geq, ">=");
        def_token(equal, "==");
        def_token(thick_arrow, "=>");
        def_token(thin_arrow, "->");
        def_token(question_mark, "?");
        def_token(bar, "|");
        def_token(tilde, "~");
        def_token(ampersand, "!");
        def_token(bang, "!");
        def_token(dollar, "$");
        def_token(colon, ":");
        def_token(semicolon, ",");
        def_token(comma, ",");
        def_token(dot, ".");
        def_token(hash, "#");
        def_token(atsign, "@");
        def_token(backtick, "`");
        def_token(backslash, "\\");
        def_token(lt, "<");
        def_token(gt, ">");
        def_token(oparen, "(");
        def_token(cparen, ")");
        def_token(obracket, "[");
        def_token(cbracket, "]");
        def_token(obraces, "{");
        def_token(cbraces, "}");
        def_token(assign, "=");
        def_token(plus, "+");
        def_token(minus, "-");
        def_token(star, "*");
        def_token(slash, "/");
        def_token(hat, "^");
        def_token(percentage, "%");

#undef def_token
    }  // namespace token

    struct identifier_t {
        string name;
        explicit identifier_t(string s) : name(s) {}
    };

    using token_t = variant<token::eol_t,
                            token::indent_t,
                            token::dedent_t,
                            token::import_t,
                            token::_public_t,
                            token::_private_t,
                            token::macro_t,
                            token::help_t,
                            token::doc_t,
                            token::pre_t,
                            token::post_t,
                            token::meta_t,
                            token::verbatim_t,
                            token::note_t,
                            token::var_t,
                            token::data_t,
                            token::object_t,
                            token::_const_t,
                            token::type_t,
                            token::fn_t,
                            token::def_t,
                            token::in_t,
                            token::_for_t,
                            token::_while_t,
                            token::repeat_t,
                            token::until_t,
                            token::_break_t,
                            token::_goto_t,
                            token::_throw_t,
                            token::_catch_t,
                            token::_if_t,
                            token::_case_t,
                            token::plus_equal_t,
                            token::minus_equal_t,
                            token::star_equal_t,
                            token::slash_equal_t,
                            token::hat_equal_t,
                            token::percentage_equal_t,
                            token::leq_t,
                            token::geq_t,
                            token::equal_t,
                            token::thick_arrow_t,
                            token::thin_arrow_t,
                            token::question_mark_t,
                            token::bar_t,
                            token::tilde_t,
                            token::ampersand_t,
                            token::bang_t,
                            token::dollar_t,
                            token::colon_t,
                            token::semicolon_t,
                            token::comma_t,
                            token::dot_t,
                            token::hash_t,
                            token::atsign_t,
                            token::backtick_t,
                            token::backslash_t,
                            token::lt_t,
                            token::gt_t,
                            token::oparen_t,
                            token::cparen_t,
                            token::obracket_t,
                            token::cbracket_t,
                            token::obraces_t,
                            token::cbraces_t,
                            token::assign_t,
                            token::plus_t,
                            token::minus_t,
                            token::star_t,
                            token::slash_t,
                            token::hat_t,
                            token::percentage_t,
                            string>;

    auto operator<<(ostream& os, const token_t& t) -> ostream& {
        visit([&](auto t) { os << t; }, t);
        return os;
    }

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(
        tokens1) = boost::hof::pipable([](string_view input) -> vector<token_t> {
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

        // clang-format off

            auto margin = x3::rule<class margin_type, vector<token_t>>("margin") =
                no_skip[
                    eps [on_margin_begin] >> (*lit(' ')) [on_margin_end]
                ];

            auto identifier = x3::rule<class identifier_type, string>("identifier") =
                lexeme[
                    (alpha | char_('_')) >> *(alnum | char_('_'))
                ];

            constexpr auto token = [](auto t) {
                return lit(std::data(decltype(t)::token)) >> attr(t);
            };

            const auto tokens = *( identifier 
                                 | token(cbraces)
                                 | token(import)
                                 | token(_public)
                                 | token(_private)
                                 | token(macro)
                                 | token(help)
                                 | token(doc)
                                 | token(pre)
                                 | token(post)
                                 | token(meta)
                                 | token(verbatim)
                                 | token(note)
                                 | token(var)
                                 | token(data)
                                 | token(object)
                                 | token(_const)
                                 | token(type)
                                 | token(fn)
                                 | token(def)
                                 | token(in)
                                 | token(_for)
                                 | token(_while)
                                 | token(repeat)
                                 | token(until)
                                 | token(_break)
                                 | token(_goto)
                                 | token(_throw)
                                 | token(_catch)
                                 | token(_if)
                                 | token(_case)
                                 | token(plus_equal)
                                 | token(minus_equal)
                                 | token(star_equal)
                                 | token(slash_equal)
                                 | token(hat_equal)
                                 | token(percentage_equal)
                                 | token(leq)
                                 | token(geq)
                                 | token(equal)
                                 | token(thick_arrow)
                                 | token(thin_arrow)
                                 | token(question_mark)
                                 | token(bar)
                                 | token(tilde)
                                 | token(ampersand)
                                 | token(bang)
                                 | token(dollar)
                                 | (token(colon) >> &!x3::eol)
                                 | token(semicolon)
                                 | token(comma)
                                 | token(dot)
                                 | token(hash)
                                 | token(atsign)
                                 | token(backtick)
                                 | token(backslash)
                                 | token(lt)
                                 | token(gt)
                                 | token(oparen)
                                 | token(cparen)
                                 | token(obracket)
                                 | token(cbracket)
                                 | token(obraces)
                                 | token(cbraces)
                                 | token(assign)
                                 | token(plus)
                                 | token(minus)
                                 | token(star)
                                 | token(slash)
                                 | token(hat)
                                 | token(percentage)
                                 );

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

        template <typename TokenVariant, typename TokenValue>
        struct value : x3::parser<value<TokenVariant, TokenValue>> {
            using base_type = x3::parser<value<TokenVariant, TokenValue>>;
            using attribute_type = TokenVariant;
            static bool const has_attribute = true;

            template <typename Iterator, typename Context, typename RContext, typename Attribute>
            bool parse(Iterator& first,
                       Iterator const& last,
                       Context const& context,
                       RContext& rcontext,
                       Attribute& attr) const {
                Iterator i = first;

                if (holds_alternative<TokenValue>(*i)) {
                    ++i;
                    attr = *first;
                    return true;
                }

                return false;
            }
        };

    }  // namespace op

    template <typename T>
    const op::value<token_t, T> tk{};

    constexpr auto t = [](auto tok) { return tk<decltype(tok)>; };

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

            constexpr auto t = [](auto tok) { return tk<decltype(tok)>; };

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

            const auto delimiters = t(oparen) | t(cparen) | t(obracket) | t(cbracket) | t(obrace) |
                                    t(cbrace) | t(indent) | t(dedent) | t(colon);

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

