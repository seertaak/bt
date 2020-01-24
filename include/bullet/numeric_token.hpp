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

            // TODO: the same rigmarole for all octal, hexadecimal, floating point and for all the
            // size (8, 16, 32, 64, size_t) variants.

            template <typename T>
            struct number_t : tag {
                T value;
                // note: adding explicit => BOOM compiler.
                number_t() = default;
                number_t(const number_t&) = default;
                number_t(number_t&&) = default;
                number_t(T value) : value(value) {}

                auto operator=(number_t that) noexcept -> number_t& {
                    value = that.value;
                    return *this;
                }

                static const string name;
            };

            template <typename T>
            const string number_t<T>::name = boost::typeindex::type_id<T>().pretty_name();

            template <typename T>
            inline auto token_name(const number_t<T>& i) -> string_view {
                return number_t<T>::name;
            }

            template <typename T>
            inline auto token_symbol(const number_t<T>& i) -> string_view {
                return number_t<T>::name;
            }

            template <typename T>
            inline auto operator<<(ostream& os, const number_t<T>& t) -> ostream& {
                os << token_name(t) << '[' << t.value << ']';
                return os;
            }

            template <typename T>
            inline auto operator==(const number_t<T>& l, const number_t<T>& r) -> bool {
                return l.value == r.value;
            }

            template <typename T>
            inline auto operator!=(const number_t<T>& l, const number_t<T>& r) -> bool {
                return !(l == r);
            }

            template <typename T>
            inline auto operator<(const number_t<T>& l, const number_t<T>& r) -> bool {
                return l.value < r.value;
            }

            template <typename T>
            inline auto operator<=(const number_t<T>& l, const number_t<T>& r) -> bool {
                return l.value <= r.value;
            }

            template <typename T>
            inline auto operator>(const number_t<T>& l, const number_t<T>& r) -> bool {
                return l.value > r.value;
            }

            template <typename T>
            inline auto operator>=(const number_t<T>& l, const number_t<T>& r) -> bool {
                return l.value >= r.value;
            }

            template struct number_t<unsigned long long int >;
            template struct number_t<long double>;

            using ullint = number_t<unsigned long long int>;
            using ldouble = number_t<long double>;
        }  // namespace numeric
    }      // namespace literal
}  // namespace lexer

BOOST_FUSION_ADAPT_STRUCT(lexer::literal::numeric::ullint, value)
BOOST_FUSION_ADAPT_STRUCT(lexer::literal::numeric::ldouble, value)
