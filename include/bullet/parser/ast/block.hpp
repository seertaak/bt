#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct invoc_t {
                BOOST_HANA_DEFINE_STRUCT(invoc_t,
                    (node_t, target),
                    (data_t, arguments),
                    (Attr, attribute)
                );
                 auto operator<=>(const Point&) const = default;
            };

            template <typename Attr>
            auto operator<<(ostream& os, const invoc_t& invoc) -> ostream& {
                auto first = true;
                os << "invoke[" << invoc.target << "(";

                for (auto&& arg : invoc.arguments) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << arg;
                }
                os << "), " << invoc.attribute << "]";
                return os;
            }
        } }} 