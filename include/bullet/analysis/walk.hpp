#pragma once

#include <boost/hana/all.hpp>

#include <bullet/analysis/error.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/parser/attribute.hpp>
#include <bullet/util.hpp>

namespace bt { namespace analysis {

    namespace details {
        template <typename OutputAttr, class... Fn, typename InputAttr>
        inline auto walk_synth_impl(const parser::syntax::attr_tree_t<InputAttr>& node, Fn*... fn)
            -> parser::syntax::attr_tree_t<OutputAttr> {
            using namespace parser::syntax;
            using namespace std;
            using in_tree_t = attr_tree_t<InputAttr>;
            using out_tree_t = attr_tree_t<OutputAttr>;

            namespace hana = boost::hana;

            auto fn_dict_t = hana::make_map(
                hana::make_pair(hana::type_c<lambda_arg_t<remove_pointer_t<decltype(fn)>>>, fn)...);

            const auto l = node.location;

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
                                (**pf)(result.template get<string_literal_t>(), node.attribute);
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
                                (**pf)(result.template get<integral_literal_t>(), node.attribute);
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
                            result.attribute = (**pf)(
                                result.template get<floating_point_literal_t>(), node.attribute);
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
                                (**pf)(result.template get<lexer::identifier_t>(), node.attribute);
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
                                (**pf)(result.template get<lexer::token::true_t>(), node.attribute);
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
                            result.attribute = (**pf)(result.template get<lexer::token::false_t>(),
                                                      node.attribute);
                        }
                        return result;
                    },
                    [&](const block_t<InputAttr>& block) {
                        auto result = out_tree_t(block_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        for (const auto& stmt : block)
                            result.template get<block_t<OutputAttr>>().push_back(
                                walk_synth_impl<OutputAttr>(stmt.get(), fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<block_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<block_t<OutputAttr>>(), node.attribute);
                        }

                        return result;
                    },
                    [&](const data_t<InputAttr>& data) {
                        auto result = out_tree_t(data_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        for (const auto& stmt : data)
                            result.template get<data_t<OutputAttr>>().push_back(
                                walk_synth_impl<OutputAttr>(stmt.get(), fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<data_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<data_t<OutputAttr>>(), node.attribute);
                        }

                        return result;
                    },
                    [&](const unary_op_t<InputAttr>& op) {
                        auto result = out_tree_t(unary_op_t<OutputAttr>{
                            op.op, walk_synth_impl<OutputAttr>(op.operand.get(), fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<unary_op_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(result.template get<unary_op_t<OutputAttr>>(),
                                                      node.attribute);
                        }

                        return result;
                    },
                    [&](const bin_op_t<InputAttr>& op) {
                        auto result = out_tree_t(bin_op_t<OutputAttr>{
                            op.op, walk_synth_impl<OutputAttr>(op.lhs.get(), fn...),
                            walk_synth_impl<OutputAttr>(op.rhs.get(), fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<bin_op_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<bin_op_t<OutputAttr>>(), node.attribute);
                        }

                        return result;
                    },
                    [&](const invoc_t<InputAttr>& i) {
                        auto result = out_tree_t(invoc_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        for (const auto& arg : i.arguments)
                            result.template get<invoc_t<OutputAttr>>().arguments.push_back(
                                walk_synth_impl<OutputAttr>(arg.get(), fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<invoc_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<invoc_t<OutputAttr>>(), node.attribute);
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
                                walk_synth_impl<OutputAttr>(i.elif_tests[j].get(), fn...));

                            o.elif_branches.push_back(
                                walk_synth_impl<OutputAttr>(i.elif_branches[j].get(), fn...));
                        }

                        o.else_branch = attr_node_t<OutputAttr>(
                            walk_synth_impl<OutputAttr>(i.else_branch.get(), fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<if_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<if_t<OutputAttr>>(), node.attribute);
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
                                walk_synth_impl<OutputAttr>(i.arg_types[j].get(), fn...));

                        o.result_type = attr_node_t<OutputAttr>(
                            walk_synth_impl<OutputAttr>(i.result_type.get(), fn...));
                        o.body = attr_node_t<OutputAttr>(
                            walk_synth_impl<OutputAttr>(i.body.get(), fn...));

                        for (int j = 0; j < i.closure_params.size(); j++) {
                            const auto& p = i.closure_params[j];
                            o.closure_params.push_back(fn_closure_param_t<OutputAttr>{
                                p.var, p.identifier,
                                walk_synth_impl<OutputAttr>(p.expression.get(), fn...)});
                        }

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<fn_expr_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(result.template get<fn_expr_t<OutputAttr>>(),
                                                      node.attribute);
                        }

                        return result;
                    },
                    [&](const var_def_t<InputAttr>& i) {
                        auto result = out_tree_t(var_def_t<OutputAttr>{
                            i.name, walk_synth_impl<OutputAttr>(i.type.get(), fn...),
                            walk_synth_impl<OutputAttr>(i.rhs.get(), fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<var_def_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(result.template get<var_def_t<OutputAttr>>(),
                                                      node.attribute);
                        }

                        return result;
                    },
                    [&](const for_t<InputAttr>& i) {
                        auto result = out_tree_t(for_t<OutputAttr>{
                            i.var_lhs, walk_synth_impl<OutputAttr>(i.var_rhs.get(), fn...),
                            walk_synth_impl<OutputAttr>(i.body.get(), fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<for_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<for_t<OutputAttr>>(), node.attribute);
                        }

                        return result;
                    },
                    [&](const while_t<InputAttr>& i) {
                        auto result = out_tree_t(
                            while_t<OutputAttr>{walk_synth_impl<OutputAttr>(i.test.get(), fn...),
                                                walk_synth_impl<OutputAttr>(i.body.get(), fn...)});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<while_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<while_t<OutputAttr>>(), node.attribute);
                        }

                        return result;
                    },
                    [&](const break_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t, hana::type_c<break_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<break_t>(), node.attribute);
                        }
                        return result;
                    },
                    [&](const continue_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t, hana::type_c<break_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<break_t>(), node.attribute);
                        }
                        return result;
                    },
                    [&](const return_t<InputAttr>& i) {
                        auto result = out_tree_t(return_t<OutputAttr>{
                            walk_synth_impl<OutputAttr>(i.value.get(), fn...),
                        });
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<return_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<return_t<OutputAttr>>(), node.attribute);
                        }

                        return result;
                    },
                    [&](const yield_t<InputAttr>& i) {
                        auto result = out_tree_t(yield_t<OutputAttr>{
                            walk_synth_impl<OutputAttr>(i.value.get(), fn...),
                        });
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<yield_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<yield_t<OutputAttr>>(), node.attribute);
                        }

                        return result;
                    },
                    [&](const struct_t<InputAttr>& i) {
                        auto result = out_tree_t(struct_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<struct_t<OutputAttr>>();

                        for (const auto& e : i)
                            o.emplace_back(e.first,
                                           walk_synth_impl<OutputAttr>(e.second.get(), fn...));

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<struct_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute =
                                (**pf)(result.template get<struct_t<OutputAttr>>(), node.attribute);
                        }

                        return result;
                    },
                    [&](const def_type_t<InputAttr>& i) {
                        auto result = out_tree_t(def_type_t<OutputAttr>{
                            i.name,
                            walk_synth_impl<OutputAttr>(i.type.get(), fn...),
                        });
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<def_type_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(result.template get<def_type_t<OutputAttr>>(),
                                                      node.attribute);
                        }

                        return result;
                    },
                    [&](const let_type_t<InputAttr>& i) {
                        auto result = out_tree_t(let_type_t<OutputAttr>{
                            i.name,
                            walk_synth_impl<OutputAttr>(i.type.get(), fn...),
                        });
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<let_type_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(result.template get<let_type_t<OutputAttr>>(),
                                                      node.attribute);
                        }

                        return result;
                    },
                    [&](const template_t<InputAttr>& i) {
                        auto result = out_tree_t(template_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<template_t<OutputAttr>>();

                        for (const auto& e : i.arguments)
                            o.arguments.emplace_back(
                                e.first, walk_synth_impl<OutputAttr>(e.second.get(), fn...));

                        o.body = attr_node_t<OutputAttr>(
                            walk_synth_impl<OutputAttr>(i.body.get(), fn...));

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<template_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(result.template get<template_t<OutputAttr>>(),
                                                      node.attribute);
                        }

                        return result;
                    },
                    [&](const attr_node_t<InputAttr>& i) {
                        auto result = out_tree_t(
                            attr_node_t<OutputAttr>(walk_synth_impl<OutputAttr>(i.get(), fn...)));
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<attr_node_t<OutputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(
                                result.template get<attr_node_t<OutputAttr>>(), node.attribute);
                        }

                        return result;
                    },
                    [&](const auto&) {
                        auto result = out_tree_t();
                        result.location = l;
                        result.attribute = OutputAttr{};
                        return result;
                    }),
                node);
        }

        template <typename OutputAttr, class... Fn, typename InputAttr>
        inline auto walk_inherit_impl(const parser::syntax::attr_tree_t<InputAttr>& node, Fn*... fn)
            -> parser::syntax::attr_tree_t<OutputAttr> {
            using namespace parser::syntax;
            using namespace std;
            using namespace boost::hana;

            using in_tree_t = attr_tree_t<InputAttr>;
            using out_tree_t = attr_tree_t<OutputAttr>;

            namespace hana = boost::hana;

            auto fn_dict_t = hana::make_map(
                hana::make_pair(hana::type_c<lambda_arg_t<remove_pointer_t<decltype(fn)>>>, fn)...);

            const auto l = node.location;

            return std::visit(
                boost::hana::overload(
                    [&](const string_literal_t& literal) {
                        auto result = out_tree_t(literal);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<string_literal_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(literal, node.attribute);
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
                            result.attribute = (**pf)(literal, node.attribute);
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
                            result.attribute = (**pf)(literal, node.attribute);
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
                            result.attribute = (**pf)(id, node.attribute);
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
                            result.attribute = (**pf)(t, node.attribute);
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
                            result.attribute = (**pf)(t, node.attribute);
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
                            result.attribute = (**pf)(block, node.attribute);
                        }

                        for (const auto& stmt : block)
                            result.template get<block_t<OutputAttr>>().push_back(
                                walk_inherit_impl<OutputAttr>(stmt.get(), fn...));

                        return result;
                    },
                    [&](const data_t<InputAttr>& data) {
                        auto result = out_tree_t(data_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<data_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(data, node.attribute);
                        }

                        for (const auto& stmt : data)
                            result.template get<data_t<OutputAttr>>().push_back(
                                walk_inherit_impl<OutputAttr>(stmt.get(), fn...));

                        return result;
                    },
                    [&](const unary_op_t<InputAttr>& op) {
                        auto result = out_tree_t(unary_op_t<OutputAttr>{});
                        auto& o = result.template get<unary_op_t<OutputAttr>>();

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<unary_op_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(op, node.attribute);
                        }

                        result.op = op.op;
                        result.operand = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(op.operand.get(), fn...));
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
                            result.attribute = (**pf)(op, node.attribute);
                        }

                        result.op = op.op;
                        result.lhs = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(op.lhs.get(), fn...));
                        result.rhs = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(op.rhs.get(), fn...));

                        return result;
                    },
                    [&](const invoc_t<InputAttr>& i) {
                        auto result = out_tree_t(invoc_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<invoc_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        for (const auto& arg : i.arguments)
                            result.template get<invoc_t<OutputAttr>>().arguments.push_back(
                                walk_inherit_impl<OutputAttr>(arg.get(), fn...));

                        return result;
                    },
                    [&](const if_t<InputAttr>& i) {
                        auto result = out_tree_t(if_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf =
                                          hana::find(fn_dict_t, hana::type_c<if_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        auto& o = result.template get<if_t<OutputAttr>>();

                        for (int j = 0; j < i.elif_tests.size(); j++) {
                            o.elif_tests.push_back(
                                walk_inherit_impl<OutputAttr>(i.elif_tests[j].get(), fn...));

                            o.elif_branches.push_back(
                                walk_inherit_impl<OutputAttr>(i.elif_branches[j].get(), fn...));
                        }

                        o.else_branch = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.else_branch.get(), fn...));

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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        auto& o = result.template get<fn_expr_t<OutputAttr>>();

                        o.arg_names = i.arg_names;

                        for (int j = 0; j < i.arg_names.size(); j++)
                            o.arg_types.push_back(
                                walk_inherit_impl<OutputAttr>(i.arg_types[j].get(), fn...));

                        o.result_type = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.result_type.get(), fn...));
                        o.body = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.body.get(), fn...));

                        for (int j = 0; j < i.closure_params.size(); j++) {
                            const auto& p = i.closure_params[j];
                            o.closure_params.push_back(fn_closure_param_t<OutputAttr>{
                                p.var, p.identifier,
                                walk_inherit_impl<OutputAttr>(p.expression.get(), fn...)});
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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        o.name = i.name;
                        o.type = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.type.get(), fn...));
                        o.rhs = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.rhs.get(), fn...));

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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        o.var_lhs = i.var_lhs,
                        o.var_rhs = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.var_rhs.get(), fn...));
                        o.body = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.body.get(), fn...));
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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        o.test = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.test.get(), fn...));
                        o.body = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.body.get(), fn...));
                        return result;
                    },
                    [&](const break_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t, hana::type_c<break_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node.attribute);
                        }
                        return result;
                    },
                    [&](const continue_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t, hana::type_c<break_t>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node.attribute);
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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        o.value = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.value.get(), fn...));

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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        o.value = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.value.get(), fn...));

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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        for (const auto& e : i)
                            o.emplace_back(e.first,
                                           walk_inherit_impl<OutputAttr>(e.second.get(), fn...));

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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        o.name = i.name;
                        o.type = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.type.get(), fn...));

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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        o.name = i.name;
                        o.type = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.type.get(), fn...));

                        return result;
                    },
                    [&](const template_t<InputAttr>& i) {
                        auto result = out_tree_t(template_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        if constexpr (auto pf = hana::find(fn_dict_t,
                                                           hana::type_c<template_t<InputAttr>>);
                                      !hana::is_nothing(pf)) {
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        auto& o = result.template get<template_t<OutputAttr>>();

                        for (const auto& e : i.arguments)
                            o.arguments.emplace_back(
                                e.first, walk_inherit_impl<OutputAttr>(e.second.get(), fn...));

                        o.body = attr_node_t<OutputAttr>(
                            walk_inherit_impl<OutputAttr>(i.body.get(), fn...));

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
                            result.attribute = (**pf)(i, node.attribute);
                        }

                        o = walk_inherit_impl<OutputAttr>(i.get(), fn...);

                        return result;
                    },
                    [&](const auto&) {
                        auto result = out_tree_t();
                        result.location = l;
                        result.attribute = OutputAttr{};
                        return result;
                    }),
                node);
        }
    }  // namespace details

    template <typename OutputAttr, class... Fn, typename InputAttr = parser::empty_attribute_t>
    inline auto walk_synth(const parser::syntax::attr_tree_t<InputAttr>& node, Fn&&... fn)
        -> parser::syntax::attr_tree_t<OutputAttr> {
        using namespace parser::syntax;
        using namespace std;
        using in_tree_t = attr_tree_t<InputAttr>;
        using out_tree_t = attr_tree_t<OutputAttr>;

        namespace hana = boost::hana;

        auto fns = std::tuple(std::move(fn)...);

        auto fn_dict_t = hana::make_map(hana::make_pair(
            hana::type_c<lambda_arg_t<remove_reference_t<decltype(fn)>>>, &std::get<Fn>(fns))...);

        return details::walk_synth_impl<OutputAttr>(node, &fn...);
    }

    template <typename OutputAttr, class... Fn, typename InputAttr = parser::empty_attribute_t>
    inline auto walk_inherit(const parser::syntax::attr_tree_t<InputAttr>& node, Fn&&... fn)
        -> parser::syntax::attr_tree_t<OutputAttr> {
        using namespace parser::syntax;
        using namespace std;
        using in_tree_t = attr_tree_t<InputAttr>;
        using out_tree_t = attr_tree_t<OutputAttr>;

        namespace hana = boost::hana;

        auto fns = std::tuple(std::move(fn)...);

        auto fn_dict_t = hana::make_map(hana::make_pair(
            hana::type_c<lambda_arg_t<remove_reference_t<decltype(fn)>>>, &std::get<Fn>(fns))...);

        return details::walk_inherit_impl<OutputAttr>(node, &fn...);
    }
}}  // namespace bt::analysis
