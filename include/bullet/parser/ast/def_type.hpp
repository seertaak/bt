#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/lexer/location.hpp>
#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt { namespace parser { namespace syntax {

    template <typename Attr>
    struct def_type_t {
        BOOST_HANA_DEFINE_STRUCT(def_type_t,
                                 (lexer::with_loc<lexer::identifier_t>, name),
                                 (attr_node_t<Attr>, type));
        auto operator<=>(const def_type_t&) const = default;
    };

    template <typename Attr>
    auto operator<<(std::ostream& os, const def_type_t<Attr>& d) -> std::ostream& {
        os << "def_type[" << d.name << "=" << d.type << "]";
        return os;
    }

}}}  // namespace bt::parser::syntax
