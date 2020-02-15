#pragma once

#include <iostream>
#include <string>
#include <string_view>

namespace bt {
    namespace lexer {
        struct string_token_t {
            std::string value;
            explicit string_token_t(std::string s) : value(s) {}

            string_token_t() = default;
            string_token_t(const string_token_t&) = default;
            string_token_t(string_token_t&&) noexcept = default;
            string_token_t& operator=(const string_token_t&) = default;
            string_token_t& operator=(string_token_t&&) = default;
        };

        auto token_value(const string_token_t& i) -> std::string_view;
        auto token_symbol(const string_token_t& i) -> std::string_view;

        auto operator<<(std::ostream& os, const string_token_t& t) -> std::ostream&;
        auto operator==(const string_token_t& l, const string_token_t& r) -> bool;
        auto operator!=(const string_token_t& l, const string_token_t& r) -> bool;
        auto operator<(const string_token_t& l, const string_token_t& r) -> bool;
        auto operator<=(const string_token_t& l, const string_token_t& r) -> bool;
        auto operator>(const string_token_t& l, const string_token_t& r) -> bool;
        auto operator>=(const string_token_t& l, const string_token_t& r) -> bool;
    }  // namespace lexer
}  // namespace bt
