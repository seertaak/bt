#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/type_index.hpp>

#include <bullet/util.hpp>

namespace lexer {
    using namespace std;
    namespace x3 = boost::spirit::x3;

    namespace literal {
        namespace numeric {
            struct tag : x3::position_tagged {};

            struct integral_t : tag {
                unsigned long long value;
                char type;
                int width;

                integral_t(unsigned long long value, char type, int width):
                    value(value), type(type), width(width) {}

                integral_t() = default;
                integral_t(const integral_t&) = default;
                integral_t& operator=(const integral_t&) = default;
            };

            inline auto token_name(const integral_t& i) -> string_view {
                return "integral_t";
            }

            inline auto token_symbol(const integral_t& i) -> string_view {
                return "integral_t";
            }

            inline auto operator<<(ostream& os, const integral_t& t) -> ostream& {
                os << token_name(t) << '[' << t.value << (t.type ? t.type : '?')
                    << t.width<< ']';
                return os;
            }

            inline auto operator==(const integral_t& l, const integral_t& r) -> bool {
                return l.value == r.value && l.width == r.width && l.type == r.type;
            }

            inline auto operator!=(const integral_t& l, const integral_t& r) -> bool {
                return !(l == r);
            }
        }  // namespace numeric
    }      // namespace literal
}  // namespace lexer

BOOST_FUSION_ADAPT_STRUCT(lexer::literal::numeric::integral_t, value, type, width)
