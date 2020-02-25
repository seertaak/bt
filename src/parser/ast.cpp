#include <iostream>
#include <memory>
#include <optional>
#include <variant>

#include <bullet/parser/ast.hpp>
#include <bullet/util.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            using namespace std;
            using namespace lexer;

            auto operator<<(ostream& os, const named_group_t& g) -> ostream& {
                auto first = true;
                os << "named_group[";
                for (const auto& [ident, subtree] : g) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << ident << ": " << subtree.get();
                }
                os << "]";
                return os;
            }

            auto operator<<(ostream& os, const unary_op_t& uop) -> ostream& {
                auto first = false;
                os << "unary_op[" << uop.op << ", " << uop.operand << "]";
                return os;
            }

            auto operator<<(ostream& os, const bin_op_t& binop) -> ostream& {
                auto first = true;
                os << "binary_op[" << binop.op << ", " << binop.lhs << ", " << binop.rhs << "]";
                return os;
            }

            auto operator<<(ostream& os, const invoc_t& invoc) -> ostream& {
                auto first = true;
                os << invoc.target << "(";

                for (auto&& arg : invoc.arguments) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << arg;
                }
                os << ")";
                return os;
            }

            auto operator<<(ostream& os, const assign_t& a) -> ostream& {
                os << "assign[" << a.lhs << ", " << a.rhs << "]";
                return os;
            }

            auto operator<<(ostream& os, const fn_expr_t& a) -> ostream& {
                os << "fn(";
                auto first = true;
                for (auto i = 0; i < a.arg_names.size(); i++) {
                    if (first) first = false;
                    else os << ", ";
                    os << a.arg_names[i] << ":" << a.arg_types[i];
                }
                os << ")";
                if (a.result_type.get())
                    os << " -> " << a.result_type;

                os << " = " << a.body;

                if (!a.closure_params.empty()) {
                    first = true;
                    for (auto&& p: a.closure_params) {
                        if (first) first = false;
                        else os << ", ";
                        os << p;
                    }
                }

                return os;
            }

            auto operator<<(ostream& os, const fn_def_t& a) -> ostream& {
                os << "def_fn[";
                os << a.name;
                os << ", args=[";
                auto first = true;
                for (auto i = 0; i < a.arg_names.size(); i++) {
                    if (first) first = false;
                    else os << ", ";
                    os << a.arg_names[i] << ": " << a.arg_types[i];
                }
                os << "]";
                if (a.result_type.get())
                    os << ", result_tye=" << a.result_type;

                os << ", body=" << a.body << "]";

                first = true;
                for (auto i = 0; i < a.arg_names.size(); i++) {
                    if (first) first = false;
                    else os << ", ";
                    os << a.arg_names[i] << ": " << a.arg_types[i];
                }

                return os;
            }

            auto operator<<(ostream& os, const var_def_t& v) -> ostream& {
                os << "var[" << v.name;
                if (v.type.get()) os << ": " << v.type;
                os << " = " << v.rhs << "]";
                return os;
            }

            auto operator<<(ostream& os, const struct_t& t) -> ostream& {
                auto first = true;
                os << "struct[";
                for (const auto& [ident, subtree] : t) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << ident << ": " << subtree;

                    /*
                    match(subtree, [&] (auto x) {
                        os << x;
                    });
                    */
                }
                os << "]";
                return os;
            }

            // def type foo = int   -- foo is a new type.
            // let type foo = int       -- foo is an alias for int, but not a new type.
            // type foo (...)
            // union type foo:

            auto operator<<(ostream& os, const def_type_t& d) -> ostream& {
                os << "def_type[" << d.name << "=" << d.type << "]";
                return os;
            }

            auto operator<<(ostream& os, const let_type_t& d) -> ostream& {
                os << "let_type[" << d.name << "=" << d.type << "]";
                return os;
            }

            auto operator<<(ostream& os, const template_t& t) -> ostream& {
                os << "template[args=" << t.arguments << ", body=" << t << "]";
                return os;
            }

            auto operator<<(ostream& os, const tree_t& n) -> ostream& {
                visit([&](const auto& e) { os << e; }, n);
                return os;
            }

            auto operator<<(ostream& os, const return_t& t) -> ostream& {
                if (t.value.get())
                    os << "return[" << t.value.get() << "]";
                else
                    os << "return[]";
                return os;
            }

            auto operator<<(ostream& os, const yield_t& t) -> ostream& {
                if (t.value.get())
                    os << "yield[" << t.value.get() << "]";
                else
                    os << "yield[]";
                return os;
            }

            auto operator<<(ostream& os, const break_t& t) -> ostream& {
                os << "break[]";
                return os;
            }
            auto operator<<(ostream& os, const continue_t& t) -> ostream& {
                os << "continue[]";
                return os;
            }

            auto operator<<(ostream& os, const node_t& t) -> ostream& {
                os << t.get();
                return os;
            }

            auto operator<<(ostream& os, const repeat_t& g) -> ostream& {
                auto first = true;
                os << "repeat[";
                for (const auto& pt : g) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << pt;
                }
                os << "]";
                return os;
            }

            auto operator<<(ostream& os, const for_t& f) -> ostream& {
                os << "for[var_lhs=" << f.var_lhs << ", var_rhs=" << f.var_rhs
                    << ", body=" << f.body << "]";
                return os;
            }

            auto operator<<(ostream& os, const while_t& f) -> ostream& {
                os << "while[test=" << f.test << ", body=" << f.body << "]";
                return os;
            }

            auto operator<<(ostream& os, const data_t& g) -> ostream& {
                auto first = true;
                os << "data[";
                for (const auto& pt : g) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << pt;
                }
                os << "]";
                return os;
            }

            auto operator<<(ostream& os, const block_t& g) -> ostream& {
                auto first = true;
                os << "block[";
                for (const auto& pt : g) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << pt;
                }
                os << "]";
                return os;
            }

            auto operator<<(ostream& os, const if_t& if_) -> ostream& {
                os << "if[";
                auto first = true;
                for (auto i = 0; i < if_.elif_tests.size(); i++) {
                    if (first) first = false;
                    else os << ", ";
                    os << if_.elif_tests[i] << " => " << if_.elif_branches[i];
                }
                if (if_.else_branch.get()) os << ", else=" << if_.else_branch;
                os << "]";
                return os;
            }

            auto operator<<(ostream& os, const elif_t& e) -> ostream& {
                os << "elif[test=" << e.test << ", body=" << e.body << "]";
                return os;
            }

            auto operator<<(ostream& os, const else_t& e) -> ostream& {
                os << "else[" << e.body << "]";
                return os;
            }

            auto operator==(const return_t& l, const return_t& r) -> bool {
                return l.value == r.value;
            }
            auto operator!=(const return_t& l, const return_t& r) -> bool {
                return !(l == r);
            }
            auto operator==(const yield_t& l, const yield_t& r) -> bool {
                return l.value == r.value;
            }
            auto operator!=(const yield_t& l, const yield_t& r) -> bool {
                return !(l == r);
            }
            auto operator==(const break_t& l, const break_t& r) -> bool {
                return true;
            }
            auto operator!=(const break_t& l, const break_t& r) -> bool {
                return !(l == r);
            }
            auto operator==(const continue_t& l, const continue_t& r) -> bool {
                return true;
            }
            auto operator!=(const continue_t& l, const continue_t& r) -> bool {
                return !(l == r);
            }

            auto operator==(const data_t& l, const data_t& r) -> bool {
                if (l.size() != r.size()) return false;
                for (auto i = 0; i < l.size(); i++)
                    if (l[i] != r[i]) return false;
                return true;
            }
            auto operator!=(const data_t& l, const data_t& r) -> bool {
                return !(l == r);
            }

            auto operator==(const block_t& l, const block_t& r) -> bool {
                if (l.size() != r.size()) return false;
                for (auto i = 0; i < l.size(); i++)
                    if (l[i] != r[i]) return false;
                return true;
            }
            auto operator!=(const block_t& l, const block_t& r) -> bool { return !(l == r); }

            auto operator==(const repeat_t& l, const repeat_t& r) -> bool {
                if (l.size() != r.size()) return false;
                for (auto i = 0; i < l.size(); i++)
                    if (l[i] != r[i]) return false;
                return true;
            }
            auto operator!=(const repeat_t& l, const repeat_t& r) -> bool { return !(l == r); }

            auto operator==(const for_t& l, const for_t& r) -> bool {
                return l.var_lhs == r.var_lhs && l.var_rhs == r.var_rhs && l.body == r.body;
            }
            auto operator!=(const for_t& l, const for_t& r) -> bool { return !(l == r); }

            auto operator==(const while_t& l, const while_t& r) -> bool {
                return l.test == r.test && l.body == r.body;
            }
            auto operator!=(const while_t& l, const while_t& r) -> bool { return !(l == r); }

            auto operator==(const named_group_t& l, const named_group_t& r) -> bool {
                if (l.size() != r.size()) return false;
                for (auto i = 0; i < l.size(); i++)
                    if (l[i] != r[i]) return false;
                return true;
            }
            auto operator!=(const named_group_t& l, const named_group_t& r) -> bool {
                return !(l == r);
            }

            auto operator==(const unary_op_t& l, const unary_op_t& r) -> bool {
                return l.op == r.op && l.operand == r.operand;
            }

            auto operator!=(const unary_op_t& l, const unary_op_t& r) -> bool { return !(l == r); }

            auto operator==(const bin_op_t& l, const bin_op_t& r) -> bool {
                return l.op == r.op && l.lhs == r.lhs && l.rhs == r.rhs;
            }

            auto operator!=(const bin_op_t& l, const bin_op_t& r) -> bool { return !(l == r); }

            auto operator==(const invoc_t& l, const invoc_t& r) -> bool {
                return l.target == r.target && l.arguments == r.arguments;
            }
            auto operator!=(const invoc_t& l, const invoc_t& r) -> bool { return !(l == r); }

            auto operator==(const if_t& l, const if_t& r) -> bool {
                if (l.elif_tests.size() != r.elif_tests.size()) return false;
                for (auto i = 0; i < l.elif_tests.size(); i++) {
                    if (l.elif_tests[i] != r.elif_tests[i]) 
                        return false;
                    if (l.elif_branches[i] != r.elif_branches[i]) 
                        return false;
                }
                return l.else_branch == r.else_branch;
            }
            auto operator!=(const if_t& l, const if_t& r) -> bool { return !(l == r); }

            auto operator==(const elif_t& l, const elif_t& r) -> bool {
                return l.test == r.test && l.body == r.body;
            }
            auto operator!=(const elif_t& l, const elif_t& r) -> bool { return !(l == r); }

            auto operator==(const else_t& l, const else_t& r) -> bool {
                return l.body == r.body;
            }
            auto operator!=(const else_t& l, const else_t& r) -> bool { return !(l == r); }

            auto operator==(const assign_t& l, const assign_t& r) -> bool {
                return l.lhs == r.lhs && l.rhs == r.rhs;
            }
            auto operator!=(const assign_t& l, const assign_t& r) -> bool { return !(l == r); }

            auto operator==(const fn_expr_t& l, const fn_expr_t& r) -> bool {
                if (l.arg_names.size() != r.arg_names.size())
                    return false;
                for (int i = 0; i < l.arg_names.size(); i++)
                    if (l.arg_names[i] != r.arg_names[i])
                        return false;

                if (l.arg_types.size() != r.arg_types.size())
                    return false;
                for (int i = 0; i < l.arg_types.size(); i++)
                    if (l.arg_types[i] != r.arg_types[i])
                        return false;

                if (l.result_type != r.result_type)
                    return false;

                if (l.body != r.body)
                    return false;

                if (l.closure_params.size() != r.closure_params.size())
                    return false;
                for (int i = 0; i < l.closure_params.size(); i++)
                    if (l.closure_params[i] != r.closure_params[i])
                        return false;

                return true;
            }
            auto operator!=(const fn_expr_t& l, const fn_expr_t& r) -> bool { return !(l == r); }

            auto operator==(const fn_def_t& l, const fn_def_t& r) -> bool {
                if (l.name != r.name)
                    return false;

                if (l.arg_names.size() != r.arg_names.size())
                    return false;
                for (int i = 0; i < l.arg_names.size(); i++)
                    if (l.arg_names[i] != r.arg_names[i])
                        return false;

                if (l.arg_types.size() != r.arg_types.size())
                    return false;
                for (int i = 0; i < l.arg_types.size(); i++)
                    if (l.arg_types[i] != r.arg_types[i])
                        return false;

                if (l.result_type != r.result_type)
                    return false;

                if (l.body != r.body)
                    return false;

                return true;
            }
            auto operator!=(const fn_def_t& l, const fn_def_t& r) -> bool { return !(l == r); }

            auto operator==(const var_def_t& l, const var_def_t& r) -> bool {
                return l.name == r.name && l.type == r.type && l.rhs == r.rhs;
            }
            auto operator!=(const var_def_t& l, const var_def_t& r) -> bool { return !(l == r); }

            auto operator==(const struct_t& l, const struct_t& r) -> bool {
                if (l.size() != r.size()) return false;
                for (auto i = 0; i < l.size(); i++)
                    if (l[i] != r[i]) return false;
                return true;
            }
            auto operator!=(const struct_t& l, const struct_t& r) -> bool { return !(l == r); }

            auto operator==(const def_type_t& l, const def_type_t& r) -> bool {
                return l.name == r.name && l.type == r.type;
            }
            auto operator!=(const def_type_t& l, const def_type_t& r) -> bool { return !(l == r); }
            auto operator==(const let_type_t& l, const let_type_t& r) -> bool {
                return l.name == r.name && l.type == r.type;
            }
            auto operator!=(const let_type_t& l, const let_type_t& r) -> bool { return !(l == r); }
            auto operator==(const template_t& l, const template_t& r) -> bool {
                return l.arguments == r.arguments && l.body == r.body;
            }

            auto operator!=(const template_t& l, const template_t& r) -> bool { return !(l == r); }

            auto operator==(const fn_closure_param_t& l, const fn_closure_param_t& r) -> bool {
                return l.var == r.var && l.identifier == r.identifier && l.expression == r.expression;
            }
            auto operator!=(const fn_closure_param_t& l, const fn_closure_param_t& r) -> bool {
                return !(l == r);
            }

            auto operator<<(std::ostream& os, const fn_closure_param_t& p) -> std::ostream& {
                if (auto v = p.var)
                    os << "var ";
                if (auto i = p.identifier)
                    os << i->name;
                if (p.expression.get())
                    os << "=" << p.expression;
                return os;
            }

        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
