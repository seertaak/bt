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
    const syntax::tree_t ast = parser::details::parse(lex_output);

    cout << lex_output.tokens << endl;
    cout << endl << endl;

    auto s = std::stringstream();
    parser::pretty_print<empty_attribute_t>(ast, s, 0);
    cout << s.str() << endl;

    using noattr = const empty_attribute_t&;

    {
        using st_node_t = symtab<node_t>;
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
            [&](const block_t<st_node_t>& block, const auto& node) {
                auto scope = st_node_t();
                for (auto& stmt : block) {
                    if (stmt.get().is<def_type_t<st_node_t>>() ||
                        stmt.get().is<let_type_t<st_node_t>>() ||
                        stmt.get().is<var_def_t<st_node_t>>()) {
                        auto& [name, def] = *stmt.get().attribute.begin();
                        if (auto pdef = scope.lookup(name)) {
                            auto msg = std::stringstream();
                            msg << "Duplicate type/variable/fn/template declaration ("
                                << "with duplicate at " << pdef->get().location << ")";
                            throw analysis::error(msg, stmt.get().location);
                        }
                        scope.insert(name, def);
                    }
                }
                return scope;
            },
            [](auto, auto) { return st_node_t(); });

        const auto var_def_top_down_pass = walk_pre_order<st_node_t>(
            name_attributes,
            [&](const identifier_t& id, const auto& node, const auto& parent_scope) {
                if (!parent_scope.lookup(id.name)) {
                    auto msg = std::stringstream();
                    msg << "Use of undefined type/variable/fn/template identifier \"" << id.name
                        << "\"";
                    throw analysis::error(msg, node.get().location);
                }
                return parent_scope;
            },
            [&](const auto& value, const auto& node, const auto& parent_scope) {
                auto scope = parent_scope;
                scope.insert(node.get().attribute);
                return scope;
            });

        auto s = std::stringstream();
        parser::pretty_print(var_def_top_down_pass, s, 0);
        cout << s.str() << endl;
    }

    return 0;
}
