#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <variant>

#include <boost/hana/all.hpp>

#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast_fwd.hpp>
#include <bullet/parser/location.hpp>
#include <bullet/util.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            struct unary_op_t {
                lexer::token_t op;
                attr_node_t<Attr> operand;
                auto operator<=>(const unary_op_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const unary_op_t<Attr>& uop) -> std::ostream& {
                auto first = false;
                os << "unary_op[" << uop.op << ", " << uop.operand << "]";
                return os;
            }
        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
