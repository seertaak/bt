#pragma once

#include <iostream>
#include <string>
#include <string_view>

namespace bt { namespace lexer {
    namespace literal {
        namespace numeric {
            struct tag {};

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

            auto token_name(const integral_t& i) -> std::string_view;
            auto token_symbol(const integral_t& i) -> std::string_view;
            auto operator<<(std::ostream& os, const integral_t& t) -> std::ostream&;
            auto operator==(const integral_t& l, const integral_t& r) -> bool;
            auto operator!=(const integral_t& l, const integral_t& r) -> bool;

            struct floating_point_t : tag {
                long double value;
                int width;

                floating_point_t(long double value, int width):
                    value(value), width(width) {}

                floating_point_t() = default;
                floating_point_t(const floating_point_t&) = default;
                floating_point_t& operator=(const floating_point_t&) = default;
            };

            auto token_name(const floating_point_t& i) -> std::string_view;
            auto token_symbol(const floating_point_t& i) -> std::string_view;
            auto operator<<(std::ostream& os, const floating_point_t& t) -> std::ostream&;
            auto operator==(const floating_point_t& l, const floating_point_t& r) -> bool;
            auto operator!=(const floating_point_t& l, const floating_point_t& r) -> bool;
        }  // namespace numeric
    }      // namespace literal
}  // namespace lexer
}
