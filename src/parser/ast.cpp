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
                os << "invoc[" << invoc.target << ": " << invoc.arguments << "]";
                return os;
            }

            auto operator<<(ostream& os, const assign_t& a) -> ostream& {
                os << "assign[" << a.lhs << ", " << a.rhs << "]";
                return os;
            }

            auto operator<<(ostream& os, const fn_def_t& a) -> ostream& {
                os << "def_fn[" << a.name << "]";
                return os;
            }

            auto operator<<(ostream& os, const var_def_t& v) -> ostream& {
                os << "var[" << v;
                if (auto t = v.type)
                    os << ": " << *t;
                os << " = " << v.rhs << "]";
                return os;
            }

            auto operator<<(ostream& os, const repeat_t& repeat) -> ostream& {
                os << "repeat[" << repeat.body << "]";
                return os;
            }

            auto operator<<(ostream& os, const named_tree_vector_t& t) -> ostream& {
                auto first = true;
                os << "group[";
                for (const auto& [ident, subtree] : t) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << ident << ": ";

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
                os << "template[args=" << t.arguments << ", body=" << t.body << "]";
                return os;
            }

            auto operator<<(ostream& os, const tree_t& n) -> ostream& {
                visit([&](const auto& e) { os << e; }, n);
                return os;
            }

            auto operator<<(ostream& os, const node_t& t) -> ostream& {
                os << t.get();
                return os;
            }

            auto operator<<(ostream& os, const group_t& g) -> ostream& {
                auto first = true;
                os << "group[";
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
                auto first = true;
                os << "if[" << if_.test << ", " << if_.then_branch;
                if (const auto& e = if_.else_branch) os << ", " << *e;
                os << "]";
                return os;
            }

            auto operator==(const group_t& l, const group_t& r) -> bool {
                if (l.size() != r.size()) return false;
                for (auto i = 0; i < l.size(); i++)
                    if (l[i] != r[i]) return false;
                return true;
            }
            auto operator!=(const group_t& l, const group_t& r) -> bool { return !(l == r); }

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
                return l.test == r.test && l.then_branch == r.then_branch &&
                       l.else_branch == r.else_branch;
            }
            auto operator!=(const if_t& l, const if_t& r) -> bool { return !(l == r); }

            auto operator==(const assign_t& l, const assign_t& r) -> bool {
                return l.lhs == r.lhs && l.rhs == r.rhs;
            }
            auto operator!=(const assign_t& l, const assign_t& r) -> bool { return !(l == r); }

            auto operator==(const fn_def_t& l, const fn_def_t& r) -> bool {
                return l.name == r.name && l.arguments == r.arguments &&
                       l.result_type == r.result_type;
            }
            auto operator!=(const fn_def_t& l, const fn_def_t& r) -> bool { return !(l == r); }

            auto operator==(const var_def_t& l, const var_def_t& r) -> bool {
                return l.name == r.name && l.type == r.type &&
                       l.rhs == r.rhs;
            }
            auto operator!=(const var_def_t& l, const var_def_t& r) -> bool { return !(l == r); }

            auto operator==(const repeat_t& l, const repeat_t& r) -> bool {
                return l.body == r.body;
            }
            auto operator!=(const repeat_t& l, const repeat_t& r) -> bool { return !(l == r); }

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
        }  // namespace syntax
    }      // namespace parser
}  // namespace bt
