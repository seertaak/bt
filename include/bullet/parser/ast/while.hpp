#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>

namespace bt { namespace parser { namespace syntax {

    template <typename Attr>
    struct while_t {
        BOOST_HANA_DEFINE_STRUCT(while_t, (attr_node_t<Attr>, test), (attr_node_t<Attr>, body));
        auto operator<=>(const while_t&) const = default;
    };

    template <typename Attr>
    auto operator<<(std::ostream& os, const while_t<Attr>& f) -> std::ostream& {
        os << "while[test=" << f.test << ", body=" << f.body << "]";
        return os;
    }

}}}  // namespace bt::parser::syntax
