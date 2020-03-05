#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt { namespace parser { namespace syntax {

    template <typename Attr>
    struct else_t {
        BOOST_HANA_DEFINE_STRUCT(else_t, (attr_node_t<Attr>, body));
        auto operator<=>(const else_t&) const = default;
    };

    template <typename Attr>
    auto operator<<(std::ostream& os, const else_t<Attr>& e) -> std::ostream& {
        os << "else[" << e.body << "]";
        return os;
    }

}}}  // namespace bt::parser::syntax
