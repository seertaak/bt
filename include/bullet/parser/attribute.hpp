#pragma once

#include <iostream>

namespace bt {
    namespace parser {
        struct empty_attribute_t {};

        inline auto operator<<(std::ostream& os, const empty_attribute_t& a) -> std::ostream& {
            os << "empty[]";
            return os;
        }

        template <typename Attr>
        struct attr_tree_t;

        template <typename Attr>
        struct attribute_t {
            Attr attribute;
        };

        template <typename Attr>
        auto operator<<(std::ostream& os, const attribute_t<Attr>& a) -> std::ostream& {
            os << "attr[" << a.attribute << "]";
            return os;
        }

        template <typename Attr>
        auto operator==(const attribute_t<Attr>& l, const attribute_t<Attr>& r) -> bool {
            return l.attribute == r.attribute;
        }

        template <typename Attr>
        auto operator!=(const attribute_t<Attr>& l, const attribute_t<Attr>& r) -> bool {
            return l.attribute != r.attribute;
        }
    }  // namespace parser
}  // namespace bt
