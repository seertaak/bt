#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>
#include <bullet/parser/ast/data.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct return_t {
                BOOST_HANA_DEFINE_STRUCT(return_t,
                    (attr_attr_node_t<Attr><Attr>, body),
                    (Attr, attribute)
                );
                 auto operator<=>(const return_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const return_t<Attr>& e) -> std::ostream& {
                if (t.value.get())
                    os << "return[" << t.value.get() << "]";
                else
                    os << "return[]";
                return os;
            }

        }}}
