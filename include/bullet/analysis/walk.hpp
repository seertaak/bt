#pragma once

#include <boost/hana/all.hpp>

#include <bullet/analysis/error.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/parser/attribute.hpp>
#include <bullet/util.hpp>

namespace bt { namespace analysis {

    namespace details {
        template <typename OutputAttr, class... Fn, typename InputAttr>
        inline auto walk_post_order_impl(const parser::syntax::attr_node_t<InputAttr>& node,
                                         Fn*... fn) -> parser::syntax::attr_tree_t<OutputAttr> {
            using namespace parser::syntax;
            using namespace std;
            using in_tree_t = attr_tree_t<InputAttr>;
            using out_tree_t = attr_tree_t<OutputAttr>;

            namespace hana = boost::hana;

            const auto f = hana::overload((*fn)...);
            const auto l = node.get().location;
            const auto& attribute = node.get().attribute;

            return std::visit(
                boost::hana::overload(
                    [&](const primitive_type_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = f(result.template get<primitive_type_t>(), node);
                        return result;
                    },
                    [&](const literal_t& literal) {
                        auto result = out_tree_t(literal);
                        result.location = l;
                        result.attribute = f(result.template get<literal_t>(), node);
                        return result;
                    },
                    [&](const lexer::identifier_t& id) {
                        auto result = out_tree_t(id);
                        result.location = l;
                        result.attribute = f(id, node);
                        return result;
                    },
                    [&](const block_t<InputAttr>& block) {
                        auto result = out_tree_t(block_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        for (const auto& stmt : block)
                            result.template get<block_t<OutputAttr>>().push_back(
                                walk_post_order_impl<OutputAttr>(stmt, fn...));

                        result.attribute = f(result.template get<block_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const data_t<InputAttr>& data) {
                        auto result = out_tree_t(data_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        for (const auto& stmt : data)
                            result.template get<data_t<OutputAttr>>().push_back(
                                walk_post_order_impl<OutputAttr>(stmt, fn...));

                        result.attribute = f(result.template get<data_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const unary_op_t<InputAttr>& op) {
                        auto result = out_tree_t(unary_op_t<OutputAttr>{
                            op.op, walk_post_order_impl<OutputAttr>(op.operand, fn...)});
                        result.location = l;
                        result.attribute = f(result.template get<unary_op_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const bin_op_t<InputAttr>& op) {
                        auto result = out_tree_t(bin_op_t<OutputAttr>{
                            op.op, walk_post_order_impl<OutputAttr>(op.lhs, fn...),
                            walk_post_order_impl<OutputAttr>(op.rhs, fn...)});
                        result.location = l;
                        result.attribute = f(result.template get<bin_op_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const invoc_t<InputAttr>& i) {
                        auto result = out_tree_t(invoc_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<invoc_t<OutputAttr>>();
                        o.target = attr_node_t<OutputAttr>(
                            walk_post_order_impl<OutputAttr>(i.target, fn...));

                        for (const auto& arg : i.arguments)
                            o.arguments.push_back(walk_post_order_impl<OutputAttr>(arg, fn...));

                        result.attribute = f(o, node);

                        return result;
                    },
                    [&](const if_t<InputAttr>& i) {
                        auto result = out_tree_t(if_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<if_t<OutputAttr>>();

                        for (int j = 0; j < i.elif_tests.size(); j++) {
                            o.elif_tests.push_back(
                                walk_post_order_impl<OutputAttr>(i.elif_tests[j], fn...));

                            o.elif_branches.push_back(
                                walk_post_order_impl<OutputAttr>(i.elif_branches[j], fn...));
                        }

                        o.else_branch = attr_node_t<OutputAttr>(
                            walk_post_order_impl<OutputAttr>(i.else_branch, fn...));

                        result.attribute = f(o, node);

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
                                walk_post_order_impl<OutputAttr>(i.arg_types[j], fn...));

                        o.result_type = attr_node_t<OutputAttr>(
                            walk_post_order_impl<OutputAttr>(i.result_type, fn...));
                        o.body = attr_node_t<OutputAttr>(
                            walk_post_order_impl<OutputAttr>(i.body, fn...));

                        for (int j = 0; j < i.closure_params.size(); j++) {
                            const auto& p = i.closure_params[j];
                            o.closure_params.push_back(fn_closure_param_t<OutputAttr>{
                                p.var, p.identifier,
                                walk_post_order_impl<OutputAttr>(p.expression, fn...)});
                        }

                        result.attribute = f(o, node);

                        return result;
                    },
                    [&](const assign_t<InputAttr>& i) {
                        auto result = out_tree_t(
                            assign_t<OutputAttr>{walk_post_order_impl<OutputAttr>(i.lhs, fn...),
                                                 walk_post_order_impl<OutputAttr>(i.rhs, fn...)});
                        result.location = l;
                        result.attribute = f(result.template get<assign_t<OutputAttr>>(), node);
                        return result;
                    },
                    [&](const let_var_t<InputAttr>& i) {
                        auto result = out_tree_t(let_var_t<OutputAttr>{
                            i.name, walk_post_order_impl<OutputAttr>(i.type, fn...),
                            walk_post_order_impl<OutputAttr>(i.rhs, fn...)});
                        result.location = l;
                        result.attribute = f(result.template get<let_var_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const var_def_t<InputAttr>& i) {
                        auto result = out_tree_t(
                            var_def_t<OutputAttr>{i.name, i.n_indirections,
                                                  walk_post_order_impl<OutputAttr>(i.type, fn...),
                                                  walk_post_order_impl<OutputAttr>(i.rhs, fn...)});
                        result.location = l;
                        result.attribute = f(result.template get<var_def_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const for_t<InputAttr>& i) {
                        auto result = out_tree_t(for_t<OutputAttr>{
                            i.var_lhs, walk_post_order_impl<OutputAttr>(i.var_rhs, fn...),
                            walk_post_order_impl<OutputAttr>(i.body, fn...)});
                        result.location = l;
                        result.attribute = f(result.template get<for_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const while_t<InputAttr>& i) {
                        auto result = out_tree_t(
                            while_t<OutputAttr>{walk_post_order_impl<OutputAttr>(i.test, fn...),
                                                walk_post_order_impl<OutputAttr>(i.body, fn...)});
                        result.location = l;
                        result.attribute = f(result.template get<while_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const break_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = f(result.template get<break_t>(), node);
                        return result;
                    },
                    [&](const continue_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = f(result.template get<continue_t>(), node);
                        return result;
                    },
                    [&](const type_expr_t<InputAttr>& i) {
                        auto result = out_tree_t(type_expr_t<OutputAttr>{
                            walk_post_order_impl<OutputAttr>(i.type, fn...),
                        });
                        result.location = l;
                        result.attribute = f(result.template get<type_expr_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const return_t<InputAttr>& i) {
                        auto result = out_tree_t(return_t<OutputAttr>{
                            walk_post_order_impl<OutputAttr>(i.value, fn...),
                        });
                        result.location = l;
                        result.attribute = f(result.template get<return_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const yield_t<InputAttr>& i) {
                        auto result = out_tree_t(yield_t<OutputAttr>{
                            walk_post_order_impl<OutputAttr>(i.value, fn...),
                        });
                        result.location = l;
                        result.attribute = f(result.template get<yield_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const struct_t<InputAttr>& i) {
                        auto result = out_tree_t(struct_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<struct_t<OutputAttr>>();

                        for (const auto& e : i)
                            o.emplace_back(e.first,
                                           walk_post_order_impl<OutputAttr>(e.second, fn...));

                        result.attribute = f(o, node);

                        return result;
                    },
                    [&](const def_type_t<InputAttr>& i) {
                        auto result = out_tree_t(def_type_t<OutputAttr>{
                            i.name,
                            walk_post_order_impl<OutputAttr>(i.type, fn...),
                        });
                        result.location = l;
                        result.attribute = f(result.template get<def_type_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const let_type_t<InputAttr>& i) {
                        auto result = out_tree_t(let_type_t<OutputAttr>{
                            i.name,
                            walk_post_order_impl<OutputAttr>(i.type, fn...),
                        });
                        result.location = l;
                        result.attribute = f(result.template get<let_type_t<OutputAttr>>(), node);

                        return result;
                    },
                    [&](const template_t<InputAttr>& i) {
                        auto result = out_tree_t(template_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = OutputAttr{};

                        auto& o = result.template get<template_t<OutputAttr>>();

                        for (const auto& e : i.arguments)
                            o.arguments.emplace_back(
                                e.first, walk_post_order_impl<OutputAttr>(e.second, fn...));

                        o.body = attr_node_t<OutputAttr>(
                            walk_post_order_impl<OutputAttr>(i.body, fn...));

                        result.attribute = f(o, node);

                        return result;
                    },
                    [&](const attr_node_t<InputAttr>& i) {
                        auto result = out_tree_t(
                            attr_node_t<OutputAttr>(walk_post_order_impl<OutputAttr>(i, fn...)));
                        result.location = l;
                        result.attribute = f(result.template get<attr_node_t<OutputAttr>>(), node);
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
        inline auto walk_pre_order_impl(const parser::syntax::attr_node_t<InputAttr>& node,
                                        const InputAttr& parent_attrib,
                                        Fn*... fn) -> parser::syntax::attr_tree_t<OutputAttr> {
            using namespace parser::syntax;
            using namespace std;
            using namespace boost::hana;

            using in_tree_t = attr_tree_t<InputAttr>;
            using out_tree_t = attr_tree_t<OutputAttr>;

            namespace hana = boost::hana;

            const auto l = node.get().location;
            const auto& attribute = node.get().attribute;

            const auto f = hana::overload((*fn)...);

            return std::visit(
                boost::hana::overload(
                    [&](const primitive_type_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);
                        return result;
                    },
                    [&](const literal_t& literal) {
                        auto result = out_tree_t(literal);
                        result.location = l;
                        result.attribute = f(literal, node, parent_attrib);
                        return result;
                    },
                    [&](const lexer::identifier_t& id) {
                        auto result = out_tree_t(id);
                        result.location = l;
                        result.attribute = f(id, node, parent_attrib);
                        return result;
                    },
                    [&](const block_t<InputAttr>& block) {
                        auto result = out_tree_t(block_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = f(block, node, parent_attrib);

                        for (const auto& stmt : block)
                            result.template get<block_t<OutputAttr>>().push_back(
                                walk_pre_order_impl<OutputAttr>(stmt, result.attribute, fn...));

                        return result;
                    },
                    [&](const data_t<InputAttr>& data) {
                        auto result = out_tree_t(data_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = f(data, node, parent_attrib);
                        for (const auto& stmt : data)
                            result.template get<data_t<OutputAttr>>().push_back(
                                walk_pre_order_impl<OutputAttr>(stmt, result.attribute, fn...));

                        return result;
                    },
                    [&](const unary_op_t<InputAttr>& op) {
                        auto result = out_tree_t(unary_op_t<OutputAttr>{});
                        result.attribute = f(op, node, parent_attrib);

                        auto& o = result.template get<unary_op_t<OutputAttr>>();
                        o.op = op.op;
                        o.operand = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(op.operand, result.attribute, fn...));
                        result.location = l;

                        return result;
                    },
                    [&](const bin_op_t<InputAttr>& op) {
                        auto result = out_tree_t(bin_op_t<OutputAttr>{});
                        auto& o = result.template get<bin_op_t<OutputAttr>>();
                        result.location = l;
                        result.attribute = f(op, node, parent_attrib);

                        o.op = op.op;
                        o.lhs = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(op.lhs, result.attribute, fn...));
                        o.rhs = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(op.rhs, result.attribute, fn...));

                        return result;
                    },
                    [&](const invoc_t<InputAttr>& i) {
                        auto result = out_tree_t(invoc_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        auto& o = result.template get<invoc_t<OutputAttr>>();
                        o.target = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.target, result.attribute, fn...));

                        for (const auto& arg : i.arguments)
                            o.arguments.push_back(
                                walk_pre_order_impl<OutputAttr>(arg, result.attribute, fn...));

                        return result;
                    },
                    [&](const if_t<InputAttr>& i) {
                        auto result = out_tree_t(if_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        auto& o = result.template get<if_t<OutputAttr>>();

                        for (int j = 0; j < i.elif_tests.size(); j++) {
                            o.elif_tests.push_back(walk_pre_order_impl<OutputAttr>(
                                i.elif_tests[j], result.attribute, fn...));

                            o.elif_branches.push_back(walk_pre_order_impl<OutputAttr>(
                                i.elif_branches[j], result.attribute, fn...));
                        }

                        o.else_branch = attr_node_t<OutputAttr>(walk_pre_order_impl<OutputAttr>(
                            i.else_branch, result.attribute, fn...));

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
                        result.attribute = f(i, node, parent_attrib);

                        auto& o = result.template get<fn_expr_t<OutputAttr>>();

                        o.arg_names = i.arg_names;

                        for (int j = 0; j < i.arg_names.size(); j++)
                            o.arg_types.push_back(walk_pre_order_impl<OutputAttr>(
                                i.arg_types[j], result.attribute, fn...));

                        o.result_type = attr_node_t<OutputAttr>(walk_pre_order_impl<OutputAttr>(
                            i.result_type, result.attribute, fn...));
                        o.body = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.body, result.attribute, fn...));

                        for (int j = 0; j < i.closure_params.size(); j++) {
                            const auto& p = i.closure_params[j];
                            o.closure_params.push_back(fn_closure_param_t<OutputAttr>{
                                p.var, p.identifier,
                                walk_pre_order_impl<OutputAttr>(p.expression, result.attribute,
                                                                fn...)});
                        }

                        return result;
                    },
                    [&](const assign_t<InputAttr>& i) {
                        auto result = out_tree_t(assign_t<OutputAttr>{});

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        auto& o = result.template get<assign_t<OutputAttr>>();

                        o.lhs = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.lhs, result.attribute, fn...));
                        o.rhs = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.rhs, result.attribute, fn...));

                        return result;
                    },
                    [&](const let_var_t<InputAttr>& i) {
                        auto result = out_tree_t(let_var_t<OutputAttr>{});

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        auto& o = result.template get<let_var_t<OutputAttr>>();

                        o.name = i.name;
                        o.type = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.type, result.attribute, fn...));
                        o.rhs = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.rhs, result.attribute, fn...));

                        return result;
                    },
                    [&](const var_def_t<InputAttr>& i) {
                        auto result = out_tree_t(var_def_t<OutputAttr>{});

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        auto& o = result.template get<var_def_t<OutputAttr>>();

                        o.name = i.name;
                        o.n_indirections = i.n_indirections;
                        o.type = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.type, result.attribute, fn...));
                        o.rhs = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.rhs, result.attribute, fn...));

                        return result;
                    },
                    [&](const for_t<InputAttr>& i) {
                        auto result = out_tree_t(for_t<OutputAttr>{});

                        auto& o = result.template get<for_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        o.var_lhs = i.var_lhs,
                        o.var_rhs = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.var_rhs, result.attribute, fn...));
                        o.body = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.body, result.attribute, fn...));
                        return result;
                    },
                    [&](const while_t<InputAttr>& i) {
                        auto result = out_tree_t(while_t<OutputAttr>{});
                        auto& o = result.template get<while_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        o.test = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.test, result.attribute, fn...));
                        o.body = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.body, result.attribute, fn...));
                        return result;
                    },
                    [&](const break_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);
                        return result;
                    },
                    [&](const continue_t& i) {
                        auto result = out_tree_t(i);
                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);
                        return result;
                    },
                    [&](const type_expr_t<InputAttr>& i) {
                        auto result = out_tree_t(type_expr_t<OutputAttr>{});
                        auto& o = result.template get<type_expr_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        o.type = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.type, result.attribute, fn...));

                        return result;
                    },
                    [&](const return_t<InputAttr>& i) {
                        auto result = out_tree_t(return_t<OutputAttr>{});
                        auto& o = result.template get<return_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        o.value = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.value, result.attribute, fn...));

                        return result;
                    },
                    [&](const yield_t<InputAttr>& i) {
                        auto result = out_tree_t(yield_t<OutputAttr>{});
                        auto& o = result.template get<yield_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        o.value = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.value, result.attribute, fn...));

                        return result;
                    },
                    [&](const struct_t<InputAttr>& i) {
                        auto result = out_tree_t(struct_t<OutputAttr>{});
                        auto& o = result.template get<struct_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        for (const auto& e : i)
                            o.emplace_back(e.first, walk_pre_order_impl<OutputAttr>(
                                                        e.second, result.attribute, fn...));

                        return result;
                    },
                    [&](const def_type_t<InputAttr>& i) {
                        auto result = out_tree_t(def_type_t<OutputAttr>{});

                        auto& o = result.template get<def_type_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        o.name = i.name;
                        o.type = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.type, result.attribute, fn...));

                        return result;
                    },
                    [&](const let_type_t<InputAttr>& i) {
                        auto result = out_tree_t(let_type_t<OutputAttr>{});

                        auto& o = result.template get<let_type_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        o.name = i.name;
                        o.type = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.type, result.attribute, fn...));

                        return result;
                    },
                    [&](const template_t<InputAttr>& i) {
                        auto result = out_tree_t(template_t<OutputAttr>{});
                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        auto& o = result.template get<template_t<OutputAttr>>();

                        for (const auto& e : i.arguments)
                            o.arguments.emplace_back(
                                e.first,
                                walk_pre_order_impl<OutputAttr>(e.second, result.attribute, fn...));

                        o.body = attr_node_t<OutputAttr>(
                            walk_pre_order_impl<OutputAttr>(i.body, result.attribute, fn...));

                        return result;
                    },
                    [&](const attr_node_t<InputAttr>& i) {
                        auto result = out_tree_t(attr_node_t<OutputAttr>());

                        auto& o = result.template get<attr_node_t<OutputAttr>>();

                        result.location = l;
                        result.attribute = f(i, node, parent_attrib);

                        o = walk_pre_order_impl<OutputAttr>(i, result.attribute, fn...);

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
    inline auto walk_post_order(const parser::syntax::attr_tree_t<InputAttr>& tree, Fn&&... fn)
        -> parser::syntax::attr_tree_t<OutputAttr> {
        const auto node = parser::syntax::attr_node_t<InputAttr>(tree);
        return details::walk_post_order_impl<OutputAttr>(node, &fn...);
    }

    template <typename OutputAttr, class... Fn, typename InputAttr = parser::empty_attribute_t>
    inline auto walk_pre_order(const parser::syntax::attr_tree_t<InputAttr>& tree, Fn&&... fn)
        -> parser::syntax::attr_tree_t<OutputAttr> {
        const auto root_attribute = InputAttr{};
        const auto node = parser::syntax::attr_node_t<InputAttr>(tree);
        return details::walk_pre_order_impl<OutputAttr>(node, root_attribute, &fn...);
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
