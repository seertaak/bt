#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast_fwd.hpp>

namespace bt { namespace parser { namespace syntax {

    template <typename Attr>
    struct if_t {
        BOOST_HANA_DEFINE_STRUCT(if_t,
                                 (std::vector<attr_node_t<Attr>>, elif_tests),
                                 (std::vector<attr_node_t<Attr>>, elif_branches),
                                 (attr_node_t<Attr>, else_branch));
        auto operator<=>(const if_t&) const = default;
    };

    template <typename Attr>
    auto operator<<(std::ostream& os, const if_t<Attr>& if_) -> std::ostream& {
        os << "if[";
        auto first = true;
        for (auto i = 0; i < if_.elif_tests.size(); i++) {
            if (first)
                first = false;
            else
                os << ", ";
            os << if_.elif_tests[i] << " => " << if_.elif_branches[i];
        }
        if (if_.else_branch.get()) os << ", else=" << if_.else_branch;
        os << "]";
        return os;
    }
}}}  // namespace bt::parser::syntax
