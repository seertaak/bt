#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/lexer/location.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt {
    namespace parser {
        namespace syntax {

            template <typename Attr>
            struct let_type_t {
                BOOST_HANA_DEFINE_STRUCT(let_type_t,
                                         (lexer::with_loc<lexer::identifier_t>, name),
                                         (attr_node_t<Attr>, type));
                auto operator<=>(const let_type_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const let_type_t<Attr>& d) -> std::ostream& {
                os << "let_type[" << d.name << "=" << d.type << "]";
                return os;
            }

        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
