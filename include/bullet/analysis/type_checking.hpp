#pragma once

#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/contains.hpp>
#include <range/v3/core.hpp>
#include <range/v3/view/zip.hpp>
#include <boost/stacktrace.hpp>

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
        -> void;

    auto type_check_block(block_t<type_t>& block, environment_t& scope) -> type_t {
        auto scope_locs = symtab<lexer::location_t>();

        type_t last_type = UNKOWN;

        // need to do an initial pass collecting the types of any functions since
        // any two functions may exhibit single or mutual recursion.
        for (auto& stmt : block) {
            if (auto pe = stmt.get().get_if<var_def_t<type_t>>()) {
                cout << "TYPE CHECK BLOCK STATEMENT: VAR DEF FN PASS." << endl;
                if (auto pfn_ast = pe->rhs.get().get_if<parser::syntax::fn_expr_t<type_t>>()) {
                    const auto& name = pe->name.name;
                    const auto s = "F:"s + name;

                    if (auto ploc = scope_locs.lookup(s)) {
                        auto err = raise<error>(stmt);
                        err << "Duplicate function \"" << name << "\" (with duplicate at " << *ploc
                            << ")";
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
                            err << "Formal parameter \"" << name << "\" is duplicated (occurring "
                                << count << " times) in function expression.";
                        }
                    }

                    for (auto j = 0; j < fn_ast.arg_names.size(); j++) {
                        auto& arg_nm = fn_ast.arg_names[j].name;
                        auto& arg_ty = fn_ast.arg_types[j];
                        type_check(arg_ty, scope);
                        o.formal_parameters.push_back(
                            types::name_and_type_t{arg_nm, arg_ty.get().attribute});
                    }

                    if (pe->type.get().attribute.get().empty()) pe->type.get().attribute = fn_type;

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
                    err << "Duplicate type name \"" << name << "\" (with duplicate at " << *ploc
                        << ") in type alias";
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
                    err << "Duplicate type name \"" << name << "\" (with duplicate at " << *ploc
                        << ") in type definition";
                }

                type_check(pe->type, scope);

                auto td_ty = pe->type.get().attribute;
                td_ty = type_t(type_value(types::nominal_type_t(name, td_ty)));

                stmt.get().attribute = td_ty;

                scope.types.insert(name, td_ty);
                scope_locs.insert(s, pe->name.location);
            } else if (auto pe = stmt.get().get_if<let_var_t<type_t>>()) {
                cout << "TYPE CHECK BLOCK STATEMENT: LET VAR." << endl;
                scope.context = context_t::var;
                type_check(stmt, scope);

                if (!pe->rhs.get().attribute.is<types::function_t>()) {
                    const auto& name = pe->name.name;
                    const auto s = "V:"s + name;

                    if (auto ploc = scope_locs.lookup(s)) {
                        auto err = raise<error>(stmt);
                        err << "Duplicate (let) variable declaration of \"" << name
                            << "\", with duplicate at " << *ploc;
                    }

                    scope.vars.insert(name, stmt.get().attribute);
                    scope_locs.insert(s, pe->name.location);
                } else {
                    auto& fn_ty = pe->rhs.get().attribute;
                    if (pe->type.get().attribute.get().empty()) pe->type.get().attribute = fn_ty;
                }
            } else if (auto pe = stmt.get().get_if<var_def_t<type_t>>()) {
                cout << "TYPE CHECK BLOCK STATEMENT: DEF VAR." << endl;
                scope.context = context_t::var;
                type_check(stmt, scope);

                cout << "TYPE CHECK BLOCK STATEMENT: DEF VAR first part DONE" << endl;

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
                    if (pe->type.get().attribute.get().empty()) pe->type.get().attribute = fn_ty;
                }
            } else {
                type_check(stmt, scope);
            }
            if (stmt.get().attribute.get()) last_type = stmt.get().attribute;
        }

        return last_type;
    }

    auto type_check(parser::syntax::attr_node_t<type_t>& ast, const environment_t& parent_scope)
        -> void {
       try { 
        ast.get().attribute = visit(
            hana::overload(
                [&](primitive_type_t& i) -> type_t {
                    return visit(
                        hana::overload([](lexer::token::char_t& t) -> type_t { return CHAR; },
                                       [](lexer::token::byte_t& t) -> type_t { return I8; },
                                       [](lexer::token::short_t& t) -> type_t { return I16; },
                                       [](lexer::token::int_t& t) -> type_t { return I32; },
                                       [](lexer::token::long_t& t) -> type_t { return I64; },
                                       [](lexer::token::ubyte_t& t) -> type_t { return U8; },
                                       [](lexer::token::ushort_t& t) -> type_t { return U16; },
                                       [](lexer::token::uint_t& t) -> type_t { return U32; },
                                       [](lexer::token::ulong_t& t) -> type_t { return U64; },
                                       [](lexer::token::float_t& t) -> type_t { return F32; },
                                       [](lexer::token::double_t& t) -> type_t { return F64; },
                                       [](lexer::token::i8_t& t) -> type_t { return I8; },
                                       [](lexer::token::i16_t& t) -> type_t { return I16; },
                                       [](lexer::token::i32_t& t) -> type_t { return I32; },
                                       [](lexer::token::i64_t& t) -> type_t { return I64; },
                                       [](lexer::token::u8_t& t) -> type_t { return U8; },
                                       [](lexer::token::u16_t& t) -> type_t { return U16; },
                                       [](lexer::token::u32_t& t) -> type_t { return U32; },
                                       [](lexer::token::u64_t& t) -> type_t { return U64; },
                                       [](lexer::token::ptr_t& t) -> type_t {
                                           return type_value(types::ptr_t{VOID});
                                       },
                                       [](lexer::token::array_t& t) -> type_t {
                                           return type_value(types::array_t{VOID, {}});
                                       },
                                       [](lexer::token::slice_t& t) -> type_t {
                                           return type_value(types::slice_t{VOID, 0, 0, 0});
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
                                    case 0: type = type_value(types::intlit_t()); break;
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
                                } else if (e.type == '?') {
                                    type = type_value(types::intlit_t());
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
                                case 0: type = type_value(types::floatlit_t()); break;
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
                    cout << "IDENTIFIER: " << id << " in context " << parent_scope.context << endl;
                    ;
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
                    return type_check_block(block, scope);
                },
                [&](data_t<type_t>& data) -> type_t {
                    if (data.empty()) return VOID;

                    auto t = types::tuple_t();

                    bool is_array = true;

                    for (auto& elt : data) {
                        type_check(elt, parent_scope);
                        const auto t_e = regularized_type(elt->attribute);
                        if (!t.empty() && t_e != t.back()) is_array = false;
                        t.push_back(t_e);
                    }

                    if (is_array) return type_value(types::array_t{t.front(), vector{t.size()}});

                    return type_value(t);
                },
                [&](unary_op_t<type_t>& op) -> type_t {
                    const auto opt = op.op;
                    const auto opstr = lexer::token_symbol(opt);

                    type_check(op.operand, parent_scope);

                    const auto& t = op.operand->attribute;

                    const auto error = [&](auto&& op_type) {
                        auto err = raise<analysis::error>(ast);
                        err << "Invalid operand to unary operator \"" << opstr << "\": \"" << t
                            << "\"";
                    };

                    using namespace lexer;

                    if (opt == TILDE) {
                        if (is_convertible_to(t, U8)) return U8;
                        if (is_convertible_to(t, U16)) return U16;
                        if (is_convertible_to(t, U32)) return U32;
                        if (is_convertible_to(t, U64)) return U64;
                        if (is_convertible_to(t, I8)) return I8;
                        if (is_convertible_to(t, I16)) return I16;
                        if (is_convertible_to(t, I32)) return I32;
                        if (is_convertible_to(t, I64)) return I64;

                        error("bitwise ");
                    } else if (opt == MINUS || opt == PLUS) {
                        if (is_convertible_to(t, U8)) return U8;
                        if (is_convertible_to(t, U16)) return U16;
                        if (is_convertible_to(t, U32)) return U32;
                        if (is_convertible_to(t, U64)) return U64;
                        if (is_convertible_to(t, I8)) return I8;
                        if (is_convertible_to(t, I16)) return I16;
                        if (is_convertible_to(t, I32)) return I32;
                        if (is_convertible_to(t, I64)) return I64;
                        if (is_convertible_to(t, F32)) return F32;
                        if (is_convertible_to(t, F64)) return F64;

                        error("arithmetic ");
                    } else if (opt == NOT) {
                        if (is_convertible_to(t, BOOL)) return BOOL;
                        error("boolean ");
                    }
                    return VOID;
                },
                [&](bin_op_t<type_t>& op) -> type_t {
                    const auto opt = op.op;
                    const auto opstr = lexer::token_symbol(opt);

                    type_check(op.lhs, parent_scope);
                    type_check(op.rhs, parent_scope);

                    auto& lhs_a = op.lhs.get().attribute;
                    auto& rhs_a = op.rhs.get().attribute;

                    using namespace lexer;

                    auto pt = promoted_type(lhs_a, rhs_a);

                    const auto error = [&](auto&& op_type) {
                        auto err = raise<analysis::error>(ast);
                        err << "Invalid arguments to " << op_type << "operator \"" << opstr
                            << "\": left-hand side has type \"" << lhs_a
                            << "\", right-hand side has type \"" << rhs_a << "\"";
                    };

                    using namespace lexer;

                    if (opt == EQUAL || opt == NOT_EQUAL || opt == LT || opt == GT || opt == GEQ ||
                        opt == LEQ || opt == IS || opt == IN) {
                        if (!pt) {
                            error("comparison ");
                            return VOID;
                        }
                        return BOOL;
                    } else if (opt == AND || opt == OR) {
                        if (!pt) {
                            error("boolean ");
                            return VOID;
                        }
                        const auto& t = *pt;
                        cout << "Bin op type checking: promoted type in bin op " << opstr << " is "
                             << *pt << endl;

                        if (is_convertible_to(t, BOOL)) return BOOL;

                        error("boolean ");

                        return VOID;
                    } else if (opt == BAR || opt == AMPERSAND || opt == HAT) {
                        if (!pt) {
                            error("bitwise ");
                            return VOID;
                        }
                        const auto& t = *pt;
                        cout << "Bin op type checking: promoted type in bin op " << opstr << " is "
                             << *pt << endl;

                        if (is_convertible_to(t, U8)) return U8;
                        if (is_convertible_to(t, U16)) return U16;
                        if (is_convertible_to(t, U32)) return U32;
                        if (is_convertible_to(t, U64)) return U64;
                        if (is_convertible_to(t, I8)) return I8;
                        if (is_convertible_to(t, I16)) return I16;
                        if (is_convertible_to(t, I32)) return I32;
                        if (is_convertible_to(t, I64)) return I64;

                        error("bitwise ");

                        return VOID;
                    } else if (opt == PLUS || opt == MINUS || opt == STAR || opt == SLASH ||
                               opt == PERCENTAGE || opt == STAR_STAR) {
                        if (!pt) {
                            error("arithmetic ");
                            return VOID;
                        }
                        cout << "Bin op type checking: promoted type in bin op " << opstr << " is "
                             << *pt << endl;

                        const auto& t = *pt;

                        if (is_convertible_to(t, U8)) return U8;
                        if (is_convertible_to(t, U16)) return U16;
                        if (is_convertible_to(t, U32)) return U32;
                        if (is_convertible_to(t, U64)) return U64;
                        if (is_convertible_to(t, I8)) return I8;
                        if (is_convertible_to(t, I16)) return I16;
                        if (is_convertible_to(t, I32)) return I32;
                        if (is_convertible_to(t, I64)) return I64;
                        if (is_convertible_to(t, F32)) return F32;
                        if (is_convertible_to(t, F64)) return F64;

                        error("arithmetic ");

                        return VOID;
                    }

                    return VOID;
                },
                [&](invoc_t<type_t>& i) -> type_t {
                    cout << "Got invocation type!" << endl;
                    auto scope = parent_scope;

                    if (scope.context != context_t::type) scope.context = context_t::fn;

                    switch (int(scope.context)) {
                    case int(context_t::fn): {
                        cout << "function call: " << i.target << " (" << i.target->attribute << ")" << endl;
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
                            act_params = TUPLE;
                        }

                        if (auto pap = act_params.get_if<types::tuple_t>()) {
                            cout << "ARGUMENT TUPLE" << endl;
                            auto problem = pap->size() != i.arguments.size();

                            if (!problem) {
                                for (auto j = 0; j < form_params.size(); j++) {
                                    if (is_convertible_to((*pap)[j], form_params[j].type)) {
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

                            if (!is_convertible_to(p->value_type, form_params.front().type)) {
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

                        auto& tgt_ty = i.target->attribute;
                        auto result_ty = UNKOWN;

                        cout << "type call statement (template): " << i.target << " (" << i.target->attribute << ")" << endl;

                        visit(hana::overload(
                                  [&](types::ptr_t& x) {
                                      if (args.size() != 1) {
                                          auto err = raise<error>(ast);
                                          err << "Pointer generic type \"ptr()\"accepts a single "
                                                 "underlying value type argument, "
                                                 "but got: \""
                                              << args << "\"";
                                      }
                                      type_check(args.front(), scope);
                                      x.value_type = args.front().get().attribute;
                                      result_ty = tgt_ty;
                                  },
                                  [&](types::array_t& x) {
                                      if (args.size() != 2) {
                                          auto err = raise<error>(ast);
                                          err << "Fixed array generic type \"array()\" accepts "
                                                 "a value type argument, and a sequence of dimension sizes, "
                                                 "but got: \""
                                              << args << "\"";
                                      }
                                      type_check(args.front(), scope);
                                      x.value_type = args.front()->attribute;
                                      type_check(args.back(), scope);
                                      x.value_type = args.back()->attribute;
                                      result_ty = tgt_ty;
                                  },
                                  [&](types::dynarr_t& x) {
                                      if (args.size() != 1) {
                                          auto err = raise<error>(ast);
                                          err << "Dynamic array generic type \"dynarr()\" accepts "
                                                 "a single value type argument, "
                                                 "but got: \""
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
                                  [&](types::tuple_t& x) {
                                      cout << "HEY GOT HERE" << endl;
                                      for (auto& arg: i.arguments) {
                                         type_check(arg, scope);
                                         x.push_back(arg->attribute);
                                      }
                                      result_ty = type_t(type_value(x));
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
                    default: cout << "BAD CONTEXT" << endl;
                    }

                    return UNKOWN;
                },
                [&](while_t<type_t>& v) -> type_t {
                    auto branch_return_tys = vector<type_t>();
                    auto scope = parent_scope;

                    type_t test_ty = VOID;

                    auto& test = v.test;

                    if (auto pblock = test->get_if<block_t<type_t>>()) {
                        test.get().attribute = test_ty = type_check_block(*pblock, scope);
                    } else {
                        scope.context = context_t::var;
                        type_check(test, scope);
                        test_ty = test.get().attribute;
                    }

                    if (!is_convertible_to(test_ty, BOOL)) {
                        auto err = raise<error>(ast);
                        err << "While test condition must have type \"bool\", found: \"" << test_ty
                            << "\"";
                    }

                    scope.context = context_t::var;
                    type_check(v.body, scope);
                    return v.body->attribute;
                },
                [&](if_t<type_t>& i) -> type_t {
                    auto branch_return_tys = vector<type_t>();

                    for (auto&& [test, branch] : views::zip(i.elif_tests, i.elif_branches)) {
                        auto scope = parent_scope;

                        type_t test_ty = UNKOWN;

                        if (auto pblock = test.get().get_if<block_t<type_t>>()) {
                            test.get().attribute = test_ty = type_check_block(*pblock, scope);
                        } else {
                            scope.context = context_t::var;
                            type_check(test, scope);
                            test_ty = test.get().attribute;
                        }

                        if (!is_convertible_to(test_ty, BOOL)) {
                            auto err = raise<error>(ast);
                            err << "If condition must have type \"bool\", found: \"" << test_ty
                                << "\"";
                        }

                        scope.context = context_t::var;
                        type_check(branch, scope);
                        branch_return_tys.push_back(branch.get().attribute);
                    }
                    auto scope = parent_scope;
                    scope.context = context_t::var;

                    type_check(i.else_branch, scope);
                    branch_return_tys.push_back(i.else_branch.get().attribute);

                    auto result = types::variant_t();

                    for (auto&& ty : branch_return_tys)
                        if (!rng::contains(result, ty)) result.push_back(ty);

                    if (result.size() == 1) return result.front();

                    return type_value(result);
                },
                [&](elif_t<type_t>& i) -> type_t { return UNKOWN; },
                [&](else_t<type_t>& i) -> type_t { return UNKOWN; },
                [&](assign_t<type_t>& i) -> type_t {
                    auto scope = parent_scope;
                    scope.context = context_t::var;
                    type_check(i.lhs, scope);
                    scope.context = context_t::var;
                    type_check(i.rhs, scope);

                    if (is_assignable_to(i.rhs->attribute, i.lhs->attribute))
                        return i.lhs->attribute;

                    auto err = raise<analysis::error>(ast);

                    return UNKOWN;
                },
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

                    if (o.result_type.get().empty()) o.result_type = f.body.get().attribute;

                    if (implicit_conversion_distance(f.body.get().attribute, o.result_type) < 0) {
                        auto err = raise<analysis::error>(ast);
                        err << "Type mismatch: function purports to return a \"" << o.result_type
                            << "\", but actually "
                            << "returns a value of type \"" << f.body.get().attribute << "\"";
                    }

                    return result;
                },
                [&](let_var_t<type_t>& f) -> type_t {
                    auto scope = parent_scope;
                    scope.context = context_t::type;
                    type_check(f.type, scope);
                    scope.context = context_t::var;
                    type_check(f.rhs, scope);

                    auto deduced_ty = f.rhs->attribute;
                    auto decl_ty = f.type->attribute;

                    if (deduced_ty->empty() && decl_ty->empty()) return UNKOWN;

                    if (decl_ty->empty()) {
                        deduced_ty = decay_ptr(deduced_ty);
                    } else if (!is_immutable(decl_ty)) {
                        auto err = raise<analysis::error>(ast);
                        err << "Let variable statement must have an immutable type, but the "
                            << "declared type \"" << decl_ty << "\" has at least \"ptr\" nested "
                            << "in it (pointers imply mutation!)";
                        return UNKOWN;
                    }

                    if (deduced_ty == INTLIT)
                        deduced_ty = I32;
                    else if (deduced_ty == FLOATLIT)
                        deduced_ty = F64;

                    if (decl_ty->empty()) decl_ty = deduced_ty;

                    if (!deduced_ty->empty()) {
                        if (is_convertible_to(deduced_ty, decl_ty)) {
                            cout << "SUCCESS! in let var; returning: " << decl_ty << endl;
                            return decl_ty;
                        }

                        auto err = raise<analysis::error>(ast);
                        err << "Can't assign value of type \"" << deduced_ty << "\" to variable \""
                            << f.name.name << "\" of type \"" << decl_ty << "\"";

                        return UNKOWN;
                    }

                    return decl_ty;
                },
                [&](var_def_t<type_t>& f) -> type_t {
                    auto scope = parent_scope;
                    scope.context = context_t::type;
                    type_check(f.type, scope);
                    scope.context = context_t::var;
                    type_check(f.rhs, scope);

                    auto deduced_ty = f.rhs->attribute;
                    auto& decl_ty = f.type->attribute;

                    cout << "Var def. initial. Decl var TYPE: " << f.type.get() << "; RHS: " << f.rhs.get() << endl;
                    cout << "Var def. initial. Decl var type: " << decl_ty << "; deduced type: " << deduced_ty << endl;


                    if (deduced_ty->empty() && decl_ty->empty()) return UNKOWN;

                    if (decl_ty->empty()) {
                        // note: behaviour in type deduction is as follows. 'var' statements
                        // are deduced, by default, as *values*. In order to allow pointers
                        // to declared in a concise syntax, we modify and extend C++'s
                        // 'auto&' notation: 'var*' will re-introduce a pointer into the
                        // deduced type, 'var**' re-introduces two pointer indirections, and
                        // so on.

                        const auto d = ptr_depth(deduced_ty);

                        const auto z = d - f.n_indirections;
                        if (z > 0)
                            deduced_ty = decay_ptr(deduced_ty, z);
                        else if (z < 0) {
                            auto err = raise<analysis::error>(ast);
                            err << "In variable declaration, the left-hand side has "
                                << f.n_indirections << " '*' (pointer) symbols suggesting the "
                                << "right-hand side has at least " << f.n_indirections << ", "
                                << "but it actually has " << d << " < " << f.n_indirections
                                << " pointer indirections";
                        }
                    }

                    if (deduced_ty == INTLIT)
                        deduced_ty = I32;
                    else if (deduced_ty == FLOATLIT)
                        deduced_ty = F64;

                    if (decl_ty->empty()) decl_ty = deduced_ty;

                    const auto var_ty = type_t(type_value(types::ptr_t{decl_ty}));

                    if (!deduced_ty->empty()) {
                        if (is_assignable_to(deduced_ty, var_ty)) return var_ty;

                        auto err = raise<analysis::error>(ast);
                        err << "Can't assign value of type \"" << deduced_ty << "\" to variable \""
                            << f.name.name << "\" of type \"" << decl_ty << "\"";

                        return UNKOWN;
                    }

                    return var_ty;
                },
                [&](for_t<type_t>& v) -> type_t {
                    auto scope = parent_scope;
                    scope.context = context_t::var;

                    scope.context = context_t::var;
                    type_check(v.var_rhs, scope);

                    auto& seq_ty = v.var_rhs->attribute;

                    const auto seq_is_ptr = seq_ty->is<types::ptr_t>();
                    v.var_rhs->attribute = deref(seq_ty);

                    const auto err = [](auto&& t) { return raise<analysis::error>(t); };

                    if (seq_ty->empty()) {
                        auto e = err(v.var_rhs);
                        e << "Unable to determine type of iteration target for iteration variable "
                             "\""
                          << v.var_lhs << "\"";
                    } else if (auto parray = seq_ty->get_if<types::array_t>()) {
                        scope.vars.insert(v.var_lhs.name, parray->value_type);
                    } else if (auto pdynarr = seq_ty->get_if<types::dynarr_t>()) {
                        scope.vars.insert(v.var_lhs.name, pdynarr->value_type);
                    } else if (auto pdynarr = seq_ty->get_if<types::string_t>()) {
                        scope.vars.insert(v.var_lhs.name, CHAR);
                    } else if (auto pdynarr = seq_ty->get_if<types::strlit_t>()) {
                        scope.vars.insert(v.var_lhs.name, CHAR);
                    } else {
                        auto e = err(v.var_rhs);
                        e << "Invalid iteration target for iteration variable \"" << v.var_lhs
                          << "\" has type \"" << v.var_rhs->attribute << "\"";
                    }

                    scope.context = context_t::var;
                    type_check(v.body, scope);
                    return v.body->attribute;
                },
                [&](break_t& v) -> type_t { return VOID; },
                [&](continue_t& v) -> type_t { return VOID; },
                [&](type_expr_t<type_t>& v) -> type_t {
                    cout << "TYPE EXPR: " << ast << endl;
                    auto scope = parent_scope;
                    scope.context = context_t::type;
                    type_check(v.type, scope);
                    return v.type->attribute;
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
        } catch (const std::bad_variant_access& e) {
            cerr << e.what() << endl;
            cerr << boost::stacktrace::stacktrace() << endl;
        }
    }

}}  // namespace bt::analysis
