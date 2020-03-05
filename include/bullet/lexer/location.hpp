#pragma once

#include <compare>
#include <iostream>
#include <utility>

namespace bt { namespace lexer {
    struct location_t {
        uint32_t line;
        uint16_t first_col;
        uint16_t last_col;

        auto operator<=>(const location_t&) const = default;
    };

    auto operator<<(std::ostream& os, const location_t&) -> std::ostream&;

    template <typename T>
    struct with_loc : T {
        using base_t = T;
        using base_t::base_t;

        location_t location;

        template <typename... Args>
        with_loc(Args&&... args, const location_t& loc) noexcept(noexcept(T{std::forward(args)...}))
            : T{std::forward(args)...}, location{loc} {}

        with_loc(const T& t, const location_t& loc) : T{t}, location{loc} {}

        with_loc(T&& t, const location_t& loc) noexcept : T{std::forward(t)}, location{loc} {}
    };

    template <typename T>
    auto operator<<(std::ostream& os, const with_loc<T>& tok) -> std::ostream& {
        os << static_cast<const T&>(tok) << " at " << tok.location;
        return os;
    }
}}  // namespace bt::lexer
