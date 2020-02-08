#include <memory>
#include <optional>
#include <variant>
#include <iostream>

#include <bullet/parser/ast.hpp>
#include <bullet/util.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            using namespace std;
            using namespace lexer;

            auto operator<<(ostream& os, const named_group_t& g) -> ostream& {
                auto first = false;
                os << "named_group[";
                for (const auto& [ident, subtree]: g) {
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
                auto first = false;
                os << "binary_op[" << binop.op << ", " << binop.lhs
                 << ", " << binop.rhs << "]";
                return os;
            }

            auto operator<<(ostream& os, const invoc_t& invoc) -> ostream& {
                auto first = false;
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


            auto operator<<(ostream& os, const def_type_t& d) -> ostream&  {
                os << "def_type[" << d.name << "=" << d.type << "]";
                return os;
            }

            auto operator<<(ostream& os, const let_type_t& d) -> ostream&  {
                os << "let_type[" << d.name << "=" << d.type << "]";
                return os;
            }

            auto operator<<(ostream& os, const template_t& t) -> ostream&  {
                os << "template[args=" << t.arguments << ", body=" << t.body << "]";
                return os;
            }

            auto operator<<(ostream& os, const tree_t& n) -> ostream&  {
                visit([&] (const auto& e) { os << e; }, n); 
                return os;
            }

            auto operator<<(ostream& os, const node_t& t) -> ostream&  {
                os << t.get();
                return os;
            }

            auto operator<<(ostream& os, const group_t& g) -> ostream&  {
                auto first = false;
                os << "group[";
                for (const auto& pt: g) {
                    if (first)
                        first = false;
                    else
                        os << ", ";
                    os << pt;
                }
                os << "]";
                return os;
            }

            auto operator<<(ostream& os, const if_t& if_) -> ostream&  {
                auto first = false;
                os << "if[" << if_.test << ", " << if_.then_branch;
                if (const auto& e = if_.else_branch)
                    os << ", " << *e;
                os << "]";
                return os;
            }

        }  // namespace ast
    }      // namespace parser
}  // namespace bt

