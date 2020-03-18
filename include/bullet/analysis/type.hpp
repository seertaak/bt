#pragma once

#include <atomic>
#include <compare>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/hana/all.hpp>
#include <boost/hana/experimental/type_name.hpp>

#include <bullet/util.hpp>

namespace bt { namespace analysis {
    namespace hana = boost::hana;

    inline auto operator<<(std::ostream& os, const std::monostate&) -> std::ostream& {
        os << "<unknown>";
        return os;
    }

    struct type_value;
    using type_t = bt::ref<type_value>;

    namespace types {
        struct void_t {
            auto operator<=>(const void_t&) const = default;
        };

        template <bool S, int W>
        struct int_t {
            static_assert(W == 8 || W == 16 || W == 32 || W == 64);

            constexpr const static bool is_signed = S;
            constexpr const static int width = W;

            auto operator<=>(const int_t&) const = default;
        };

        template <int W>
        struct float_t {
            static_assert(W == 32 || W == 64);

            constexpr const static int width = W;
            constexpr const static int is_signed = true;

            auto operator<=>(const float_t&) const = default;
        };

        struct bool_t {
            constexpr const static bool is_signed = false;
            constexpr const static int width = 8;

            auto operator<=>(const bool_t&) const = default;
        };

        struct char_t {
            constexpr const static bool is_signed = false;
            constexpr const static int width = 32;

            auto operator<=>(const char_t&) const = default;
        };

        struct name_and_type_t {
            std::string name;
            type_t type;
        };
        using name_and_type_vector_t = std::vector<name_and_type_t>;

        struct function_t {
            type_t result_type;
            name_and_type_vector_t formal_parameters;
        };

        struct struct_t : name_and_type_vector_t {
            using base_t = name_and_type_vector_t;
            using base_t::base_t;
        };

        struct tuple_t : std::vector<type_t> {
            using base_t = std::vector<type_t>;
            using base_t::base_t;
        };

        struct ptr_t {
            type_t value_type;
        };

        struct array_t {
            type_t value_type;
            std::vector<size_t> size;
        };

        struct dynarr_t {
            type_t value_type;
        };

        struct slice_t {
            type_t value_type;
            int64_t begin, end, stride;
        };

        struct strlit_t {
            int size;
        };

        struct intlit_t {};

        struct floatlit_t {};

        struct variant_t : std::vector<type_t> {
            using base_t = std::vector<type_t>;
            using base_t::base_t;
        };

        struct string_t {};

        struct nominal_type_t {
            int id;
            std::string fqn;
            type_t type;

            explicit nominal_type_t(std::string name, type_t type);
            nominal_type_t(const nominal_type_t&) = default;
            nominal_type_t& operator=(const nominal_type_t&) = default;

            static std::atomic<int> next_id;
        };

        struct placeholder_t {};

        using i8_t = int_t<true, 8>;
        using i16_t = int_t<true, 16>;
        using i32_t = int_t<true, 32>;
        using i64_t = int_t<true, 64>;

        using u8_t = int_t<false, 8>;
        using u16_t = int_t<false, 16>;
        using u32_t = int_t<false, 32>;
        using u64_t = int_t<false, 64>;

        template struct float_t<32>;
        template struct float_t<64>;
        using f32_t = float_t<32>;
        using f64_t = float_t<64>;

        template <typename T>
        struct is_int {
            constexpr static bool value = false;
        };

        template <bool S, int W>
        struct is_int<int_t<S, W>> {
            constexpr static bool value = true;
        };

        template <typename T>
        constexpr auto is_int_v = is_int<std::remove_const_t<std::remove_reference_t<T>>>::value;

        template <typename T>
        struct is_float {
            constexpr static bool value = false;
        };

        template <int W>
        struct is_float<float_t<W>> {
            constexpr static bool value = true;
        };

        template <typename T>
        constexpr auto is_float_v =
            is_float<std::remove_const_t<std::remove_reference_t<T>>>::value;

        auto operator<<(std::ostream& os, const void_t&) -> std::ostream&;
        template <bool S, int W>
        auto operator<<(std::ostream& os, const int_t<S, W>& t) -> std::ostream& {
            os << (S ? 'i' : 'u') << W;
            return os;
        }

        template <int W>
        auto operator<<(std::ostream& os, const float_t<W>& t) -> std::ostream& {
            os << 'f' << W;
            return os;
        }

        auto operator<<(std::ostream& os, const bool_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const char_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const name_and_type_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const struct_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const function_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const name_and_type_vector_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const tuple_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const ptr_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const array_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const dynarr_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const slice_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const strlit_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const variant_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const string_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const nominal_type_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const intlit_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const floatlit_t&) -> std::ostream&;
        auto operator<<(std::ostream& os, const placeholder_t&) -> std::ostream&;

        auto operator==(const name_and_type_t& lhs, const name_and_type_t& rhs) -> bool;
        auto operator==(const name_and_type_vector_t& lhs, const name_and_type_vector_t& rhs)
            -> bool;
        auto operator==(const function_t&, const function_t&) -> bool;
        auto operator==(const struct_t&, const struct_t&) -> bool;
        auto operator==(const tuple_t&, const tuple_t&) -> bool;
        auto operator==(const ptr_t&, const ptr_t&) -> bool;
        auto operator==(const array_t&, const array_t&) -> bool;
        auto operator==(const dynarr_t&, const dynarr_t&) -> bool;
        auto operator==(const slice_t&, const slice_t&) -> bool;
        auto operator==(const strlit_t&, const strlit_t&) -> bool;
        auto operator==(const variant_t&, const variant_t&) -> bool;
        auto operator==(const string_t&, const string_t&) -> bool;
        auto operator==(const nominal_type_t&, const nominal_type_t&) -> bool;
        auto operator==(const intlit_t&, const intlit_t&) -> bool;
        auto operator==(const floatlit_t&, const floatlit_t&) -> bool;
        auto operator==(const placeholder_t&, const placeholder_t&) -> bool;

        auto operator!=(const name_and_type_t& lhs, const name_and_type_t& rhs) -> bool;
        auto operator!=(const name_and_type_vector_t& lhs, const name_and_type_vector_t& rhs)
            -> bool;
        auto operator!=(const function_t&, const function_t&) -> bool;
        auto operator!=(const struct_t&, const struct_t&) -> bool;
        auto operator!=(const tuple_t&, const tuple_t&) -> bool;
        auto operator!=(const ptr_t&, const ptr_t&) -> bool;
        auto operator!=(const array_t&, const array_t&) -> bool;
        auto operator!=(const dynarr_t&, const dynarr_t&) -> bool;
        auto operator!=(const slice_t&, const slice_t&) -> bool;
        auto operator!=(const strlit_t&, const strlit_t&) -> bool;
        auto operator!=(const variant_t&, const variant_t&) -> bool;
        auto operator!=(const string_t&, const string_t&) -> bool;
        auto operator!=(const nominal_type_t&, const nominal_type_t&) -> bool;
        auto operator!=(const intlit_t&, const intlit_t&) -> bool;
        auto operator!=(const floatlit_t&, const floatlit_t&) -> bool;
        auto operator!=(const placeholder_t&, const placeholder_t&) -> bool;
    }  // namespace types

    using type_base_t = std::variant<std::monostate,
                                     types::void_t,
                                     types::i8_t,
                                     types::i16_t,
                                     types::i32_t,
                                     types::i64_t,
                                     types::u8_t,
                                     types::u16_t,
                                     types::u32_t,
                                     types::u64_t,
                                     types::f32_t,
                                     types::f64_t,
                                     types::intlit_t,
                                     types::floatlit_t,
                                     types::bool_t,
                                     types::char_t,
                                     types::function_t,
                                     types::struct_t,
                                     types::tuple_t,
                                     types::ptr_t,
                                     types::array_t,
                                     types::dynarr_t,
                                     types::slice_t,
                                     types::strlit_t,
                                     types::variant_t,
                                     types::string_t,
                                     types::nominal_type_t>;

    struct type_value : type_base_t {
        using type_base_t::type_base_t;

        inline operator bool() const { return !is<std::monostate>(); }

        auto empty() const -> bool { return is<std::monostate>(); }
        auto valid() const -> bool { return !empty(); }

        template <typename U>
        inline auto is() const -> bool {
            return std::holds_alternative<U>(*this);
        }

        template <typename U>
        inline auto get() const -> const U& {
            return std::get<U>(*this);
        }

        template <typename U>
        inline auto get() -> U& {
            return std::get<U>(*this);
        }

        template <typename U>
        inline auto as() const -> const U& {
            return std::get<U>(*this);
        }

        template <typename U>
        inline auto as() -> U& {
            return std::get<U>(*this);
        }

        template <typename U>
        inline auto get_if() const -> const U* {
            return std::get_if<U>(this);
        }

        template <typename U>
        inline auto get_if() -> U* {
            return std::get_if<U>(this);
        }
    };

    auto operator<<(std::ostream& os, const type_value&) -> std::ostream&;
    auto operator<<(std::ostream& os, const type_t&) -> std::ostream&;

    const type_t VOID = type_value(types::void_t{});
    const type_t I8 = type_value(types::i8_t{});
    const type_t I16 = type_value(types::i16_t{});
    const type_t I32 = type_value(types::i32_t{});
    const type_t I64 = type_value(types::i64_t{});
    const type_t U8 = type_value(types::u8_t{});
    const type_t U16 = type_value(types::u16_t{});
    const type_t U32 = type_value(types::u32_t{});
    const type_t U64 = type_value(types::u64_t{});
    const type_t F32 = type_value(types::f32_t{});
    const type_t F64 = type_value(types::f64_t{});
    const type_t INTLIT = type_value(types::intlit_t{});
    const type_t FLOATLIT = type_value(types::floatlit_t{});
    const type_t BOOL = type_value(types::bool_t{});
    const type_t CHAR = type_value(types::char_t{});
    const type_t PTR = type_value(types::ptr_t{});
    const type_t SLICE = type_value(types::slice_t{});
    const type_t STRUCT = type_value(types::struct_t{});
    const type_t ARRAY = type_value(types::array_t{});
    const type_t DYNARR = type_value(types::dynarr_t{});
    const type_t VARIANT = type_value(types::variant_t{});
    const type_t FUNCTION = type_value(types::function_t{});
    const type_t TUPLE = type_value(types::tuple_t{});
    const type_t UNKOWN = type_value(std::monostate{});
    const type_t STRLIT = type_value(types::strlit_t{});
    const type_t STRING = type_value(types::string_t{});

    auto is_integral(const type_t& t) -> bool;
    auto is_floating_point(const type_t& t) -> bool;
    auto is_signed(const type_t& t) -> bool;
    auto width(const type_t& t) -> int;
    auto promoted_type(const type_t& t, const type_t& u) -> std::optional<type_t>;
    auto is_convertible_to(const type_t& src, const type_t& dst) -> bool;
    auto implicit_conversion_distance(const type_t& src, const type_t& dst) -> int;
    auto is_assignable_to(const type_t& value, const type_t& target) -> bool;
    auto deref(const type_t& t) -> type_t;
    auto ptr_depth(type_t t) -> int;
    auto decay_ptr(type_t p) -> type_t;
    auto decay_ptr(type_t p, int n_levels) -> type_t;
    auto is_immutable(const type_t& t) -> bool;
    auto regularized_type(const type_t& t) -> type_t;
}}  // namespace bt::analysis
