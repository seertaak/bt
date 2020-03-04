#pragma once

#include <iostream>
#include <tuple>
#include <vector>

#include <bullet/parser/ast_fwd.hpp>
#include <bullet/util.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            using named_node_t = std::pair<lexer::identifier_t, attr_node_t<Attr>>;

            template <typename Attr>
            using named_tree_vector_t = std::vector<named_node_t<Attr>>;

            template <typename Attr>
            struct named_group_t : named_tree_vector_t<Attr> {
                using base_t = named_tree_vector_t<Attr>;
                using base_t::base_t;
                Attr attribute;
                auto operator<=>(const named_group_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const named_group_t<Attr>& g) -> std::ostream& {
                auto first = true;
                os << "named_group[";
                for (const auto& [ident, subtree] : g) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << ident << ": " << subtree.get();
                }
                os << ", attr=" << g.attribute << "]";
                return os;
            }
        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
