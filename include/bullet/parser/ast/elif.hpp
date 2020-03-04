#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>
#include <bullet/parser/ast/data.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct elif_t {
                BOOST_HANA_DEFINE_STRUCT(elif_t,
                    (attr_node_t<Attr>, test),
                    (attr_node_t<Attr>, body),
                    (Attr, attribute)
                );
                 auto operator<=>(const elif_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const elif_t<Attr>& e) -> std::ostream& {
                os << "elif[test=" << e.test << ", body=" << e.body << "]";
                return os;
            }

        }}}
