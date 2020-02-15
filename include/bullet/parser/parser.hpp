#pragma once

#include <string_view>
#include <vector>

#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/util.hpp>

#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/hof.hpp>
#include <boost/type_index.hpp>

#include <sstream>

namespace bt {
    namespace parser {

        using input_t = lexer::output_t;

        namespace details {

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
                auto eat_if(Fn&&... fns) -> decltype(auto) {
                    auto loc_tok = eat();
                    return std::visit(boost::hana::overload(fns...),
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

                auto top_level() -> tree_t {
                    // cout << __PRETTY_FUNCTION__ << endl;
                    return or_test();
                }

                auto or_test() -> tree_t {
                    // cout << __PRETTY_FUNCTION__ << endl;
                    const auto lhs = and_test();

                    const auto or_ = maybe<token::or_t>();

                    if (!or_) return lhs;
                    eat();

                    return bin_op_t{*or_, lhs, or_test()};
                }

                auto and_test() -> tree_t {
                    // cout << __PRETTY_FUNCTION__ << endl;
                    const auto lhs = not_test();

                    const auto and_ = maybe<token::and_t>();

                    if (const auto and_ = eat_if<token::and_t>())
                        return bin_op_t{and_->token, lhs, and_test()};

                    return lhs;
                }

                auto not_test() -> tree_t {
                    if (const auto not_ = eat_if<token::not_t>())
                        return unary_op_t{not_->token, not_test()};
                    return comparison();
                }

                auto comparison() -> tree_t {
                    const auto lhs = atom();

                    using namespace token;

                    auto cmp =
                        eat_if<leq_t, geq_t, lt_t, gt_t, equal_t, not_equal_t, in_t, is_t, not_t>();
                    if (!cmp) return lhs;

                    if (cmp->token == NOT)
                        return unary_op_t{cmp->token,
                                          node_t(bin_op_t{expect<in_t>(), lhs, atom()})};
                    else if (cmp->token == IS) {
                        if (auto not_ = eat_if<not_t>()) {
                            return unary_op_t{not_->token,
                                              node_t(bin_op_t{cmp->token, lhs, atom()})};
                        }
                    }

                    return bin_op_t{cmp->token, lhs, atom()};
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

            auto parse(input_t&& input) -> tree_t {
                auto p = parser(forward<input_t>(input));
                return p.parse();
            }
        }  // namespace details

        BOOST_HOF_STATIC_LAMBDA_FUNCTION(parse) =
            boost::hof::pipable([](input_t&& input) -> syntax::tree_t {
                return details::parse(std::forward<input_t>(input));
            });
    }  // namespace parser
}  // namespace bt
