#pragma once

#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

#include <boost/hana.hpp>

#include <bullet/identifier.hpp>

//-------------------------------------------
// Note: this file was generated based on:
//      cd tools
//      python3 gen_tokens.py
//
// Token table is stored in tools/tokens.csv.
//-------------------------------------------

namespace lexer {
    using namespace std;
    namespace hana = boost::hana;

    namespace token {
        struct token_tag : x3::position_tagged {};

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

        template <typename T>
        auto token_name(T) -> enable_if_t<is_base_of_v<token_tag, T>, string_view> {
            return T::name;
        }

        template <typename T>
        auto token_symbol(T) -> enable_if_t<is_base_of_v<token_tag, T>, string_view> {
            return T::token;
        }

        struct verbatim_t : token_tag {
            static constexpr const std::string_view name{"VERBATIM"};
            static constexpr const std::string_view token{"verbatim"};
        };
        struct private_t : token_tag {
            static constexpr const std::string_view name{"PRIVATE"};
            static constexpr const std::string_view token{"private"};
        };
        struct import_t : token_tag {
            static constexpr const std::string_view name{"IMPORT"};
            static constexpr const std::string_view token{"import"};
        };
        struct object_t : token_tag {
            static constexpr const std::string_view name{"OBJECT"};
            static constexpr const std::string_view token{"object"};
        };
        struct public_t : token_tag {
            static constexpr const std::string_view name{"PUBLIC"};
            static constexpr const std::string_view token{"public"};
        };
        struct repeat_t : token_tag {
            static constexpr const std::string_view name{"REPEAT"};
            static constexpr const std::string_view token{"repeat"};
        };
        struct break_t : token_tag {
            static constexpr const std::string_view name{"BREAK"};
            static constexpr const std::string_view token{"break"};
        };
        struct catch_t : token_tag {
            static constexpr const std::string_view name{"CATCH"};
            static constexpr const std::string_view token{"catch"};
        };
        struct const_t : token_tag {
            static constexpr const std::string_view name{"CONST"};
            static constexpr const std::string_view token{"const"};
        };
        struct macro_t : token_tag {
            static constexpr const std::string_view name{"MACRO"};
            static constexpr const std::string_view token{"macro"};
        };
        struct throw_t : token_tag {
            static constexpr const std::string_view name{"THROW"};
            static constexpr const std::string_view token{"throw"};
        };
        struct until_t : token_tag {
            static constexpr const std::string_view name{"UNTIL"};
            static constexpr const std::string_view token{"until"};
        };
        struct while_t : token_tag {
            static constexpr const std::string_view name{"WHILE"};
            static constexpr const std::string_view token{"while"};
        };
        struct case_t : token_tag {
            static constexpr const std::string_view name{"CASE"};
            static constexpr const std::string_view token{"case"};
        };
        struct data_t : token_tag {
            static constexpr const std::string_view name{"DATA"};
            static constexpr const std::string_view token{"data"};
        };
        struct goto_t : token_tag {
            static constexpr const std::string_view name{"GOTO"};
            static constexpr const std::string_view token{"goto"};
        };
        struct help_t : token_tag {
            static constexpr const std::string_view name{"HELP"};
            static constexpr const std::string_view token{"help"};
        };
        struct meta_t : token_tag {
            static constexpr const std::string_view name{"META"};
            static constexpr const std::string_view token{"meta"};
        };
        struct note_t : token_tag {
            static constexpr const std::string_view name{"NOTE"};
            static constexpr const std::string_view token{"note"};
        };
        struct post_t : token_tag {
            static constexpr const std::string_view name{"POST"};
            static constexpr const std::string_view token{"post"};
        };
        struct type_t : token_tag {
            static constexpr const std::string_view name{"TYPE"};
            static constexpr const std::string_view token{"type"};
        };
        struct def_t : token_tag {
            static constexpr const std::string_view name{"DEF"};
            static constexpr const std::string_view token{"def"};
        };
        struct doc_t : token_tag {
            static constexpr const std::string_view name{"DOC"};
            static constexpr const std::string_view token{"doc"};
        };
        struct for_t : token_tag {
            static constexpr const std::string_view name{"FOR"};
            static constexpr const std::string_view token{"for"};
        };
        struct pre_t : token_tag {
            static constexpr const std::string_view name{"PRE"};
            static constexpr const std::string_view token{"pre"};
        };
        struct var_t : token_tag {
            static constexpr const std::string_view name{"VAR"};
            static constexpr const std::string_view token{"var"};
        };
        struct backslash_t : token_tag {
            static constexpr const std::string_view name{"BACKSLASH"};
            static constexpr const std::string_view token{"\\"};
        };
        struct equal_t : token_tag {
            static constexpr const std::string_view name{"EQUAL"};
            static constexpr const std::string_view token{"=="};
        };
        struct fn_t : token_tag {
            static constexpr const std::string_view name{"FN"};
            static constexpr const std::string_view token{"fn"};
        };
        struct geq_t : token_tag {
            static constexpr const std::string_view name{"GEQ"};
            static constexpr const std::string_view token{">="};
        };
        struct hat_equal_t : token_tag {
            static constexpr const std::string_view name{"HAT_EQUAL"};
            static constexpr const std::string_view token{"^="};
        };
        struct if_t : token_tag {
            static constexpr const std::string_view name{"IF"};
            static constexpr const std::string_view token{"if"};
        };
        struct in_t : token_tag {
            static constexpr const std::string_view name{"IN"};
            static constexpr const std::string_view token{"in"};
        };
        struct leq_t : token_tag {
            static constexpr const std::string_view name{"LEQ"};
            static constexpr const std::string_view token{"<="};
        };
        struct minus_equal_t : token_tag {
            static constexpr const std::string_view name{"MINUS_EQUAL"};
            static constexpr const std::string_view token{"-="};
        };
        struct percentage_equal_t : token_tag {
            static constexpr const std::string_view name{"PERCENTAGE_EQUAL"};
            static constexpr const std::string_view token{"%="};
        };
        struct plus_equal_t : token_tag {
            static constexpr const std::string_view name{"PLUS_EQUAL"};
            static constexpr const std::string_view token{"+="};
        };
        struct slash_equal_t : token_tag {
            static constexpr const std::string_view name{"SLASH_EQUAL"};
            static constexpr const std::string_view token{"/="};
        };
        struct star_equal_t : token_tag {
            static constexpr const std::string_view name{"STAR_EQUAL"};
            static constexpr const std::string_view token{"*="};
        };
        struct thick_arrow_t : token_tag {
            static constexpr const std::string_view name{"THICK_ARROW"};
            static constexpr const std::string_view token{"=>"};
        };
        struct thin_arrow_t : token_tag {
            static constexpr const std::string_view name{"THIN_ARROW"};
            static constexpr const std::string_view token{"->"};
        };
        struct ampersand_t : token_tag {
            static constexpr const std::string_view name{"AMPERSAND"};
            static constexpr const std::string_view token{"!"};
        };
        struct assign_t : token_tag {
            static constexpr const std::string_view name{"ASSIGN"};
            static constexpr const std::string_view token{"="};
        };
        struct atsign_t : token_tag {
            static constexpr const std::string_view name{"ATSIGN"};
            static constexpr const std::string_view token{"@"};
        };
        struct backtick_t : token_tag {
            static constexpr const std::string_view name{"BACKTICK"};
            static constexpr const std::string_view token{"`"};
        };
        struct bang_t : token_tag {
            static constexpr const std::string_view name{"BANG"};
            static constexpr const std::string_view token{"!"};
        };
        struct bar_t : token_tag {
            static constexpr const std::string_view name{"BAR"};
            static constexpr const std::string_view token{"|"};
        };
        struct cbraces_t : token_tag {
            static constexpr const std::string_view name{"CBRACES"};
            static constexpr const std::string_view token{"}"};
        };
        struct cbracket_t : token_tag {
            static constexpr const std::string_view name{"CBRACKET"};
            static constexpr const std::string_view token{"]"};
        };
        struct colon_t : token_tag {
            static constexpr const std::string_view name{"COLON"};
            static constexpr const std::string_view token{":"};
        };
        struct cparen_t : token_tag {
            static constexpr const std::string_view name{"CPAREN"};
            static constexpr const std::string_view token{")"};
        };
        struct dollar_t : token_tag {
            static constexpr const std::string_view name{"DOLLAR"};
            static constexpr const std::string_view token{"$"};
        };
        struct dot_t : token_tag {
            static constexpr const std::string_view name{"DOT"};
            static constexpr const std::string_view token{"."};
        };
        struct gt_t : token_tag {
            static constexpr const std::string_view name{"GT"};
            static constexpr const std::string_view token{">"};
        };
        struct hash_t : token_tag {
            static constexpr const std::string_view name{"HASH"};
            static constexpr const std::string_view token{"#"};
        };
        struct hat_t : token_tag {
            static constexpr const std::string_view name{"HAT"};
            static constexpr const std::string_view token{"^"};
        };
        struct lt_t : token_tag {
            static constexpr const std::string_view name{"LT"};
            static constexpr const std::string_view token{"<"};
        };
        struct minus_t : token_tag {
            static constexpr const std::string_view name{"MINUS"};
            static constexpr const std::string_view token{"-"};
        };
        struct obraces_t : token_tag {
            static constexpr const std::string_view name{"OBRACES"};
            static constexpr const std::string_view token{"{"};
        };
        struct obracket_t : token_tag {
            static constexpr const std::string_view name{"OBRACKET"};
            static constexpr const std::string_view token{"["};
        };
        struct oparen_t : token_tag {
            static constexpr const std::string_view name{"OPAREN"};
            static constexpr const std::string_view token{"("};
        };
        struct percentage_t : token_tag {
            static constexpr const std::string_view name{"PERCENTAGE"};
            static constexpr const std::string_view token{"%"};
        };
        struct plus_t : token_tag {
            static constexpr const std::string_view name{"PLUS"};
            static constexpr const std::string_view token{"+"};
        };
        struct question_mark_t : token_tag {
            static constexpr const std::string_view name{"QUESTION_MARK"};
            static constexpr const std::string_view token{"?"};
        };
        struct slash_t : token_tag {
            static constexpr const std::string_view name{"SLASH"};
            static constexpr const std::string_view token{"/"};
        };
        struct star_t : token_tag {
            static constexpr const std::string_view name{"STAR"};
            static constexpr const std::string_view token{"*"};
        };
        struct tilde_t : token_tag {
            static constexpr const std::string_view name{"TILDE"};
            static constexpr const std::string_view token{"~"};
        };
        struct comma_t : token_tag {
            static constexpr const std::string_view name{"COMMA"};
            static constexpr const std::string_view token{""};
        };
        struct dedent_t : token_tag {
            static constexpr const std::string_view name{"DEDENT"};
            static constexpr const std::string_view token{""};
        };
        struct eol_t : token_tag {
            static constexpr const std::string_view name{"EOL"};
            static constexpr const std::string_view token{""};
        };
        struct indent_t : token_tag {
            static constexpr const std::string_view name{"INDENT"};
            static constexpr const std::string_view token{""};
        };
        struct semicolon_t : token_tag {
            static constexpr const std::string_view name{"SEMICOLON"};
            static constexpr const std::string_view token{""};
        };

        constexpr auto types = hana::tuple_t<verbatim_t,
                                             private_t,
                                             import_t,
                                             object_t,
                                             public_t,
                                             repeat_t,
                                             break_t,
                                             catch_t,
                                             const_t,
                                             macro_t,
                                             throw_t,
                                             until_t,
                                             while_t,
                                             case_t,
                                             data_t,
                                             goto_t,
                                             help_t,
                                             meta_t,
                                             note_t,
                                             post_t,
                                             type_t,
                                             def_t,
                                             doc_t,
                                             for_t,
                                             pre_t,
                                             var_t,
                                             backslash_t,
                                             equal_t,
                                             fn_t,
                                             geq_t,
                                             hat_equal_t,
                                             if_t,
                                             in_t,
                                             leq_t,
                                             minus_equal_t,
                                             percentage_equal_t,
                                             plus_equal_t,
                                             slash_equal_t,
                                             star_equal_t,
                                             thick_arrow_t,
                                             thin_arrow_t,
                                             ampersand_t,
                                             assign_t,
                                             atsign_t,
                                             backtick_t,
                                             bang_t,
                                             bar_t,
                                             cbraces_t,
                                             cbracket_t,
                                             colon_t,
                                             cparen_t,
                                             dollar_t,
                                             dot_t,
                                             gt_t,
                                             hash_t,
                                             hat_t,
                                             lt_t,
                                             minus_t,
                                             obraces_t,
                                             obracket_t,
                                             oparen_t,
                                             percentage_t,
                                             plus_t,
                                             question_mark_t,
                                             slash_t,
                                             star_t,
                                             tilde_t,
                                             comma_t,
                                             dedent_t,
                                             eol_t,
                                             indent_t,
                                             semicolon_t>;
    }  // namespace token

    using token_t = variant<token::verbatim_t,
                            token::private_t,
                            token::import_t,
                            token::object_t,
                            token::public_t,
                            token::repeat_t,
                            token::break_t,
                            token::catch_t,
                            token::const_t,
                            token::macro_t,
                            token::throw_t,
                            token::until_t,
                            token::while_t,
                            token::case_t,
                            token::data_t,
                            token::goto_t,
                            token::help_t,
                            token::meta_t,
                            token::note_t,
                            token::post_t,
                            token::type_t,
                            token::def_t,
                            token::doc_t,
                            token::for_t,
                            token::pre_t,
                            token::var_t,
                            token::backslash_t,
                            token::equal_t,
                            token::fn_t,
                            token::geq_t,
                            token::hat_equal_t,
                            token::if_t,
                            token::in_t,
                            token::leq_t,
                            token::minus_equal_t,
                            token::percentage_equal_t,
                            token::plus_equal_t,
                            token::slash_equal_t,
                            token::star_equal_t,
                            token::thick_arrow_t,
                            token::thin_arrow_t,
                            token::ampersand_t,
                            token::assign_t,
                            token::atsign_t,
                            token::backtick_t,
                            token::bang_t,
                            token::bar_t,
                            token::cbraces_t,
                            token::cbracket_t,
                            token::colon_t,
                            token::cparen_t,
                            token::dollar_t,
                            token::dot_t,
                            token::gt_t,
                            token::hash_t,
                            token::hat_t,
                            token::lt_t,
                            token::minus_t,
                            token::obraces_t,
                            token::obracket_t,
                            token::oparen_t,
                            token::percentage_t,
                            token::plus_t,
                            token::question_mark_t,
                            token::slash_t,
                            token::star_t,
                            token::tilde_t,
                            token::comma_t,
                            token::dedent_t,
                            token::eol_t,
                            token::indent_t,
                            token::semicolon_t,
                            identifier_t>;

    auto operator<<(ostream& os, const token_t& t) -> ostream& {
        visit([&](auto t) { os << t; }, t);
        return os;
    }

    auto token_name(const token_t& t) -> string_view {
        return visit([&](auto t) { return token_name(t); }, t);
    }

    auto token_symbol(const token_t& t) -> string_view {
        return visit([&](auto t) { return token_symbol(t); }, t);
    }

    const token_t VERBATIM{token::verbatim_t{}};
    const token_t PRIVATE{token::private_t{}};
    const token_t IMPORT{token::import_t{}};
    const token_t OBJECT{token::object_t{}};
    const token_t PUBLIC{token::public_t{}};
    const token_t REPEAT{token::repeat_t{}};
    const token_t BREAK{token::break_t{}};
    const token_t CATCH{token::catch_t{}};
    const token_t CONST{token::const_t{}};
    const token_t MACRO{token::macro_t{}};
    const token_t THROW{token::throw_t{}};
    const token_t UNTIL{token::until_t{}};
    const token_t WHILE{token::while_t{}};
    const token_t CASE{token::case_t{}};
    const token_t DATA{token::data_t{}};
    const token_t GOTO{token::goto_t{}};
    const token_t HELP{token::help_t{}};
    const token_t META{token::meta_t{}};
    const token_t NOTE{token::note_t{}};
    const token_t POST{token::post_t{}};
    const token_t TYPE{token::type_t{}};
    const token_t DEF{token::def_t{}};
    const token_t DOC{token::doc_t{}};
    const token_t FOR{token::for_t{}};
    const token_t PRE{token::pre_t{}};
    const token_t VAR{token::var_t{}};
    const token_t BACKSLASH{token::backslash_t{}};
    const token_t EQUAL{token::equal_t{}};
    const token_t FN{token::fn_t{}};
    const token_t GEQ{token::geq_t{}};
    const token_t HAT_EQUAL{token::hat_equal_t{}};
    const token_t IF{token::if_t{}};
    const token_t IN{token::in_t{}};
    const token_t LEQ{token::leq_t{}};
    const token_t MINUS_EQUAL{token::minus_equal_t{}};
    const token_t PERCENTAGE_EQUAL{token::percentage_equal_t{}};
    const token_t PLUS_EQUAL{token::plus_equal_t{}};
    const token_t SLASH_EQUAL{token::slash_equal_t{}};
    const token_t STAR_EQUAL{token::star_equal_t{}};
    const token_t THICK_ARROW{token::thick_arrow_t{}};
    const token_t THIN_ARROW{token::thin_arrow_t{}};
    const token_t AMPERSAND{token::ampersand_t{}};
    const token_t ASSIGN{token::assign_t{}};
    const token_t ATSIGN{token::atsign_t{}};
    const token_t BACKTICK{token::backtick_t{}};
    const token_t BANG{token::bang_t{}};
    const token_t BAR{token::bar_t{}};
    const token_t CBRACES{token::cbraces_t{}};
    const token_t CBRACKET{token::cbracket_t{}};
    const token_t COLON{token::colon_t{}};
    const token_t CPAREN{token::cparen_t{}};
    const token_t DOLLAR{token::dollar_t{}};
    const token_t DOT{token::dot_t{}};
    const token_t GT{token::gt_t{}};
    const token_t HASH{token::hash_t{}};
    const token_t HAT{token::hat_t{}};
    const token_t LT{token::lt_t{}};
    const token_t MINUS{token::minus_t{}};
    const token_t OBRACES{token::obraces_t{}};
    const token_t OBRACKET{token::obracket_t{}};
    const token_t OPAREN{token::oparen_t{}};
    const token_t PERCENTAGE{token::percentage_t{}};
    const token_t PLUS{token::plus_t{}};
    const token_t QUESTION_MARK{token::question_mark_t{}};
    const token_t SLASH{token::slash_t{}};
    const token_t STAR{token::star_t{}};
    const token_t TILDE{token::tilde_t{}};
    const token_t COMMA{token::comma_t{}};
    const token_t DEDENT{token::dedent_t{}};
    const token_t EOL{token::eol_t{}};
    const token_t INDENT{token::indent_t{}};
    const token_t SEMICOLON{token::semicolon_t{}};
}  // namespace lexer
