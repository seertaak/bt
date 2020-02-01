#pragma once

#include <memory>
#include <variant>
#include <optional>

namespace bt {

    namespace ast {
        namespace type {
            struct void_t {};
            constexpr auto void_c = hana::type_c<void_t>;

            template <bool S, int W>
            struct integral_t {
                constexpr const static bool is_signed = S;
                constexpr const static int width = W;
            };

            template struct integral_t<true, 8>;
            template struct integral_t<true, 16>;
            template struct integral_t<true, 32>;
            template struct integral_t<true, 64>;

            template struct integral_t<false, 8>;
            template struct integral_t<false, 16>;
            template struct integral_t<false, 32>;
            template struct integral_t<false, 64>;

            using i8_t = integral_t<true, 8>;
            using i16_t = integral_t<true, 16>;
            using i32_t = integral_t<true, 32>;
            using i64_t = integral_t<true, 64>;

            constexpr auto i8_c = hana::type_c<i8_t>;
            constexpr auto i16_c = hana::type_c<i16_t>;
            constexpr auto i32_c = hana::type_c<i32_t>;
            constexpr auto i64_c = hana::type_c<i64_t>;

            using u8_t = integral_t<false, 8>;
            using u16_t = integral_t<false, 16>;
            using u32_t = integral_t<false, 32>;
            using u64_t = integral_t<false, 64>;

            constexpr auto u8_c = hana::type_c<u8_t>;
            constexpr auto u16_c = hana::type_c<u16_t>;
            constexpr auto u32_c = hana::type_c<u32_t>;
            constexpr auto u64_c = hana::type_c<u64_t>;

            template <int W>
            struct floating_point_t {
                constexpr const static int width = W;
            };

            template struct floating_point_t<32>;
            template struct floating_point_t<64>;

            using f32_t = floating_point_t<32>;
            using f64_t = floating_point_t<64>;

            struct void_t {};

            template <int W>
            struct floating_point_t {
                constexpr const static bool is_signed = true;
                constexpr const static int width = W;
            };

            struct boolean_t {
                constexpr const static bool is_signed = false;
                constexpr const static int width = 8;
            };

            struct character_t {
                constexpr const static bool is_signed = false;
                constexpr const static int width = 32;
            };

            // def foo(x: float, xs..: int):
            //    ...

            using primitive_types = std::variant<
                i8_t,
                i16_t,
                i32_t,
                i64_t,
                u8_t,
                u16_t,
                u32_t,
                u64_t,
                f32_t,
                f64_t,
                boolean_t,
                character_t
            >;

            struct type;
            using any_type_t = std::shared_ptr<type>;

            struct name_and_type_t {
                std::string name;
                any_type type;
            };

            using name_and_type_vector_t = std::vector<name_and_type_t>;

            struct function_t {
                any_type_t result_type;
                name_and_type_vector_t arguments;
            };

            struct record_t : name_and_type_vector_t { 
                using name_and_type_vector_t::name_and_type_vector_t;
            };

            using kinds = hana::tuple_t<
                void_t,
                i8_t,
                i16_t,
                i32_t,
                i64_t,
                u8_t,
                u16_t,
                u32_t,
                u64_t,
                f32_t,
                f64_t,
                boolean_t,
                character_t,
                function_t,
                record_t
            >;

            using kind_t = decltype(hana::unpack(all_types, hana::template_<std::variant>))::type;
        }


        struct unary_op_t {
            BOOST_HANA_DEFINE_STRUCT(unary_op_t,
                (lexer::token_t, operand)
                (lexer::token_t, operand)
            )
        };
        
        /*

        */

        struct bin_op_t {};
        struct bool_op_t {};
        struct compare_op_t {};
        struct fn_call_t {};
        struct if_expr_t {};
        struct assign_t {};
        struct raise_t {};
        struct assert_t {};
        struct pass_t {};
        struct import_t {};
        struct import_from_t {};
        struct if_statement_t {};
        struct for_statement_t {};
        struct while_statement_t {};
        struct break_t {};
        struct continue_t{};
        struct try_t {};
        struct fn_def_t {};
        struct fn_lambda_t {};
        struct fn_args_t {};
        struct fn_arg_t {};
        struct return_t {};
        struct struct_t {} ;
        struct union_t {};
        struct array_t {};
    }  // namespace ast
}  // namespace bt
