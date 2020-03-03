#pragma once

#include <iostream>
#include <vector>
#include <tuple>

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
            struct named_group_t : named_tree_vector_t {
                using base_t = named_tree_vector_t;
                using base_t::base_t;
            };
             
            template <typename Attr>
            auto operator<<(ostream& os, const named_group_t<Attr>& g) -> ostream& {
                auto first = true;
                os << "named_group[";
                for (const auto& [ident, subtree] : g) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << ident << ": " << subtree.get();
                    os << ", " << subtree.get().attribute;
                }
                os << "]";
                return os;
            }
            template <typename Attr>
            auto operator==(const named_group_t<Attr>& l, const named_group_t<Attr>& r) -> bool {
                if (l.size() != r.size()) return false;
                for (auto i = 0; i < l.size(); i++)
                    if (l[i] != r[i]) return false;
                return true;
            }
            template <typename Attr>
            auto operator!=(const named_group_t& l, const named_group_t& r) -> bool {
                return !(l == r);
            }
     }
  }
}


