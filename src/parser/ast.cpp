#include <iostream>
#include <memory>
#include <optional>
#include <variant>

#include <bullet/parser/ast.hpp>
#include <bullet/util.hpp>

namespace bt { namespace parser { namespace syntax {
    using namespace std;
    using namespace lexer;

    /*

    auto operator<<(ostream& os, const assign_t& a) -> ostream& {
        os << "assign[" << a.lhs << ", " << a.rhs << "]";
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
            os << a.arg_names[i];
            const auto& t = a.arg_types[i];
            if (t.get())
                os << ":" << t;
        }
        os << "]";
        if (a.result_type.get())
            os << ", result_tye=" << a.result_type;

        os << ", body=" << a.body << "]";

        return os;
    }

    // def type foo = int   -- foo is a new type.
    // let type foo = int       -- foo is an alias for int, but not a new type.
    // type foo (...)
    // union type foo:

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




    */

}}}  // namespace bt::parser::syntax
