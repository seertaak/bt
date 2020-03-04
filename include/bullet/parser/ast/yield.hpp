#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct yield_t {
                BOOST_HANA_DEFINE_STRUCT(yield_t, (attr_node_t<Attr>, value), (Attr, attribute));
                auto operator<=>(const yield_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const yield_t<Attr>& e) -> std::ostream& {
                os << "yield[" << e.value.get() << "]";
                return os;
            }

        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
