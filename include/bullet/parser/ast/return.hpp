#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt { namespace parser { namespace syntax {

    template <typename Attr>
    struct return_t {
        BOOST_HANA_DEFINE_STRUCT(return_t, (attr_node_t<Attr>, value));
        auto operator<=>(const return_t&) const = default;
    };

    template <typename Attr>
    auto operator<<(std::ostream& os, const return_t<Attr>& e) -> std::ostream& {
        if (e.value.get())
            os << "return[" << e.value.get() << "]";
        else
            os << "return[]";
        return os;
    }

}}}  // namespace bt::parser::syntax
