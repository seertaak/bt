#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <variant>
#include <sstream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>
#include <bullet/lexer/token.hpp>
#include <bullet/util.hpp>
#include <bullet/parser/ast/unary_op.hpp>
#include <bullet/parser/ast/bin_op.hpp>
#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast/invoc.hpp>
#include <bullet/parser/ast/if.hpp>
#include <bullet/parser/ast/elif.hpp>
#include <bullet/parser/ast/else.hpp>
#include <bullet/parser/ast/assign.hpp>
#include <bullet/parser/ast/var_def.hpp>
#include <bullet/parser/ast/fn_expr.hpp>
#include <bullet/parser/attribute.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            /*
            struct return_t { 
                BOOST_HANA_DEFINE_STRUCT(return_t,
                    (node_t, value)
                );
            };
            auto operator<<(std::ostream& os, const return_t&) -> std::ostream&;
            auto operator==(const return_t&, const return_t&) -> bool;
            auto operator!=(const return_t&, const return_t&) -> bool;

            struct yield_t { 
                BOOST_HANA_DEFINE_STRUCT(yield_t,
                    (node_t, value)
                );
            };
            auto operator<<(std::ostream& os, const yield_t&) -> std::ostream&;
            auto operator==(const yield_t&, const yield_t&) -> bool;
            auto operator!=(const yield_t&, const yield_t&) -> bool;

            struct break_t {
                BOOST_HANA_DEFINE_STRUCT(break_t);
            };
            auto operator<<(std::ostream& os, const break_t&) -> std::ostream&;
            auto operator==(const break_t&, const break_t&) -> bool;
            auto operator!=(const break_t&, const break_t&) -> bool;

            struct continue_t {
                BOOST_HANA_DEFINE_STRUCT(continue_t);
            };
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
                BOOST_HANA_DEFINE_STRUCT(while_t,
                    (node_t, test), 
                    (node_t, body)
                );
            };

            auto operator==(const while_t&, const while_t&) -> bool;
            auto operator!=(const while_t&, const while_t&) -> bool;
            auto operator<<(std::ostream& os, const while_t& while_) -> std::ostream&;

            struct for_t {
                BOOST_HANA_DEFINE_STRUCT(for_t,
                    (lexer::identifier_t, var_lhs),
                    (node_t, var_rhs),
                    (node_t, body)
                );
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
                BOOST_HANA_DEFINE_STRUCT(def_type_t,
                    (lexer::identifier_t, name),
                    (node_t, type)
                );
            };

            auto operator==(const def_type_t&, const def_type_t&) -> bool;
            auto operator!=(const def_type_t&, const def_type_t&) -> bool;
            auto operator<<(std::ostream& os, const def_type_t& d) -> std::ostream&;

            struct let_type_t {
                BOOST_HANA_DEFINE_STRUCT(let_type_t,
                    (lexer::identifier_t, name),
                    (node_t, type)
                );
            };

            auto operator==(const let_type_t&, const let_type_t&) -> bool;
            auto operator!=(const let_type_t&, const let_type_t&) -> bool;
            auto operator<<(std::ostream& os, const let_type_t& d) -> std::ostream&;

            struct template_t {
                BOOST_HANA_DEFINE_STRUCT(template_t,
                    (named_group_t, arguments),
                    (node_t, body)
                );
            };

            auto operator==(const template_t&, const template_t&) -> bool;
            auto operator!=(const template_t&, const template_t&) -> bool;
            auto operator<<(std::ostream& os, const template_t& t) -> std::ostream&;
            */

            using string_literal_t = lexer::string_token_t;
            using integral_literal_t = lexer::literal::numeric::integral_t;
            using floating_point_literal_t = lexer::literal::numeric::floating_point_t;

            template <typename Attr>
            using node_base_t = std::variant<std::monostate,
                                             string_literal_t,
                                             integral_literal_t,
                                             floating_point_literal_t,
                                             lexer::identifier_t,
                                             lexer::token::true_t,
                                             lexer::token::false_t,
                                             unary_op_t<Attr>,
                                             bin_op_t<Attr>,
                                             data_t<Attr>,
                                             invoc_t<Attr>,
                                             if_t<Attr>,
                                             elif_t<Attr>,
                                             else_t<Attr>,
                                             var_def_t<Attr>,
                                             fn_expr_t<Attr>,
                                             /*
                                             block_t,
                                             assign_t,
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
                                             */
                                             attr_node_t<Attr>>;


            template <typename Attr>
            struct attr_tree_t : node_base_t<Attr> {
                using base_t = node_base_t<Attr>;
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

            using tree_t = attr_tree_t<empty_attribute_t>;

            //auto operator<<(std::ostream& os, const node_t& t) -> std::ostream&;
            //auto operator<<(std::ostream& os, const tree_t& pt) -> std::ostream&;
            //auto operator<<(std::ostream& os, const if_t& if_) -> std::ostream&;
        }  // namespace syntax

        /*
        inline void pretty_print(const syntax::tree_t& tree, std::stringstream& out, int indent_level) {
            const auto margin = [&] {
                auto out = std::stringstream(); 
                for (auto i = 0; i < indent_level; i++)
                    out << ' ';
                return out.str();
            };

            const auto indent = [&] { indent_level += 4; return ""; };
            const auto dedent = [&] { indent_level -= 4; return ""; };

            using namespace syntax;
            using namespace std;

            std::visit(
                boost::hana::overload(
                    [&] (const string_literal_t& s) { 
                        out << margin() << s << endl; 
                    },
                    [&] (const integral_literal_t& s) { 
                        out << margin() << s << endl; 
                    },
                    [&] (const floating_point_literal_t& f) { 
                        out << margin() << f << endl; 
                    },
                    [&] (const lexer::identifier_t& i) { 
                        out << margin() << i << endl; 
                    },
                    [&] (const lexer::token::true_t& i) { 
                        out << margin() << i << endl; 
                    },
                    [&] (const lexer::token::false_t& i) { 
                        out << margin() << i << endl; 
                    },
                    [&] (const block_t& block) { 
                        out << margin() << "block:" << endl;
                        indent();
                        for (const auto& n: block)
                            pretty_print(n.get(), out, indent_level);
                        dedent();
                    },
                    [&] (const data_t& data) { 
                        out << margin() << "data:" << endl;
                        indent();
                        for (const auto& n: data)
                            pretty_print(n.get(), out, indent_level);
                        dedent();
                    },
                    [&] (const unary_op_t& op) {
                        out << margin() << "unary_op:" << endl;
                        indent();
                        out << margin() << op << endl;
                        pretty_print(op.operand.get(), out, indent_level);
                        dedent();
                    },
                    [&] (const bin_op_t& op) {
                        out << margin() << "binary_op:" << endl;
                        indent();
                        out << margin() << op << endl;
                        pretty_print(op.lhs.get(), out, indent_level);
                        pretty_print(op.rhs.get(), out, indent_level);
                        dedent();
                    },
                    [&] (const invoc_t& i) {
                        out << margin() << "invoke:" << endl;
                        indent();
                        out << margin() << "target:" << endl;
                        indent();
                        pretty_print(i.target.get(), out, indent_level);
                        dedent();
                        out << margin() << "arguments:" << endl;
                        indent();
                        for (const auto& n: i.arguments)
                            pretty_print(n.get(), out, indent_level);
                        dedent();
                        dedent();
                    },
                    [&] (const if_t& i) {
                        out << margin() << "if:" << endl;
                        indent();

                        for (int j = 0; j < i.elif_tests.size(); j++) {
                            out << margin() << "cond:" << endl;
                            indent();
                            pretty_print(i.elif_tests[j].get(), out, indent_level);
                            dedent();
                            out << margin() << "then:" << endl;
                            indent();
                            pretty_print(i.elif_branches[j].get(), out, indent_level);
                            dedent();
                        }
                        out << margin() << "else:" << endl;
                        indent();
                        pretty_print(i.else_branch.get(), out, indent_level);
                        dedent();

                        dedent();
                    },
                    [&] (const elif_t& i) {
                        out << margin() << "elif:" << endl;
                        indent();

                        out << margin() << "test:" << endl;
                        indent();
                        pretty_print(i.test.get(), out, indent_level);
                        dedent();
                        out << margin() << "then:" << endl;
                        indent();
                        pretty_print(i.body.get(), out, indent_level);
                        dedent();

                        dedent();
                    },
                    [&] (const else_t& i) {
                        out << margin() << "else:" << endl;
                        indent();
                        pretty_print(i.body, out, indent_level);
                        dedent();
                    },
                    [&] (const assign_t& i) {
                        out << margin() << "assign:" << endl;

                        indent();
                        out << margin() << "lhs:" << endl;
                        indent();
                        pretty_print(i.lhs.get(), out, indent_level);
                        dedent();
                        dedent();

                        indent();
                        out << margin() << " rhs:" << endl;
                        indent();
                        pretty_print(i.rhs.get(), out, indent_level);
                        dedent();
                        dedent();
                    },
                    [&] (const fn_def_t& f) {
                        out << margin() << "fn_def:" << endl;

                        indent();
                        out << margin() << "name:" << f.name.name << endl;
                        out << margin() << "arguments:" << endl;
                        indent();
                        for (auto i = 0; i < f.arg_names.size(); i++) {
                            out << margin() << "name:" << f.arg_names[i].name << endl;
                            out << margin() << "type:" << endl;
                            indent();
                            pretty_print(f.arg_types[i].get(), out, indent_level);
                            dedent();
                        }
                        dedent();

                        out << margin() << "result_type:" << endl;
                        indent();
                        pretty_print(f.result_type.get(), out, indent_level);
                        dedent();

                        out << margin() << "body:" << endl;
                        indent();
                        pretty_print(f.body.get(), out, indent_level);
                        dedent();

                        dedent();
                    },
                    [&] (const fn_expr_t& f) {
                        out << margin() << "fn_expr:" << endl;

                        indent();
                        out << margin() << "arguments:" << endl;
                        indent();
                        for (auto i = 0; i < f.arg_names.size(); i++) {
                            out << margin() << "name:" << f.arg_names[i].name << endl;
                            out << margin() << "type:" << endl;
                            indent();
                            pretty_print(f.arg_types[i].get(), out, indent_level);
                            dedent();
                        }
                        dedent();

                        out << margin() << "result_type:" << endl;
                        indent();
                        pretty_print(f.result_type.get(), out, indent_level);
                        dedent();

                        out << margin() << "body:" << endl;
                        indent();
                        pretty_print(f.body.get(), out, indent_level);
                        dedent();

                        dedent();
                    },
                    [&] (const var_def_t& f) {
                        out << margin() << "var_def:" << endl;

                        indent();
                        out << margin() << "name:" << f.name.name << endl;
                        out << margin() << "type:" << endl;
                        indent();
                        pretty_print(f.type.get(), out, indent_level);
                        dedent();
                        out << margin() << "rhs:" << endl;
                        indent();
                        pretty_print(f.rhs.get(), out, indent_level);
                        dedent();
                        dedent();
                    },
                    [&] (const repeat_t& v) {
                        out << margin() << "repeat:" << endl;

                        indent();
                        for (const auto& n: v)
                            pretty_print(n.get(), out, indent_level);
                        dedent();
                    },
                    [&] (const for_t& v) {
                        out << margin() << "for:" << endl;

                        indent();
                        out << margin() << "var:" << v.var_lhs << endl;
                        out << margin() << "sequence:" << endl;
                        indent();
                        pretty_print(v.var_rhs.get(), out, indent_level);
                        dedent();
                        out << margin() << "body:" << endl;
                        indent();
                        pretty_print(v.body.get(), out, indent_level);
                        dedent();
                        dedent();
                    },
                    [&] (const while_t& v) {
                        out << margin() << "while:" << endl;

                        indent();
                        out << margin() << "cond:" << endl;
                        indent();
                        pretty_print(v.test.get(), out, indent_level);
                        dedent();
                        out << margin() << "body:" << endl;
                        indent();
                        pretty_print(v.body.get(), out, indent_level);
                        dedent();
                        dedent();
                    },
                    [&] (const break_t& v) { 
                        out << margin() << "break" << endl; 
                    },
                    [&] (const continue_t& v) { 
                        out << margin() << "continue" << endl; 
                    },
                    [&] (const return_t& v) { 
                        out << margin() << "return:" << endl; 
                        indent();
                        pretty_print(v.value.get(), out, indent_level);
                        dedent();
                    },
                    [&] (const yield_t& v) { 
                        out << margin() << "return:" << endl; 
                        indent();
                        pretty_print(v.value.get(), out, indent_level);
                        dedent();
                    },
                    [&] (const struct_t& v) {
                        out << margin() << "struct:" << endl; 
                        indent();
                        for (const auto& e: v) {
                            out << margin() << e.first.name << ":" << endl;
                            indent();
                            pretty_print(e.second.get(), out, indent_level);
                            dedent();
                        }
                        dedent();
                    },
                    [&] (const def_type_t& f) {
                        out << margin() << "def_type:" << endl;

                        indent();
                        out << margin() << "name:" << f.name.name << endl;
                        out << margin() << "defn:" << endl;
                        indent();
                        pretty_print(f.type.get(), out, indent_level);
                        dedent();
                        dedent();
                    },
                    [&] (const let_type_t& f) {
                        out << margin() << "let_type:" << endl;

                        indent();
                        out << margin() << "name:" << f.name.name << endl;
                        out << margin() << "defn:" << endl;
                        indent();
                        pretty_print(f.type.get(), out, indent_level);
                        dedent();
                        dedent();
                    },
                    [&] (const template_t& v) {
                        out << margin() << "template" << endl;
                    },
                    [&] (const node_t&) {},
                    [&] (const std::monostate&) {},
                    [] (auto) {}
                ), tree
            );
        }
        */

    }      // namespace parser
}  // namespace bt
