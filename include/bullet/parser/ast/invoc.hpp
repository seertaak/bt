#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct invoc_t {
                BOOST_HANA_DEFINE_STRUCT(invoc_t,
                                         (attr_node_t<Attr>, target),
                                         (data_t<Attr>, arguments));
                auto operator<=>(const invoc_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const invoc_t<Attr>& invoc) -> std::ostream& {
                auto first = true;
                os << "invoke[" << invoc.target << "(";

                for (auto&& arg : invoc.arguments) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << arg;
                }
                os << ")]";
                return os;
            }
        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
