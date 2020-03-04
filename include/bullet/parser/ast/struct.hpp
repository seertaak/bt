#pragma once

#include <iostream>

#include <bullet/parser/ast_fwd.hpp>
#include <bullet/parser/ast/named_group.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            struct struct_t : named_tree_vector_t<Attr> {
                using base_t = named_tree_vector_t<Attr>;
                using base_t::base_t;
                Attr attribute;
                auto operator<=>(const struct_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const struct_t<Attr>& t) -> std::ostream& {
                auto first = true;
                os << "struct[";
                for (const auto& [ident, subtree] : t) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << ident << ": " << subtree;
                }
                os << "]";
                return os;
            }
        }}} 
