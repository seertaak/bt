#pragma once

#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/zip.hpp>

#include <bullet/analysis/error.hpp>
#include <bullet/analysis/symtab.hpp>
#include <bullet/analysis/type.hpp>
#include <bullet/lexer/location.hpp>
#include <bullet/parser/ast.hpp>

namespace bt { namespace analysis {
    using namespace std;
    namespace hana = boost::hana;
    using namespace parser::syntax;

    namespace rng = ranges;
    namespace views = rng::views;

    using st_type_t = symtab<type_t>;

    enum class context_t { var, fn, type };

    inline auto operator<<(ostream& os, context_t ctx) -> ostream& {
        if (ctx == context_t::var)
            os << "<VAR>";
        else if (ctx == context_t::fn)
            os << "<FN>";
        else if (ctx == context_t::type)
            os << "<TYPE>";
        return os;
    }

    struct environment_t {
        std::string name;
        context_t context;
        int next_type_id = 0;
        st_type_t vars{}, fns{}, types{};
    };

    auto type_check(parser::syntax::attr_node_t<type_t>& ast, const environment_t& parent_scope)
        -> void {
        ast.get().attribute = visit(
            hana::overload(
                [&](primitive_type_t& i) -> type_t {
                    return visit(
                        hana::overload([](lexer::token::char_t& t) -> type_t { return CHAR_T; },
                                       [](lexer::token::byte_t& t) -> type_t { return I8_T; },
                                       [](lexer::token::short_t& t) -> type_t { return I16_T; },
                                       [](lexer::token::int_t& t) -> type_t { return I32_T; },
                                       [](lexer::token::long_t& t) -> type_t { return I64_T; },
                                       [](lexer::token::ubyte_t& t) -> type_t { return U8_T; },
                                       [](lexer::token::ushort_t& t) -> type_t { return U16_T; },
                                       [](lexer::token::uint_t& t) -> type_t { return U32_T; },
                                       [](lexer::token::ulong_t& t) -> type_t { return U64_T; },
                                       [](lexer::token::float_t& t) -> type_t { return F32_T; },
                                       [](lexer::token::double_t& t) -> type_t { return F64_T; },
                                       [](lexer::token::i8_t& t) -> type_t { return I8_T; },
                                       [](lexer::token::i16_t& t) -> type_t { return I16_T; },
                                       [](lexer::token::i32_t& t) -> type_t { return I32_T; },
                                       [](lexer::token::i64_t& t) -> type_t { return I64_T; },
                                       [](lexer::token::u8_t& t) -> type_t { return U8_T; },
                                       [](lexer::token::u16_t& t) -> type_t { return U16_T; },
                                       [](lexer::token::u32_t& t) -> type_t { return U32_T; },
                                       [](lexer::token::u64_t& t) -> type_t { return U64_T; },
                                       [](lexer::token::ptr_t& t) -> type_t {
                                           return type_value(types::ptr_t{VOID_T});
                                       },
                                       [](lexer::token::array_t& t) -> type_t {
                                           return type_value(types::array_t{VOID_T, {}});
                                       },
                                       [](lexer::token::slice_t& t) -> type_t {
                                           return type_value(types::slice_t{VOID_T, 0, 0, 0});
                                       },
                                       [](lexer::token::variant_t& t) -> type_t {
                                           return type_value(types::variant_t{});
                                       },
                                       [](lexer::token::tuple_t& t) -> type_t {
                                           return type_value(types::tuple_t{});
                                       }),
                        i);
                },
                [&](literal_t& i) -> type_t {
                    return visit(
                        hana::overload(
                            [&](integral_literal_t& e) -> type_t {
                                type_value type;
                                if (e.type == 'i') {
                                    switch (e.width) {
                                    case 8: type = type_value(types::i8_t()); break;
                                    case 16: type = type_value(types::i16_t()); break;
                                    case 32: type = type_value(types::i32_t()); break;
                                    case 64: type = type_value(types::i64_t()); break;
                                    default: {
                                        auto err = raise<error>(ast);
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
                                        auto err = raise<analysis::error>(ast);
                                        err << "Illegal integer literal width " << e.width
                                            << ", should be 8, 16, 32, or 64 (or unspecified)";
                                    }
                                    }
                                } else {
                                    auto err = raise<analysis::error>(ast);
                                    err << "Integer literals should be either signed (i) or "
                                           "unsigned "
                                           "(u)";
                                }
                                return type;
                            },
                            [](string_literal_t& s) -> type_t {
                                return analysis::type_t(
                                    types::strlit_t{static_cast<int>(s.value.length())});
                            },
                            [&](floating_point_literal_t& e) -> type_t {
                                type_value type;
                                switch (e.width) {
                                case 32: type = type_value(types::f32_t()); break;
                                case 64: type = type_value(types::f64_t()); break;
                                default: {
                                    auto err = raise<analysis::error>(ast);
                                    err << "Illegal floating point literal width " << e.width
                                        << ", should be 32 or 64 (or unspecified)";
                                }
                                }
                                return type;
                            },
                            [](lexer::token::true_t&) -> type_t {
                                return analysis::type_t(type_value(types::bool_t{}));
                            },
                            [](lexer::token::false_t&) -> type_t {
                                return analysis::type_t(type_value(types::bool_t{}));
                            },
                            [](auto&) -> type_t { return UNKOWN; }),
                        i);
                },
                [&](lexer::identifier_t& id) -> type_t {
                    cout << "IDENTIFIER: " << id << " in context " << parent_scope.context << endl;;
                    switch (int(parent_scope.context)) {
                    case int(context_t::fn): {
                        if (auto pt = parent_scope.fns.lookup(id.name)) return *pt;
                        break;
                    }
                    case int(context_t::type): {
                        if (auto pt = parent_scope.types.lookup(id.name)) return *pt;
                        break;
                    }
                    default:
                    case int(context_t::var): {
                        if (auto pt = parent_scope.vars.lookup(id.name)) {
                            cout << "found: " << *pt << endl;
                            return *pt;
                        }
                        break;
                    }
                    }

                    cout << "not found." << endl;

                    auto err = raise<analysis::error>(ast);
                    err << "Internal error. No type information in inherited scope for \""
                        << id.name << "\" in context " << parent_scope.context;

                    return UNKOWN;
                },
                [&](block_t<type_t>& block) -> type_t {
                    auto scope = parent_scope;
                    auto scope_locs = symtab<lexer::location_t>();

                    type_t last_type = UNKOWN;

                    // need to do an initial pass collecting the types of any functions since
                    // any two functions may exhibit single or mutual recursion.
                    for (auto& stmt : block) {
                        if (auto pe = stmt.get().get_if<var_def_t<type_t>>()) {
                            if (auto pfn_ast = pe->rhs.get().get_if<parser::syntax::fn_expr_t<type_t>>()) {
                                const auto& name = pe->name.name;
                                const auto s = "F:"s + name;

                                if (auto ploc = scope_locs.lookup(s)) {
                                    auto err = raise<error>(stmt);
                                    err << "Duplicate function \"" << name << "\" (with duplicate at "
                                        << *ploc << ")";
                                }

                                auto& fn_ast = *pfn_ast;

                                type_t fn_type = type_value(types::function_t{});
                                auto& o = fn_type.get().get<types::function_t>();

                                scope.context = context_t::type;
                                type_check(fn_ast.result_type, scope);
                                o.result_type = fn_ast.result_type.get().attribute;

                                auto names = unordered_map<string, int>();
                                for (auto&& arg_id : fn_ast.arg_names) names[arg_id.name]++;

                                for (auto&& [name, count] : names) {
                                    if (count > 1) {
                                        auto err = raise<analysis::error>(stmt);
                                        err << "Formal parameter \"" << name << "\" is duplicated (occurring " << count
                                            << " times) in function expression.";
                                    }
                                }

                                for (auto j = 0; j < fn_ast.arg_names.size(); j++) {
                                    auto& arg_nm = fn_ast.arg_names[j].name;
                                    auto& arg_ty = fn_ast.arg_types[j];
                                    type_check(arg_ty, scope);
                                    o.formal_parameters.push_back(
                                        types::name_and_type_t{arg_nm, arg_ty.get().attribute});
                                }

                                if (pe->type.get().attribute.get().empty())
                                    pe->type.get().attribute = fn_type;

                                cout << pe->name << " :: " << fn_type << endl;
                                pe->rhs.get().attribute = fn_type;

                                scope.fns.insert(name, fn_type);
                                scope_locs.insert(s, pe->name.location);

                                scope.context = context_t::var;
                            }
                        }
                    }

                    for (auto& stmt : block) {
                        if (auto pe = stmt.get().get_if<let_type_t<type_t>>()) {
                            scope.context = context_t::type;
                            const auto& name = pe->name.name;

                            const auto s = "T:"s + name;

                            if (auto ploc = scope_locs.lookup(s)) {
                                auto err = raise<error>(stmt);
                                err << "Duplicate type name \"" << name << "\" (with duplicate at "
                                    << *ploc << ") in type alias";
                            }

                            type_check(pe->type, scope);

                            stmt.get().attribute = pe->type.get().attribute;

                            scope.types.insert(name, pe->type.get().attribute);
                            scope_locs.insert(s, pe->name.location);
                        } else if (auto pe = stmt.get().get_if<def_type_t<type_t>>()) {
                            scope.context = context_t::type;

                            const auto& name = pe->name.name;

                            const auto s = "T:"s + name;

                            if (auto ploc = scope_locs.lookup(s)) {
                                auto err = raise<error>(stmt);
                                err << "Duplicate type name \"" << name << "\" (with duplicate at "
                                    << *ploc << ") in type definition";
                            }

                            type_check(pe->type, scope);

                            auto td_ty = pe->type.get().attribute;
                            td_ty = type_t(type_value(types::nominal_type_t(name, td_ty)));

                            stmt.get().attribute = td_ty;

                            scope.types.insert(name, td_ty);
                            scope_locs.insert(s, pe->name.location);
                        } else if (auto pe = stmt.get().get_if<var_def_t<type_t>>()) {
                            scope.context = context_t::var;
                            type_check(stmt, scope);

                            if (!pe->rhs.get().attribute.is<types::function_t>()) {
                                const auto& name = pe->name.name;
                                const auto s = "V:"s + name;

                                if (auto ploc = scope_locs.lookup(s)) {
                                    auto err = raise<error>(stmt);
                                    err << "Duplicate variable declaration of \"" << name
                                        << "\", with duplicate at " << *ploc;
                                }

                                scope.vars.insert(name, stmt.get().attribute);
                                scope_locs.insert(s, pe->name.location);
                            } else {
                                auto& fn_ty = pe->rhs.get().attribute;
                                if (pe->type.get().attribute.get().empty())
                                    pe->type.get().attribute = fn_ty;
                            }
                        } else {
                            type_check(stmt, scope);
                        }
                        if (stmt.get().attribute.get()) last_type = stmt.get().attribute;
                    }

                    cout << "RETURNING WITH: " << last_type << endl;

                    return last_type;
                },
                [&](data_t<type_t>& data) -> type_t {
                    if (data.empty()) return VOID_T;

                    auto t = types::tuple_t();

                    bool is_array = true;

                    for (auto& elt : data) {
                        type_check(elt, parent_scope);
                        if (!t.empty() && elt.get().attribute != t.back()) is_array = false;
                        t.push_back(elt.get().attribute);
                    }

                    if (is_array) return type_value(types::array_t{t.front(), vector{t.size()}});

                    return type_value(t);
                },
                [&](unary_op_t<type_t>& op) -> type_t {
                    type_check(op.operand, parent_scope);
                    return op.operand.get().attribute;
                },
                [&](bin_op_t<type_t>& op) -> type_t {
                    type_check(op.lhs, parent_scope);
                    type_check(op.rhs, parent_scope);

                    auto& lhs_a = op.lhs.get().attribute;
                    auto& rhs_a = op.lhs.get().attribute;

                    if (auto pt = promoted_type(lhs_a, rhs_a)) return *pt;

                    return type_value(types::variant_t{lhs_a, rhs_a});
                },
                [&](invoc_t<type_t>& i) -> type_t {
                    auto scope = parent_scope;

                    if (scope.context != context_t::type)
                        scope.context = context_t::fn;

                    switch (int(scope.context)) {
                    case int(context_t::fn): {
                        scope.context = context_t::fn;
                        type_check(i.target, scope);

                        if (!i.target.get().attribute.is<types::function_t>()) {
                            auto err = raise<error>(ast);
                            auto& tgt_ty = i.target.get().attribute.get();
                            err << "Expected function type, got " << tgt_ty;
                        }

                        scope.context = context_t::var;

                        auto& fn_ty = i.target.get().attribute.as<types::function_t>();
                        auto& form_params = fn_ty.formal_parameters;

                        auto act_params_node = attr_node_t<type_t>(i.arguments);
                        type_check(act_params_node, scope);

                        auto& act_params = act_params_node.get().attribute;

                        if (not(act_params.is<types::tuple_t>() or
                                act_params.is<types::array_t>())) {
                            auto err = raise<analysis::error>(ast);
                            err << "Expected actual argument pack (data), got: " << act_params;
                            act_params = TUPLE_T;
                        }

                        if (auto pap = act_params.get_if<types::tuple_t>()) {
                            cout << "ARGUMENT TUPLE" << endl;
                            auto problem = pap->size() != i.arguments.size();

                            if (!problem) {
                                for (auto j = 0; j < form_params.size(); j++) {
                                    if (is_assignable_to((*pap)[j], form_params[j].type)) {
                                        problem = true;
                                        break;
                                    }
                                }
                            }

                            if (problem) {
                                auto err = raise<analysis::error>(ast);
                                err << "Mismatch between actual and formal parameters. "
                                    << "\nFormal parameters: " << form_params
                                    << "\nActual parameters: " << act_params;
                            }
                        } else if (auto p = act_params.get_if<types::array_t>()) {
                            cout << "ARGUMENT ARRAY" << endl;
                            if (i.arguments.size() != 1) {
                                auto err = raise<analysis::error>(ast);
                                err << "Mismatch between actual and formal parameters. "
                                    << "\nFormal parameters: " << form_params
                                    << "\nActual parameters: " << act_params;
                            }

                            if (!is_assignable_to(p->value_type, form_params.front().type)) {
                                auto err = raise<analysis::error>(ast);
                                err << "Mismatch between actual and formal parameters. "
                                    << "\nFormal parameters: " << form_params
                                    << "\nActual parameters: " << act_params;
                            }
                        }

                        return fn_ty.result_type;
                    }
                    case int(context_t::type): {
                        type_check(i.target, scope);
                        auto& args = i.arguments;

                        auto& tgt_ty = i.target.get().attribute;
                        auto result_ty = UNKOWN;

                        visit(hana::overload(
                                  [&](types::ptr_t& x) {
                                      if (args.size() != 1) {
                                          auto err = raise<error>(ast);
                                          err << "Pointer generic type \"ptr()\"accepts a single "
                                                 "underlying value type argument, "
                                                 "but got following argument pack: \""
                                              << args << "\"";
                                      }
                                      type_check(args.front(), scope);
                                      x.value_type = args.front().get().attribute;
                                      result_ty = tgt_ty;
                                  },
                                  [&](types::dynarr_t& x) {
                                      if (args.size() != 1) {
                                          auto err = raise<error>(ast);
                                          err << "Dynamic array generic type \"dynarr()\" accepts "
                                                 "a single value type argument, "
                                                 "but got following argument pack: \""
                                              << args << "\"";
                                      }
                                      type_check(args.front(), scope);
                                      x.value_type = args.front().get().attribute;
                                      result_ty = tgt_ty;
                                  },
                                  [&](types::strlit_t& x) {
                                      if (args.size() != 1) {
                                          auto err = raise<error>(ast);
                                          err << "Static (literal) \"strlit()\" generic type "
                                                 "accepts a single value type argument, "
                                                 "but got following argument pack: \""
                                              << args << "\"";
                                      }
                                      x.size = visit(
                                          hana::overload([](parser::syntax::integral_literal_t& i)
                                                             -> int { return i.value; },
                                                         [](auto& v) -> int { return 0; }),
                                          args.front().get().get<literal_t>());
                                      result_ty = tgt_ty;
                                  },
                                  [&](types::function_t& x) {
                                      if (args.size() < 1) {
                                          auto err = raise<error>(ast);
                                          err << "Function generic type \"fn()\" accepts at least "
                                                 "a single value type argument "
                                                 "(the return type), but got following argument "
                                                 "pack: \""
                                              << args << "\"";
                                      }
                                  },
                                  [&](auto&) {
                                      auto err = raise<error>(ast);
                                      auto& tgt_ty = i.target.get().attribute.get();
                                      err << "Expected generic type, got " << tgt_ty;
                                  }),
                              tgt_ty.get());

                        return result_ty;
                    }
                    default:
                        cout << "BAD CONTEXT" << endl;
                    }

                    return UNKOWN;
                },
                [&](if_t<type_t>& i) -> type_t {
                    for (auto& [test, branch]: views::zip(i.elif_tests, i.elif_branches)) {
                        auto scope = parent_scope;
                        type_check(test, scope);

                        // 1. is test's type bool?
                        // 2. Is test a block? If so, are there any var declarations?
                        //    If yes, then stuff them into scope.
                    }

                    /*
                    for (auto j = 0; j < i.elif_branches.size(); j++) {
                        type_check(i.elif_branches[j], scope);
                    }
                    */

                    return UNKOWN;
                },
                [&](elif_t<type_t>& i) -> type_t { return UNKOWN; },
                [&](else_t<type_t>& i) -> type_t { return UNKOWN; },
                [&](assign_t<type_t>& i) -> type_t { return UNKOWN; },
                [&](fn_expr_t<type_t>& f) -> type_t {
                    cout << "TYPE CHECKING FN EXPR: " << f << endl;
                    type_t result = type_value(types::function_t{});
                    auto& o = result.get().get<types::function_t>();
                    auto scope = parent_scope;

                    scope.context = context_t::type;
                    type_check(f.result_type, scope);
                    o.result_type = f.result_type.get().attribute;

                    auto names = unordered_map<string, int>();
                    for (auto&& arg_id : f.arg_names) names[arg_id.name]++;

                    for (auto&& [name, count] : names) {
                        if (count > 1) {
                            auto err = raise<analysis::error>(ast);
                            err << "Name \"" << name << "\" is duplicated (occurring " << count
                                << " times) in function expression.";
                        }
                    }

                    for (auto j = 0; j < f.arg_names.size(); j++) {
                        auto& arg_nm = f.arg_names[j].name;
                        auto& arg_ty = f.arg_types[j];
                        type_check(arg_ty, scope);
                        o.formal_parameters.push_back(
                            types::name_and_type_t{arg_nm, arg_ty.get().attribute});

                        scope.vars.insert(arg_nm, arg_ty.get().attribute);
                    }

                    scope.context = context_t::var;

                    type_check(f.body, scope);

                    if (o.result_type.get().empty())
                        o.result_type = f.body.get().attribute;

                    if (implicit_conversion_distance(f.body.get().attribute, o.result_type) < 0) {
                        auto err = raise<analysis::error>(ast);
                        err << "Type mismatch: function purports to return a \"" << o.result_type << "\", but actually "
                            << "returns a value of type \"" << f.body.get().attribute << "\"";
                    }

                    return result;
                },
                [&](var_def_t<type_t>& f) -> type_t {
                    auto scope = parent_scope;
                    scope.context = context_t::type;
                    type_check(f.type, scope);
                    scope.context = context_t::var;
                    type_check(f.rhs, scope);

                    auto decl_ty = f.type.get().attribute.get();
                    auto& deduced_ty = f.rhs.get().attribute.get();

                    if (decl_ty.empty()) {
                        decl_ty = deduced_ty;
                        if (decl_ty.is<types::ptr_t>())
                            decl_ty = decl_ty.get<types::ptr_t>().value_type;
                    }

                    const auto mkptr = [&](auto&& t) -> type_t {
                        return type_value(types::ptr_t{t});
                    };

                    if (!decl_ty.empty() && !deduced_ty.empty()) {
                        if (is_assignable_to(deduced_ty, decl_ty)) return mkptr(decl_ty);

                        auto err = raise<analysis::error>(ast);
                        err << "Can't assign value of type \"" << deduced_ty
                            << "\" to value of type \"" << decl_ty << "\"";
                    } else if (decl_ty.empty() && !deduced_ty.empty()) {
                        return mkptr(deduced_ty);
                    } else if (!decl_ty.empty() && deduced_ty.empty()) {
                        return mkptr(decl_ty);
                    }
                    return UNKOWN;
                },
                [&](for_t<type_t>& v) -> type_t { return UNKOWN; },
                [&](while_t<type_t>& v) -> type_t { return UNKOWN; },
                [&](break_t& v) -> type_t { return VOID_T; },
                [&](continue_t& v) -> type_t { return VOID_T; },
                [&](type_expr_t<type_t>& v) -> type_t {
                    auto scope = parent_scope;
                    scope.context = context_t::type;
                    type_check(v.type, scope);
                    return v.type.get().attribute;
                },
                [&](return_t<type_t>& v) -> type_t {
                    type_check(v.value, parent_scope);
                    return v.value.get().attribute;
                },
                [&](yield_t<type_t>& v) -> type_t {
                    type_check(v.value, parent_scope);
                    return v.value.get().attribute;
                },
                [&](struct_t<type_t>& v) -> type_t { return UNKOWN; },
                [&](def_type_t<type_t>& f) -> type_t { return UNKOWN; },
                [&](let_type_t<type_t>& f) -> type_t { return UNKOWN; },
                [&](template_t<type_t>& v) -> type_t { return UNKOWN; },
                [&](attr_node_t<type_t>& n) -> type_t { return UNKOWN; },
                [&](std::monostate& e) -> type_t { return UNKOWN; },
                [&](auto&& e) -> type_t { return UNKOWN; }),
            ast.get());
    }

}}  // namespace bt::analysis
