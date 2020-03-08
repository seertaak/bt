#include <iostream>

#include <array>
#include <fstream>
#include <sstream>

#include <catch2/catch.hpp>
#include <rang.hpp>
#include <range/v3/algorithm/copy.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/view/transform.hpp>

#include <bullet/analysis/symtab.hpp>
#include <bullet/analysis/walk.hpp>
#include <bullet/lexer/lexer.hpp>
#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/parser/parser.hpp>

using namespace std;
using namespace bt;
using namespace lexer;
using namespace lexer::token;
using namespace parser;
using namespace syntax;
using namespace rang;
using namespace analysis;

constexpr auto title = R"(
     / /_  __  __/ / /__  / /_
    / __ \/ / / / / / _ \/ __/
   / /_/ / /_/ / / /  __/ /_    
  /_.___/\__,_/_/_/\___/\__/
)"sv;

auto errors = vector<unique_ptr<runtime_error>>();

template <typename T>
struct raise {
    stringstream msg;
    parser::location_t loc;

    template <typename Node>
    raise(const Node& node) : loc(node.get().location) {}
    ~raise() { errors.emplace_back(unique_ptr<runtime_error>(new T(msg, loc))); }
};

template <typename T, typename V>
auto operator<<(raise<T>& r, const V& v) -> raise<T>& {
    r.msg << v;
    return r;
}

template <typename T>
auto print_kind(raise<T>& err, auto invoc, auto type) -> raise<T>& {
    if (type && invoc)
        err << "template";
    else if (type && !invoc)
        err << "type";
    else if (!type && invoc)
        err << "function";
    else
        err << "variable";
    return err;
};

int main(int argc, const char* argv[]) {
    cout << style::bold << fg::blue << title << style::reset << style::bold << fg::green
         << style::italic << "\n  fast. " << fg::blue << "expressive." << style::blink << fg::red
         << " dangerous.\n"
         << style::reset << fg::reset << endl;

    if (argc <= 1) return 0;

    auto source = [&] {
        auto input = ifstream(argv[1]);
        auto buffer = stringstream();
        buffer << input.rdbuf();
        return buffer.str();
    }();

    const lexer::output_t lex_output = source | tokenize;

    cout << lex_output.tokens << endl;
    cout << endl << endl;

    const syntax::tree_t ast = parser::details::parse(lex_output);

    auto s = std::stringstream();
    parser::pretty_print<empty_attribute_t>(ast, s, 0);
    cout << s.str() << endl;

    using noattr = const empty_attribute_t&;

    {
        using st_node_t = symtab<node_t>;
        bool invoc = false;
        bool type = false;

        const auto name_attributes = walk_post_order<st_node_t>(
            ast,
            [](const def_type_t<st_node_t>& e, const auto& node) {
                auto result = st_node_t();
                result.insert(e.name.name, node);
                return result;
            },
            [](const let_type_t<st_node_t>& e, const auto& node) {
                auto result = st_node_t();
                result.insert(e.name.name, node);
                return result;
            },
            [](const var_def_t<st_node_t>& e, const auto& node) {
                auto result = st_node_t();
                result.insert(e.name.name, node);
                return result;
            },
            [&](const type_expr_t<st_node_t>& e, const auto& node) {
                type = true;
                return st_node_t();
            },
            [&](const invoc_t<st_node_t>& e, const auto& node) {
                invoc = true;
                return st_node_t();
            },
            [&](const block_t<st_node_t>& block, const auto& node) {
                auto scope = st_node_t();
                for (auto& stmt : block) {
                    if (stmt.get().is<def_type_t<st_node_t>>() ||
                        stmt.get().is<let_type_t<st_node_t>>() ||
                        stmt.get().is<var_def_t<st_node_t>>()) {
                        auto& [name, def] = *stmt.get().attribute.begin();
                        if (auto pdef = scope.lookup(name)) {
                            auto err = raise<analysis::error>(stmt);
                            err << "Duplicate ";
                            print_kind(err, invoc, type);
                            err << " declaration of \"" << name << "\", with duplicate at "
                                << pdef->get().location;
                        }
                        scope.insert(name, def);
                    }
                }
                return scope;
            },
            [&](auto, auto) {
                invoc = type = false;
                return st_node_t();
            });

        invoc = type = false;

        const auto type_check_pass = walk_pre_order<st_node_t>(
            name_attributes,
            [&](const literal_t& e, const auto& node, const auto& parent_scope) {
                return parent_scope;
            },
            [&](const type_expr_t<st_node_t>& e, const auto& node, const auto& parent_scope) {
                type = true;
                return parent_scope;
            },
            [&](const invoc_t<st_node_t>& e, const auto& node, const auto& parent_scope) {
                invoc = true;
                return parent_scope;
            },
            [&](const fn_expr_t<st_node_t>& e, const auto& node, const auto& parent_scope) {
                return parent_scope;
            },
            [&](const syntax::assign_t<st_node_t>& e, const auto& node, const auto& parent_scope) {
                
                return parent_scope;
            },
            [&](const identifier_t& id, const auto& node, const auto& parent_scope) {
                if (!parent_scope.lookup(id.name)) {
                    auto err = raise<analysis::error>(node);
                    err << "Use of undefined ";
                    print_kind(err, invoc, type);
                    err << " \"" << id.name << "\"";
                }
                return parent_scope;
            },
            [&](const bin_op_t<st_node_t>& e, const auto& node, const auto& parent_scope) {
                return parent_scope;
            },
            [&](const auto& value, const auto& node, const auto& parent_scope) {
                invoc = type = false;

                auto scope = parent_scope;
                scope.insert(node.get().attribute);
                return scope;
            });

        if (errors.empty()) {
            auto s = std::stringstream();
            parser::pretty_print(type_check_pass, s, 0);
            cout << s.str() << endl;
        } else {
            cout << fg::red;
            for (auto&& e : errors) cout << e->what() << endl;
            cout << style::reset << endl;
        }
    }

    return 0;
}
