#pragma once

#include <iostream>

#include <bullet/parser/ast_fwd.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            struct block_t : std::vector<attr_node_t<Attr>> {
                using base_t = std::vector<attr_node_t<Attr>>;
                using base_t::base_t;
                auto operator<=>(const block_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const block_t<Attr>& data) -> std::ostream& {
                auto first = true;
                os << "block[";
                for (const auto& pt : data) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << pt;
                }
                os << "]";
                return os;
            }
        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
