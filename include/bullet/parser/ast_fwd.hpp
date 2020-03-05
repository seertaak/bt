#pragma once

#include <bullet/parser/location.hpp>
#include <bullet/util.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            struct attr_tree_t;

            template <typename Attr>
            using attr_node_t = ref<attr_tree_t<Attr>>;

            template <typename Attr>
            struct with_attr {
                Attr attribute;
            };

        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
