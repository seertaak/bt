#include <iostream>

#include <array>
#include <fstream>
#include <sstream>

#include <catch2/catch.hpp>
#include <rang.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/zip.hpp>

#include <bullet/analysis/symtab.hpp>
#include <bullet/analysis/type.hpp>
#include <bullet/analysis/walk.hpp>
#include <bullet/lexer/lexer.hpp>
#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/parser/parser.hpp>

namespace views = ranges::views;

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
            [&](const fn_expr_t<st_node_t>& e, const auto& node) {
                auto scope = st_node_t();
                for (auto&& [id, ty] : views::zip(e.arg_names, e.arg_types))
                    scope.insert(id.name, node);
                return scope;
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

        using st_type_t = symtab<analysis::type_t>;

        const auto calc_expression_types = walk_post_order<st_type_t>(
            ast,
            [](const block_t<st_type_t>& b, const auto& node) {
                return !b.empty() ? b.back().get().attribute
                                  : st_type_t(analysis::type_t(type_value(types::void_t{})));
            },
            [](const unary_op_t<st_type_t>& e, const auto& node) {
                const auto& ty_ref = e.operand.get().attribute.get();
                const auto& ty = ty_ref.get();

                if (e.op == PLUS || e.op == MINUS) {
                    if (ty != I8_T && ty != I16_T && ty != I32_T && ty != I64_T && ty != U8_T &&
                        ty != U16_T && ty != U32_T && ty != U64_T && ty != F32_T && ty != F64_T) {
                        auto err = raise<analysis::error>(node);
                        err << "Unary plus/minus must be applied to an integral or floating point "
                               "value, not to type "
                            << ty;
                    }
                } else if (e.op == TILDE) {
                    if (ty != I8_T && ty != I16_T && ty != I32_T && ty != I64_T && ty != U8_T &&
                        ty != U16_T && ty != U32_T && ty != U64_T) {
                        auto err = raise<analysis::error>(node);
                        err << "Bitwise complement \"~\" must be applied to an integral value, not "
                               "to type "
                            << ty;
                    }
                } else if (e.op == NOT) {
                    if (ty != BOOL_T) {
                        auto err = raise<analysis::error>(node);
                        err << "Boolean operator \"not\" must be applied to a value of type "
                               "\"bool\"";
                    }
                } else {
                    auto err = raise<analysis::error>(node);
                    err << "Unhandled unary operator " << e.op;
                }
                return st_type_t(ty);
            },
            [](const literal_t& literal, const auto& node) {
                return std::visit(
                    hana::overload(
                        [&](const integral_literal_t& e) {
                            auto result = st_type_t();

                            type_value type;
                            if (e.type == 'i') {
                                switch (e.width) {
                                case 8: type = type_value(types::i8_t()); break;
                                case 16: type = type_value(types::i16_t()); break;
                                case 32: type = type_value(types::i32_t()); break;
                                case 64: type = type_value(types::i64_t()); break;
                                default: {
                                    auto err = raise<analysis::error>(node);
                                    err << "Illegal integer literal width " << e.width
                                        << ", should be 8, 16, 32, or 64 (or unspecified)";
                                }
                                }
                            } else if (e.type == 'u') {
                                switch (e.width) {
                                case 8: type = type_value(types::i8_t()); break;
                                case 16: type = type_value(types::i16_t()); break;
                                case 32: type = type_value(types::i32_t()); break;
                                case 64: type = type_value(types::i64_t()); break;
                                default: {
                                    auto err = raise<analysis::error>(node);
                                    err << "Illegal integer literal width " << e.width
                                        << ", should be 8, 16, 32, or 64 (or unspecified)";
                                }
                                }
                            } else {
                                auto err = raise<analysis::error>(node);
                                err << "Integer literals should be either signed (i) or unsigned "
                                       "(u)";
                            }
                            result.insert(analysis::type_t(type));
                            return result;
                        },
                        [](const string_literal_t& s) {
                            auto result = st_type_t();
                            result.insert(analysis::type_t(
                                types::strlit_t{static_cast<int>(s.value.length())}));
                            return result;
                        },
                        [&](const floating_point_literal_t& e) {
                            auto result = st_type_t();
                            type_value type;
                            switch (e.width) {
                            case 32: type = type_value(types::f32_t()); break;
                            case 64: type = type_value(types::f64_t()); break;
                            default: {
                                auto err = raise<analysis::error>(node);
                                err << "Illegal floating point literal width " << e.width
                                    << ", should be 32 or 64 (or unspecified)";
                            }
                            }
                            result.insert(analysis::type_t(type));
                            return result;
                        },
                        [](const lexer::token::true_t&) {
                            return st_type_t(analysis::type_t(type_value(types::bool_t{})));
                        },
                        [](const lexer::token::false_t&) {
                            return st_type_t(analysis::type_t(type_value(types::bool_t{})));
                        }),
                    literal);
            },
            [&](auto, auto) { return st_type_t(); });

        {
            auto s = std::stringstream();
            parser::pretty_print(calc_expression_types, s, 0);
            cout << "TYPES" << endl << s.str() << endl;
        }

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
                auto scope = parent_scope;
                scope.insert(node.get().attribute);
                return scope;
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
