#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <variant>

#include <boost/hana/all.hpp>

#include <bullet/lexer/location.hpp>
#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast/assign.hpp>
#include <bullet/parser/ast/bin_op.hpp>
#include <bullet/parser/ast/block.hpp>
#include <bullet/parser/ast/break.hpp>
#include <bullet/parser/ast/continue.hpp>
#include <bullet/parser/ast/data.hpp>
#include <bullet/parser/ast/def_type.hpp>
#include <bullet/parser/ast/elif.hpp>
#include <bullet/parser/ast/else.hpp>
#include <bullet/parser/ast/fn_expr.hpp>
#include <bullet/parser/ast/for.hpp>
#include <bullet/parser/ast/if.hpp>
#include <bullet/parser/ast/invoc.hpp>
#include <bullet/parser/ast/let_type.hpp>
#include <bullet/parser/ast/return.hpp>
#include <bullet/parser/ast/struct.hpp>
#include <bullet/parser/ast/template.hpp>
#include <bullet/parser/ast/unary_op.hpp>
#include <bullet/parser/ast/var_def.hpp>
#include <bullet/parser/ast/while.hpp>
#include <bullet/parser/ast/yield.hpp>
#include <bullet/parser/ast_fwd.hpp>
#include <bullet/parser/attribute.hpp>
#include <bullet/util.hpp>

namespace bt { namespace parser {
    namespace syntax {
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
                                         break_t,
                                         continue_t,
                                         unary_op_t<Attr>,
                                         bin_op_t<Attr>,
                                         data_t<Attr>,
                                         invoc_t<Attr>,
                                         if_t<Attr>,
                                         elif_t<Attr>,
                                         else_t<Attr>,
                                         var_def_t<Attr>,
                                         fn_expr_t<Attr>,
                                         block_t<Attr>,
                                         assign_t<Attr>,
                                         for_t<Attr>,
                                         while_t<Attr>,
                                         return_t<Attr>,
                                         yield_t<Attr>,
                                         struct_t<Attr>,
                                         def_type_t<Attr>,
                                         let_type_t<Attr>,
                                         template_t<Attr>,
                                         attr_node_t<Attr>>;

        template <typename Attr>
        struct attr_tree_t : node_base_t<Attr>, with_attr<Attr> {
            using base_t = node_base_t<Attr>;
            using base_t::base_t;

            location_t location;

            template <typename T>
            inline auto is() const -> bool {
                return std::holds_alternative<T>(*this);
            }

            inline operator bool() const { return !is<std::monostate>(); }

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

        template <typename Attr>
        auto operator<<(std::ostream& os, const attr_node_t<Attr>& t) -> std::ostream& {
            os << t.get();
            return os;
        }

        inline auto operator<<(std::ostream& os, const std::monostate& n) -> std::ostream& {
            return os;
        }

        template <typename Attr>
        auto operator<<(std::ostream& os, const attr_tree_t<Attr>& n) -> std::ostream& {
            std::visit([&](const auto& e) { os << e; }, n);
            return os;
        }
    }  // namespace syntax

    template <typename Attr>
    inline void pretty_print(const syntax::attr_tree_t<Attr>& tree,
                             std::stringstream& out,
                             int indent_level) {
        const auto margin = [&] {
            auto out = std::stringstream();
            for (auto i = 0; i < indent_level; i++) out << ' ';
            return out.str();
        };

        const auto indent = [&] {
            indent_level += 4;
            return "";
        };
        const auto dedent = [&] {
            indent_level -= 4;
            return "";
        };

        using namespace syntax;
        using namespace std;

        out << margin() << tree.location << endl;
        std::visit(
            boost::hana::overload(
                [&](const string_literal_t& s) { out << margin() << s << endl; },
                [&](const integral_literal_t& s) { out << margin() << s << endl; },
                [&](const floating_point_literal_t& f) { out << margin() << f << endl; },
                [&](const lexer::identifier_t& i) { out << margin() << i << endl; },
                [&](const lexer::token::true_t& i) { out << margin() << i << endl; },
                [&](const lexer::token::false_t& i) { out << margin() << i << endl; },
                [&](const block_t<Attr>& block) {
                    out << margin() << "block:" << endl;
                    indent();
                    for (const auto& n : block) pretty_print<Attr>(n.get(), out, indent_level);
                    dedent();
                },
                [&](const data_t<Attr>& data) {
                    out << margin() << "data:" << endl;
                    indent();
                    for (const auto& n : data) pretty_print<Attr>(n.get(), out, indent_level);
                    dedent();
                },
                [&](const unary_op_t<Attr>& op) {
                    out << margin() << "unary_op:" << endl;
                    indent();
                    out << margin() << op.op << endl;
                    pretty_print<Attr>(op.operand.get(), out, indent_level);
                    dedent();
                },
                [&](const bin_op_t<Attr>& op) {
                    out << margin() << "binary_op:" << endl;
                    indent();
                    out << margin() << op.op << endl;
                    pretty_print<Attr>(op.lhs.get(), out, indent_level);
                    pretty_print<Attr>(op.rhs.get(), out, indent_level);
                    dedent();
                },
                [&](const invoc_t<Attr>& i) {
                    out << margin() << "invoke:" << endl;
                    indent();
                    out << margin() << "target:" << endl;
                    indent();
                    pretty_print<Attr>(i.target.get(), out, indent_level);
                    dedent();
                    out << margin() << "arguments:" << endl;
                    indent();
                    for (const auto& n : i.arguments)
                        pretty_print<Attr>(n.get(), out, indent_level);
                    dedent();
                    dedent();
                },
                [&](const if_t<Attr>& i) {
                    out << margin() << "if:" << endl;
                    indent();

                    for (int j = 0; j < i.elif_tests.size(); j++) {
                        out << margin() << "cond:" << endl;
                        indent();
                        pretty_print<Attr>(i.elif_tests[j].get(), out, indent_level);
                        dedent();
                        out << margin() << "then:" << endl;
                        indent();
                        pretty_print<Attr>(i.elif_branches[j].get(), out, indent_level);
                        dedent();
                    }
                    out << margin() << "else:" << endl;
                    indent();
                    pretty_print<Attr>(i.else_branch.get(), out, indent_level);
                    dedent();

                    dedent();
                },
                [&](const elif_t<Attr>& i) {
                    out << margin() << "elif:" << endl;
                    indent();

                    out << margin() << "test:" << endl;
                    indent();
                    pretty_print<Attr>(i.test.get(), out, indent_level);
                    dedent();
                    out << margin() << "then:" << endl;
                    indent();
                    pretty_print<Attr>(i.body.get(), out, indent_level);
                    dedent();

                    dedent();
                },
                [&](const else_t<Attr>& i) {
                    out << margin() << "else:" << endl;
                    indent();
                    pretty_print<Attr>(i.body, out, indent_level);
                    dedent();
                },
                [&](const assign_t<Attr>& i) {
                    out << margin() << "assign:" << endl;

                    indent();
                    out << margin() << "lhs:" << endl;
                    indent();
                    pretty_print<Attr>(i.lhs.get(), out, indent_level);
                    dedent();
                    dedent();

                    indent();
                    out << margin() << " rhs:" << endl;
                    indent();
                    pretty_print<Attr>(i.rhs.get(), out, indent_level);
                    dedent();
                    dedent();
                },
                [&](const fn_expr_t<Attr>& f) {
                    out << margin() << "fn_expr:" << endl;

                    indent();
                    out << margin() << "arguments:" << endl;
                    indent();
                    for (auto i = 0; i < f.arg_names.size(); i++) {
                        out << margin() << "name:" << f.arg_names[i] << endl;
                        out << margin() << "type:" << endl;
                        indent();
                        pretty_print<Attr>(f.arg_types[i].get(), out, indent_level);
                        dedent();
                    }
                    dedent();

                    out << margin() << "result_type:" << endl;
                    indent();
                    pretty_print<Attr>(f.result_type.get(), out, indent_level);
                    dedent();

                    out << margin() << "body:" << endl;
                    indent();
                    pretty_print<Attr>(f.body.get(), out, indent_level);
                    dedent();

                    dedent();
                },
                [&](const var_def_t<Attr>& f) {
                    out << margin() << "var_def:" << endl;

                    indent();
                    out << margin() << "name:" << f.name << endl;
                    out << margin() << "type:" << endl;
                    indent();
                    pretty_print<Attr>(f.type.get(), out, indent_level);
                    dedent();
                    out << margin() << "rhs:" << endl;
                    indent();
                    pretty_print<Attr>(f.rhs.get(), out, indent_level);
                    dedent();
                    dedent();
                },
                [&](const for_t<Attr>& v) {
                    out << margin() << "for:" << endl;

                    indent();
                    out << margin() << "var:" << v.var_lhs << endl;
                    out << margin() << "sequence:" << endl;
                    indent();
                    pretty_print<Attr>(v.var_rhs.get(), out, indent_level);
                    dedent();
                    out << margin() << "body:" << endl;
                    indent();
                    pretty_print<Attr>(v.body.get(), out, indent_level);
                    dedent();
                    dedent();
                },
                [&](const while_t<Attr>& v) {
                    out << margin() << "while:" << endl;

                    indent();
                    out << margin() << "cond:" << endl;
                    indent();
                    pretty_print<Attr>(v.test.get(), out, indent_level);
                    dedent();
                    out << margin() << "body:" << endl;
                    indent();
                    pretty_print<Attr>(v.body.get(), out, indent_level);
                    dedent();
                    dedent();
                },
                [&](const break_t& v) { out << margin() << "break" << endl; },
                [&](const continue_t& v) { out << margin() << "continue" << endl; },
                [&](const return_t<Attr>& v) {
                    out << margin() << "return:" << endl;
                    indent();
                    pretty_print<Attr>(v.value.get(), out, indent_level);
                    dedent();
                },
                [&](const yield_t<Attr>& v) {
                    out << margin() << "return:" << endl;
                    indent();
                    pretty_print<Attr>(v.value.get(), out, indent_level);
                    dedent();
                },
                [&](const struct_t<Attr>& v) {
                    out << margin() << "struct:" << endl;
                    indent();
                    for (const auto& e : v) {
                        out << margin() << e.first << ":" << endl;
                        indent();
                        pretty_print<Attr>(e.second.get(), out, indent_level);
                        dedent();
                    }
                    dedent();
                },
                [&](const def_type_t<Attr>& f) {
                    out << margin() << "def_type:" << endl;

                    indent();
                    out << margin() << "name:" << f.name << endl;
                    out << margin() << "defn:" << endl;
                    indent();
                    pretty_print<Attr>(f.type.get(), out, indent_level);
                    dedent();
                    dedent();
                },
                [&](const let_type_t<Attr>& f) {
                    out << margin() << "let_type:" << endl;

                    indent();
                    out << margin() << "name:" << f.name << endl;
                    out << margin() << "defn:" << endl;
                    indent();
                    pretty_print<Attr>(f.type.get(), out, indent_level);
                    dedent();
                    dedent();
                },
                [&](const template_t<Attr>& v) { out << margin() << "template" << endl; },
                [&](const attr_node_t<Attr>&) {}, [&](const std::monostate&) {}, [](auto) {}),
            tree);
    }

}}  // namespace bt::parser
