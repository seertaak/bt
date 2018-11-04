#pragma once

#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

#include <bullet/util.hpp>

namespace lexer {
    using namespace std;

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
}  // namespace lexer


