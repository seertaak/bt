#pragma once

#include <compare>
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <boost/hana/all.hpp>

#include <bullet/util.hpp>

namespace bt { namespace analysis {
    struct type_value;
    using type_t = bt::ref<type_value>;

    auto operator<<(std::ostream& os, const type_t&) -> std::ostream&;

    namespace types {
        namespace hana = boost::hana;

        struct void_t {
            auto operator<=>(const void_t&) const = default;
        };
        auto operator<<(std::ostream& os, const void_t&) -> std::ostream&;

        template <bool S, int W>
        struct int_t {
            static_assert(W == 8 || W == 16 || W == 32 || W == 64);

            constexpr const static bool is_signed = S;
            constexpr const static int width = W;

            auto operator<=>(const int_t&) const = default;
        };

        template <bool S, int W>
        auto operator<<(std::ostream& os, const int_t<S, W>& t) -> std::ostream& {
            os << (S ? 'i' : 'u') << W;
            return os;
        }

        using i8_t = int_t<true, 8>;
        using i16_t = int_t<true, 16>;
        using i32_t = int_t<true, 32>;
        using i64_t = int_t<true, 64>;

        using u8_t = int_t<false, 8>;
        using u16_t = int_t<false, 16>;
        using u32_t = int_t<false, 32>;
        using u64_t = int_t<false, 64>;

        template <int W>
        struct float_t {
            static_assert(W == 32 || W == 64);

            constexpr const static int width = W;
            constexpr const static int is_signed = true;

            auto operator<=>(const float_t&) const = default;
        };

        template <int W>
        auto operator<<(std::ostream& os, const float_t<W>& t) -> std::ostream& {
            os << 'f' << W;
            return os;
        }

        template struct float_t<32>;
        template struct float_t<64>;

        using f32_t = float_t<32>;
        using f64_t = float_t<64>;

        struct bool_t {
            constexpr const static bool is_signed = false;
            constexpr const static int width = 8;

            auto operator<=>(const bool_t&) const = default;
        };

        auto operator<<(std::ostream& os, const bool_t&) -> std::ostream&;

        struct char_t {
            constexpr const static bool is_signed = false;
            constexpr const static int width = 32;

            auto operator<=>(const char_t&) const = default;
        };
        auto operator<<(std::ostream& os, const char_t&) -> std::ostream&;

        struct name_and_type_t {
            std::string name;
            type_t type;
        };

        auto operator==(const name_and_type_t& lhs, const name_and_type_t& rhs) -> bool;
        auto operator!=(const name_and_type_t& lhs, const name_and_type_t& rhs) -> bool;
        auto operator<<(std::ostream& os, const name_and_type_t&) -> std::ostream&;

        using name_and_type_vector_t = std::vector<name_and_type_t>;

        auto operator==(const name_and_type_vector_t& lhs, const name_and_type_vector_t& rhs)
            -> bool;
        auto operator!=(const name_and_type_vector_t& lhs, const name_and_type_vector_t& rhs)
            -> bool;
        auto operator<<(std::ostream& os, const name_and_type_vector_t&) -> std::ostream&;

        struct function_t {
            type_t result_type;
            name_and_type_vector_t formal_parameters;
        };

        auto operator==(const function_t&, const function_t&) -> bool;
        auto operator!=(const function_t&, const function_t&) -> bool;
        auto operator<<(std::ostream& os, const function_t&) -> std::ostream&;

        struct struct_t : name_and_type_vector_t {
            using base_t = name_and_type_vector_t;
            using base_t::base_t;
        };

        auto operator==(const struct_t&, const struct_t&) -> bool;
        auto operator!=(const struct_t&, const struct_t&) -> bool;
        auto operator<<(std::ostream& os, const struct_t&) -> std::ostream&;

        struct tuple_t : std::vector<type_t> {
            using base_t = std::vector<type_t>;
            using base_t::base_t;
        };

        auto operator==(const tuple_t&, const tuple_t&) -> bool;
        auto operator!=(const tuple_t&, const tuple_t&) -> bool;
        auto operator<<(std::ostream& os, const tuple_t&) -> std::ostream&;

        struct ptr_t {
            type_t value_type;
        };

        auto operator==(const ptr_t&, const ptr_t&) -> bool;
        auto operator!=(const ptr_t&, const ptr_t&) -> bool;
        auto operator<<(std::ostream& os, const ptr_t&) -> std::ostream&;

        struct array_t {
            type_t value_type;
            std::vector<size_t> size;
        };

        auto operator==(const array_t&, const array_t&) -> bool;
        auto operator!=(const array_t&, const array_t&) -> bool;
        auto operator<<(std::ostream& os, const array_t&) -> std::ostream&;

        struct dynarr_t {
            type_t value_type;
        };

        auto operator==(const dynarr_t&, const dynarr_t&) -> bool;
        auto operator!=(const dynarr_t&, const dynarr_t&) -> bool;
        auto operator<<(std::ostream& os, const dynarr_t&) -> std::ostream&;

        struct slice_t {
            type_t value_type;
            int64_t begin, end, stride;
        };

        auto operator==(const slice_t&, const slice_t&) -> bool;
        auto operator!=(const slice_t&, const slice_t&) -> bool;
        auto operator<<(std::ostream& os, const slice_t&) -> std::ostream&;

        struct strlit_t {
            int size;
        };

        auto operator==(const strlit_t&, const strlit_t&) -> bool;
        auto operator!=(const strlit_t&, const strlit_t&) -> bool;
        auto operator<<(std::ostream& os, const strlit_t&) -> std::ostream&;

        struct variant_t : std::vector<type_t> {
            using base_t = std::vector<type_t>;
            using base_t::base_t;
        };

        auto operator==(const variant_t&, const variant_t&) -> bool;
        auto operator!=(const variant_t&, const variant_t&) -> bool;
        auto operator<<(std::ostream& os, const variant_t&) -> std::ostream&;

        struct string_t {};

        auto operator==(const string_t&, const string_t&) -> bool;
        auto operator!=(const string_t&, const string_t&) -> bool;
        auto operator<<(std::ostream& os, const string_t&) -> std::ostream&;
    }  // namespace types

    using type_base_t = std::variant<types::void_t,
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
                                     types::string_t>;

    struct type_value : type_base_t {
        using type_base_t::type_base_t;
    };

    const type_value VOID_T = types::void_t{};
    const type_value I8_T = types::i8_t{};
    const type_value I16_T = types::i16_t{};
    const type_value I32_T = types::i32_t{};
    const type_value I64_T = types::i64_t{};
    const type_value U8_T = types::u8_t{};
    const type_value U16_T = types::u16_t{};
    const type_value U32_T = types::u32_t{};
    const type_value U64_T = types::u64_t{};
    const type_value F32_T = types::f32_t{};
    const type_value F64_T = types::f64_t{};
    const type_value BOOL_T = types::bool_t{};
    const type_value CHAR_T = types::char_t{};

    auto operator<<(std::ostream& os, const type_value&) -> std::ostream&;

}}  // namespace bt::analysis
