#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include <bullet/util.hpp>

namespace lexer {
    using namespace std;
    namespace x3 = boost::spirit::x3;

    struct identifier_t : x3::position_tagged {
        string name;
        identifier_t(string s) : name(s) {}
    };

    inline auto token_name(const identifier_t& i) -> string_view { return i.name; }
    inline auto token_symbol(const identifier_t& i) -> string_view { return i.name; }

    inline auto operator<<(ostream& os, const identifier_t& t) -> ostream& {
        os << "ident[" << token_name(t) << "]";
        return os;
    }

    inline auto operator==(const identifier_t& l, const identifier_t& r) -> bool {
        return l.name == r.name;
    }

    inline auto operator!=(const identifier_t& l, const identifier_t& r) -> bool {
        return !(l == r);
    }

    inline auto operator<(const identifier_t& l, const identifier_t& r) -> bool {
        return std::lexicographical_compare(begin(l.name), end(l.name), begin(r.name), end(r.name));
    }

    inline auto operator<=(const identifier_t& l, const identifier_t& r) -> bool {
        return l < r || l == r;
    }

    inline auto operator>(const identifier_t& l, const identifier_t& r) -> bool {
        return !(l <= r);
    }

    inline auto operator>=(const identifier_t& l, const identifier_t& r) -> bool {
        return l > r || l == r;
    }
}  // namespace lexer

BOOST_FUSION_ADAPT_STRUCT(lexer::identifier_t, name)

