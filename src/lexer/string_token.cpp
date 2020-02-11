#include <algorithm>

#include <bullet/lexer/string_token.hpp>

namespace bt {
    namespace lexer {
        using namespace std;

        auto token_value(const string_token_t& i) -> string_view { return i.value; }
        auto token_symbol(const string_token_t& i) -> string_view { return "STRING_LITERAL"sv; }

        auto operator<<(ostream& os, const string_token_t& t) -> ostream& {
            os << '"' << token_value(t) << '"';
            return os;
        }

        auto operator==(const string_token_t& l, const string_token_t& r) -> bool {
            return l.value == r.value;
        }

        auto operator!=(const string_token_t& l, const string_token_t& r) -> bool {
            return !(l == r);
        }

        auto operator<(const string_token_t& l, const string_token_t& r) -> bool {
            return lexicographical_compare(begin(l.value), end(l.value), begin(r.value),
                                           end(r.value));
        }

        auto operator<=(const string_token_t& l, const string_token_t& r) -> bool {
            return l < r || l == r;
        }

        auto operator>(const string_token_t& l, const string_token_t& r) -> bool {
            return !(l <= r);
        }

        auto operator>=(const string_token_t& l, const string_token_t& r) -> bool {
            return l > r || l == r;
        }
    }  // namespace lexer
}  // namespace bt
