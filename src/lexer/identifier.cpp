#include <bullet/lexer/identifier.hpp>

#include <algorithm>

namespace bt { namespace lexer {
    using namespace std;

    auto token_name(const identifier_t& i) -> string_view { return i.name; }
    auto token_symbol(const identifier_t& i) -> string_view { return "IDENTIFIER"sv; }

    auto operator<<(ostream& os, const identifier_t& t) -> ostream& {
        os << "ident[" << token_name(t) << "]";
        return os;
    }

    auto operator==(const identifier_t& l, const identifier_t& r) -> bool {
        return l.name == r.name;
    }

    auto operator!=(const identifier_t& l, const identifier_t& r) -> bool { return !(l == r); }

    auto operator<(const identifier_t& l, const identifier_t& r) -> bool {
        return lexicographical_compare(begin(l.name), end(l.name), begin(r.name), end(r.name));
    }

    auto operator<=(const identifier_t& l, const identifier_t& r) -> bool {
        return l < r || l == r;
    }

    auto operator>(const identifier_t& l, const identifier_t& r) -> bool { return !(l <= r); }

    auto operator>=(const identifier_t& l, const identifier_t& r) -> bool {
        return l > r || l == r;
    }
}}  // namespace bt::lexer
