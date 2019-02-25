#pragma once

#include <string_view>
#include <vector>

#include <boost/hof.hpp>

#include <bullet/token.hpp>

namespace lexer {
    namespace details {
        auto tokens1(string_view input) -> vector<token_t>;
    }

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(tokens1) =
        boost::hof::pipable([](string_view input) -> vector<token_t> {
            return details::tokens1(input);
        });
}  // namespace lexer
