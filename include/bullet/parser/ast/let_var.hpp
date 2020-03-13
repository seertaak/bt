#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/lexer/location.hpp>
#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast/type_expr.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt { namespace parser { namespace syntax {

    template <typename Attr>
    struct let_var_t {
        BOOST_HANA_DEFINE_STRUCT(let_var_t,
                                 (lexer::with_loc<lexer::identifier_t>, name),
                                 (attr_node_t<Attr>, type),
                                 (attr_node_t<Attr>, rhs));
        auto operator<=>(const let_var_t&) const = default;
    };

    template <typename Attr>
    auto operator<<(std::ostream& os, const let_var_t<Attr>& v) -> std::ostream& {
        os << "let[" << v.name;
        if (v.type.get()) os << ": " << v.type;
        os << " = " << v.rhs << "]";
        return os;
    }
}}}  // namespace bt::parser::syntax
