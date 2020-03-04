#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>
#include <bullet/parser/ast/data.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct var_def_t {
                BOOST_HANA_DEFINE_STRUCT(var_def_t,
                    (lexer::identifier_t, name),
                    (attr_node_t<Attr>, type),
                    (attr_node_t<Attr>, rhs),
                    (Attr, attribute)
                );
                 auto operator<=>(const var_def_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const var_def_t<Attr>& v) -> std::ostream& {
                os << "var[" << v.name;
                if (v.type.get()) os << ": " << v.type;
                os << " = " << v.rhs << "]";
                return os;
            }
        }}}