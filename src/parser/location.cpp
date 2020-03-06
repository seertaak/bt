#include <bullet/parser/location.hpp>

namespace bt { namespace parser {
    using namespace std;

    auto operator<<(ostream& os, const location_t& l) -> ostream& {
        os << '(' << l.first_line << ":" << l.first_col << "-" << l.last_line << ":" << l.last_col
           << ')';
        return os;
    }

}}  // namespace bt::parser
