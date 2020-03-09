#pragma once

#include <variant>
#include <vector>
#include <tuple>
#include <type_traits>

#include <bullet/parser/ast.hpp>
#include <bullet/analysis/symtab.hpp>
#include <bullet/analysis/type.hpp>

namespace bt { namespace analysis {
    using namespace std;
    namespace hana = boost::hana;
    using namespace parser::syntax;

    using st_type_t = symtab<type_t>;

    auto type_check(const parser::syntax::tree_t& ast, const symtab<type_t>& symtab) 
        -> parser::syntax::attr_tree_t<st_type_t> {
        return visit(hana::overload(
                       [&](const primitive_type_t& i) { 
                            return visit(
                                hana::overload([](const lexer::token::char_t& t) { return st_type_t(CHAR_T); },
                                               [](const lexer::token::byte_t& t) { return st_type_t(I8_T); },
                                               [](const lexer::token::short_t& t) { return st_type_t(I16_T); },
                                               [](const lexer::token::int_t& t) { return st_type_t(I32_T); },
                                               [](const lexer::token::long_t& t) { return st_type_t(I64_T); },
                                               [](const lexer::token::ubyte_t& t) { return st_type_t(U8_T); },
                                               [](const lexer::token::ushort_t& t) { return st_type_t(U16_T); },
                                               [](const lexer::token::uint_t& t) { return st_type_t(U32_T); },
                                               [](const lexer::token::ulong_t& t) { return st_type_t(U64_T); },
                                               [](const lexer::token::float_t& t) { return st_type_t(F32_T); },
                                               [](const lexer::token::double_t& t) { return st_type_t(F64_T); },
                                               [](const lexer::token::i8_t& t) { return st_type_t(I8_T); },
                                               [](const lexer::token::i16_t& t) { return st_type_t(I16_T); },
                                               [](const lexer::token::i32_t& t) { return st_type_t(I32_T); },
                                               [](const lexer::token::i64_t& t) { return st_type_t(I64_T); },
                                               [](const lexer::token::u8_t& t) { return st_type_t(U8_T); },
                                               [](const lexer::token::u16_t& t) { return st_type_t(U16_T); },
                                               [](const lexer::token::u32_t& t) { return st_type_t(U32_T); },
                                               [](const lexer::token::u64_t& t) { return st_type_t(U64_T); },
                                               [](const lexer::token::ptr_t& t) {
                                                   return st_type_t(type_value(types::ptr_t{VOID_T}));
                                               },
                                               [](const lexer::token::array_t& t) {
                                                   return st_type_t(type_value(types::array_t{VOID_T, {}}));
                                               },
                                               [](const lexer::token::slice_t& t) {
                                                   return st_type_t(type_value(types::slice_t{VOID_T, 0, 0, 0}));
                                               },
                                               [](const lexer::token::variant_t& t) {
                                                   return st_type_t(type_value(types::variant_t{}));
                                               },
                                               [](const lexer::token::tuple_t& t) {
                                                   return st_type_t(type_value(types::tuple_t{}));
                                               }),
                                e);
                       },
                       [&](const literal_t& i) { 
                            return visit(
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
                                        result.insert(
                                            analysis::type_t(types::strlit_t{static_cast<int>(s.value.length())}));
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
                       [&](const lexer::identifier_t& id) { 
                            
                       },
                       [&](const block_t<Attr>& block) {
                            auto scope = st_node_t();
                            for (auto& stmt : block) {
                                if (stmt.get().is<var_def_t<st_node_t>>()) {
                                    
                                }
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
                       [&](const data_t<Attr>& data) {
                       },
                       [&](const unary_op_t<Attr>& op) {
                       },
                       [&](const bin_op_t<Attr>& op) {
                       },
                       [&](const invoc_t<Attr>& i) {
                       },
                       [&](const if_t<Attr>& i) {
                       },
                       [&](const elif_t<Attr>& i) {
                       },
                       [&](const else_t<Attr>& i) {
                       },
                       [&](const assign_t<Attr>& i) {
                       },
                       [&](const fn_expr_t<Attr>& f) {
                       },
                       [&](const var_def_t<Attr>& f) {
                       },
                       [&](const for_t<Attr>& v) {
                       },
                       [&](const while_t<Attr>& v) {
                       },
                       [&](const break_t& v) { out << margin() << "break" << endl; },
                       [&](const continue_t& v) { out << margin() << "continue" << endl; },
                       [&](const type_expr_t<Attr>& v) {
                       },
                       [&](const return_t<Attr>& v) {
                       },
                       [&](const yield_t<Attr>& v) {
                       },
                       [&](const struct_t<Attr>& v) {
                       },
                       [&](const def_type_t<Attr>& f) {
                       },
                       [&](const let_type_t<Attr>& f) {
                       },
                       [&](const template_t<Attr>& v) { 
                       },
                       [&](const attr_node_t<Attr>& n) { 
                       },
                       [&](const monostate&) {}, 
                       [](auto) {}),
                   tree);
    }

} }
