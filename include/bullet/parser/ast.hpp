#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <iostream>

#include <bullet/lexer/token.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename T>
            using ptr = std::shared_ptr<T>;

            template <typename T>
            struct ref {
                ptr<T> value;

                ref(T&& t): value{std::forward(t)} {}

                ref() = default;
                ref(const ref&) = default;
                ref(ref&&) noexcept = default;
                ref& operator=(const ref&) = default;
                ref& operator=(ref&&) noexcept = default;
                ref& operator=(const T& t) {
                    *value = t;
                }
                ref& operator=(T&& t) noexcept {
                    value.emplace(std::forward(t));
                }
                
                T& get() { return *value; }
                const T& get() const { return *value; }

                operator T&() { return *value; }
                operator const T&() const { return *value; }
            };

            struct tree_t;
            using node_t = ref<tree_t>;

            struct group_t: std::vector<node_t> {
                using base_t = std::vector<node_t>;
                using base_t::base_t;
            };
            auto operator<<(std::ostream& os, const group_t& g) -> std::ostream&;

            using named_node_t = std::pair<lexer::identifier_t, node_t>;
            using named_tree_vector_t = std::vector<named_node_t>;

            struct named_group_t: named_tree_vector_t {
                using base_t = named_tree_vector_t;
                using base_t::base_t;
            };

            auto operator<<(std::ostream& os, const named_group_t& g) -> std::ostream&;

            struct unary_op_t {
                lexer::token_t op;
                node_t operand;
            };

            auto operator<<(std::ostream& os, const unary_op_t& uop) -> std::ostream&;

            struct bin_op_t {
                lexer::token_t op;
                node_t lhs, rhs;
            };

            auto operator<<(std::ostream& os, const bin_op_t& binop) -> std::ostream&;

            struct invoc_t {
                lexer::identifier_t target;
                group_t arguments;
            };

            auto operator<<(std::ostream& os, const invoc_t& invoc)  -> std::ostream&;

            struct if_t {
                node_t test;
                node_t then_branch;
                std::optional<node_t> else_branch;
            };

            auto operator<<(std::ostream& os, const if_t& if_) -> std::ostream&;

            struct assign_t {
                node_t lhs, rhs;
            };
            
            auto operator<<(std::ostream& os, const assign_t& a) -> std::ostream&;

            struct fn_def_t {
                lexer::identifier_t name;
                named_group_t arguments;
                node_t result_type;
            };
            
            auto operator<<(std::ostream& os, const fn_def_t& a) -> std::ostream&;

            struct repeat_t {
                group_t body;
            };

            auto operator<<(std::ostream& os, const repeat_t& repeat) -> std::ostream&;

            struct struct_t: named_tree_vector_t {
                using base_t = named_tree_vector_t;
                using base_t::base_t;
            };

            auto operator<<(std::ostream& os, const named_tree_vector_t& t) -> std::ostream&;

            struct def_type_t {
                lexer::identifier_t name;
                node_t type;
            };

            auto operator<<(std::ostream& os, const def_type_t& d) -> std::ostream&;

            struct let_type_t {
                lexer::identifier_t name;
                node_t type;
            };

            auto operator<<(std::ostream& os, const let_type_t& d) -> std::ostream&;

            struct template_t {
                named_group_t arguments;
                node_t body;
            };

            auto operator<<(std::ostream& os, const template_t& t) -> std::ostream&;


            using node_base_t = std::variant<
                lexer::token_t,
                unary_op_t,
                bin_op_t,
                invoc_t,
                if_t,
                assign_t,
                fn_def_t,
                repeat_t,
                struct_t,
                def_type_t,
                let_type_t,
                template_t,
                node_t
            >;

            struct tree_t : node_base_t {
                using base_t = node_base_t;
                using base_t::base_t;
            };

            auto operator<<(std::ostream& os, const node_t& t) -> std::ostream&;
            auto operator<<(std::ostream& os, const tree_t& pt) -> std::ostream&;
            auto operator<<(std::ostream& os, const group_t& g) -> std::ostream&;
            auto operator<<(std::ostream& os, const if_t& if_) -> std::ostream&;

        }  // namespace ast
    }      // namespace parser
}  // namespace bt
