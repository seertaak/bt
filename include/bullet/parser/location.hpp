#pragma once

#include <compare>
#include <iostream>
#include <string_view>

#include <bullet/lexer/token.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            struct location_t {
                uint32_t first_line, last_line;
                uint16_t first_col, last_col;

                location_t() : first_line(1), last_line(1), first_col(1), last_col(1) {}
                location_t(uint32_t first_line,
                           uint16_t first_col,
                           uint32_t last_line,
                           uint16_t last_col)
                    : first_line(first_line),
                      last_line(last_line),
                      first_col(first_col),
                      last_col(last_col) {}

                location_t(const lexer::location_t& first, const lexer::location_t& last)
                    : first_line(first.line),
                      last_line(last.line),
                      first_col(first.first_col),
                      last_col(last.last_col) {}

                auto operator<=>(const location_t&) const = default;
            };

            auto operator<<(std::ostream& os, const location_t& l) -> std::ostream&;
        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
