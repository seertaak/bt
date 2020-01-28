#pragma once

#include <string_view>
#include <vector>

#include <boost/hof.hpp>
#include <bullet/token.hpp>

namespace lexer {
    struct output_t {
        source_token_list_t tokens;
        std::vector<uint32_t> eol_locations;
    };

    namespace details {
        auto tokenize(string_view input) -> output_t;
    }

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(tokenize) =
        boost::hof::pipable([](string_view input) -> output_t {
            return details::tokenize(input);
        });

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(tokens) = boost::hof::pipable(
        [](const output_t& output) -> const source_token_list_t& { return output.tokens; });
}  // namespace lexer
