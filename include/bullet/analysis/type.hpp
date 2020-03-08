#pragma once

#include <compare>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <boost/hana/all.hpp>

#include <bullet/util.hpp>

namespace bt { namespace analysis {
    namespace types {
        namespace hana = boost::hana;

        struct void_t {};
        inline auto operator<<(std::ostream& os, const void_t&) -> std::ostream& {
            os << "void";
            return os;
        }

        template <bool S, int W>
        struct int_t {
            static_assert(W == 8 || W == 16 || W == 32 || W == 64);

            constexpr const static bool is_signed = S;
            constexpr const static int width = W;

            auto operator<=>(const int_t&) const = default;
        };

        template <bool S, int W>
        auto operator<<(std::ostream& os, const int_t<S, W>& t) {
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
        auto operator<<(std::ostream& os, const float_t<W>& t) {
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

        struct char_t {
            constexpr const static bool is_signed = false;
            constexpr const static int width = 32;

            auto operator<=>(const char_t&) const = default;
        };

        struct type;
        using any_type_t = bt::ref<type>;

        struct name_and_type_t {
            std::string name;
            any_type_t type;

            auto operator<=>(const name_and_type_t&) const = default;
        };

        using name_and_type_vector_t = std::vector<name_and_type_t>;

        struct function_t {
            any_type_t result_type;
            name_and_type_vector_t formal_parameters;
        };

        struct struct_t : name_and_type_vector_t {
            using base_t = name_and_type_vector_t;
            using base_t::base_t;
        };

        struct optional_name_and_type_t {
            std::optional<std::string> name;
            any_type_t type;
        };

        struct tuple_t : std::vector<optional_name_and_type_t> {
            using base_t = std::vector<optional_name_and_type_t>;
            using base_t::base_t;
        };

        struct ptr_t {
            any_type_t value_type;
            std::optional<function_t> allocator;
        };

        struct array_t {
            any_type_t value_type;
            std::vector<size_t> size;
        };

        struct dynarr_t {
            any_type_t value_type;
            std::optional<function_t> allocator;
        };

        struct slice_t {
            any_type_t value_type;
            int64_t begin, end, stride;
        };

        struct strlit_t {
            int size;
        };

        struct variant_t : std::vector<any_type_t> {
            using base_t = std::vector<any_type_t>;
            using base_t::base_t;
        };

        struct string_t {
            std::optional<function_t> allocator;
        };
    }  // namespace types

    using type_base_t = std::variant<types::i8_t,
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

    struct type : type_base_t {
        using type_base_t::type_base_t;
    };

}}  // namespace bt::analysis
