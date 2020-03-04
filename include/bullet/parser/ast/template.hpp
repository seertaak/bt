#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast/named_group.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            struct template_t {
                BOOST_HANA_DEFINE_STRUCT(template_t,
                                         (named_group_t<Attr>, arguments),
                                         (attr_node_t<Attr>, body),
                                         (Attr, attribute));
                auto operator<=>(const template_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const template_t<Attr>& t) -> std::ostream& {
                os << "template[args=" << t.arguments << ", body=" << t << "]";
                return os;
            }

        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
