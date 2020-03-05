#pragma once

#include <bullet/lexer/lexer.hpp>
#include <bullet/parser/ast.hpp>

namespace bt { namespace parser {
    using input_t = lexer::output_t;

    namespace details {
        auto parse(const input_t& input) -> syntax::tree_t;
        auto parse(input_t&& input) -> syntax::tree_t;
    }  // namespace details

    BOOST_HOF_STATIC_LAMBDA_FUNCTION(parse) =
        boost::hof::pipable([](input_t&& input) -> syntax::tree_t {
            return details::parse(std::forward<input_t>(input));
        });
}}  // namespace bt::parser
