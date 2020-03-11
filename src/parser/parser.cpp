#include <bullet/parser/parser.hpp>

#include <sstream>
#include <string_view>
#include <vector>

#include <boost/hof.hpp>

#include <bullet/parser/error.hpp>
#include <bullet/util.hpp>

namespace bt { namespace parser {
    namespace {
        using namespace std;
        using namespace lexer;
        using namespace syntax;
        using namespace literal::numeric;

        using p_block_t = block_t<empty_attribute_t>;
        using p_data_t = data_t<empty_attribute_t>;
        using p_node_t = attr_node_t<empty_attribute_t>;

        class parser {
            using source_token_list_t = decltype(lexer::output_t().tokens);
            using iterator = typename source_token_list_t::const_iterator;

            input_t input;
            iterator it;
            bool code = true;

            auto peek() -> source_token_t {
                if (it == end(input.tokens)) return EOI;
                return *it;
            }

            void throw_error(const std::string& s) { throw ::bt::parser::error(s, peek()); }

            void throw_error(std::stringstream& s) { throw ::bt::parser::error(s, peek()); }

            auto loc_first() { return peek().location; }

            auto loc_last() { return (it - 1)->location; }

            auto eat() -> source_token_t {
                if (it == end(input.tokens)) throw_error("End of input.");
                return *it++;
            }

            template <typename... T>
            auto eat_if() -> optional<source_token_t> {
                const auto t = peek();

                const auto f = hana::fix([&](auto f, auto types) -> optional<source_token_t> {
                    if constexpr (hana::is_empty(types))
                        return nullopt;
                    else {
                        constexpr auto first = hana::front(types);
                        using T_i = typename decltype(first)::type;

                        if (holds_alternative<T_i>(t.token)) return eat();

                        return f(hana::drop_front(types));
                    }
                });

                return f(hana::tuple_t<T...>);
            }

            template <typename T>
            auto maybe() -> std::optional<T> {
                const auto t = peek();
                if (auto pt = get_if<T>(&t.token)) return {*pt};
                return std::nullopt;
            }

            template <typename T>
            auto expect() -> with_loc<T> {
                const auto t = peek();
                if (!holds_alternative<T>(t.token)) {
                    auto msg = stringstream();
                    msg << "Expected a \"" << token_symbol(T{}) << "\" but got a "
                        << token_symbol(t.token);
                    throw_error(msg);
                }
                ++it;
                return t.get_with_loc<T>();
            }

            auto eat(token_t&& token) -> source_token_t {
                const auto t = peek();
                if (t != token) {
                    auto msg = stringstream();
                    msg << "Expected a \"" << token << "\",  but got a \"" << t << "\"";
                    throw_error(msg);
                }
                ++it;
                return t;
            }

            template <typename... Fn>
            auto eat_if(Fn&&... fn) -> tree_t {
                const auto eat_then_apply = [this](auto&& fn) {
                    using fn_t = remove_reference_t<decltype(fn)>;
                    using arg_t = lambda_arg_t<fn_t>;
                    return [&](arg_t&& x) -> tree_t {
                        eat();
                        return fn(x);
                    };
                };

                auto loc_tok = peek();

                return std::visit(boost::hana::overload(eat_then_apply(forward<Fn>(fn))...,
                                                        [](auto) -> tree_t { return tree_t{}; }),
                                  std::forward<token_t>(loc_tok.token));
            }

            template <typename... Fn>
            auto eat_or_error(Fn&&... fns) -> tree_t {
                auto loc_tok = eat();
                const auto on_error = [&](auto t) -> tree_t {
                    auto msg = stringstream();

                    constexpr auto valid_token_types = hana::tuple_t<lambda_arg_t<Fn>...>;

                    msg << "Couldn't match token: " << t << " against "
                        << "possibilities: <";

                    auto first = true;
                    hana::for_each(valid_token_types, [&](auto T_c) {
                        using T = typename decltype(T_c)::type;
                        const auto t = T{};

                        if (first)
                            first = false;
                        else
                            msg << "> or <";

                        msg << token_symbol(t);
                    });

                    msg << ">";
                    throw_error(msg);

                    return tree_t();
                };
                return std::visit(boost::hana::overload(fns..., on_error),
                                  std::forward<token_t>(loc_tok.token));
            }

        public:
            parser(const input_t& in) : input(in) { it = begin(input.tokens); }
            parser(input_t&& in) noexcept : input(std::move(in)) { it = begin(input.tokens); }

            auto top_level() -> tree_t { return block(); }

            auto block() -> tree_t {
                const auto l = loc_first();
                auto result = p_block_t();
                do {
                    parse_block_line(result);
                } while (eat_if<token::line_end_t>());

                const auto L = loc_last();
                auto rv = result.size() == 1 ? result.front().get() : result;
                rv.location = location_t(l, L);
                return rv;
            }

            void parse_block_line(p_block_t& block) {
                do {
                    const auto s = statement();
                    if (s.is<else_t<empty_attribute_t>>()) {
                        if (block.empty() || !block.back().get().is<if_t<empty_attribute_t>>())
                            throw_error(
                                "Dangling \"else\" block (prior statement is not \"if\" or "
                                "\"elif\"");
                        block.back().get().as<if_t<empty_attribute_t>>().else_branch = p_node_t(s);
                    } else if (s.is<elif_t<empty_attribute_t>>()) {
                        if (block.empty() || !block.back().get().is<if_t<empty_attribute_t>>())
                            throw_error(
                                "Dangling \"elif\" block (prior statement is not \"if\" or "
                                "\"elif\"");
                        auto& if_ = block.back().get().as<if_t<empty_attribute_t>>();
                        const auto& elif = s.as<elif_t<empty_attribute_t>>();
                        if_.elif_tests.push_back(elif.test);
                        if_.elif_branches.push_back(elif.body);
                    } else {
                        block.push_back(p_node_t(s));
                    }
                } while (eat_if<token::semicolon_t>());
            }

            void parse_fn_def_args_line_end(vector<with_loc<identifier_t>>& arg_names,
                                            vector<p_node_t>& arg_types) {
                do {
                    parse_fn_def_args_comma(arg_names, arg_types);
                } while (eat_if<token::line_end_t>());
            }

            void parse_fn_def_args_comma(vector<with_loc<identifier_t>>& arg_names,
                                         vector<p_node_t>& arg_types) {
                do {
                    do {
                        arg_names.emplace_back(expect<identifier_t>());
                    } while (eat_if<token::comma_t>());

                    if (eat_if<token::colon_t>()) {
                        arg_types.emplace_back(type_expr_t<empty_attribute_t>{expression()});

                        while (!arg_types.empty() && arg_types.size() < arg_names.size())
                            arg_types.push_back(arg_types.back());
                    } else {
                        while (arg_types.size() < arg_names.size()) arg_types.push_back(p_node_t());
                    }
                } while (eat_if<token::comma_t>());
            }

            auto delimited_code() -> tree_t {
                const auto l = loc_first();
                const auto old_code = code;
                code = false;
                expect<token::oparen_t>();
                auto result = block();
                if (!result.is<p_block_t>()) {
                    auto b = p_block_t();
                    b.push_back(p_node_t(result));
                    result = b;
                }
                expect<token::cparen_t>();
                code = old_code;
                const auto L = loc_last();
                result.location = location_t(l, L);
                return result;
            }

            auto delimited_data() -> tree_t {
                const auto l = loc_first();

                const auto old_code = code;
                code = false;
                expect<token::oparen_t>();
                tree_t result = data();
                expect<token::cparen_t>();
                code = old_code;

                const auto L = loc_last();
                result.location = location_t(l, L);
                return result;
            }

            auto statement() -> tree_t {
                const auto l = loc_first();
                if (auto result = eat_if(
                        [this](token::type_t) -> tree_t {
                            auto name = optional<with_loc<identifier_t>>();
                            if (const auto ident = eat_if<identifier_t>()) {
                                name = ident->get_with_loc<identifier_t>();
                                if (eat_if<token::assign_t>())
                                    return def_type_t<empty_attribute_t>{
                                        *name, p_node_t(type_expr_t<empty_attribute_t>{
                                                   p_node_t(atom_expr())})};
                            }

                            auto arg_names = std::vector<with_loc<lexer::identifier_t>>();
                            auto arg_types = std::vector<p_node_t>();

                            auto type = struct_t<empty_attribute_t>();

                            if (eat_if<token::oparen_t>()) {
                                if (!eat_if<token::cparen_t>()) {
                                    parse_fn_def_args_line_end(arg_names, arg_types);
                                    expect<token::cparen_t>();
                                    for (auto i = 0; i < arg_names.size(); i++)
                                        type.emplace_back(arg_names[i], arg_types[i]);
                                }
                            }

                            if (name)
                                return def_type_t<empty_attribute_t>{
                                    *name,
                                    p_node_t(type_expr_t<empty_attribute_t>{p_node_t(type)})};
                            return type;
                        },
                        [this](token::alias_t) -> tree_t {
                            const auto name = expect<identifier_t>();
                            expect<token::assign_t>();
                            return let_type_t<empty_attribute_t>{
                                name,
                                p_node_t(type_expr_t<empty_attribute_t>{p_node_t(atom_expr())})};
                        },
                        [this](token::var_t) -> tree_t {
                            const auto lhs = expect<lexer::identifier_t>();

                            auto result_type = p_node_t(tree_t());
                            if (eat_if<token::colon_t>())
                                result_type =
                                    p_node_t(type_expr_t<empty_attribute_t>{p_node_t(atom_expr())});

                            auto rhs = tree_t();
                            if (eat_if<token::assign_t>()) {
                                const auto old_code = code;
                                code = false;
                                rhs = assignment_stmt();
                                code = old_code;
                            }

                            return var_def_t<empty_attribute_t>{lhs, result_type, rhs};
                        },
                        [this](token::def_t) -> tree_t {
                            const auto fn_name = expect<lexer::identifier_t>();

                            auto arg_names = std::vector<with_loc<lexer::identifier_t>>();
                            auto arg_types = std::vector<p_node_t>();

                            if (eat_if<token::oparen_t>()) {
                                if (!eat_if<token::cparen_t>()) {
                                    parse_fn_def_args_line_end(arg_names, arg_types);
                                    expect<token::cparen_t>();
                                }
                            }

                            // x -> x + y / z.size() | y, var z

                            auto result_type = p_node_t(tree_t());
                            if (eat_if<token::colon_t>())
                                result_type = p_node_t(
                                    type_expr_t<empty_attribute_t>{p_node_t(expression())});

                            expect<token::assign_t>();

                            const auto body = expression();

                            return var_def_t<empty_attribute_t>{
                                fn_name,
                                p_node_t(),  // TODO!
                                p_node_t(fn_expr_t<empty_attribute_t>{arg_names, arg_types,
                                                                      result_type, body})};
                        },
                        [this](token::if_t) -> tree_t {
                            auto ast = if_t<empty_attribute_t>{};

                            ast.elif_tests.push_back(p_node_t(delimited_code()));

                            const auto old_code = code;
                            code = true;
                            ast.elif_branches.push_back(p_node_t(expression()));
                            code = old_code;

                            if (eat_if<token::else_t>()) {
                                const auto old_code = code;
                                code = true;
                                ast.else_branch = p_node_t(expression());
                                code = old_code;
                            }

                            return ast;
                        },
                        [this](token::while_t) -> tree_t {
                            auto ast = while_t<empty_attribute_t>{};

                            ast.test = p_node_t(delimited_code());

                            const auto old_code = code;
                            code = true;
                            ast.body = p_node_t(expression());
                            code = old_code;

                            return ast;
                        },
                        [this](token::for_t) -> tree_t {
                            auto ast = for_t<empty_attribute_t>();

                            const auto old_code = code;
                            code = true;

                            expect<token::oparen_t>();
                            ast.var_lhs = expect<identifier_t>();
                            expect<token::colon_t>();
                            ast.var_rhs =
                                p_node_t(type_expr_t<empty_attribute_t>{p_node_t(expression())});
                            expect<token::cparen_t>();
                            ast.body = p_node_t(expression());

                            code = old_code;

                            return ast;
                        },
                        [this](token::elif_t) -> tree_t {
                            auto ast = elif_t<empty_attribute_t>{};
                            ast.test = p_node_t(delimited_code());

                            const auto old_code = code;
                            code = true;
                            ast.body = p_node_t(expression());
                            code = old_code;

                            return ast;
                        },
                        [this](token::else_t) -> tree_t {
                            auto ast = else_t<empty_attribute_t>{};

                            const auto old_code = code;
                            code = true;
                            ast.body = p_node_t(expression());
                            code = old_code;

                            return ast;
                        })) {
                    result.location = location_t(l, loc_last());
                    return result;
                }

                return assignment_stmt();
            }

            auto assignment_stmt() -> tree_t {
                const auto l = loc_first();
                const auto lhs = expression();

                if (eat_if<token::assign_t>()) {
                    auto rhs = assignment_stmt();
                    rhs.location = location_t(l, loc_last());
                    return assign_t<empty_attribute_t>{lhs, rhs};
                }

                return lhs;
            }

            auto capture_expression() -> fn_closure_param_t<empty_attribute_t> {
                auto var = eat_if<token::var_t>();
                auto id = eat_if<identifier_t>();
                p_node_t expr;
                if (eat_if<token::assign_t>()) expr = p_node_t(expression());

                if (!var && !id && !expr.get()) throw_error("Bad capture expression.");

                optional<with_loc<lexer::identifier_t>> ident;
                if (id) ident.emplace(id->get_with_loc<lexer::identifier_t>());

                return {!!var, ident, expr};
            }

            auto capture_expressions() -> std::vector<fn_closure_param_t<empty_attribute_t>> {
                auto result = std::vector<fn_closure_param_t<empty_attribute_t>>();
                do {
                    capture_semicolon_expressions(result);
                } while (eat_if<token::line_end_t>());
                return result;
            }

            void capture_semicolon_expressions(
                std::vector<fn_closure_param_t<empty_attribute_t>>& result) {
                do {
                    capture_comma_expressions(result);
                } while (eat_if<token::semicolon_t>());
            }

            void capture_comma_expressions(
                std::vector<fn_closure_param_t<empty_attribute_t>>& result) {
                do {
                    result.push_back(capture_expression());
                } while (eat_if<token::comma_t>());
            }

            auto expression() -> tree_t {
                const auto l = loc_first();
                if (auto result = eat_if(
                        [this](token::if_t) -> tree_t {
                            auto ast = if_t<empty_attribute_t>{};

                            const auto old_code = code;
                            code = true;

                            ast.elif_tests.push_back(p_node_t(delimited_code()));
                            ast.elif_branches.push_back(p_node_t(expression()));

                            if (eat_if<token::else_t>()) ast.else_branch = p_node_t(expression());

                            code = old_code;

                            return ast;
                        },
                        [this](token::fn_t) -> tree_t {
                            auto ast = fn_expr_t<empty_attribute_t>();

                            if (eat_if<token::oparen_t>()) {
                                if (!eat_if<token::cparen_t>()) {
                                    parse_fn_def_args_line_end(ast.arg_names, ast.arg_types);
                                    expect<token::cparen_t>();
                                }
                            }

                            if (eat_if<token::colon_t>())
                                ast.result_type = p_node_t(
                                    type_expr_t<empty_attribute_t>{p_node_t(expression())});

                            expect<token::assign_t>();

                            ast.body = p_node_t(expression());

                            if (eat_if<token::with_t>()) {
                                bool indent = !!eat_if<token::oparen_t>();
                                ast.closure_params = capture_expressions();
                                if (indent) expect<token::cparen_t>();
                            }

                            return ast;
                        },
                        [this](token::break_t) -> tree_t { return syntax::break_t{}; },
                        [this](token::continue_t) -> tree_t { return syntax::continue_t{}; },
                        [this](token::return_t) -> tree_t {
                            auto t = peek().token;
                            if (t == LINE_END || t == SEMICOLON || t == COMMA || t == CPAREN)
                                return syntax::return_t<empty_attribute_t>{};
                            return syntax::return_t<empty_attribute_t>{p_node_t(expression())};
                        },
                        [this](token::yield_t) -> tree_t {
                            auto t = peek().token;
                            if (t == LINE_END || t == SEMICOLON || t == COMMA || t == CPAREN)
                                return syntax::yield_t<empty_attribute_t>{};
                            return syntax::yield_t<empty_attribute_t>{p_node_t(expression())};
                        })) {
                    result.location = location_t(l, loc_last());
                    return result;
                }
                return or_test();
            }

            auto location(const lexer::location_t& l) -> location_t {
                return location_t(l, loc_last());
            }

            auto or_test() -> tree_t {
                const auto l = loc_first();

                auto result = and_test();

                while (const auto or_ = eat_if<token::or_t>())
                    result = bin_op_t<empty_attribute_t>{or_->token, result, and_test()};

                result.location = location(l);

                return result;
            }

            auto and_test() -> tree_t {
                const auto l = loc_first();
                auto result = not_test();
                while (const auto and_ = eat_if<token::and_t>())
                    result = bin_op_t<empty_attribute_t>{and_->token, result, not_test()};
                result.location = location(l);
                return result;
            }

            auto not_test() -> tree_t {
                const auto l = loc_first();
                if (const auto not_ = eat_if<token::not_t>()) {
                    auto result = tree_t(unary_op_t<empty_attribute_t>{not_->token, not_test()});
                    result.location = location(l);
                    return result;
                }
                return comparison();
            }

            auto comparison() -> tree_t {
                const auto l = loc_first();
                tree_t result = bit_or_expr();

                using namespace token;

                auto cmp =
                    eat_if<leq_t, geq_t, lt_t, gt_t, equal_t, not_equal_t, in_t, is_t, not_t>();
                if (!cmp) return result;

                if (cmp->token == NOT) {
                    result = unary_op_t<empty_attribute_t>{
                        cmp->token,
                        p_node_t(bin_op_t<empty_attribute_t>{expect<in_t>(), result, atom_expr()})};
                    result.location = location(l);

                    return result;
                } else if (cmp->token == IS) {
                    if (auto not_ = eat_if<not_t>()) {
                        result = unary_op_t<empty_attribute_t>{
                            not_->token,
                            p_node_t(bin_op_t<empty_attribute_t>{cmp->token, result, atom_expr()})};
                        result.location = location(l);
                        return result;
                    }
                }

                result = bin_op_t<empty_attribute_t>{cmp->token, result, atom_expr()};
                result.location = location(l);
                return result;
            }

            auto bit_or_expr() -> tree_t {
                const auto l = loc_first();
                tree_t result = bit_xor_expr();

                while (const auto op = eat_if<token::bar_t>())
                    result = bin_op_t<empty_attribute_t>{op->token, result, bit_xor_expr()};

                result.location = location(l);

                return result;
            }

            auto bit_xor_expr() -> tree_t {
                const auto l = loc_first();
                auto result = bit_and_expr();

                while (const auto op = eat_if<token::hat_t>())
                    result = bin_op_t<empty_attribute_t>{op->token, result, bit_and_expr()};

                result.location = location(l);
                return result;
            }

            auto bit_and_expr() -> tree_t {
                const auto l = loc_first();
                auto result = bit_shift_expr();

                while (const auto op = eat_if<token::ampersand_t>())
                    result = bin_op_t<empty_attribute_t>{op->token, result, bit_shift_expr()};
                result.location = location(l);
                return result;
            }

            auto bit_shift_expr() -> tree_t {
                const auto l = loc_first();
                auto result = arithmetic_expr();

                using namespace token;

                while (const auto op = eat_if<left_left_t, right_right_t>())
                    result = bin_op_t<empty_attribute_t>{op->token, result, arithmetic_expr()};

                result.location = location(l);
                return result;
            }

            auto arithmetic_expr() -> tree_t {
                const auto l = loc_first();
                auto result = term();

                while (const auto op = eat_if<token::plus_t, token::minus_t>())
                    result = bin_op_t<empty_attribute_t>{op->token, result, term()};

                result.location = location(l);
                return result;
            }

            auto term() -> tree_t {
                const auto l = loc_first();
                auto result = factor();

                using namespace token;

                while (const auto op = eat_if<star_t, slash_t, percentage_t, colon_star_t,
                                              colon_slash_t, colon_percentage_t>())
                    result = bin_op_t<empty_attribute_t>{op->token, result, factor()};

                result.location = location(l);
                return result;
            }

            auto factor() -> tree_t {
                using namespace token;
                const auto l = loc_first();
                if (const auto op = eat_if<plus_t, minus_t, tilde_t>()) {
                    tree_t result = unary_op_t<empty_attribute_t>{op->token, factor()};
                    result.location = location(l);
                    return result;
                }
                return power();
            }

            auto power() -> tree_t {
                const auto l = loc_first();
                tree_t result = atom_expr();

                if (const auto op = eat_if<token::star_star_t>())
                    result = bin_op_t<empty_attribute_t>{op->token, result, factor()};

                result.location = location(l);

                return result;
            }

            auto data() -> p_data_t {
                const auto l = loc_first();

                const auto old_code = code;
                code = false;

                auto result = p_data_t();
                do {
                    result.emplace_back(invoc_args_semicolon());
                } while (eat_if<token::line_end_t>());

                code = old_code;

                if (result.size() == 1 && result.front().get().is<p_data_t>())
                    result = result.front().get().get<p_data_t>();

                return result;
            }

            auto invoc_args_semicolon() -> tree_t {
                auto result = p_data_t();
                do {
                    result.emplace_back(invoc_args_comma());
                } while (eat_if<token::semicolon_t>());
                if (result.size() == 1) return result.front();
                return result;
            }

            auto invoc_args_comma() -> tree_t {
                if (peek().token == OPAREN) return delimited_data();

                auto result = p_data_t();
                do {
                    result.emplace_back(expression());
                } while (eat_if<token::comma_t>());
                if (result.size() == 1) return result.front();
                return result;
            }

            auto simple_atom_expr() -> tree_t {
                auto result = atom();

                while (const auto invoc = eat_if([&](token::oparen_t) -> tree_t {
                           const auto args = data();
                           expect<token::cparen_t>();
                           return invoc_t<empty_attribute_t>{result, move(args)};
                       }))
                    result = invoc;

                return result;
            }

            auto atom_expr() -> tree_t {
                const auto l = loc_first();

                p_node_t result = simple_atom_expr();
                while (eat_if<token::dot_t>()) {
                    p_node_t rhs = simple_atom_expr();
                    if (const auto pinvoc = rhs.get().get_if<invoc_t<empty_attribute_t>>()) {
                        auto& args = pinvoc->arguments;
                        args.insert(args.begin(), result);
                        result = rhs;
                    } else {
                        result = p_node_t(invoc_t<empty_attribute_t>{
                            rhs,
                            p_data_t{result},
                        });
                    }
                }
                tree_t rv = result;
                rv.location = location(l);
                return rv;
            }

            auto atom() -> tree_t {
                const auto l = loc_first();
                tree_t result = eat_or_error(
                    [this](token::oparen_t) -> tree_t {
                        if (code) {
                            const auto e = block();
                            expect<token::cparen_t>();
                            return e;
                        }

                        const auto d = data();
                        expect<token::cparen_t>();
                        return d;
                    },
                    [this](token::data_t) -> tree_t { return delimited_data(); },
                    [this](token::do_t) -> tree_t { return delimited_code(); },
                    [](identifier_t id) { return tree_t(id); },
                    [](string_literal_t s) { return tree_t(literal_t(s)); },
                    [](integral_literal_t i) { return tree_t(literal_t(i)); },
                    [](floating_point_literal_t x) { return tree_t(literal_t(x)); },
                    [](token::true_t b) { return tree_t(literal_t(b)); },
                    [](token::false_t b) { return tree_t(literal_t(b)); },
                    [](token::char_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::byte_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::short_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::int_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::long_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::ushort_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::uint_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::ulong_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::float_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::double_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::i8_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::i16_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::i32_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::i64_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::u8_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::u16_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::u32_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::u64_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::ptr_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::array_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::slice_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::variant_t s) { return tree_t(primitive_type_t(s)); },
                    [](token::tuple_t s) { return tree_t(primitive_type_t(s)); });

                result.location = location(l);
                return result;
            }

            auto parse() -> tree_t { return top_level(); }
        };
    }  // namespace

    namespace details {
        auto parse(const input_t& input) -> tree_t {
            auto p = parser(input);
            return p.parse();
        }
        auto parse(input_t&& input) -> tree_t {
            auto p = parser(forward<input_t>(input));
            return p.parse();
        }
    }  // namespace details
}}     // namespace bt::parser
