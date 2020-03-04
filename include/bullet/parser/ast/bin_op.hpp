#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <variant>

#include <boost/hana/all.hpp>

#include <bullet/lexer/token.hpp>
#include <bullet/util.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            struct bin_op_t {
                BOOST_HANA_DEFINE_STRUCT(bin_op_t,
                                         (lexer::token_t, op),
                                         (attr_node_t<Attr>, lhs),
                                         (attr_node_t<Attr>, rhs),
                                         (Attr, attribute));
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const bin_op_t<Attr>& binop) -> std::ostream& {
                auto first = true;
                os << "binary_op[" << binop.op << ", " << binop.lhs << ", " << binop.rhs << ", "
                   << binop.attribute << "]";
                return os;
            }

            template <typename Attr>
            auto operator==(const bin_op_t<Attr>& l, const bin_op_t<Attr>& r) -> bool {
                return l.op == r.op && l.lhs == r.lhs && l.rhs == r.rhs;
            }

            template <typename Attr>
            auto operator!=(const bin_op_t<Attr>& l, const bin_op_t<Attr>& r) -> bool {
                return !(l == r);
            }
        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
