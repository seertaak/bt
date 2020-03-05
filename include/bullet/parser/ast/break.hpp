#pragma once

#include <compare>
#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt { namespace parser { namespace syntax {

    struct break_t {
        BOOST_HANA_DEFINE_STRUCT(break_t);
        auto operator<=>(const break_t&) const = default;
    };

    inline auto operator<<(std::ostream& os, const break_t& e) -> std::ostream& {
        os << "break";
        return os;
    }

}}}  // namespace bt::parser::syntax
