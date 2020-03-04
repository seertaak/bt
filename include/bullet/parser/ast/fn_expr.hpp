#pragma once

#include <iostream>

#include <boost/hana/all.hpp>

#include <bullet/parser/ast_fwd.hpp>
#include <bullet/parser/ast/data.hpp>

namespace bt {
    namespace parser {
        namespace syntax {
            template <typename Attr>
            struct fn_closure_param_t {
                BOOST_HANA_DEFINE_STRUCT(fn_closure_param_t,
                    (bool, var),
                    (std::optional<lexer::identifier_t>, identifier),
                    (attr_node_t<Attr>, expression)
                );

                 auto operator<=>(const fn_closure_param_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const fn_closure_param_t<Attr>& p) -> std::ostream& {
                if (auto v = p.var)
                    os << "var ";
                if (auto i = p.identifier)
                    os << i->name;
                if (p.expression.get())
                    os << "=" << p.expression;
                return os;
            }

            template <typename Attr>
            struct fn_expr_t {
                BOOST_HANA_DEFINE_STRUCT(fn_expr_t,
                    (std::vector<lexer::identifier_t>, arg_names),
                    (std::vector<attr_node_t<Attr>>, arg_types),
                    (attr_node_t<Attr>, result_type),
                    (attr_node_t<Attr>, body),
                    (std::vector<fn_closure_param_t<Attr>>, closure_params),
                    (Attr, attribute)
                );
                 auto operator<=>(const fn_expr_t&) const = default;
            };

            template <typename Attr>
            auto operator<<(std::ostream& os, const fn_expr_t<Attr>& a) -> std::ostream& {
                os << "fn(";
                auto first = true;
                for (auto i = 0; i < a.arg_names.size(); i++) {
                    if (first) first = false;
                    else os << ", ";
                    os << a.arg_names[i];
                    const auto& t = a.arg_types[i];
                    if (t.get())
                        os << ":" << t;
                }
                os << ")";
                if (a.result_type.get())
                    os << " -> " << a.result_type;

                os << " = " << a.body;

                if (!a.closure_params.empty()) {
                    first = true;
                    os << " with ";
                    for (auto&& p: a.closure_params) {
                        if (first) first = false;
                        else os << ", ";
                        os << p;
                    }
                }

                return os;
            }

        }}}
