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
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <range/v3/core.hpp>

#include <bullet/token.hpp>
#include <bullet/util.hpp>

namespace lexer {
    namespace x3 = boost::spirit::x3;
    using namespace std;

    namespace hana = boost::hana;
    namespace r = ranges;

    using namespace hana::literals;

    using r::back;
    using r::front;
    using std::begin;
    using std::end;
    using std::size;

    using x3::error_handler_tag;
    using error_handler_type = x3::error_handler<string_view::iterator>;

    using iterator = string_view::iterator;
    using const_iterator = string_view::const_iterator;

    struct output_t {
        source_token_list_t tokens;
        std::vector<uint32_t> eol_locations;
    };

    namespace {
        struct state_tag {};
        struct eol_locations_tag {};

        struct state_t {
            string_view input;
            vector<pair<int16_t, bool>> margins{{0, true}};
            bool colon_indent = false;
            const_iterator i_begin;
            const_iterator i_end;
            const_iterator b_ws;
            const_iterator i_colon;
            const_iterator i_line_end;
        };

        template <typename Context>
        auto _state(Context& ctx) -> state_t& {
            return x3::get<state_tag>(ctx).get();
        }

        template <typename Context>
        auto _eol_locations(Context& ctx) -> std::vector<uint32_t>& {
            return x3::get<eol_locations_tag>(ctx).get();
        }

        template <typename Context>
        auto _located(Context& ctx, token_t tok, const_iterator b, const_iterator e) -> source_token_t {
            cout << "located token begin: " << tok << endl;
            auto src_tok = source_token_t(tok);
            const auto curr_line_pos = _eol_locations(ctx).back();

            src_tok.location.line = _eol_locations(ctx).size() + 1;
            src_tok.location.first_col = static_cast<uint16_t>((b - _state(ctx).i_begin) - curr_line_pos + 1);
            src_tok.location.last_col = static_cast<uint16_t>((e - _state(ctx).i_begin) - curr_line_pos + 1);

            cout << "located token end: " << src_tok << endl;

            return src_tok;
        }

        template <typename Context>
        auto _located(Context& ctx, token_t t) -> source_token_t {
            const auto rng = _where(ctx);
            const auto b = std::begin(rng);
            const auto e = std::end(rng);

            return _located(ctx, t, b, e);
        }

        struct token_locator_t {
            template <typename Iterator, typename Context>
            inline void on_success(const Iterator& first, const Iterator& last
              , source_token_t& src_tok, Context const& context)
            {
                cout << "token_locator_t sees: " << src_tok << endl;
                src_tok = _located(context, src_tok.token);
                cout << "token_locator_t sees (end): " << src_tok << endl;
            }
        };
    }

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(
        tokenize) = boost::hof::pipable([](string_view input) -> output_t {
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

        auto state = state_t{
            .input = input, 
            .i_begin = std::begin(input), 
            .i_end = std::end(input), 
            .b_ws = std::end(input), 
            .i_line_end = std::end(input), 
            .i_colon = std::end(input)
        };
        auto output = output_t{};
        output.eol_locations.push_back(0);

        auto i = state.i_begin;
        auto error_handler = error_handler_type(i, state.i_end, std::cerr);

        const auto on_line_begin = [](auto& ctx) {
            _val(ctx) = {};
            _state(ctx).b_ws = begin(_where(ctx));
        };

        const auto on_line_end = [](auto& ctx) {
            auto& st = _state(ctx);

            const auto i = begin(_where(ctx));
            st.i_line_end = i;
            _eol_locations(ctx).push_back(i - st.i_begin);
        };

        const auto on_colon = [](auto& ctx) { 
            auto& st = _state(ctx);

            st.colon_indent = true; 
            st.i_colon = begin(_where(ctx));
        };

        const auto on_margin_end = [](auto& ctx) {
            auto& st = _state(ctx);
            const auto e_ws = begin(_where(ctx));

            const int n = e_ws - st.b_ws;
            const auto [margin, real_indent] = back(st.margins);

            const auto push_tok = [&] (auto tok, auto b, auto e) {
                _val(ctx).push_back(_located(ctx, tok, b, e));
            };

            if (n == margin) {
                if (st.colon_indent) throw runtime_error("Indent expected");
                if (st.b_ws != st.i_begin) push_tok(LINE_END, st.i_line_end, st.b_ws);
            } else if (n > margin) {
                if (st.colon_indent) push_tok(OPAREN, st.i_colon, st.i_line_end);
                st.margins.emplace_back(n, st.colon_indent);
            } else {
                if (st.colon_indent) throw runtime_error("Indent expected");

                while (!empty(st.margins) && n < get<int16_t>(back(st.margins))) {
                    if (get<bool>(back(st.margins))) push_tok(CPAREN, st.i_line_end, st.b_ws);

                    push_tok(LINE_END, st.i_line_end, st.b_ws);

                    st.margins.pop_back();
                }
            }

            st.colon_indent = false;
        };

        // Lexer grammar definition:

        // clang-format off

        auto margin = x3::rule<class margin_type, source_token_list_t>("margin") =
            no_skip[
                eps[on_line_begin] >> (*lit(' ')) [on_margin_end]
            ];

        const auto convert_to_identifier = [] (auto& ctx) {
            const string s = _attr(ctx);
            _val(ctx) = _located(ctx, token_t(identifier_t(s)));
        };

        auto identifier_pre 
            = x3::rule<class identifier_pre_type, string>("identifier_pre") 
            = lexeme[
                (alpha | char_('_')) >> *(alnum | char_('_'))
              ];

        struct identifier_type;
        auto identifier 
            = x3::rule<identifier_type, source_token_t>("identifier") 
            = identifier_pre[convert_to_identifier];
        struct identifier_type : token_locator_t {};

        constexpr auto token = [](auto t) {
            const auto r
                = x3::rule<struct _, source_token_t>("foo")
                = (
                       lit(std::data(token_symbol(t)))
                    >> attr(source_token_t(t))
                )/*[([](auto& ctx) {
                        _attr(ctx) = _located(ctx, _attr(ctx).token);
                        _val(ctx) = _attr(ctx);
                        cout << "PARSED: " << _attr(ctx) << " at " << 
                            _attr(ctx).location << "; value = " << 
                            _val(ctx) << " at " << _val(ctx).location << endl;
                })]*/;
            return r;
                /*
            return lit(std::data(token_symbol(t)))[([&] (auto& ctx) {
                        b = begin(_where(ctx));
                        e = end(_where(ctx));
                   })]
                >> attr(source_token_t(t))[([=](auto& ctx) {
                    cout << "PARSING: " << _attr(ctx) << " at " << 
                        _attr(ctx).location << endl;
                    _attr(ctx) = _located(ctx, _attr(ctx).token, b, e);
                    cout << "PARSING DONE: " << _attr(ctx) << " at " << 
                        _attr(ctx).location << endl;
                })];
                */
        };

        using namespace literal::numeric;

        struct naked_integral_type; 
        const auto naked_integral_token 
            = x3::rule<naked_integral_type, integral_t>("integral") 
            = x3::ulong_long >> attr('i') >> attr(64);
        struct naked_integral_type : token_locator_t {};

        const auto integral_token 
            = x3::rule<class integral_type, integral_t>("naked_integral") 
            = x3::no_skip[
                   x3::ulong_long >> (x3::char_('i') | x3::char_('u')) >> x3::int_
              ];
        
        struct naked_floating_point_type;
        const auto naked_floating_point_token 
            = x3::rule<naked_floating_point_type, floating_point_t>(
                    "naked_floating_point") 
            =  !x3::no_skip[x3::ulong_long >> (" " | x3::eol | x3::eoi)] 
            >>  x3::long_double 
            >>  attr(64);
        struct naked_floating_point_type : token_locator_t {};

        struct floating_point_type;
        const auto floating_point_token 
            = x3::rule<class floating_point_type, floating_point_t>("floating_point") 
            = x3::no_skip[x3::long_double >> "f" >> x3::int_];
        struct floating_point_type : token_locator_t {};
    
        auto unesc_char = x3::symbols<char>();
        unesc_char.add
            ("\\n", '\n') 
            ("\\t", '\t')
            ("\\\\", '\\') 
            ("\\\"", '\"');

        const auto regular_char = char_ - '"';
        const auto string_content
            = x3::rule<class string_content_type, string>("string_content") 
            = lexeme['"' >> *(unesc_char | regular_char) >> '"'];

        const auto convert_to_string_token = [] (auto& ctx) {
            _val(ctx) = _located(ctx, token_t(string_token_t(_attr(ctx))));
        };

        struct string_token_type;
        const auto string_token
            = x3::rule<string_token_type, source_token_t>("string_token") 
            = string_content[convert_to_string_token];
        struct string_token_type : token_locator_t {};

        struct basic_token_type;

        const auto tokens 
            = x3::rule<class basic_token_type, source_token_t>("basic_token") 
            =                   ( /*floating_point_token
                                | integral_token
                                | naked_floating_point_token
                                | naked_integral_token
                                | */token(VERBATIM)
                                | token(PRIVATE)
                                | token(IMPORT)
                                | token(OBJECT)
                                | token(PUBLIC)
                                | token(REPEAT)
                                | token(BREAK)
                                | token(CATCH)
                                | token(CONST)
                                | token(FALSE)
                                | token(MACRO)
                                | token(THROW)
                                | token(UNTIL)
                                | token(WHILE)
                                | token(CASE)
                                | token(DATA)
                                | token(GOTO)
                                | token(HELP)
                                | token(META)
                                | token(NOTE)
                                | token(NULL_LIT)
                                | token(POST)
                                | token(TRUE)
                                | token(TYPE)
                                | token(DEF)
                                | token(DOC)
                                | token(FOR)
                                | token(PRE)
                                | token(VAR)
                                | token(BACKSLASH)
                                | token(EQUAL)
                                | token(FN)
                                | token(GEQ)
                                | token(HAT_EQUAL)
                                | token(IF)
                                | token(IN)
                                | token(LEQ)
                                | token(MINUS_EQUAL)
                                | token(PERCENTAGE_EQUAL)
                                | token(PLUS_EQUAL)
                                | token(SLASH_EQUAL)
                                | token(STAR_EQUAL)
                                | token(THICK_ARROW)
                                | token(THIN_ARROW)
                                | token(AMPERSAND)
                                | token(ASSIGN)
                                | token(ATSIGN)
                                | token(BACKTICK)
                                | token(BANG)
                                | token(BAR)
                                | token(CBRACES)
                                | token(CBRACKET)
                                | (token(COLON) >> &!x3::eol)
                                | token(COMMA)
                                | token(CPAREN)
                                | token(DOLLAR)
                                | token(DOT)
                                | token(GT)
                                | token(HASH)
                                | token(HAT)
                                | token(LT)
                                | token(MINUS)
                                | token(OBRACES)
                                | token(OBRACKET)
                                | token(OPAREN)
                                | token(PERCENTAGE)
                                | token(PLUS)
                                | token(QUESTION_MARK)
                                | token(SEMICOLON)
                                | token(SLASH)
                                | token(STAR)
                                | token(TILDE)
                                /*| string_token
                                | identifier*/
                )[([] (auto& ctx) {
                    cout << "JUST PARSED tokens = ...: " << _attr(ctx) << endl;
                    //_val(ctx) = _attr(ctx);
                    cout << "JUST PARSED tokens = ...: " << _val(ctx) << endl;
                })];
                
        struct basic_token_type : token_locator_t {};

        // NB: we need to define a rule here in order to force the attribute type
        // of this parser to be vector<source_token_t> (even though it will always return
        // an empty vector, because it matches empty lines!).
        auto empty_line 
            = x3::rule<class empty_line_type, source_token_list_t>("empty_line") 
            = no_skip[*lit(' ')] >> x3::eol;

        const auto non_empty_line = (
                margin
            >> (+(tokens[([](auto& ctx){
                    cout << "JUST PARSED +tokens: " << _attr(ctx) << endl;
               })]))
            >> -lit(':')[on_colon] 
            >> x3::eol[on_line_end]
        );

        const auto lines = *(empty_line | non_empty_line);

        // clang-format on

        using x3::with;

        auto const tokenizer 
            = with<state_tag>(std::ref(state))[
                with<eol_locations_tag>(std::ref(output.eol_locations))[
                    with<error_handler_tag>(std::ref(error_handler))[
                        lines
                    ]
                ]
              ];

        if (!phrase_parse(i, std::end(input), tokenizer, lit(' '), output.tokens))
            throw runtime_error("Failed to tokenize.");

        while (size(state.margins) > 1) {
            if (get<bool>(back(state.margins))) {
                auto tok = source_token_t(CPAREN);
                tok.location.line = output.eol_locations.size() + 1;
                tok.location.first_col = static_cast<uint16_t>(input.size() - output.eol_locations.back());
                tok.location.last_col = tok.location.first_col;
                output.tokens.emplace_back(tok);
            }
            state.margins.pop_back();
        }

        cout << "DOUBLE CHECK: " << output.tokens << endl;

        return output;
    });

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(
        tokens) = boost::hof::pipable([](const output_t& output) -> const source_token_list_t& {
        return output.tokens;
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

    //    template <typename T>
    //    const op::value<token_t, T> tk{};
    //
    //    constexpr auto t = [](auto tok) { return tk<decltype(tok)>; };
    //
    //    BOOST_HOF_STATIC_LAMBDA_FUNCTION(parse_tokens) =
    //        boost::hof::pipable([](const auto& input, auto&& grammar) -> vector<token_t> {
    //            auto i = begin(input);
    //            auto output = vector<token_t>();
    //            output.reserve(size(input));
    //
    //            if (!parse(i, end(input), grammar, output)) throw runtime_error("Unable to
    //            parse.");
    //
    //            return output;
    //        });
    //
    //    BOOST_HOF_STATIC_LAMBDA_FUNCTION(tokens2) =
    //        boost::hof::pipable([](vector<token_t> input) -> vector<token_t> {
    //            using namespace boost::spirit;
    //            using x3::alnum;
    //            using x3::alpha;
    //            using x3::attr;
    //            using x3::double_;
    //            using x3::eoi;
    //            using x3::eps;
    //            using x3::int_;
    //            using x3::lexeme;
    //            using x3::lit;
    //            using x3::no_skip;
    //            using x3::omit;
    //            using x3::raw;
    //            using x3::ascii::char_;
    //            using x3::ascii::space;
    //
    //            using namespace token;
    //
    //            // Below, some helpers to handle indent/dedent via margin stack.  Very similar to
    //            python
    //            // implementation, but in addition to storing the current margin x-pos, we also
    //            record
    //            // whether a given indent was preceeded by a colon (':'). If so, the indent is
    //            "real",
    //            // and we emit open and close parentheses. Otherwise, the new lines are considered
    //            to be
    //            // an extension of the previous.
    //
    //            auto margins = vector<pair<int16_t, bool>>{{0, true}};
    //            auto colon_indent = false;
    //            auto b_ws = begin(input);
    //
    //            const auto on_margin_begin = [&](auto& ctx) {
    //                _val(ctx) = {};
    //                b_ws = begin(_where(ctx));
    //            };
    //
    //            const auto on_colon = [&](auto& ctx) { colon_indent = true; };
    //
    //            const auto on_margin_end = [&margins, &colon_indent, &b_ws, input](auto& ctx) {
    //                const auto e_ws = begin(_where(ctx));
    //
    //                const int n = e_ws - b_ws;
    //                const auto [margin, real_indent] = back(margins);
    //
    //                if (n == margin) {
    //                    if (colon_indent) throw runtime_error("Indent expected");
    //                    if (real_indent) _val(ctx).emplace_back(EOL);
    //                } else if (n > margin) {
    //                    if (colon_indent) _val(ctx).emplace_back(INDENT);
    //                    margins.emplace_back(n, colon_indent);
    //                } else {
    //                    if (colon_indent) throw runtime_error("Indent expected");
    //
    //                    while (!empty(margins) && n < get<int16_t>(back(margins))) {
    //                        if (get<bool>(back(margins))) _val(ctx).emplace_back(DEDENT);
    //
    //                        _val(ctx).push_back(token_t(EOL));
    //
    //                        margins.pop_back();
    //                    }
    //                }
    //
    //                colon_indent = false;
    //            };
    //
    //
    //            auto margin = x3::rule<class margin_type, vector<token_t>>("margin") =
    //                no_skip[
    //                    eps [on_margin_begin] >> (*lit(' ')) [on_margin_end]
    //                ];
    //
    //            auto identifier = x3::rule<class identifier_type, string>("identifier") =
    //                lexeme[
    //                    (alpha | char_('_')) >> *(alnum | char_('_'))
    //                ];
    //
    //            constexpr auto token = [](auto t) {
    //                return lit(std::data(token_name(t))) >> attr(t);
    //            };
    //
    //            const auto tokens = *( identifier
    //                                 | token(CBRACES)
    //                                 | token(IMPORT)
    //                                 | token(PUBLIC)
    //                                 | token(PRIVATE)
    //                                 | token(MACRO)
    //                                 | token(HELP)
    //                                 | token(DOC)
    //                                 | token(PRE)
    //                                 | token(POST)
    //                                 | token(META)
    //                                 | token(VERBATIM)
    //                                 | token(NOTE)
    //                                 | token(VAR)
    //                                 | token(DATA)
    //                                 | token(OBJECT)
    //                                 | token(CONST)
    //                                 | token(TYPE)
    //                                 | token(FN)
    //                                 | token(DEF)
    //                                 | token(IN)
    //                                 | token(FOR)
    //                                 | token(WHILE)
    //                                 | token(REPEAT)
    //                                 | token(UNTIL)
    //                                 | token(BREAK)
    //                                 | token(GOTO)
    //                                 | token(THROW)
    //                                 | token(CATCH)
    //                                 | token(IF)
    //                                 | token(CASE)
    //                                 | token(PLUS_EQUAL)
    //                                 | token(MINUS_EQUAL)
    //                                 | token(STAR_EQUAL)
    //                                 | token(SLASH_EQUAL)
    //                                 | token(HAT_EQUAL)
    //                                 | token(PERCENTAGE_EQUAL)
    //                                 | token(LEQ)
    //                                 | token(GEQ)
    //                                 | token(EQUAL)
    //                                 | token(THICK_ARROW)
    //                                 | token(THIN_ARROW)
    //                                 | token(QUESTION_MARK)
    //                                 | token(BAR)
    //                                 | token(TILDE)
    //                                 | token(AMPERSAND)
    //                                 | token(BANG)
    //                                 | token(DOLLAR)
    //                                 | (token(COLON) >> &!x3::eol)
    //                                 | token(SEMICOLON)
    //                                 | token(COMMA)
    //                                 | token(DOT)
    //                                 | token(HASH)
    //                                 | token(ATSIGN)
    //                                 | token(BACKTICK)
    //                                 | token(BACKSLASH)
    //                                 | token(LT)
    //                                 | token(GT)
    //                                 | token(OPAREN)
    //                                 | token(CPAREN)
    //                                 | token(OBRACKET)
    //                                 | token(CBRACKET)
    //                                 | token(OBRACES)
    //                                 | token(CBRACES)
    //                                 | token(ASSIGN)
    //                                 | token(PLUS)
    //                                 | token(MINUS)
    //                                 | token(STAR)
    //                                 | token(SLASH)
    //                                 | token(HAT)
    //                                 | token(PERCENTAGE)
    //                                 );
    //
    //
    //            constexpr auto t = [](auto tok) { return tk<decltype(tok)>; };
    //
    //            auto atom = x3::rule<class atom_type, vector<token_t>>("atom");
    //
    //            /*
    //            auto inline_group = x3::rule<class inline_group_type,
    //            vector<token_t>>("inline_group"); const auto lines_group = inline_group % t(EOL);
    //            const auto indent_group = t(INDENT) >> lines_group >> t(DEDENT);
    //            const auto comma_group = atom % t(COMMA);
    //            const auto semicolon_group = comma_group % t(SEMICOLON);
    //            inline_group = semicolon_group >> -(t(COLON) >> +inline_group);
    //
    //            const auto brace_group = t(OBRACES) >> -inline_group >> t(CBRACES);
    //            const auto bracket_group = t(OBRACKET) >> -inline_group >> t(CBRACKET);
    //            const auto paren_group = t(OPAREN) >> -inline_group >> t(CPAREN);
    //
    //            const auto group = indent_group | brace_group | bracket_group | paren_group;
    //
    //            */
    //            const auto delimiters = t(OPAREN) | t(CPAREN) | t(OBRACKET) | t(CBRACKET) |
    //            t(OBRACES) |
    //                                    t(CBRACES) | t(INDENT) | t(DEDENT) | t(COLON);
    //
    //            const auto separators = (t(EOL) | t(SEMICOLON) | t(COMMA));
    //
    //            const auto basic_token = tokens - delimiters - separators;
    //
    //            // atom = basic_token | group;
    //            const auto atom_def = basic_token;
    //
    //            BOOST_SPIRIT_DEFINE(atom);
    //
    //            using namespace token;
    //
    //            auto output = vector<token_t>();
    //            auto i = begin(input);
    //
    //            if (!parse(i, end(input), atom, output)) throw runtime_error("Unable to parse.");
    //
    //            return output;
    //        });
}  // namespace lexer
