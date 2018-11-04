#include <boost/preprocessor.hpp>

#define def_token_struct(tok_name, tok)                                             \
    struct BOOST_PP_CAT(tok_name, _t) : token_tag {                                 \
        static constexpr const std::string_view name{BOOST_PP_STRINGIZE(tok_name)}; \
        static constexpr const std::string_view token{tok};                         \
    } tok_name;
#define invoke_def_token_struct(_, __, x) def_token_struct x

#define def_token_var(tok_name, tok) const auto tok_name = token_t(BOOST_PP_CAT(tok_name, _t));
#define invoke_def_token_var(_, __, x) def_token_var x

#define def_tokens(...) \
    BOOST_PP_SEQ_FOR_EACH(invoke_def_token_struct, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

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
           (case_, "case"),
           (plus_equal, "+="),
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
           (obraces, "{"));
