#pragma once

#include <compare>
#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt { namespace parser { namespace syntax {

    struct continue_t {
        BOOST_HANA_DEFINE_STRUCT(continue_t);
        auto operator<=>(const continue_t&) const = default;
    };

    inline auto operator<<(std::ostream& os, const continue_t& e) -> std::ostream& {
        os << "continue";
        return os;
    }

}}}  // namespace bt::parser::syntax
