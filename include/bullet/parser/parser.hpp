#pragma once

#include <string_view>
#include <vector>

#include <bullet/lexer/token.hpp>
#include <bullet/parser/ast.hpp>
#include <bullet/util.hpp>

#include <boost/hof.hpp>

#include <sstream>

namespace bt {
    namespace parser {

        using input_t = lexer::output_t;

        namespace details {

            using namespace std;
            using namespace lexer;
            using namespace syntax;

            class parser {
                using source_token_list_t = decltype(lexer::output_t().tokens);
                using iterator = typename source_token_list_t::const_iterator;

                input_t input;
                iterator it;

                auto peek() -> source_token_t { return *it; }

                auto eat() -> source_token_t { return *it++; }

                auto eat(token_t&& token) -> source_token_t {
                    const auto t = peek();
                    if (t != token) throw runtime_error("FUCK RUASDFASDF");
                    ++it;
                    return t;
                }

                template <typename... Fn>
                auto eat(Fn&&... fns) -> tree_t {
                    auto loc_tok = eat();
                    return std::visit(
                        boost::hana::overload(
                            fns...,
                            [](auto t) -> tree_t {
                                auto msg = stringstream();

                                // TODO: the possibilities are simply the argument types
                                // of the supplied functors!!!!

                                msg << "Couldn't match token: " << t << " against "
                                    << "possibilities: TODO!";

                                throw runtime_error(msg.str());
                            }
                        ),
                        std::forward<token_t>(loc_tok.token)
                    );
                }

            public:
                parser(const input_t& in) : input(in) {
                    it = begin(input.tokens);
                }
                parser(input_t&& in) noexcept : input(std::move(in)) {
                    it = begin(input.tokens);
                }

                auto parse() -> tree_t { 
                    return eat(
                        [] (string_literal_t l) {
                            return tree_t(l);
                        },
                        [] (integral_literal_t l) {
                            return tree_t(l);
                        },
                        [] (floating_point_literal_t l) {
                            return tree_t(l);
                        }
                    );
                }
            };

            auto parse(input_t&& input) -> tree_t {
                auto p = parser(forward<input_t>(input));
                return p.parse();
            }
        }  // namespace details

        BOOST_HOF_STATIC_LAMBDA_FUNCTION(parse) = boost::hof::pipable(
            [](input_t&& input) -> syntax::tree_t { return details::parse(std::forward<input_t>(input)); });
    }  // namespace parser
}  // namespace bt
