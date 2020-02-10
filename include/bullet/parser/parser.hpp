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
        namespace details {
            using namespace std;
            using namespace lexer;

            class parser {
                using source_token_list_t = decltype(lexer::output_t().tokens);
                using iterator = typename source_token_list_t::const_iterator;

                lexer::output_t input;
                iterator it;

                auto peek() -> lexer::source_token_t { return *it; }

                auto eat() -> lexer::source_token_t { return *it++; }

                auto eat(token_t&& token) -> lexer::source_token_t {
                    const auto t = peek();
                    if (t != token) throw runtime_error("FUCK RUASDFASDF");
                    ++it;
                    return t;
                }

                template <typename... Fn>
                auto eat(Fn&&... fns) -> decltype(auto) {
                    return match(
                        forward<source_token_t>(eat()),
                        forward(fns)...,
                        [](auto t) {
                            auto msg = stringstream();

                            // TODO: the possibilities are simply the argument types
                            // of the supplied functors!!!!

                            msg << "Couldn't match token: " << t << " against "
                                << "possibilities: TODO!";

                            throw runtime_error(msg.str());
                        }
                        );
                }

                auto literal

            public:
                parser(lexer::output_t input) : input(input), it(begin(input.tokens)) {}

                auto parse() -> syntax::tree_t { 
                    // stop clang-format
                    return syntax::tree_t();
                }
            };

            auto parse(lexer::output_t input) -> syntax::tree_t {
                auto p = parser(input);
                return p.parse();
            }
        }  // namespace details

        BOOST_HOF_STATIC_LAMBDA_FUNCTION(parse) = boost::hof::pipable(
            [](lexer::output_t input) -> syntax::tree_t { return details::parse(input); });
    }  // namespace parser
}  // namespace bt
