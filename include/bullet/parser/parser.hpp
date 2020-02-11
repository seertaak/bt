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

                template <typename T>
                auto maybe() -> std::optional<T> {
                    const auto t = peek();
                    if (auto pt = get_if<T>(&t.token))
                        return {*pt};
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
                auto eat(Fn&&... fns) -> tree_t {
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
                    cout << __PRETTY_FUNCTION__ << endl;
                    return or_test();
                }   

                auto or_test() -> tree_t {
                    cout << __PRETTY_FUNCTION__ << endl;
                    const auto lhs = and_test();

                    const auto or_ = maybe<token::or_t>();

                    if (!or_) return lhs;
                    eat();
                        
                    return bin_op_t{*or_, lhs, or_test()};
                }

                auto and_test() -> tree_t {
                    cout << __PRETTY_FUNCTION__ << endl;
                    const auto lhs = not_test();

                    const auto and_ = maybe<token::and_t>();

                    if (!and_) return lhs;
                    eat();
                        
                    return bin_op_t{*and_, lhs, and_test()};
                }

                auto not_test() -> tree_t {
                    cout << __PRETTY_FUNCTION__ << endl;
                    const auto not_ = maybe<token::not_t>();
                    //if (!not_) return comparison();
                    if (!not_) return atom();
                    eat();
                    
                    return unary_op_t{*not_, not_test()};
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
                    cout << __PRETTY_FUNCTION__ << endl;
                    return eat(
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
                        [](token::false_t b) { return tree_t(b); }
                    );
                }

                auto parse() -> tree_t { 
                    cout << __PRETTY_FUNCTION__ << endl;
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
