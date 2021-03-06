#include <iostream>

#include <array>
#include <fstream>
#include <sstream>

#include <catch2/catch.hpp>
#include <rang.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/zip.hpp>

#include <boost/stacktrace.hpp>
#include <bullet/analysis/error.hpp>
#include <bullet/analysis/prelude_environment.hpp>
#include <bullet/analysis/symtab.hpp>
#include <bullet/analysis/type.hpp>
#include <bullet/analysis/type_checking.hpp>
#include <bullet/analysis/walk.hpp>
#include <bullet/banner.hpp>
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
    // try {
    cout << coloured_banner() << endl;

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
    cout << fg::cyan << "AST:" << style::reset << endl;
    parser::pretty_print<empty_attribute_t>(ast, s, 0);
    cout << s.str() << endl;

    attr_node_t<analysis::type_t> typed_ast =
        walk_post_order<analysis::type_t>(ast, [](auto, auto) { return analysis::type_t(); });

    auto builtins = lang::prelude::environment();
    type_check(typed_ast, builtins);

    {
        auto s = std::stringstream();
        cout << fg::cyan << "Typed AST:" << style::reset << endl;
        parser::pretty_print(typed_ast.get(), s, 0);
        cout << s.str() << endl << endl;

        cout << fg::red << "ERRORS:";
        for (auto&& e : analysis::error::errors) cout << e->what() << endl;
        cout << style::reset << endl;
    }

    /*
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
                        auto& [name, def] = stmt.get().attribute.single_binding();
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

        static_assert(analysis::types::is_int_v<types::int_t<true, 32>>);
        static_assert(not analysis::types::is_int_v<types::float_t<32>>);

        const auto calc_expression_types = walk_post_order<st_type_t>(
            ast,
            [](const block_t<st_type_t>& b, const auto& node) {
                return !b.empty() ? b.back().get().attribute
                                  : st_type_t(analysis::type_t(type_value(types::void_t{})));
            },
            [](const syntax::break_t&, const auto& node) { return st_type_t(VOID); },
            [](const syntax::continue_t&, const auto& node) { return st_type_t(VOID); },
            [](const primitive_type_t& e, const auto& node) {
                return visit(hana::overload(
                                 [](const lexer::token::char_t& t) { return st_type_t(CHAR); },
                                 [](const lexer::token::byte_t& t) { return st_type_t(I8); },
                                 [](const lexer::token::short_t& t) { return st_type_t(I16); },
                                 [](const lexer::token::int_t& t) { return st_type_t(I32); },
                                 [](const lexer::token::long_t& t) { return st_type_t(I64); },
                                 [](const lexer::token::ubyte_t& t) { return st_type_t(U8); },
                                 [](const lexer::token::ushort_t& t) { return st_type_t(U16); },
                                 [](const lexer::token::uint_t& t) { return st_type_t(U32); },
                                 [](const lexer::token::ulong_t& t) { return st_type_t(U64); },
                                 [](const lexer::token::float_t& t) { return st_type_t(F32); },
                                 [](const lexer::token::double_t& t) { return st_type_t(F64); },
                                 [](const lexer::token::i8_t& t) { return st_type_t(I8); },
                                 [](const lexer::token::i16_t& t) { return st_type_t(I16); },
                                 [](const lexer::token::i32_t& t) { return st_type_t(I32); },
                                 [](const lexer::token::i64_t& t) { return st_type_t(I64); },
                                 [](const lexer::token::u8_t& t) { return st_type_t(U8); },
                                 [](const lexer::token::u16_t& t) { return st_type_t(U16); },
                                 [](const lexer::token::u32_t& t) { return st_type_t(U32); },
                                 [](const lexer::token::u64_t& t) { return st_type_t(U64); },
                                 [](const lexer::token::ptr_t& t) {
                                     return st_type_t(type_value(types::ptr_t{VOID}));
                                 },
                                 [](const lexer::token::array_t& t) {
                                     return st_type_t(type_value(types::array_t{VOID, {}}));
                                 },
                                 [](const lexer::token::slice_t& t) {
                                     return st_type_t(type_value(types::slice_t{VOID, 0, 0, 0}));
                                 },
                                 [](const lexer::token::variant_t& t) {
                                     return st_type_t(type_value(types::variant_t{}));
                                 },
                                 [](const lexer::token::tuple_t& t) {
                                     return st_type_t(type_value(types::tuple_t{}));
                                 }),
                             e);
            },
            [](const bin_op_t<st_type_t>& e, const auto& node) {
                const auto& lhs_ty_ref = e.lhs.get().attribute.value();
                const auto& lhs_ty = lhs_ty_ref.get();

                const auto& rhs_ty_ref = e.rhs.get().attribute.value();
                const auto& rhs_ty = rhs_ty_ref.get();

                if (e.op == PLUS || e.op == MINUS || e.op == SLASH || e.op == STAR ||
                    e.op == PERCENTAGE) {
                    if (auto promoted_ty = promoted_type(lhs_ty, rhs_ty))
                        return st_type_t(*promoted_ty);
                    else {
                        auto err = raise<analysis::error>(node);
                        err << "No implicit conversion exists for types \"" << lhs_ty << "\" and \""
                            << rhs_ty << "\"";
                    }
                } else if (e.op == AND || e.op == OR) {
                    if (lhs_ty == BOOL && rhs_ty == BOOL)
                        return st_type_t(BOOL);
                    else {
                        auto err = raise<analysis::error>(node);
                        if (lhs_ty != BOOL && rhs_ty == BOOL)
                            err << "Boolean binary operator applied to non-boolean type \""
                                << lhs_ty << "\"";
                        else if (rhs_ty != BOOL && lhs_ty == BOOL)
                            err << "Boolean binary operator applied to non-boolean type \""
                                << rhs_ty << "\"";
                        else
                            err << "Boolean binary operator applied to non-boolean types \""
                                << lhs_ty << "\" and \"" << rhs_ty << "\"";
                    }
                } else {
                    auto err = raise<analysis::error>(node);
                    err << "Unhandled binary operator " << e.op;
                }
                return st_type_t(VOID);
            },
            [](const unary_op_t<st_type_t>& e, const auto& node) {
                const auto& ty_ref = e.operand.get().attribute.value();
                const auto& ty = ty_ref.get();

                if (e.op == PLUS || e.op == MINUS) {
                    if (!analysis::is_integral(ty) && !analysis::is_floating_point(ty)) {
                        auto err = raise<analysis::error>(node);
                        err << "Unary plus/minus must be applied to an integral or floating point "
                               "value, not to type "
                            << ty;
                    }
                } else if (e.op == TILDE) {
                    if (!analysis::is_integral(ty)) {
                        auto err = raise<analysis::error>(node);
                        err << "Bitwise complement \"~\" must be applied to an integral value, not "
                               "to type "
                            << ty;
                    }
                } else if (e.op == NOT) {
                    if (ty != BOOL) {
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
                                case 8: type = type_value(types::u8_t()); break;
                                case 16: type = type_value(types::u16_t()); break;
                                case 32: type = type_value(types::u32_t()); break;
                                case 64: type = type_value(types::u64_t()); break;
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
    */

    return 0;
    //} catch (...) {
    // cout << "OH SHIT" << endl;
    // std::cout << boost::stacktrace::stacktrace() << endl;
    //}
}
