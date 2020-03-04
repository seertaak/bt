#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>
#include <bullet/parser/ast/data.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct assign_t {
                BOOST_HANA_DEFINE_STRUCT(assign_t,
                    (attr_node_t<Attr>, lhs),
                    (attr_node_t<Attr>, rhs)
                );
                 auto operator<=>(const assign_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const assign_t<Attr>& a) -> std::ostream& {
                os << "assign[" << a.lhs << ", " << a.rhs << "]";
                return os;
            }

        }}}
