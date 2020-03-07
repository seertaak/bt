#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt { namespace parser { namespace syntax {

    template <typename Attr>
    struct type_expr_t {
        BOOST_HANA_DEFINE_STRUCT(type_expr_t, (attr_node_t<Attr>, type));
        auto operator<=>(const type_expr_t&) const = default;
    };

    template <typename Attr>
    auto operator<<(std::ostream& os, const type_expr_t<Attr>& e) -> std::ostream& {
        if (e.type.get())
            os << e.type.get();
        else
            os << "IMPLDEF";
        return os;
    }

}}}  // namespace bt::parser::syntax
