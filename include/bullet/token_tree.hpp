#pragma once

#include <vector>
#include <iostream>

#include <boost/variant.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/tail.hpp>

#include <bullet/util.hpp>
#include <bullet/token.hpp>

namespace lexer {
    namespace x3 = boost::spirit::x3;
    using namespace ranges;

    using ranges::empty;

    template <typename T>
    using rec = typename boost::recursive_wrapper<T>;

    struct token_list_t;

    using token_tree_t = boost::variant<token_t, rec<token_list_t>>;

    inline auto operator<<(std::ostream& os, const token_tree_t& t) -> std::ostream&;
    auto operator<<(std::ostream& os, const token_list_t& t) -> std::ostream&;

    struct token_list_t : std::vector<token_tree_t> {
        using base_t = std::vector<token_tree_t>;
        using base_t::base_t;
        using base_t::operator=;
    };

    inline auto operator<<(std::ostream& os, const token_tree_t& t) -> std::ostream& {
        if (const auto* tok = boost::get<token_t>(&t)) {
            os << *tok;
        } else {
            os << boost::get<token_list_t>(t);
        }
        return os;
    }

    inline auto operator<<(std::ostream& os, const token_list_t& t) -> std::ostream& {
        os << '[';
        if (!empty(t)) {
            os << front(t);
            for (const auto& v: t | views::tail)
                os << ", " << v;
        }
        os << ']';
        return os;
    }
}
