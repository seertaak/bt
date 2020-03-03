#pragma once

#include <iostream>
#include <string_view>

namespace bt {
    namespace parser {
        namespace syntax {
            struct location_t {
                std::string_view file;
                uint32_t line_begin, line_end;
                uint16_t col_begin, col_end;

                location_t(std::string_view file,
                          uint32_t line_begin, uint16_t col_begin, 
                           uint32_t line_end, uint16_t col_end):
                           file(file),
                     line_begin(line_begin), line_end(line_end),
                     col_begin(col_begin), col_end(col_end)
                {}
            };

        auto operator==(const location_t& lhs, const location_t& rhs) -> bool {
            return lhs.file == rhs.file && 
                lhs.line_begin == rhs.line_begin && 
                lhs.line_end == rhs.line_end && 
                lhs.col_begin == rhs.col_begin && 
                lhs.col_end == rhs.col_end;
        }

        auto operator!=(const location_t& lhs, const location_t& rhs) -> bool {
            return !(lhs == rhs);
        }

        auto operator<<(ostream& os, const location_t& l) -> ostream& {
            os << '"' << l.file << "\" from " << l.line_begin << ":" 
            << l.col_begin << " to " << l.line_end << ":" << l.line_end;
            return os;
        }

} } }

