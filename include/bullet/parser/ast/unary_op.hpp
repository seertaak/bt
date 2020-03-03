#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <variant>
#include <sstream>

#include <boost/hana/all.hpp>

#include <bullet/parser/location.hpp>
#include <bullet/parser/ast_fwd.hpp>
#include <bullet/lexer/token.hpp>
#include <bullet/util.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            struct unary_op_t : location_t {
                lexer::token_t op;
                attr_node_t<Attr> operand;
                Attr attribute;
            };

            template <typename Attr>
            auto operator<<(ostream& os, const unary_op_t& uop) -> ostream& {
                auto first = false;
                os << "unary_op[" << uop.op << ", " << uop.operand 
                   << ", attribute=[" << uop.attribute << uop.location "]]";
                return os;
            }

            template <typename Attr>
            auto operator==(const unary_op_t& l, const unary_op_t& r) -> bool {
                if (static_cast<const location_t&>(l) != static_cast<const location_t&>(r))
                    return false;
                return l.op == r.op && l.operand == r.operand && l.attribute == r.attribute;
                    
            }

            template <typename Attr>
            auto operator!=(const unary_op_t& l, const unary_op_t& r) -> bool { return !(l == r); }


            auto operator==(const unary_op_t&, const unary_op_t&) -> bool;
            auto operator!=(const unary_op_t&, const unary_op_t&) -> bool;
            auto operator<<(std::ostream& os, const unary_op_t& uop) -> std::ostream&;

} } }

