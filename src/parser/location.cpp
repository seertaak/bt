#include <bullet/parser/location.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            using namespace std;

            auto operator==(const location_t& lhs, const location_t& rhs) -> bool {
                return lhs.file == rhs.file && lhs.line_begin == rhs.line_begin &&
                       lhs.line_end == rhs.line_end && lhs.col_begin == rhs.col_begin &&
                       lhs.col_end == rhs.col_end;
            }

            auto operator!=(const location_t& lhs, const location_t& rhs) -> bool {
                return !(lhs == rhs);
            }

            auto operator<<(ostream& os, const location_t& l) -> ostream& {
                os << '"' << l.file << "\" from " << l.line_begin << ":" << l.col_begin << " to "
                   << l.line_end << ":" << l.line_end;
                return os;
            }

        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
