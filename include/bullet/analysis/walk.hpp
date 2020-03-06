#pragma once

#include <boost/hana/all.hpp>

#include <bullet/analysis/error.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/parser/attribute.hpp>
#include <bullet/util.hpp>

namespace bt { namespace analysis {

    namespace details {
        template <typename OutputAttr, class... Fn, typename InputAttr>
        inline auto walk_synth_impl(const parser::syntax::attr_node_t<InputAttr>& node, Fn*... fn)
            -> parser::syntax::attr_tree_t<OutputAttr> {
            using namespace parser::syntax;
            using namespace std;
            using in_tree_t = attr_tree_t<InputAttr>;
            using out_tree_t = attr_tree_t<OutputAttr>;

            namespace hana = boost::hana;

            auto fn_dict_t = hana::make_map(
                hana::make_pair(hana::type_c<lambda_arg_t<remove_pointer_t<decltype(fn)>>>, fn)...);

            const auto l = node.get().location;
            const auto& attribute = node.get().attribute;

            return std::visit(
                boost::hana::overload(
                    [&](const string_literal_t& literal) {
                        auto result = out_tree_t(literal);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<string_literal_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<string_literal_t>(), node);
                        }
                        return result;
                    },
                    [&](const integral_literal_t& literal) {
                        auto result = out_tree_t(literal);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<integral_literal_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<integral_literal_t>(), node);
                        }
                        return result;
                    },
                    [&](const floating_point_literal_t& literal) {
                        auto result = out_tree_t(literal);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<floating_point_literal_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<floating_point_literal_t>(), node);
                        }
                        return result;
                    },
                    [&](const lexer::identifier_t& id) {
                        auto result = out_tree_t(id);
                        result.location = l;
                        result.attribute = OutputAttr{};
                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<lexer::identifier_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<lexer::identifier_t>(), node);
                        }
                        return result;
                    },
                    [&](const lexer::token::true_t& t) {
                        auto result = out_tree_t(t);
                        result.location = l;
                        result.attribute = OutputAttr{};
                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<lexer::token::true_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<lexer::token::true_t>(), node);
                        }
                        return result;
                    },
                    [&](const lexer::token::false_t& t) {
                        auto result = out_tree_t(t);
                        result.location = l;
                        result.attribute = OutputAttr{};
                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<lexer::token::false_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<lexer::token::false_t>(), node);
                        }
                        return result;
                    },
                    [&](const block_t<InputAttr>& block) {
                        auto result = out_tree_t(block_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        for (const auto& stmt : block)
                            result.template get<block_t<OutputAttr>>().push_back(
                                walk_synth_impl<OutputAttr>(stmt, fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<block_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<block_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const data_t<InputAttr>& data) {
                        auto result = out_tree_t(data_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        for (const auto& stmt : data)
                            result.template get<data_t<OutputAttr>>().push_back(
                                walk_synth_impl<OutputAttr>(stmt, fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<data_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<data_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const unary_op_t<InputAttr>& op) {
                        auto result = out_tree_t(unary_op_t<OutputAttr>{
                            op.op, walk_synth_impl<OutputAttr>(op.operand, fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<unary_op_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<unary_op_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const bin_op_t<InputAttr>& op) {
                        auto result = out_tree_t(
                            bin_op_t<OutputAttr>{op.op, walk_synth_impl<OutputAttr>(op.lhs, fn...),
                                                 walk_synth_impl<OutputAttr>(op.rhs, fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<bin_op_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<bin_op_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const invoc_t<InputAttr>& i) {
                        auto result = out_tree_t(invoc_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<invoc_t<OutputAttr>>();
                        o.target =
                            attr_node_t<OutputAttr>(walk_synth_impl<OutputAttr>(i.target, fn...));

                        for (const auto& arg : i.arguments)
                            o.arguments.push_back(walk_synth_impl<OutputAttr>(arg, fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<invoc_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<invoc_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const if_t<InputAttr>& i) {
                        auto result = out_tree_t(if_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<if_t<OutputAttr>>();

                        for (int j = 0; j < i.elif_tests.size(); j++) {
                            o.elif_tests.push_back(
                                walk_synth_impl<OutputAttr>(i.elif_tests[j], fn...));

                            o.elif_branches.push_back(
                                walk_synth_impl<OutputAttr>(i.elif_branches[j], fn...));
                        }

                        o.else_branch = attr_node_t<OutputAttr>(
                            walk_synth_impl<OutputAttr>(i.else_branch, fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<if_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<if_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const elif_t<InputAttr>& i) -> out_tree_t {
                        throw error("Illegal dangling \"elif\"", l);
                    },
                    [&](const else_t<InputAttr>& i) -> out_tree_t {
                        throw error("Illegal dangling \"else\"", l);
                    },
                    [&](const fn_expr_t<InputAttr>& i) {
                        auto result = out_tree_t(fn_expr_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<fn_expr_t<OutputAttr>>();

                        o.arg_names = i.arg_names;

                        for (int j = 0; j < i.arg_names.size(); j++)
                            o.arg_types.push_back(
                                walk_synth_impl<OutputAttr>(i.arg_types[j], fn...));

                        o.result_type = attr_node_t<OutputAttr>(
                            walk_synth_impl<OutputAttr>(i.result_type, fn...));
                        o.body =
                            attr_node_t<OutputAttr>(walk_synth_impl<OutputAttr>(i.body, fn...));

                        for (int j = 0; j < i.closure_params.size(); j++) {
                            const auto& p = i.closure_params[j];
                            o.closure_params.push_back(fn_closure_param_t<OutputAttr>{
                                p.var, p.identifier,
                                walk_synth_impl<OutputAttr>(p.expression, fn...)});
                        }

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<fn_expr_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<fn_expr_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const var_def_t<InputAttr>& i) {
                        auto result = out_tree_t(var_def_t<OutputAttr>{
                            i.name, walk_synth_impl<OutputAttr>(i.type, fn...),
                            walk_synth_impl<OutputAttr>(i.rhs, fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<var_def_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<var_def_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const for_t<InputAttr>& i) {
                        auto result = out_tree_t(for_t<OutputAttr>{
                            i.var_lhs, walk_synth_impl<OutputAttr>(i.var_rhs, fn...),
                            walk_synth_impl<OutputAttr>(i.body, fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<for_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<for_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const while_t<InputAttr>& i) {
                        auto result = out_tree_t(
                            while_t<OutputAttr>{walk_synth_impl<OutputAttr>(i.test, fn...),
                                                walk_synth_impl<OutputAttr>(i.body, fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<while_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<while_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const break_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t, hana::type_c<break_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(result.template get<break_t>(), node);
                        }
                        return result;
                    },
                    [&](const continue_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t, hana::type_c<break_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(result.template get<break_t>(), node);
                        }
                        return result;
                    },
                    [&](const return_t<InputAttr>& i) {
                        auto result = out_tree_t(return_t<OutputAttr>{
                            walk_synth_impl<OutputAttr>(i.value, fn...),
                        });
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<return_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<return_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const yield_t<InputAttr>& i) {
                        auto result = out_tree_t(yield_t<OutputAttr>{
                            walk_synth_impl<OutputAttr>(i.value, fn...),
                        });
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<yield_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<yield_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const struct_t<InputAttr>& i) {
                        auto result = out_tree_t(struct_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<struct_t<OutputAttr>>();

                        for (const auto& e : i)
                            o.emplace_back(e.first, walk_synth_impl<OutputAttr>(e.second, fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<struct_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<struct_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const def_type_t<InputAttr>& i) {
                        auto result = out_tree_t(def_type_t<OutputAttr>{
                            i.name,
                            walk_synth_impl<OutputAttr>(i.type, fn...),
                        });
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<def_type_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<def_type_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const let_type_t<InputAttr>& i) {
                        auto result = out_tree_t(let_type_t<OutputAttr>{
                            i.name,
                            walk_synth_impl<OutputAttr>(i.type, fn...),
                        });
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<let_type_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<let_type_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const template_t<InputAttr>& i) {
                        auto result = out_tree_t(template_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<template_t<OutputAttr>>();

                        for (const auto& e : i.arguments)
                            o.arguments.emplace_back(e.first,
                                                     walk_synth_impl<OutputAttr>(e.second, fn...));

                        o.body =
                            attr_node_t<OutputAttr>(walk_synth_impl<OutputAttr>(i.body, fn...));

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<template_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<template_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const attr_node_t<InputAttr>& i) {
                        auto result = out_tree_t(
                            attr_node_t<OutputAttr>(walk_synth_impl<OutputAttr>(i, fn...)));
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<attr_node_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<attr_node_t<OutputAttr>>(), node);
                        }

                        return result;
                    },
                    [&](const auto&) {
                        auto result = out_tree_t();
                        result.location = l;
                        result.attribute = OutputAttr{};
                        return result;
                    }),
                node.get());
        }

        template <typename OutputAttr, class... Fn, typename InputAttr>
        inline auto walk_inherit_impl(const parser::syntax::attr_node_t<InputAttr>& node, Fn*... fn)
            -> parser::syntax::attr_tree_t<OutputAttr> {
            using namespace parser::syntax;
            using namespace std;
            using namespace boost::hana;

            using in_tree_t = attr_tree_t<InputAttr>;
            using out_tree_t = attr_tree_t<OutputAttr>;

            namespace hana = boost::hana;

            auto fn_dict_t = hana::make_map(
                hana::make_pair(hana::type_c<lambda_arg_t<remove_pointer_t<decltype(fn)>>>, fn)...);

            const auto l = node.get().location;
            const auto& attribute = node.get().attribute;

            return std::visit(
                boost::hana::overload(
                    [&](const string_literal_t& literal) {
                        auto result = out_tree_t(literal);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<string_literal_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(literal, node);
                        }
                        return result;
                    },
                    [&](const integral_literal_t& literal) {
                        auto result = out_tree_t(literal);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<integral_literal_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(literal, node);
                        }
                        return result;
                    },
                    [&](const floating_point_literal_t& literal) {
                        auto result = out_tree_t(literal);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<floating_point_literal_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(literal, node);
                        }
                        return result;
                    },
                    [&](const lexer::identifier_t& id) {
                        auto result = out_tree_t(id);
                        result.location = l;
                        result.attribute = OutputAttr{};
                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<lexer::identifier_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(id, node);
                        }
                        return result;
                    },
                    [&](const lexer::token::true_t& t) {
                        auto result = out_tree_t(t);
                        result.location = l;
                        result.attribute = OutputAttr{};
                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<lexer::token::true_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(t, node);
                        }
                        return result;
                    },
                    [&](const lexer::token::false_t& t) {
                        auto result = out_tree_t(t);
                        result.location = l;
                        result.attribute = OutputAttr{};
                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<lexer::token::false_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(t, node);
                        }
                        return result;
                    },
                    [&](const block_t<InputAttr>& block) {
                        auto result = out_tree_t(block_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<block_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(block, node);
                        }

                        for (const auto& stmt : block)
                            result.template get<block_t<OutputAttr>>().push_back(
                                walk_inherit_impl<OutputAttr>(stmt, fn...));

                        return result;
                    },
                    [&](const data_t<InputAttr>& data) {
                        auto result = out_tree_t(data_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<data_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(data, node);
                        }

                        for (const auto& stmt : data)
                            result.template get<data_t<OutputAttr>>().push_back(
                                walk_inherit_impl<OutputAttr>(stmt, fn...));

                        return result;
                    },
                    [&](const unary_op_t<InputAttr>& op) {
                        auto result = out_tree_t(unary_op_t<OutputAttr>{});
                        auto& o = result.template get<unary_op_t<OutputAttr>>();

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<unary_op_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(op, node);
                        }

                        result.op = op.op;
                        result.operand = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(op.operand, fn...));
                        result.location = l;

                        return result;
                    },
                    [&](const bin_op_t<InputAttr>& op) {
                        auto result = out_tree_t(bin_op_t<OutputAttr>{});
                        auto& o = result.template get<bin_op_t<OutputAttr>>();
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<bin_op_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(op, node);
                        }

                        result.op = op.op;
                        result.lhs =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(op.lhs, fn...));
                        result.rhs =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(op.rhs, fn...));

                        return result;
                    },
                    [&](const invoc_t<InputAttr>& i) {
                        auto result = out_tree_t(invoc_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<invoc_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        auto& o = result.template get<invoc_t<OutputAttr>>();
                        o.target =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.target, fn...));

                        for (const auto& arg : i.arguments)
                            o.arguments.push_back(walk_inherit_impl<OutputAttr>(arg, fn...));

                        return result;
                    },
                    [&](const if_t<InputAttr>& i) {
                        auto result = out_tree_t(if_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<if_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        auto& o = result.template get<if_t<OutputAttr>>();

                        for (int j = 0; j < i.elif_tests.size(); j++) {
                            o.elif_tests.push_back(
                                walk_inherit_impl<OutputAttr>(i.elif_tests[j], fn...));

                            o.elif_branches.push_back(
                                walk_inherit_impl<OutputAttr>(i.elif_branches[j], fn...));
                        }

                        o.else_branch = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.else_branch, fn...));

                        return result;
                    },
                    [&](const elif_t<InputAttr>& i) -> out_tree_t {
                        throw error("Illegal dangling \"elif\"", l);
                    },
                    [&](const else_t<InputAttr>& i) -> out_tree_t {
                        throw error("Illegal dangling \"else\"", l);
                    },
                    [&](const fn_expr_t<InputAttr>& i) {
                        auto result = out_tree_t(fn_expr_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<fn_expr_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        auto& o = result.template get<fn_expr_t<OutputAttr>>();

                        o.arg_names = i.arg_names;

                        for (int j = 0; j < i.arg_names.size(); j++)
                            o.arg_types.push_back(
                                walk_inherit_impl<OutputAttr>(i.arg_types[j], fn...));

                        o.result_type = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.result_type, fn...));
                        o.body =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.body, fn...));

                        for (int j = 0; j < i.closure_params.size(); j++) {
                            const auto& p = i.closure_params[j];
                            o.closure_params.push_back(fn_closure_param_t<OutputAttr>{
                                p.var, p.identifier,
                                walk_inherit_impl<OutputAttr>(p.expression, fn...)});
                        }

                        return result;
                    },
                    [&](const var_def_t<InputAttr>& i) {
                        auto result = out_tree_t(var_def_t<OutputAttr>{});

                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<var_def_t<OutputAttr>>();

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<var_def_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        o.name = i.name;
                        o.type =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.type, fn...));
                        o.rhs =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.rhs, fn...));

                        return result;
                    },
                    [&](const for_t<InputAttr>& i) {
                        auto result = out_tree_t(for_t<OutputAttr>{});

                        auto& o = result.template get<for_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<for_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        o.var_lhs = i.var_lhs, o.var_rhs = attr_node_t<OutputAttr>(
                                                   walk_inherit_impl<OutputAttr>(i.var_rhs, fn...));
                        o.body =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.body, fn...));
                        return result;
                    },
                    [&](const while_t<InputAttr>& i) {
                        auto result = out_tree_t(while_t<OutputAttr>{});
                        auto& o = result.template get<while_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<while_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        o.test =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.test, fn...));
                        o.body =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.body, fn...));
                        return result;
                    },
                    [&](const break_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t, hana::type_c<break_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }
                        return result;
                    },
                    [&](const continue_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t, hana::type_c<break_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }
                        return result;
                    },
                    [&](const return_t<InputAttr>& i) {
                        auto result = out_tree_t(return_t<OutputAttr>{});
                        auto& o = result.template get<return_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<return_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        o.value =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.value, fn...));

                        return result;
                    },
                    [&](const yield_t<InputAttr>& i) {
                        auto result = out_tree_t(yield_t<OutputAttr>{});
                        auto& o = result.template get<yield_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<yield_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        o.value =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.value, fn...));

                        return result;
                    },
                    [&](const struct_t<InputAttr>& i) {
                        auto result = out_tree_t(struct_t<OutputAttr>{});
                        auto& o = result.template get<struct_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<struct_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        for (const auto& e : i)
                            o.emplace_back(e.first, walk_inherit_impl<OutputAttr>(e.second, fn...));

                        return result;
                    },
                    [&](const def_type_t<InputAttr>& i) {
                        auto result = out_tree_t(def_type_t<OutputAttr>{});

                        auto& o = result.template get<def_type_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<def_type_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        o.name = i.name;
                        o.type =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.type, fn...));

                        return result;
                    },
                    [&](const let_type_t<InputAttr>& i) {
                        auto result = out_tree_t(let_type_t<OutputAttr>{});

                        auto& o = result.template get<let_type_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<let_type_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        o.name = i.name;
                        o.type =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.type, fn...));

                        return result;
                    },
                    [&](const template_t<InputAttr>& i) {
                        auto result = out_tree_t(template_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<template_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        auto& o = result.template get<template_t<OutputAttr>>();

                        for (const auto& e : i.arguments)
                            o.arguments.emplace_back(
                                e.first, walk_inherit_impl<OutputAttr>(e.second, fn...));

                        o.body =
                            attr_node_t<OutputAttr>(walk_inherit_impl<OutputAttr>(i.body, fn...));

                        return result;
                    },
                    [&](const attr_node_t<InputAttr>& i) {
                        auto result = out_tree_t(attr_node_t<OutputAttr>());

                        auto& o = result.template get<attr_node_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<attr_node_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node);
                        }

                        o = walk_inherit_impl<OutputAttr>(i, fn...);

                        return result;
                    },
                    [&](const auto&) {
                        auto result = out_tree_t();
                        result.location = l;
                        result.attribute = OutputAttr{};
                        return result;
                    }),
                node.get());
        }
    }  // namespace details

    template <typename OutputAttr, class... Fn, typename InputAttr = parser::empty_attribute_t>
    inline auto walk_synth(const parser::syntax::attr_tree_t<InputAttr>& tree, Fn&&... fn)
        -> parser::syntax::attr_tree_t<OutputAttr> {
        using namespace parser::syntax;
        using namespace std;
        using in_tree_t = attr_tree_t<InputAttr>;
        using out_tree_t = attr_tree_t<OutputAttr>;

        namespace hana = boost::hana;

        auto fns = std::tuple(std::move(fn)...);

        auto fn_dict_t = hana::make_map(hana::make_pair(
            hana::type_c<lambda_arg_t<remove_reference_t<decltype(fn)>>>, &std::get<Fn>(fns))...);

        const auto node = parser::syntax::attr_node_t<InputAttr>(tree);
        return details::walk_synth_impl<OutputAttr>(node, &fn...);
    }

    template <typename OutputAttr, class... Fn, typename InputAttr = parser::empty_attribute_t>
    inline auto walk_inherit(const parser::syntax::attr_tree_t<InputAttr>& tree, Fn&&... fn)
        -> parser::syntax::attr_tree_t<OutputAttr> {
        using namespace parser::syntax;
        using namespace std;
        using in_tree_t = attr_tree_t<InputAttr>;
        using out_tree_t = attr_tree_t<OutputAttr>;

        namespace hana = boost::hana;

        auto fns = std::tuple(std::move(fn)...);

        auto fn_dict_t = hana::make_map(hana::make_pair(
            hana::type_c<lambda_arg_t<remove_reference_t<decltype(fn)>>>, &std::get<Fn>(fns))...);

        const auto node = parser::syntax::attr_node_t<InputAttr>(tree);
        return details::walk_inherit_impl<OutputAttr>(node, &fn...);
    }

    template <typename Attr>
    inline auto purify(const parser::syntax::attr_tree_t<Attr>& node)
        -> parser::syntax::attr_tree_t<parser::empty_attribute_t> {
        return walk_synth<parser::empty_attribute_t>(node);
    }

    template <typename Attr, typename T>
    inline auto purify(T value) -> parser::syntax::attr_tree_t<parser::empty_attribute_t> {
        return walk_synth<parser::empty_attribute_t>(parser::syntax::attr_tree_t<Attr>(value));
    }

}}  // namespace bt::analysis
