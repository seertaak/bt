#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct for_t {
                BOOST_HANA_DEFINE_STRUCT(for_t,
                                         (lexer::identifier_t, var_lhs),
                                         (attr_node_t<Attr>, var_rhs),
                                         (attr_node_t<Attr>, body));
                auto operator<=>(const for_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const for_t<Attr>& f) -> std::ostream& {
                os << "for[var_lhs=" << f.var_lhs << ", var_rhs=" << f.var_rhs
                   << ", body=" << f.body << "]";
                return os;
            }

        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
