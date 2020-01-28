#include <bullet/lexer/numeric_token.hpp>

namespace lexer {
    using namespace std;
    namespace literal {
        namespace numeric {
            auto token_name(const integral_t& i) -> string_view { return "integral_t"; }

            auto token_symbol(const integral_t& i) -> string_view { return "integral_t"; }

            auto operator<<(ostream& os, const integral_t& t) -> ostream& {
                os << token_name(t) << '[' << t.value << (t.type ? t.type : '?') << t.width << ']';
                return os;
            }

            auto operator==(const integral_t& l, const integral_t& r) -> bool {
                return l.value == r.value && l.width == r.width && l.type == r.type;
            }

            auto operator!=(const integral_t& l, const integral_t& r) -> bool { return !(l == r); }

            auto token_name(const floating_point_t& i) -> string_view { return "floating_point_t"; }

            auto token_symbol(const floating_point_t& i) -> string_view {
                return "floating_point_t";
            }

            auto operator<<(ostream& os, const floating_point_t& t) -> ostream& {
                os << token_name(t) << '[' << t.value << "f" << t.width << ']';
                return os;
            }

            auto operator==(const floating_point_t& l, const floating_point_t& r) -> bool {
                return l.value == r.value && l.width == r.width;
            }

            auto operator!=(const floating_point_t& l, const floating_point_t& r) -> bool {
                return !(l == r);
            }
        }  // namespace numeric
    }      // namespace literal
}  // namespace lexer
