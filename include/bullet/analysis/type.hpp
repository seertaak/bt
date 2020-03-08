namespace bt { namespace analysis { namespace type {
    struct void_t {
        auto operator<=>(const void_t&) const = default;
    };
    inline auto operator<<(std::ostream& os, const void_t&) {
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

    template <typename FromType, typename ToType>
    template struct int_t<true, 8>;
    template struct int_t<true, 16>;
    template struct int_t<true, 32>;
    template struct int_t<true, 64>;

    template struct int_t<false, 8>;
    template struct int_t<false, 16>;
    template struct int_t<false, 32>;
    template struct int_t<false, 64>;

    using i8_t = int_t<true, 8>;
    using i16_t = int_t<true, 16>;
    using i32_t = int_t<true, 32>;
    using i64_t = int_t<true, 64>;

    constexpr auto i8_c = hana::type_c<i8_t>;
    constexpr auto i16_c = hana::type_c<i16_t>;
    constexpr auto i32_c = hana::type_c<i32_t>;
    constexpr auto i64_c = hana::type_c<i64_t>;

    using u8_t = int_t<false, 8>;
    using u16_t = int_t<false, 16>;
    using u32_t = int_t<false, 32>;
    using u64_t = int_t<false, 64>;

    constexpr auto u8_c = hana::type_c<u8_t>;
    constexpr auto u16_c = hana::type_c<u16_t>;
    constexpr auto u32_c = hana::type_c<u32_t>;
    constexpr auto u64_c = hana::type_c<u64_t>;

    template <int W>
    struct float_t {
        static_assert(W == 32 || W == 64);

        constexpr const static int width = W;
        constexpr const static int is_signed = true;
    };

    template <int W>
    auto operator<<(std::ostream& os, const float_t<S, W>& t) {
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
        auto operator<=>(const bool_t&) const = default;
    };

    using primitive_types = hana::tuple_t<i8_t, i16_t, i32_t, i64_t, u8_t, u16_t, u32_t, u64_t, f32_t, f64_t, bool_t, char_t>;

    struct type;
    using any_type_t = bt::ref<type>;

    struct name_and_type_t {
        std::string name;
        any_type type;
        auto operator<=>(const name_and_type_t&) const = default;
    };

    using name_and_type_vector_t = std::vector<name_and_type_t>;

    struct function_t {
        any_type_t result_type;
        name_and_type_vector_t formal_parameters;
        auto operator<=>(const function_t&) const = default;
    };

    struct struct_t : name_and_type_vector_t {
        using base_t = name_and_type_vector_t;
        using base_t::base_t;
        auto operator<=>(const struct_t&) const = default;
    };

    struct optional_name_and_type_t {
        std::optional<std::string> name;
        any_type type;
        auto operator<=>(const optional_name_and_type_t&) const = default;
    };

    struct tuple_t : std::vector<optional_name_and_type_t> {
        using base_t = std::vector<optional_name_and_type_t>;
        using base_t::base_t;
        auto operator<=>(const tuple_t&) const = default;
    };

    struct ptr_t {
        any_type_t value_type;
        std::optional<function_t> allocator;
        auto operator<=>(const ptr_t&) const = default;
    };

    struct array_t {
        any_type_t value_type;
        std::vector<size_t> size;
        auto operator<=>(const array_t&) const = default;
    };

    struct dynarr_t {
        any_type_t value_type;
        std::optional<function_t> allocator;
        auto operator<=>(const dynarr_t&) const = default;
    };

    struct slice_t {
        any_type_t value_type;
        int64_t begin, end, stride;
        auto operator<=>(const dynarr_t&) const = default;
    };

    struct strlit_t {
        int size;
        auto operator<=>(const textlit_t&) const = default;
    };

    struct variant_t : std::vector<any_type_t> {
        using base_t = std::vector<any_type_t>;
        using base_t::base_t;
        auto operator<=>(const variant_t&) const = default;
    };
    
    struct string_t {
        std::optional<function_t> allocator;
        auto operator<=>(const string_t&) const = default;
    };

    using compound_types = hana::tuple_t<function_t,
                                         struct_t,
                                         tuple_t,
                                         ptr_t,
                                         array_t,
                                         dynarr_t,
                                         slice_t,
                                         strlit_t,
                                         variant_t,
                                         string_t>;

    using builtin_types = hana::concat(primitive_types, compound_types);

    using kind_t = decltype(hana::unpack(builtin_types, hana::template_<std::variant>))::type;
}}}  // namespace bt::analysis::type
