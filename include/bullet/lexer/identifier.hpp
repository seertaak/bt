#pragma once

#include <iostream>
#include <string>
#include <string_view>

namespace bt { namespace lexer {
    struct identifier_t {
        std::string name;
        explicit identifier_t(std::string s) : name(s) {}
        identifier_t() = default;
        identifier_t(const identifier_t&) = default;
        identifier_t(identifier_t&&) noexcept = default;
        identifier_t& operator=(const identifier_t&) = default;
        identifier_t& operator=(identifier_t&&) noexcept = default;
    };

    auto token_name(const identifier_t& i) -> std::string_view;
    auto token_symbol(const identifier_t& i) -> std::string_view;
    auto operator<<(std::ostream& os, const identifier_t& t) -> std::ostream&;
    auto operator==(const identifier_t& l, const identifier_t& r) -> bool;
    auto operator!=(const identifier_t& l, const identifier_t& r) -> bool;
    auto operator<(const identifier_t& l, const identifier_t& r) -> bool;
    auto operator<=(const identifier_t& l, const identifier_t& r) -> bool;
    auto operator>(const identifier_t& l, const identifier_t& r) -> bool;
    auto operator>=(const identifier_t& l, const identifier_t& r) -> bool;
}  // namespace lexer
}
