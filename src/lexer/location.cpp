#include <bullet/lexer/location.hpp>

using namespace std;

namespace bt { namespace lexer {
    auto operator<<(ostream& os, const location_t& l) -> ostream& {
        os << l.line << ':' << l.first_col;
        return os;
    }
}}  // namespace bt::lexer
