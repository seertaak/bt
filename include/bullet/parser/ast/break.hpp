#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>
#include <bullet/parser/ast/data.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct break_t {
                BOOST_HANA_DEFINE_STRUCT(break_t,
                    (Attr, attribute)
                );
                 auto operator<=>(const break_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const break_t<Attr>& e) -> std::ostream& {
                os << "break";
                return os;
            }

        }}}
