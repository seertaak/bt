#include <bullet/parser/parser.hpp>

#include <sstream>
#include <string_view>
#include <vector>

#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/hof.hpp>
#include <boost/type_index.hpp>

#include <bullet/util.hpp>

namespace bt {
    namespace parser {
        namespace {
            using namespace std;
            using namespace lexer;
            using namespace syntax;
            using namespace literal::numeric;

            template <typename Fn>
            using lambda_arg_t =
                std::remove_const_t<std::remove_reference_t<typename boost::mpl::at_c<
                    boost::function_types::parameter_types<decltype(&Fn::operator())>,
                    1>::type>>;

            class parser {
                using source_token_list_t = decltype(lexer::output_t().tokens);
                using iterator = typename source_token_list_t::const_iterator;

                input_t input;
                iterator it;

                auto peek() -> source_token_t {
                    if (it == end(input.tokens)) return EOI;
                    return *it;
                }

                auto eat() -> source_token_t {
                    if (it == end(input.tokens)) throw std::runtime_error("End of input.");
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
                auto expect() -> T {
                    const auto t = peek();
                    if (!holds_alternative<T>(t.token)) {
                        auto msg = stringstream();
                        msg << "Expected a " << token_symbol(T{}) << " but got a "
                            << token_symbol(t.token) << " at " << t.location;
                        throw std::runtime_error(msg.str());
                    }
                    ++it;
                    return get<T>(t.token);
                }

                auto eat(token_t&& token) -> source_token_t {
                    const auto t = peek();
                    if (t != token) throw runtime_error("FUCK RUASDFASDF");
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

                    return std::visit(
                        boost::hana::overload(eat_then_apply(forward<Fn>(fn))...,
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

                        msg << ">, at " << loc_tok.location;

                        throw runtime_error(msg.str());
                    };
                    return std::visit(boost::hana::overload(fns..., on_error),
                                      std::forward<token_t>(loc_tok.token));
                }

            public:
                parser(const input_t& in) : input(in) { it = begin(input.tokens); }
                parser(input_t&& in) noexcept : input(std::move(in)) { it = begin(input.tokens); }

                auto top_level() -> tree_t { return statement(); }

                auto statement() -> tree_t {
                    if (const auto result = eat_if([this](token::var_t) -> tree_t {
                            const auto lhs = expect<lexer::identifier_t>();
                            expect<token::equal_t>();
                            const auto rhs = assignment_stmt();
                            return var_def_t{std::move(lhs), nullopt, std::move(rhs)};
                        }))
                        return result;

                    return assignment_stmt();
                }

                auto assignment_stmt() -> tree_t {
                    const auto lhs = or_test();

                    if (eat_if<token::assign_t>()) return assign_t{lhs, assignment_stmt()};

                    return lhs;
                }

                auto or_test() -> tree_t {
                    // cout << __PRETTY_FUNCTION__ << endl;
                    auto result = and_test();

                    while (const auto or_ = eat_if<token::or_t>())
                        result = bin_op_t{or_->token, result, and_test()};

                    return result;
                }

                auto and_test() -> tree_t {
                    auto result = not_test();
                    while (const auto and_ = eat_if<token::and_t>())
                        result = bin_op_t{and_->token, result, not_test()};
                    return result;
                }

                auto not_test() -> tree_t {
                    if (const auto not_ = eat_if<token::not_t>())
                        return unary_op_t{not_->token, not_test()};
                    return comparison();
                }

                auto comparison() -> tree_t {
                    const auto result = bit_or_expr();

                    using namespace token;

                    auto cmp =
                        eat_if<leq_t, geq_t, lt_t, gt_t, equal_t, not_equal_t, in_t, is_t, not_t>();
                    if (!cmp) return result;

                    if (cmp->token == NOT)
                        return unary_op_t{cmp->token,
                                          node_t(bin_op_t{expect<in_t>(), result, atom()})};
                    else if (cmp->token == IS) {
                        if (auto not_ = eat_if<not_t>()) {
                            return unary_op_t{not_->token,
                                              node_t(bin_op_t{cmp->token, result, atom()})};
                        }
                    }

                    return bin_op_t{cmp->token, result, atom()};
                }

                auto bit_or_expr() -> tree_t {
                    auto result = bit_xor_expr();

                    while (const auto op = eat_if<token::bar_t>())
                        result = bin_op_t{op->token, result, bit_xor_expr()};

                    return result;
                }

                auto bit_xor_expr() -> tree_t {
                    auto result = bit_and_expr();

                    while (const auto op = eat_if<token::hat_t>())
                        result = bin_op_t{op->token, result, bit_and_expr()};

                    return result;
                }

                auto bit_and_expr() -> tree_t {
                    auto result = bit_shift_expr();

                    while (const auto op = eat_if<token::ampersand_t>())
                        result = bin_op_t{op->token, result, bit_shift_expr()};

                    return result;
                }

                auto bit_shift_expr() -> tree_t {
                    auto result = arithmetic_expr();

                    using namespace token;

                    while (const auto op = eat_if<left_left_t, right_right_t>())
                        result = bin_op_t{op->token, result, arithmetic_expr()};

                    return result;
                }

                auto arithmetic_expr() -> tree_t {
                    auto result = term();

                    while (const auto op = eat_if<token::plus_t, token::minus_t>())
                        result = bin_op_t{op->token, result, term()};

                    return result;
                }

                auto term() -> tree_t {
                    auto result = factor();

                    using namespace token;

                    while (const auto op = eat_if<star_t, slash_t, percentage_t, colon_star_t,
                                                  colon_slash_t, colon_percentage_t>())
                        result = bin_op_t{op->token, result, factor()};

                    return result;
                }

                auto factor() -> tree_t {
                    using namespace token;
                    if (const auto op = eat_if<plus_t, minus_t, tilde_t>())
                        return unary_op_t{op->token, factor()};
                    return power();
                }

                auto power() -> tree_t {
                    const auto result = atom();

                    if (const auto op = eat_if<token::star_star_t>())
                        return bin_op_t{op->token, result, factor()};

                    return result;
                }

                /*
                [this](token::var_t) -> tree_t {
                    const auto ident = expect<identifier_t>();
                    expect<token::assign_t>();
                    const auto literal = node_t(parse_literal());
                    return var_def_t{move(ident), std::nullopt, move(literal)};
                },
                */

                auto atom() -> tree_t {
                    // cout << __PRETTY_FUNCTION__ << endl;
                    return eat_or_error(
                        [this](token::oparen_t) -> tree_t {
                            const auto e = top_level();
                            expect<token::cparen_t>();
                            return e;
                        },
                        [](identifier_t id) { return tree_t(id); },
                        [](string_literal_t s) { return tree_t(s); },
                        [](integral_literal_t i) { return tree_t(i); },
                        [](floating_point_literal_t x) { return tree_t(x); },
                        [](token::true_t b) { return tree_t(b); },
                        [](token::false_t b) { return tree_t(b); });
                }

                auto parse() -> tree_t {
                    // cout << __PRETTY_FUNCTION__ << endl;
                    return top_level();
                }
            };
        }  // namespace

        namespace details {
            auto parse(input_t&& input) -> tree_t {
                auto p = parser(forward<input_t>(input));
                return p.parse();
            }
        }  // namespace details
    }      // namespace parser
}  // namespace bt
