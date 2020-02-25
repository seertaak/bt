#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <variant>

#include <bullet/lexer/token.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename T>
            using ptr = std::shared_ptr<T>;

            template <typename T>
            struct ref {
                ptr<T> value;

                ref(const T& t) : value{std::make_shared<T>(t)} {}
                ref(T&& t) noexcept : value{std::make_shared<T>(std::move(t))} {}

                ref() : value{std::make_shared<T>()} {}
                ref(const ref&) = default;
                ref(ref&&) noexcept = default;
                ref& operator=(const ref&) = default;
                ref& operator=(ref&&) noexcept = default;
                ref& operator=(const T& t) { *value = t; }
                ref& operator=(T&& t) noexcept { value.emplace(std::forward(t)); }

                T& get() { return *value; }
                const T& get() const { return *value; }

                operator T&() { return *value; }
                operator const T&() const { return *value; }
            };

            template <typename T>
            auto operator==(const ref<T>& l, const ref<T>& r) -> bool {
                return l.get() == r.get();
            }

            template <typename T>
            auto operator==(const ref<T>& l, const T& r) -> bool {
                return l.get() == r;
            }

            template <typename T>
            auto operator==(const T& l, const ref<T>& r) -> bool {
                return l == r.get();
            }

            template <typename T>
            auto operator!=(const ref<T>& l, const ref<T>& r) -> bool {
                return !(l == r);
            }

            template <typename T>
            auto operator!=(const ref<T>& l, const T& r) -> bool {
                return !(l == r);
            }

            template <typename T>
            auto operator!=(const T& l, const ref<T>& r) -> bool {
                return !(l == r);
            }

            struct tree_t;
            using node_t = ref<tree_t>;

            struct group_t : std::vector<node_t> {
                using base_t = std::vector<node_t>;
                using base_t::base_t;
            };
            auto operator<<(std::ostream& os, const group_t& g) -> std::ostream&;
            auto operator==(const group_t&, const group_t&) -> bool;
            auto operator!=(const group_t&, const group_t&) -> bool;

            using named_node_t = std::pair<lexer::identifier_t, node_t>;
            using named_tree_vector_t = std::vector<named_node_t>;

            struct named_group_t : named_tree_vector_t {
                using base_t = named_tree_vector_t;
                using base_t::base_t;
            };
            auto operator==(const named_group_t&, const named_group_t&) -> bool;
            auto operator!=(const named_group_t&, const named_group_t&) -> bool;
            auto operator<<(std::ostream& os, const named_group_t& g) -> std::ostream&;

            struct unary_op_t {
                lexer::token_t op;
                node_t operand;
            };

            auto operator==(const unary_op_t&, const unary_op_t&) -> bool;
            auto operator!=(const unary_op_t&, const unary_op_t&) -> bool;
            auto operator<<(std::ostream& os, const unary_op_t& uop) -> std::ostream&;

            struct bin_op_t {
                lexer::token_t op;
                node_t lhs, rhs;
            };

            auto operator==(const bin_op_t&, const bin_op_t&) -> bool;
            auto operator!=(const bin_op_t&, const bin_op_t&) -> bool;
            auto operator<<(std::ostream& os, const bin_op_t& binop) -> std::ostream&;

            struct data_t : std::vector<node_t> {
                using base_t = std::vector<node_t>;
                using base_t::base_t;
            };

            auto operator==(const data_t&, const data_t&) -> bool;
            auto operator!=(const data_t&, const data_t&) -> bool;
            auto operator<<(std::ostream& os, const data_t& repeat) -> std::ostream&;

            struct invoc_t {
                node_t target;
                data_t arguments;
            };

            auto operator==(const invoc_t&, const invoc_t&) -> bool;
            auto operator!=(const invoc_t&, const invoc_t&) -> bool;
            auto operator<<(std::ostream& os, const invoc_t& invoc) -> std::ostream&;
            
            struct if_t {
                std::vector<node_t> elif_tests, elif_branches;
                node_t else_branch;
            };

            auto operator==(const if_t&, const if_t&) -> bool;
            auto operator!=(const if_t&, const if_t&) -> bool;
            auto operator<<(std::ostream& os, const if_t& if_) -> std::ostream&;

            struct elif_t {
                node_t test;
                node_t body;
            };

            auto operator==(const elif_t&, const elif_t&) -> bool;
            auto operator!=(const elif_t&, const elif_t&) -> bool;
            auto operator<<(std::ostream& os, const elif_t& elif_) -> std::ostream&;

            struct else_t {
                node_t body;
            };

            auto operator==(const else_t&, const else_t&) -> bool;
            auto operator!=(const else_t&, const else_t&) -> bool;
            auto operator<<(std::ostream& os, const else_t& else_) -> std::ostream&;

            struct assign_t {
                node_t lhs, rhs;
            };

            auto operator==(const assign_t&, const assign_t&) -> bool;
            auto operator!=(const assign_t&, const assign_t&) -> bool;
            auto operator<<(std::ostream& os, const assign_t& a) -> std::ostream&;

            struct var_def_t {
                lexer::identifier_t name;
                node_t type;
                node_t rhs;
            };

            auto operator==(const var_def_t&, const var_def_t&) -> bool;
            auto operator!=(const var_def_t&, const var_def_t&) -> bool;
            auto operator<<(std::ostream& os, const var_def_t& a) -> std::ostream&;

            struct fn_closure_param_t {
                bool var;
                std::optional<lexer::identifier_t> identifier;
                node_t expression;
            };

            auto operator==(const fn_closure_param_t&, const fn_closure_param_t&) -> bool;
            auto operator!=(const fn_closure_param_t&, const fn_closure_param_t&) -> bool;
            auto operator<<(std::ostream& os, const fn_closure_param_t& a) -> std::ostream&;

            struct fn_expr_t {
                std::vector<lexer::identifier_t> arg_names;
                std::vector<node_t> arg_types;
                node_t result_type;
                node_t body;
                std::vector<fn_closure_param_t> closure_params;
            };

            auto operator==(const fn_expr_t&, const fn_expr_t&) -> bool;
            auto operator!=(const fn_expr_t&, const fn_expr_t&) -> bool;
            auto operator<<(std::ostream& os, const fn_expr_t& a) -> std::ostream&;

            struct fn_def_t {
                lexer::identifier_t name;
                std::vector<lexer::identifier_t> arg_names;
                std::vector<node_t> arg_types;
                node_t result_type;
                node_t body;
            };

            auto operator==(const fn_def_t&, const fn_def_t&) -> bool;
            auto operator!=(const fn_def_t&, const fn_def_t&) -> bool;
            auto operator<<(std::ostream& os, const fn_def_t& a) -> std::ostream&;

            struct return_t { 
                node_t value;
            };
            auto operator<<(std::ostream& os, const return_t&) -> std::ostream&;
            auto operator==(const return_t&, const return_t&) -> bool;
            auto operator!=(const return_t&, const return_t&) -> bool;

            struct yield_t { 
                node_t value;
            };
            auto operator<<(std::ostream& os, const yield_t&) -> std::ostream&;
            auto operator==(const yield_t&, const yield_t&) -> bool;
            auto operator!=(const yield_t&, const yield_t&) -> bool;

            struct break_t {};
            auto operator<<(std::ostream& os, const break_t&) -> std::ostream&;
            auto operator==(const break_t&, const break_t&) -> bool;
            auto operator!=(const break_t&, const break_t&) -> bool;

            struct continue_t {};
            auto operator<<(std::ostream& os, const continue_t&) -> std::ostream&;
            auto operator==(const continue_t&, const continue_t&) -> bool;
            auto operator!=(const continue_t&, const continue_t&) -> bool;

            struct repeat_t : std::vector<node_t> {
                using base_t = std::vector<node_t>;
                using base_t::base_t;
            };

            auto operator==(const repeat_t&, const repeat_t&) -> bool;
            auto operator!=(const repeat_t&, const repeat_t&) -> bool;
            auto operator<<(std::ostream& os, const repeat_t& repeat) -> std::ostream&;

            struct while_t {
                node_t test, body;
            };

            auto operator==(const while_t&, const while_t&) -> bool;
            auto operator!=(const while_t&, const while_t&) -> bool;
            auto operator<<(std::ostream& os, const while_t& while_) -> std::ostream&;

            struct for_t {
                lexer::identifier_t var_lhs;
                node_t var_rhs, body;
            };

            auto operator==(const for_t&, const for_t&) -> bool;
            auto operator!=(const for_t&, const for_t&) -> bool;
            auto operator<<(std::ostream& os, const for_t& for_) -> std::ostream&;

            struct block_t : std::vector<node_t> {
                using base_t = std::vector<node_t>;
                using base_t::base_t;
            };

            auto operator==(const block_t&, const block_t&) -> bool;
            auto operator!=(const block_t&, const block_t&) -> bool;
            auto operator<<(std::ostream& os, const block_t& block) -> std::ostream&;

            struct struct_t : named_tree_vector_t {
                using base_t = named_tree_vector_t;
                using base_t::base_t;
            };

            auto operator==(const struct_t&, const struct_t&) -> bool;
            auto operator!=(const struct_t&, const struct_t&) -> bool;
            auto operator<<(std::ostream& os, const struct_t& t) -> std::ostream&;

            struct def_type_t {
                lexer::identifier_t name;
                node_t type;
            };

            auto operator==(const def_type_t&, const def_type_t&) -> bool;
            auto operator!=(const def_type_t&, const def_type_t&) -> bool;
            auto operator<<(std::ostream& os, const def_type_t& d) -> std::ostream&;

            struct let_type_t {
                lexer::identifier_t name;
                node_t type;
            };

            auto operator==(const let_type_t&, const let_type_t&) -> bool;
            auto operator!=(const let_type_t&, const let_type_t&) -> bool;
            auto operator<<(std::ostream& os, const let_type_t& d) -> std::ostream&;

            struct template_t {
                named_group_t arguments;
                node_t body;
            };

            auto operator==(const template_t&, const template_t&) -> bool;
            auto operator!=(const template_t&, const template_t&) -> bool;
            auto operator<<(std::ostream& os, const template_t& t) -> std::ostream&;

            using string_literal_t = lexer::string_token_t;
            using integral_literal_t = lexer::literal::numeric::integral_t;
            using floating_point_literal_t = lexer::literal::numeric::floating_point_t;

            using node_base_t = std::variant<std::monostate,
                                             string_literal_t,
                                             integral_literal_t,
                                             floating_point_literal_t,
                                             lexer::identifier_t,
                                             lexer::token::true_t,
                                             lexer::token::false_t,
                                             block_t,
                                             data_t,
                                             unary_op_t,
                                             bin_op_t,
                                             invoc_t,
                                             if_t,
                                             elif_t,
                                             else_t,
                                             assign_t,
                                             fn_def_t,
                                             fn_expr_t,
                                             var_def_t,
                                             repeat_t,
                                             for_t,
                                             while_t,
                                             break_t,
                                             continue_t,
                                             return_t,
                                             yield_t,
                                             struct_t,
                                             def_type_t,
                                             let_type_t,
                                             template_t,
                                             node_t>;



            struct tree_t : node_base_t {
                using base_t = node_base_t;
                using base_t::base_t;

                template <typename T>
                inline auto is() const -> bool {
                    return std::holds_alternative<T>(*this);
                }

                inline operator bool() const {
                    return !is<std::monostate>();
                }

                template <typename T>
                inline auto get() const -> const T& {
                    return std::get<T>(*this);
                }

                template <typename T>
                inline auto get() -> T& {
                    return std::get<T>(*this);
                }

                template <typename T>
                inline auto as() const -> const T& {
                    return std::get<T>(*this);
                }

                template <typename T>
                inline auto as() -> T& {
                    return std::get<T>(*this);
                }

                template <typename T>
                inline auto get_if() const -> const T* {
                    return std::get_if<T>(this);
                }

                template <typename T>
                inline auto get_if() -> T* {
                    return std::get_if<T>(this);
                }
            };

            auto operator<<(std::ostream& os, const node_t& t) -> std::ostream&;
            auto operator<<(std::ostream& os, const tree_t& pt) -> std::ostream&;
            auto operator<<(std::ostream& os, const group_t& g) -> std::ostream&;
            auto operator<<(std::ostream& os, const if_t& if_) -> std::ostream&;
        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
