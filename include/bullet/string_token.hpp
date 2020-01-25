#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
//#include <boost/spirit/home/x3.hpp>
//#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

#include <bullet/util.hpp>

namespace lexer {
    using namespace std;
    //namespace x3 = boost::spirit::x3;

    //struct string_token_t : x3::position_tagged {
    struct string_token_t {
        string value;
        explicit string_token_t(string s) : value(s) {}

        string_token_t() = default;
        string_token_t(const string_token_t&) = default;
        string_token_t(string_token_t&&) noexcept = default;
        string_token_t& operator=(const string_token_t&) = default;
        string_token_t& operator=(string_token_t&&) = default;
    };

    inline auto token_value(const string_token_t& i) -> string_view { return i.value; }
    inline auto token_symbol(const string_token_t& i) -> string_view { return i.value; }

    inline auto operator<<(ostream& os, const string_token_t& t) -> ostream& {
        os << '"' << token_value(t) << '"';
        return os;
    }

    inline auto operator==(const string_token_t& l, const string_token_t& r) -> bool {
        return l.value == r.value;
    }

    inline auto operator!=(const string_token_t& l, const string_token_t& r) -> bool {
        return !(l == r);
    }

    inline auto operator<(const string_token_t& l, const string_token_t& r) -> bool {
        return std::lexicographical_compare(begin(l.value), end(l.value), begin(r.value),
                                            end(r.value));
    }

    inline auto operator<=(const string_token_t& l, const string_token_t& r) -> bool {
        return l < r || l == r;
    }

    inline auto operator>(const string_token_t& l, const string_token_t& r) -> bool {
        return !(l <= r);
    }

    inline auto operator>=(const string_token_t& l, const string_token_t& r) -> bool {
        return l > r || l == r;
    }
}  // namespace lexer

BOOST_FUSION_ADAPT_STRUCT(lexer::string_token_t, value)
