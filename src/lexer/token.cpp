#include <bullet/lexer/token.hpp>

namespace lexer {
    using namespace std;
    namespace hana = boost::hana;

    auto operator<<(ostream& os, const token_t& t) -> ostream& {
        visit([&](auto t) { os << t; }, t);
        return os;
    }

    auto token_name(const token_t& t) -> string_view {
        return visit([&](auto t) { return token_name(t); }, t);
    }

    auto token_symbol(const token_t& t) -> string_view {
        return visit([&](auto t) { return token_symbol(t); }, t);
    }

    auto operator==(const location_t& lhs, const location_t& rhs) -> bool {
        return lhs.line == rhs.line && lhs.first_col == rhs.first_col &&
               lhs.last_col == rhs.last_col;
    }

    auto operator!=(const location_t& lhs, const location_t& rhs) -> bool { return !(lhs == rhs); }

    auto operator<<(ostream& os, const location_t& l) -> ostream& {
        os << l.line << ':' << l.first_col;
        return os;
    }

    auto operator<<(ostream& os, const source_token_t& t) -> ostream& {
        os << t.token;
        return os;
    }

    auto operator==(const source_token_t& lhs, const source_token_t& rhs) -> bool {
        return lhs.token == rhs.token && lhs.location == rhs.location;
    }

    auto operator==(const source_token_t& lhs, const token_t& rhs) -> bool {
        return lhs.token == rhs;
    }
    auto operator==(const token_t& lhs, const source_token_t& rhs) -> bool { return rhs == lhs; }
    auto operator!=(const source_token_t& lhs, const token_t& rhs) -> bool { return !(lhs == rhs); }
    auto operator!=(const token_t& lhs, const source_token_t& rhs) -> bool { return !(lhs == rhs); }

    auto operator!=(const source_token_t& lhs, const source_token_t& rhs) -> bool {
        return !(lhs == rhs);
    }

    using source_token_list_t = std::vector<source_token_t>;
    using token_list_t = std::vector<token_t>;

    auto operator==(const source_token_list_t& lhs, const source_token_list_t& rhs) -> bool {
        if (lhs.size() != rhs.size()) return false;
        const auto n = lhs.size();
        for (auto i = 0; i < n; i++)
            if (lhs[i] != rhs[i]) return false;
        return true;
    }

    auto operator==(const source_token_list_t& lhs, const token_list_t& rhs) -> bool {
        if (lhs.size() != rhs.size()) return false;
        const auto n = lhs.size();
        for (auto i = 0; i < n; i++)
            if (lhs[i] != rhs[i]) return false;
        return true;
    }

    auto operator==(const token_list_t& lhs, const source_token_list_t& rhs) -> bool {
        return rhs == lhs;
    }

    auto operator<<(std::ostream& os, const source_token_list_t& t) -> std::ostream& {
        os << '[';
        if (!t.empty()) {
            auto first = true;
            for (const auto& v : t) {
                if (first)
                    first = false;
                else
                    os << ", ";
                os << v;
            }
        }
        os << ']';
        return os;
    }

    auto operator==(const token_list_t& lhs, const token_list_t& rhs) -> bool {
        if (lhs.size() != rhs.size()) return false;
        const auto n = lhs.size();
        for (auto i = 0; i < n; i++)
            if (lhs[i] != rhs[i]) return false;
        return true;
    }

    auto operator<<(std::ostream& os, const token_list_t& t) -> std::ostream& {
        os << '[';
        if (!t.empty()) {
            auto first = true;
            for (const auto& v : t) {
                if (first)
                    first = false;
                else
                    os << ", ";
                os << v;
            }
        }
        os << ']';
        return os;
    }
}  // namespace lexer
